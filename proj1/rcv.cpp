#include <vector>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <ctime>
#include <set>
#include <dirent.h>
#include "utils/packet.h"
#include "utils/net_include.h"
#include "utils/sendto_dbg.h"
#include "utils/sender_info.h"
#include "utils/file_helper.h"

using namespace std;

static void Usage(int argc, char *argv[]);
static void Print_help();
static int Cmp_time(struct timeval t1, struct timeval t2);
void init_receive(FILE *pd, struct net_pkt *pkt);
void test_result();
static const struct timeval Zero_time = {0, 0};
// IO
static int Port;
static bool isLAN;
FILE *pd;
// data structures
static vector<net_pkt *> window;
static set<string> senders;
static long long cum_seq = 0;
long long W_SIZE; // for linker
int Pid = -1;
int Ip = -1;

int main(int argc, char *argv[])
{
    ncp_addr ncp;
    struct sockaddr_in name;
    struct sockaddr_in from_addr;
    struct sockaddr_in *tmp_from_addr;
    vector<sockaddr_in*> sender_q;
    socklen_t from_len;
    int sock;
    fd_set mask;
    fd_set read_mask;
    int bytes;
    int done = 0;
    double total_trans = 0;
    double success_trans = 0;
    double last_record_bytes = 0;
    bool rcv_start = false;
    int num;
    char mess_buf[sizeof(net_pkt)];
    struct timeval last_recv_time = {0, 0};
    struct timeval now;
    struct timeval last_record_time = {0, 0};
    struct timeval trans_start = {0, 0};
    struct timeval diff_time;
    struct timeval timeout;

    /***** INITIALIZE *****/
    {
        /* Parse commandline args */
        Usage(argc, argv);
        printf("Listening for messages on port %d\n", Port);
        /* Open socket for receiving */
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            perror("rcv: socket");
            exit(1);
        }
        /* Bind receive socket to listen for incoming messages on specified port */
        name.sin_family = AF_INET;
        name.sin_addr.s_addr = INADDR_ANY;
        name.sin_port = htons(Port);
        if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
        {
            perror("rcv: bind");
            exit(1);
        }
        /* Set up mask for file descriptors we want to read from */
        FD_ZERO(&read_mask);
        FD_SET(sock, &read_mask);
    }

    for (;;)
    {
        /* (Re)set mask and timeout */
        mask = read_mask;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        /* Wait for message or timeout */
        num = select(FD_SETSIZE, &mask, NULL, NULL, &timeout);
        if (num > 0)
        {
            if (FD_ISSET(sock, &mask))
            {
                struct net_pkt *pkt;
                /******** INITIALIZE *******/
                {
                    from_len = sizeof(from_addr);
                    memset(mess_buf, 0, PKT_DT_SIZE);
                    tmp_from_addr = (sockaddr_in*)malloc(sizeof(tmp_from_addr));
                    bytes = recvfrom(sock, mess_buf, sizeof(net_pkt), 0,
                                    (struct sockaddr *)tmp_from_addr,
                                    &from_len);
                    if (bytes == 0)
                        continue;                        // didn't receive anything
                    gettimeofday(&last_recv_time, NULL); // record time of receival
                    pkt = (struct net_pkt *)mess_buf; // parse pkt
                    total_trans += sizeof(*pkt);
                }
                /********* HANDLE SENDER REQUEST *********/
                {
                    bool sender_present = false;
                    for (int i = 0; i < (int)sender_q.size(); i++) 
                    {
                        if (sender_q[i]->sin_addr.s_addr == tmp_from_addr->sin_addr.s_addr && 
                            sender_q[i]->sin_port == tmp_from_addr->sin_port)
                            {
                                sender_present = true;
                                break;
                            }
                    }
                    if (!sender_present) 
                        sender_q.push_back(tmp_from_addr);
                    if (sender_q[0]->sin_addr.s_addr != tmp_from_addr->sin_addr.s_addr || sender_q[0]->sin_port != tmp_from_addr->sin_port)
                    {
                        struct ack_pkt tmp_p;
                        tmp_p.cum_seq = -1;
                        sendto_dbg(sock, (char *)&tmp_p, sizeof(tmp_p), 0, (struct sockaddr *)tmp_from_addr,
                               sizeof(from_addr)); 
                        continue;
                    }   
                    if (!rcv_start)
                    {
                        gettimeofday(&last_record_time, NULL);
                        pd = fopen(pkt->d_fname, "wb");
                        trans_start.tv_sec = last_record_time.tv_sec;
                        rcv_start = true;
                    }
                    from_addr = *sender_q[0];
                }

                /************ HANDLE RECEIVE ***************/
                struct ack_pkt p;

                /* OLD PKT */
                if (pkt->seq <= cum_seq)
                {
                    p.cum_seq = cum_seq; // re-sent the cum_ack
                    p.is_nack = false;
                    sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                               sizeof(from_addr));
                }
                /* SEQUENTIAL PKT */
                else if (pkt->seq == cum_seq + 1)
                {
                    p.cum_seq = cum_seq = pkt->seq;
                    p.data_size = pkt->dt_size;
                    p.is_nack = false;
                    if (pkt->is_end)
                    {
                        done = 1;
                    }
                    sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                               sizeof(from_addr)); // send the new cum-ack
                    success_trans += pkt->dt_size;
                    init_receive(pd, pkt); //write to disk
                    // dequeue buffer
                    while (window.size() != 0 && window.front()->seq == cum_seq + 1)
                    {
                        p.cum_seq = cum_seq = pkt->seq;
                        p.is_nack = false;
                        sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                                   sizeof(from_addr));
                        window.erase(window.begin()); // delete the buffered pkt
                        success_trans += pkt->dt_size;
                        init_receive(pd, pkt); //write to disk
                    }
                }
                /* FUTURE PKT */
                else if ((long long)window.size() < pkt->w_size)
                {
                    // check seq is already in the window
                    bool present = false;
                    for(long long i = 0; i < (long long)window.size(); i++) {
                        if (window[i]->seq == pkt->seq) {
                            present = true;
                            break;
                        }
                    }
                    if (present) continue;

                    window.push_back(pkt);
                    sort(window.begin(), window.end(),
                         [](auto &a, auto &b) -> bool
                         {
                             return a->seq < b->seq;
                         });

                    // send gapped NACKs
                    long long lo_ind = -1;
                    long long hi_ind = 0;
                    while (hi_ind < (long long)window.size())
                    {
                        long long lo, hi;
                        if (lo_ind == -1)
                            lo = cum_seq;
                        else
                            lo = window[lo_ind]->seq;
                        hi = window[hi_ind]->seq;

                        for (long i = lo + 1; i < hi; i++)
                        {
                            p.cum_seq = i;
                            p.data_size = pkt->w_size;
                            p.is_nack = true;
                            sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                                       sizeof(from_addr));
                        }

                        lo_ind++;
                        hi_ind++;
                    }
                }

                /************* HANDLE PRINT **************/
                if (done == 0) /* MIDWAY */
                {
                    if (total_trans - last_record_bytes >= 10 * MEGABYTES)
                    {
                        gettimeofday(&now, NULL);
                        timersub(&now, &last_record_time, &diff_time);
                        double trans_data = (double)(total_trans - last_record_bytes);
                        print_statistics(diff_time, trans_data, (double)success_trans / MEGABYTES);
                        last_record_time.tv_sec = now.tv_sec;
                        last_record_bytes = total_trans;
                    }
                }
                else if (done == 1) /* SENDER's COMPLETED */
                {
                    gettimeofday(&now, NULL);
                    timersub(&now, &trans_start, &diff_time);
                    print_statistics_finish(diff_time, total_trans, (double)success_trans / MEGABYTES, false);
                    last_record_time.tv_sec = 0;
                    last_record_time.tv_usec = 0;
                    last_record_bytes = 0;
                    total_trans = 0;
                    success_trans = 0;
                    last_record_bytes = 0;
                    rcv_start = false;
                    fflush(pd);
                    fclose(pd);
                }
            }
        }
        else
        {
            
            /******* MSG *********/
            {
                printf("timeout...nothing received for 5 seconds.\n");
                gettimeofday(&now, NULL);
                if (Cmp_time(last_recv_time, Zero_time) > 0)
                {
                    timersub(&now, &last_recv_time, &diff_time);
                    printf("last msg received %lf seconds ago.\n\n",
                        diff_time.tv_sec + (diff_time.tv_usec / 1000000.0));
                }
            }
             /******* SWITCH SENDER *******/
            if (done == 1) 
            {
                if (sender_q.size() != 0) 
                {
                    sender_q.erase(sender_q.begin());
                }
                done = 0;
                rcv_start = false;
                cum_seq = 0;
            } 
            
            // call the next sender if present
            if(sender_q.size() > 0) 
            {
                struct ack_pkt tmp_p;
                tmp_p.cum_seq = cum_seq;
                sendto_dbg(sock, (char *)&tmp_p, sizeof(tmp_p), 0, (struct sockaddr *)sender_q[0],
                        sizeof(from_addr)); 
                printf("switching sender!\n");
            }
        }   
    }

    return 0; // should not invoke
}

void init_receive(FILE *pd, net_pkt *pkt)
{
    fwrite((const char *)pkt->data, 1, pkt->dt_size, pd);
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    if (argc != 4)
    {
        Print_help();
    }

    sendto_dbg_init(atoi(argv[1]));

    if (sscanf(argv[2], "%d", &Port) != 1)
    {
        Print_help();
    }

    isLAN = argv[3][0] == 'L';
}

static void Print_help()
{
    printf("Usage: rcv <loss_rate_percent> <port> <env>\n");
    exit(0);
}

/* Returns 1 if t1 > t2, -1 if t1 < t2, 0 if equal */
static int Cmp_time(struct timeval t1, struct timeval t2)
{
    if (t1.tv_sec > t2.tv_sec)
        return 1;
    else if (t1.tv_sec < t2.tv_sec)
        return -1;
    else if (t1.tv_usec > t2.tv_usec)
        return 1;
    else if (t1.tv_usec < t2.tv_usec)
        return -1;
    else
        return 0;
}
