#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits>
#include <map>

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include "udp_stream_common.h"
#include "sendto_dbg.h"
#include "stat_display.h"

using namespace std;

static char *svr_ip;
static int svr_port, app_port, loss_perc, delta = 0xffff;
static long long unsigned int cum_seq = 0;
static map<chrono::steady_clock::time_point, long long unsigned int> timetable;
static map<long long unsigned int, char *> window;
static map<long long unsigned int, int> size_map;


static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[])
{
    struct timeval startTime, recordTime, currentTime;
    double one_delay, avg_delay = chrono::milliseconds::zero().count(),
        min_delay = chrono::milliseconds::max().count(),
        max_delay = chrono::milliseconds::min().count(),
        data_bits = 0;
    int soc, host_num, data_pkts = 0;
    long int duration;
    long long unsigned int max_seq = 0;
    bool isStart = false;
    //static long long unsigned int last_record_seq = 0;
    struct sockaddr_in send_addr, app_addr;
    struct hostent h_ent, *p_h_ent;
    fd_set read_mask, tmp_mask;
    {
        Usage(argc, argv);
        soc = socket(AF_INET, SOCK_DGRAM, 0);
        if (soc < 0)
        {
            perror("error opening sending socket");
            exit(1);
        }
        p_h_ent = gethostbyname(svr_ip);
        memcpy(&h_ent, p_h_ent, sizeof(h_ent));
        memcpy(&host_num, h_ent.h_addr_list[0], sizeof(host_num));
        send_addr.sin_family = AF_INET;
        send_addr.sin_addr.s_addr = host_num;
        send_addr.sin_port = htons(svr_port);

        p_h_ent = gethostbyname("localhost");
        memcpy(&h_ent, p_h_ent, sizeof(h_ent));
        memcpy(&host_num, h_ent.h_addr_list[0], sizeof(host_num));
        app_addr.sin_family = AF_INET;
        app_addr.sin_addr.s_addr = host_num;
        app_addr.sin_port = htons(app_port);

        FD_ZERO(&read_mask);
        FD_ZERO(&tmp_mask);
        FD_SET(soc, &read_mask);
        printf(RESET "Successfully initialized with:\n");
        printf("\tloss rate = %d\n", loss_perc);
        printf("\tTarget IP = %s\n", svr_ip);
        printf("\tsvr Port = %d\n", svr_port);
        printf("\tapp Port = %d\n", app_port);
    }

    for (;;)
    {
        // good style to re-initialize
        tmp_mask = read_mask;
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_S;
        timeout.tv_usec = TIMEOUT_MS;
        int num = select(FD_SETSIZE, &tmp_mask, NULL, NULL, &timeout);

        if (num > 0)
        { // ON RECEIVED

            if (FD_ISSET(soc, &tmp_mask))
            {
                struct ack_pkt *tmp_pkt = (ack_pkt *)malloc(sizeof(ack_pkt));
                struct net_pkt *data_pkt = (net_pkt *)malloc(sizeof(net_pkt));
                if (recvfrom(soc, data_pkt, sizeof(net_pkt), 0, NULL, NULL) == 0) continue;
                one_delay = chrono::duration_cast<MS>(Time::now() - data_pkt->senderTS).count(); // calculate oneway delay for each pkt
                /* BLOCKED BY SVR */
                if (data_pkt->seq == 0xffffffff) {
                    cout << BOLDRED << "blocked by server!" << RESET << "\n";
                    exit(0);
                }
                /* phase I */
                if (cum_seq == 0 && delta == 0xffff)
                {
                    delta = chrono::duration_cast<MS>(Time::now() - data_pkt->senderTS).count();
                    cum_seq = 1;
                    cout << BOLDGREEN << "delta acquired: " << delta << " miliseconds" << RESET << "\n";
                    continue;
                }
                /* phase II */
                else if (data_pkt->seq == cum_seq) /* SEQUENTIAL */
                { 
                    if (!isStart) // [stat]
                    {
                        gettimeofday(&startTime, NULL);
                        recordTime.tv_sec = startTime.tv_sec;
                        recordTime.tv_usec = startTime.tv_usec;
                        isStart = true;
                    }

                    tmp_pkt->is_nack = false;
                    tmp_pkt->seq = cum_seq;
                    cum_seq = tmp_pkt->seq + 1;
                    long long unsigned int i = cum_seq;
                    for (; i < cum_seq + W_SIZE; i++)
                    { // acknowledging all sequential packets
                        if (window.find(i) == window.end()) break;
                        tmp_pkt->seq = i;
                    }
                    cum_seq = i; // update cum_seq outside the for-loop in case the entire window is filled (painful)
                    sendto_dbg(soc, (char *)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
                }

                /* cache for timed delivery */
                timetable[data_pkt->senderTS] = data_pkt->seq;
                window[data_pkt->seq] = (char *)malloc(sizeof(data_pkt->dt_size));
                memcpy(window[data_pkt->seq], data_pkt->data, sizeof(data_pkt->dt_size));

                struct stream_pkt app_pkt;
                memcpy(&app_pkt, window[data_pkt->seq], data_pkt->dt_size);

                data_pkts += 1;                 //[stat] success receive one pkt
                data_bits += sizeof(*data_pkt); //[stat] success receive data in bits
                if (data_pkt->seq > max_seq)
                    max_seq = data_pkt->seq; //[stat] record highest packet seq received
                avg_delay += (one_delay) / data_pkts; //[stat] avg one delay per pkt
                if (min_delay > one_delay)
                    min_delay = one_delay; //[stat] min one delay
                else if (max_delay < one_delay)
                    max_delay = one_delay; //[stat] max one delay
            }
        }
        else
        { // SEND NACKS
            for (long long unsigned int i = cum_seq; i < cum_seq + W_SIZE; i++)
            {
                if (window.find(i) == window.end())
                {
                    struct ack_pkt *tmp_pkt = (ack_pkt *)malloc(sizeof(ack_pkt));
                    tmp_pkt->seq = i;
                    tmp_pkt->is_nack = true;
                    sendto_dbg(soc, (char *)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
                }
            }
        }
        /****** Print stats every 5 seconds ******/
        gettimeofday(&currentTime, NULL);
        if (currentTime.tv_sec - recordTime.tv_sec >= 5 && isStart)
        {
            duration = currentTime.tv_sec - startTime.tv_sec;
            duration += (currentTime.tv_usec - startTime.tv_usec) / 1000000;
            print_stat(duration, max_seq, data_bits, data_pkts, true, avg_delay, min_delay, max_delay, 0);
            recordTime.tv_sec = currentTime.tv_sec;
            recordTime.tv_usec = currentTime.tv_usec;
            //last_record_seq = max_seq;
        }

        /* DELIVER PACKETS to app (painful) */
        while (timetable.size() != 0 && chrono::duration_cast<MS>(Time::now() - timetable.begin()->first).count() > delta + LATENCY)
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            long long unsigned int tmp_seq = timetable.begin()->second;
            cum_seq = (tmp_seq > cum_seq) ? (long long unsigned int)tmp_seq : cum_seq; // update cum_seq if needed
            sendto(soc, window[tmp_seq], sizeof(*window[tmp_seq]), 0, (struct sockaddr *)&app_addr, sizeof(app_addr)); // deliver

            timetable.erase(timetable.begin());
            window.erase(tmp_seq);
        }
    }

    return 0;
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    char *port_str;

    if (argc != 4)
    {
        Print_help();
    }
    loss_perc = atoi(argv[1]);
    sendto_dbg_init(loss_perc);
    svr_ip = strtok(argv[2], ":");
    if (svr_ip == NULL)
    {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    port_str = strtok(NULL, ":");
    if (port_str == NULL)
    {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    svr_port = atoi(port_str);
    port_str = argv[3];
    if (port_str == NULL)
    {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    app_port = atoi(port_str);
}

static void Print_help()
{
    printf("Usage: rt_rcv <loss_rate_percent> <server_ip>:<server_port> <app_port>\n");
    exit(0);
}
