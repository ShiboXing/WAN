
#include <map>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

static int svr_port, app_port, loss_perc;
static map<long long unsigned int, chrono::steady_clock::time_point> timetable;
static map<long long unsigned int, net_pkt *> window;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[])
{
    int client_soc, app_soc;
    struct sockaddr_in client_addr, app_addr, curr_client_addr; /* addresses of receiver */
    struct timeval startTime, recordTime, currentTime;
    fd_set read_mask, tmp_mask;
    socklen_t from_len = sizeof(client_addr);
    double duration;
    long long unsigned int max_seq = 0, total_pkts = 0, cum_seq = 0, cache_seq = 1;
    bool isStart = false;

    curr_client_addr.sin_addr.s_addr = 0;

    struct timeval timeout;
    {
        Usage(argc, argv);
        client_soc = socket(AF_INET, SOCK_DGRAM, 0);
        app_soc = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_soc < 0 or app_soc < 0)
        {
            perror("error opening sending socket");
            exit(1);
        }

        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = INADDR_ANY;
        client_addr.sin_port = htons(svr_port);
        if (::bind(client_soc, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        { // bind client
            perror("server bind error");
            exit(1);
        }

        app_addr.sin_family = AF_INET;
        app_addr.sin_addr.s_addr = INADDR_ANY;
        app_addr.sin_port = htons(app_port);
        if (::bind(app_soc, (struct sockaddr *)&app_addr, sizeof(app_addr)) < 0)
        { // bind app
            perror("server bind error");
            exit(1);
        }
        FD_ZERO(&read_mask);
        FD_ZERO(&tmp_mask);
        FD_SET(client_soc, &read_mask);
        FD_SET(app_soc, &read_mask);
        printf(RESET "Successfully initialized with:\n");
        printf("\tloss rate = %d\n", loss_perc);
        printf("\tsvr Port = %d\n", svr_port);
        printf("\tapp Port = %d\n", app_port);
    }

    for (;;)
    {
        tmp_mask = read_mask;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        int num = select(FD_SETSIZE, &tmp_mask, NULL, NULL, &timeout);
        if (num > 0)
        {
            struct ack_pkt *tmp_pkt = (ack_pkt *)malloc(sizeof(ack_pkt));
            struct net_pkt *data_pkt = (net_pkt *)malloc(sizeof(net_pkt));
            /****** Receiving from client ******/
            if (FD_ISSET(client_soc, &tmp_mask))
            {
                data_pkt->senderTS = Time::now();
                if (recvfrom(client_soc, tmp_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, &from_len) <= 0)
                    continue;

                if (tmp_pkt->seq == 0) /* Phase I, delta calibration */
                {
                    if (curr_client_addr.sin_addr.s_addr != 0 && (client_addr.sin_addr.s_addr != curr_client_addr.sin_addr.s_addr || curr_client_addr.sin_port != client_addr.sin_port)) // extra sender, block it
                    {
                        data_pkt->seq = 0xffffffff;
                        sendto_dbg(client_soc, (char *)data_pkt, sizeof(*data_pkt), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                    else // requesting sender, send back the timestamp
                    {
                        sendto_dbg(client_soc, (char *)data_pkt, sizeof(*data_pkt), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        curr_client_addr = client_addr;
                    }
                }
                else /* Phase II, RT transmission*/
                {
                    if (tmp_pkt->is_nack == true)
                    { /* NACK */

                        if (!isStart) // [stat] record time of start
                        {   
                            gettimeofday(&startTime, NULL);
                            recordTime.tv_sec = startTime.tv_sec;
                            recordTime.tv_usec = startTime.tv_usec;
                            isStart = true;
                        }

                        if (window.find(tmp_pkt->seq) != window.end())
                        {
                            if (timetable.find(tmp_pkt->seq) == timetable.end()) // new request
                                timetable[tmp_pkt->seq] = data_pkt->senderTS;
                            else // retransmit request
                                data_pkt->senderTS = timetable[tmp_pkt->seq];
                            sendto_dbg(client_soc, (char *)window[tmp_pkt->seq], sizeof(*data_pkt), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            max_seq = tmp_pkt->seq > max_seq ? tmp_pkt->seq : max_seq;  // [stat] record highest sent pkt seq
                            total_pkts ++;                                              // [stat] count sent pkts
                        }
                    }
                    else if (cum_seq < tmp_pkt->seq)
                    { /* ACK */
                        cum_seq = tmp_pkt->seq;         // remove useless cache
                        while (timetable.begin()->first < cum_seq)
                            timetable.erase(timetable.begin());
                        while (window.begin()->first < cum_seq)
                            window.erase(window.begin());
                    }
                }
            }

            /****** Receiving from app ******/
            if (FD_ISSET(app_soc, &tmp_mask))
            {
                char tmp[MAX_PKT_LEN];
                int bytes = recvfrom(app_soc, tmp, sizeof(tmp), 0, (struct sockaddr *)&app_addr, &from_len);
                if (bytes <= 0)
                    continue;
                data_pkt->dt_size = bytes;
                data_pkt->seq = cache_seq;
                memcpy(data_pkt->data, tmp, data_pkt->dt_size);
                window[data_pkt->seq] = data_pkt;
                cache_seq++;
            }

            /****** Print stats every 5 seconds ******/
            gettimeofday(&currentTime, NULL);
            if (currentTime.tv_sec - recordTime.tv_sec >= 5 && isStart)
            {
                duration = currentTime.tv_sec - startTime.tv_sec + (currentTime.tv_usec - startTime.tv_usec) / 1000000;
                print_stat(false, duration, max_seq, cum_seq, 
                    total_pkts > cum_seq ? total_pkts - cum_seq : cum_seq - total_pkts, 0, .0, .0, .0);
                recordTime.tv_sec = currentTime.tv_sec;
                recordTime.tv_usec = currentTime.tv_usec;
            }
        }
        else
            cout << timeout.tv_sec << " seconds, " << timeout.tv_usec << " microseconds passed with no request or data received...\n";
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
    port_str = argv[2];
    if (port_str == NULL)
    {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    app_port = atoi(port_str);
    port_str = argv[3];
    if (port_str == NULL)
    {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    svr_port = atoi(port_str);
}

static void Print_help()
{
    printf("Usage: rt_rcv <loss_rate_percent> <app_port> <client_port>\n");
    exit(0);
}
