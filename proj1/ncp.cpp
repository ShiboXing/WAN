#include <vector>
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <algorithm>

#include "utils/net_include.h"
#include "utils/sendto_dbg.h"
#include "utils/file_helper.h"

using namespace std;

static void Usage(int argc, char *argv[]);
static void Print_help();
static void fill_win();

// IO
static char *Server_IP;
static int Port;
static int Pid;
static bool isLAN;
char* s_fname;
char* d_fname;
static struct sockaddr_in send_addr;
static struct sockaddr_in from_addr;
static FILE *pd;
static FILE *pd_end;


// window variables
static vector<net_pkt *> window;
unordered_map<long long, char> umap; // labels for packets. unacked = 'u', sent = 's'
static long long last_pkt = -1;
static long long pkt_cnt = 1;
long long W_SIZE;

int main(int argc, char *argv[])
{
    socklen_t from_len;
    struct hostent h_ent;
    struct hostent *p_h_ent;
    struct timeval timeout;
    struct timeval trans_start = {0, 0};
    struct timeval last_record = {0, 0};
    struct timeval trans_curr;
    struct timeval diff_time;
    int host_num;
    int sock;
    fd_set mask;
    fd_set read_mask;
    char mess_buf[sizeof(ack_pkt)];
    double total_trans = 0;
    double success_trans = 0;
    double last_record_bytes = 0;
    bool start_trans = false;
    int num;

    /* Parse commandline args */
    {
        Usage(argc, argv);
        printf("Sending to %s at port %d\n", Server_IP, Port);
        if (isLAN) W_SIZE = L_WSIZE;
        else W_SIZE = W_WSIZE;
        // read pd
        pd = fopen(s_fname, "rb");
        pd_end = fopen(s_fname, "rb");
        fseek(pd_end, 0, SEEK_END); // get end pointer
        /* Open socket for sending */
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            perror("rcv: socket");
            exit(1);
        }
        /* Convert string IP address (or hostname) to format we need */
        p_h_ent = gethostbyname(Server_IP);
        Pid = (long long)getpid();
        if (p_h_ent == NULL)
        {
            printf("rcv: gethostbyname error.\n");
            exit(1);
        }
        memcpy(&h_ent, p_h_ent, sizeof(h_ent));
        memcpy(&host_num, h_ent.h_addr_list[0], sizeof(host_num));
        send_addr.sin_family = AF_INET;
        send_addr.sin_addr.s_addr = host_num;
        send_addr.sin_port = htons(Port);
        printf("Sender IP: %d, pid: %d", host_num, Pid);
        /* Set up mask for file descriptors we want to read from */
        FD_ZERO(&read_mask);
        FD_SET(sock, &read_mask);
    }

    for (;;)
    {
        /* (Re)set mask */
        mask = read_mask;
        timeout.tv_sec = TIMEOUT_S;
        timeout.tv_usec = TIMEOUT_MS;

        num = select(FD_SETSIZE, &mask, NULL, NULL, &timeout);
        // receive ACKS, NACKS
        if (num > 0)
        {
            if (FD_ISSET(sock, &mask))
            {
                from_len = sizeof(from_addr);
                recvfrom(sock, mess_buf, sizeof(mess_buf), 0,
                                 (struct sockaddr *)&from_addr,
                                 &from_len);
                struct ack_pkt *ack_p = (struct ack_pkt *)mess_buf; // parse

                if (!ack_p->is_nack)
                {
                    while (window.size() != 0 && window[0]->seq <= ack_p->cum_seq)
                    { // dequeue buffer
                        window.erase(window.begin());
                        success_trans += ack_p->data_size;
                        umap.erase(ack_p->cum_seq);
                    }

                    if (last_pkt != -1 && ack_p->cum_seq >= last_pkt)
                    {
                        gettimeofday(&trans_curr, NULL);
                        timersub(&trans_curr, &trans_start, &diff_time);

                        print_statistics_finish(diff_time, total_trans, (double)success_trans / MEGABYTES, true);
                        return 0; // job is done
                    }
                }
                else
                {
                    struct net_pkt *pkt_it = *find_if(window.begin(), window.end(),
                                                      [ack_p](struct net_pkt *p)
                                                      {
                                                          return p->seq == ack_p->cum_seq;
                                                      });
                    sendto_dbg(sock, (char *)pkt_it, sizeof(*pkt_it), 0,
                               (struct sockaddr *)&send_addr, sizeof(send_addr));
                    umap[ack_p->cum_seq] = 's';
                }
            }
        }
        else
        {
            if (!start_trans)
            {
                start_trans = true;
                gettimeofday(&trans_start, NULL);
                last_record.tv_sec = trans_start.tv_sec;
            }
            // send un-acked packets
            fill_win();
            for (long long i = 0; i < (long long)window.size(); i++)
            {
                auto p = window[i];
                total_trans += sizeof(*p);
                if (umap[p->seq] == 'u')
                    sendto_dbg(sock, (char *)p, sizeof(*p), 0,
                            (struct sockaddr *)&send_addr, sizeof(send_addr));
                else
                    umap[p->seq] = 'u';

                if (p->is_end)
                    last_pkt = p->seq; // record the last pkt for termination
            }
        }
        if (total_trans - last_record_bytes >= 10 * MEGABYTES)
        {
            gettimeofday(&trans_curr, NULL);
            timersub(&trans_curr, &last_record, &diff_time);
            double trans_data = (double)(total_trans - last_record_bytes);
            print_statistics(diff_time, trans_data, (double)success_trans / MEGABYTES);
            last_record.tv_sec = trans_curr.tv_sec;
            last_record_bytes = total_trans;
        }
    }

    // shouldn't invoke
    return 0;
}

void fill_win()
{

    if ((long long)window.size() < W_SIZE && last_pkt == -1)
    {
        int new_amt = W_SIZE - window.size();
        struct net_pkt *pkt;
        for (int i = 0; i < new_amt; i++)
        {

            pkt = (struct net_pkt *)malloc(sizeof(struct net_pkt));
            window.push_back(pkt);
            fetch_next(pd, pd_end, pkt);
            pkt->seq = pkt_cnt++;
            pkt->pid = Pid;
            strcpy(pkt->d_fname, d_fname);
            if (ftell(pd) == -1) break; // guard for end packet
            umap[pkt->seq] = 'u';
        }
    }
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    if (argc != 5)
    {
        Print_help();
    }

    sendto_dbg_init((int)atoi(argv[1]));
    
    isLAN = argv[2][0] == 'L';
    
    s_fname = argv[3];
    
    d_fname = strtok(argv[4], "@:");
    Server_IP = strtok(NULL, "@:");
    if (Server_IP == NULL)
    {
        printf("Error: no server IP provided\n");
        Print_help();
    }
    Port = atoi(strtok(NULL, "@:"));
}

static void Print_help()
{
    printf("Usage: ncp <loss_rate_percent> <env> <source_file_name> <dest_file_name>@<ip_address>:<port>\n");
    exit(0);
}
