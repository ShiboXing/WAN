#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

static int svr_port;
static int app_port;
static int loss_perc;
static unsigned long long int cum_seq = 0;
static map<unsigned long long int, chrono::steady_clock::time_point> timetable;
static map<unsigned long long int, char*> window;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[]) {
    int client_soc, app_soc;                
    struct sockaddr_in client_addr, app_addr; /* address of receiver */
    fd_set read_mask, tmp_mask;
    socklen_t from_len = sizeof(client_addr);
    struct timeval timeout;
    {
        Usage(argc, argv);
        client_soc = socket(AF_INET, SOCK_DGRAM, 0);
        app_soc = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_soc < 0 or app_soc < 0) {
            perror("error opening sending socket");
            exit(1);
        } 
        client_addr.sin_family = AF_INET; 
        client_addr.sin_addr.s_addr = INADDR_ANY; 
        client_addr.sin_port = htons(svr_port);
        if (::bind(client_soc, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0 ) { // bind client
            perror("server bind error");
            exit(1);
        }
        app_addr.sin_family = AF_INET; 
        app_addr.sin_addr.s_addr = INADDR_ANY; 
        app_addr.sin_port = htons(app_port);
        if (::bind(app_soc, (struct sockaddr *)&app_addr, sizeof(app_addr)) < 0 ) { // bind app
            perror("server bind error");
            exit(1);
        }
        FD_ZERO(&read_mask);
        FD_ZERO(&tmp_mask);
        FD_SET(client_soc, &read_mask); 
        FD_SET(app_soc, &read_mask);
        printf("Successfully initialized with:\n");
        printf("\tloss rate = %d\n", loss_perc);
        printf("\tsvr Port = %d\n", svr_port);
        printf("\tapp Port = %d\n", app_port);
    }

    for (;;) {
        tmp_mask = read_mask;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        int num = select(FD_SETSIZE, &tmp_mask, NULL, NULL, &timeout);
        if (num > 0) {
            struct ack_pkt* tmp_pkt = (ack_pkt*)malloc(sizeof(ack_pkt));
            struct net_pkt* data_pkt = (net_pkt*)malloc(sizeof(net_pkt));
            struct stream_pkt* app_pkt = (stream_pkt*)malloc(sizeof(stream_pkt));
            /****** Receiving from client ******/
            if (FD_ISSET(client_soc, &tmp_mask)) {
                data_pkt->senderTS = Time::now();
                if (recvfrom(client_soc, tmp_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, &from_len) <= 0) continue;
                if (tmp_pkt->seq == 0) { /* Phase I */
                    sendto_dbg(client_soc, (char*)data_pkt, sizeof(*data_pkt), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                } else { /* Phase II */
                    if (tmp_pkt->is_nack == true) { // NACK
                        if (window.find(tmp_pkt->seq) != window.end()) {
                            data_pkt->seq = tmp_pkt->seq;
                            memcpy(data_pkt->data, window[tmp_pkt->seq], sizeof(data_pkt->data));
                            sendto_dbg(client_soc, (char*)data_pkt, sizeof(*data_pkt), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            if (timetable.find(tmp_pkt->seq) == timetable.end()) {
                                timetable[tmp_pkt->seq] = data_pkt->senderTS;
                            } else {
                                data_pkt->senderTS = timetable[tmp_pkt->seq];
                            }
                            // printf(YELLOW "first 5 chars %d%d%d%d%d\n" RESET, data_pkt->data[0], data_pkt->data[1], data_pkt->data[2], data_pkt->data[3],  data_pkt->data[4]);
                        }
                    } else if (cum_seq < tmp_pkt->seq) { // ACK 
                        cum_seq = tmp_pkt->seq;
                        while (timetable.begin()->first < cum_seq) timetable.erase(timetable.begin());
                        while (window.begin()->first < cum_seq) window.erase(window.begin());
                        printf(YELLOW "ACK seq %llu\n" RESET, tmp_pkt->seq);
                    }
                }
            }
            /****** Receiving from app ******/
            if (FD_ISSET(app_soc, &tmp_mask)) {
                if (recvfrom(app_soc, app_pkt, sizeof(stream_pkt), 0, (struct sockaddr *)&app_addr, &from_len) <= 0) continue;
                // printf(YELLOW "from app first 5 chars %d%d%d%d%d\n" RESET, app_pkt->data[0], app_pkt->data[1], app_pkt->data[2], app_pkt->data[3],  app_pkt->data[4]);
                window[(unsigned long long int)app_pkt->seq] = (char*)malloc(sizeof(app_pkt->data));
                window[(unsigned long long int)app_pkt->seq] = app_pkt->data;
            }
        } else {
            printf("%ld seconds, %d microseconds passed with no request or data received...\n", timeout.tv_sec, timeout.tv_usec);
        }
    }

    return 0;
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[]) {
    char *port_str;

    if (argc != 4) {
        Print_help();
    }
    loss_perc = atoi(argv[1]);
    sendto_dbg_init(loss_perc);
    port_str = argv[2];
    if (port_str == NULL) {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    app_port = atoi(port_str);
    port_str = argv[3];
    if (port_str == NULL) {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    svr_port = atoi(port_str);
}

static void Print_help() {
    printf("Usage: rt_rcv <loss_rate_percent> <app_port> <client_port>\n");
    exit(0);
}
