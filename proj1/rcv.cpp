#include <vector>
#include <iostream>
#include <stdio.h>

#include "utils/packet.h"
#include "utils/net_include.h"
#include "utils/sendto_dbg.h"
#include "utils/sender_info.h"

static void Usage(int argc, char *argv[]);
static void Print_help();
static int Cmp_time(struct timeval t1, struct timeval t2);
static void print_statistics(timeval &diff_time, double tran_data, double success_trans);

void wr_npc(npc_addr *npc, int &from_ip);
void init_receive(FILE *payload, struct net_pkt *pkt);
void test_result();
static const struct timeval Zero_time = {0, 0};

// IO
static int Port;
FILE *payload;

// data structures
static std::vector<net_pkt *> window;
static long long cum_seq = 0;
long long W_SIZE; // for linker
long long PID;    // for linker

int main(int argc, char *argv[])
{
    npc_addr npc;
    struct sockaddr_in name;
    struct sockaddr_in from_addr;
    int from_ip;
    socklen_t from_len;
    int sock;
    fd_set mask;
    fd_set read_mask;
    int bytes;

    double total_trans = 0;
    double success_trans = 0;
    double last_record_bytes = 0;
    bool start_rcv = false;

    int num;
    char mess_buf[sizeof(net_pkt)];
    struct timeval timeout;
    struct timeval last_recv_time = {0, 0};
    struct timeval now;
    struct timeval last_record_time = {0, 0};
    struct timeval diff_time;
    /* Parse commandline args */
    Usage(argc, argv);
    printf("Listening for messages on port %d\n", Port);

    // open destination
    payload = fopen("./rcv_payload/payload.txt", "wb");

    /* Open socket for receiving */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("udp_server: socket");
        exit(1);
    }

    /* Bind receive socket to listen for incoming messages on specified port */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(Port);

    if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("udp_server: bind");
        exit(1);
    }

    /* Set up mask for file descriptors we want to read from */
    FD_ZERO(&read_mask);
    FD_SET(sock, &read_mask);

    for (;;)
    {
        /* (Re)set mask and timeout */
        mask = read_mask;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        /* Wait for message or timeout */
        num = select(FD_SETSIZE, &mask, NULL, NULL, NULL);
        if (num > 0)
        {
            if (FD_ISSET(sock, &mask))
            {
                from_len = sizeof(from_addr);
                memset(mess_buf, 0, PKT_DT_SIZE);
                bytes = recvfrom(sock, mess_buf, sizeof(net_pkt), 0,
                                 (struct sockaddr *)&from_addr,
                                 &from_len);
                if (bytes == 0)
                    continue;                        // didn't receive anything
                gettimeofday(&last_recv_time, NULL); // record time of receival
                from_ip = from_addr.sin_addr.s_addr;
                struct net_pkt *pkt = (struct net_pkt *)mess_buf; // parse
                // printf("Received from npc (%d.%d.%d.%d) pkt.seq: %lld\n",
                //        (htonl(from_ip) & 0xff000000) >> 24,
                //        (htonl(from_ip) & 0x00ff0000) >> 16,
                //        (htonl(from_ip) & 0x0000ff00) >> 8,
                //        (htonl(from_ip) & 0x000000ff), pkt->seq);
                total_trans += sizeof(*pkt);
                /* HANDLE REIVE */
                struct ack_pkt p;
                if (pkt->seq <= cum_seq)
                {
                    p.cum_seq = cum_seq; // re-sent the cum_ack
                    p.is_nack = false;

                    sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                               sizeof(from_addr));
                }
                else if (pkt->seq == cum_seq + 1)
                {
                    p.cum_seq = cum_seq = pkt->seq;
                    p.data_size = pkt->dt_size;
                    p.is_nack = false;

                    sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                               sizeof(from_addr)); // send the new cum-ack
                    success_trans += pkt->dt_size;
                    init_receive(payload, pkt); //write to disk
                    // dequeue buffer
                    while (window.size() != 0 && window.front()->seq == cum_seq + 1)
                    {
                        p.cum_seq = cum_seq = pkt->seq;
                        p.is_nack = false;

                        sendto_dbg(sock, (char *)&p, sizeof(p), 0, (struct sockaddr *)&from_addr,
                                   sizeof(from_addr));
                        window.erase(window.begin()); // delete the buffered pkt

                        success_trans += pkt->dt_size;

                        init_receive(payload, pkt); //write to disk
                    }
                }
                else if (window.size() < pkt->w_size)
                { // buffer the deferred pkt
                    window.push_back(pkt);
                    sort(window.begin(), window.end(),
                         [](auto &a, auto &b) -> bool
                         {
                             return a->seq < b->seq;
                         });
                }
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
        }
        else
        {
            // if (npc.blked)
            // {
            //     wr_npc(&npc, from_ip);
            // }

            printf("timeout...nothing received for 10 seconds.\n");
            gettimeofday(&now, NULL);
            if (Cmp_time(last_recv_time, Zero_time) > 0)
            {
                timersub(&now, &last_recv_time, &diff_time);
                printf("last msg received %lf seconds ago.\n\n",
                       diff_time.tv_sec + (diff_time.tv_usec / 1000000.0));
            }
        }
    }

    fclose(payload);
    return 0;
}

// std::string convertToString(char *format_ip, int size)
// {
//     int i;
//     std::string s = "";
//     for (i = 0; i < size; i++)
//     {
//         s = format_ip[i];
//     }
//     return s;
// }
void wr_npc(npc_addr *p, int &from_ip)
{
    char format_ip[100];
    printf("sender %d.%d.%d.%d pid: %d, blocked",
           (htonl(from_ip) & 0xff000000) >> 24,
           (htonl(from_ip) & 0x00ff0000) >> 16,
           (htonl(from_ip) & 0x0000ff00) >> 8,
           (htonl(from_ip) & 0x000000ff), -1);

    FILE *npc_info = fopen(format_ip, "wb");
    char data[sizeof(npc_addr)];
    memcpy(data, p, sizeof(npc_addr));
    fwrite(data, sizeof(data), 1, npc_info);
    fflush(npc_info);
    fclose(npc_info);
}

void init_receive(FILE *payload, net_pkt *pkt)
{
    fwrite((const char *)pkt->data, 1, pkt->dt_size, payload);
    if (pkt->is_end)
    {
        fflush(payload);
    }
    return;
}

static void print_statistics(timeval &diff_time, double trans_data, double success_trans)
{

    double rate = ((trans_data / MEGABYTES) * MEGABITS) / (long int)(diff_time.tv_sec + (diff_time.tv_usec / 1000000.0));
    printf("The total amount of %f megabytes data successfully transferred so far.\nThe average transfer rate of the last 10 megabytes sent/received %f megabits/sec.\n\n", success_trans, rate);
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        Print_help();
    }

    if (sscanf(argv[1], "%d", &Port) != 1)
    {
        Print_help();
    }
}

static void Print_help()
{
    printf("Usage: udp_server <port>\n");
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
