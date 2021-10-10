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

static int svr_port;
static int app_port;
static int loss_perc;
static unsigned long long int cum_seq = 0;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[]) {
    int soc;                
    struct sockaddr_in send_addr; /* address of receiver */
    fd_set read_mask, tmp_mask;
    struct timeval timeout;
    {
        Usage(argc, argv);
        soc = socket(AF_INET, SOCK_DGRAM, 0);
        if (soc < 0) {
            perror("error opening sending socket");
            exit(1);
        } 
        send_addr.sin_family = AF_INET; 
        send_addr.sin_addr.s_addr = INADDR_ANY; 
        send_addr.sin_port = htons(svr_port);
        if (bind(soc, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0) { 
            perror("server bind error");
            exit(1);
        }
        FD_ZERO(&read_mask);
        FD_ZERO(&tmp_mask);
        FD_SET(soc, &read_mask); 
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
            if (FD_ISSET(soc, &tmp_mask)) {
                struct ack_pkt* tmp_pkt = (ack_pkt*)malloc(sizeof(ack_pkt));
                struct net_pkt* data_pkt = (net_pkt*)malloc(sizeof(net_pkt));
                socklen_t from_len = sizeof(send_addr);
                if (recvfrom(soc, tmp_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&send_addr, &from_len) <= 0) continue;
                if (tmp_pkt->seq == 0) { /* Phase I */
                    data_pkt->senderTS = Time::now();
                    sendto_dbg(soc, (char*)data_pkt, sizeof(*data_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
                } else { /* Phase II */
                    
                }
            }
        } else {
            printf("%ld seconds, %d microseconds passed with no data received...", timeout.tv_sec, timeout.tv_usec);
                // struct net_pkt* tmp_pkt = (net_pkt*) malloc(sizeof(net_pkt));
                // memcpy(tmp_pkt->data, &mock_seq, sizeof(mock_seq));
                // tmp_pkt->seq = mock_seq;
                // printf(YELLOW "first 5 chars %d%d%d%d%d\n" RESET, tmp_pkt->data[0], tmp_pkt->data[1], tmp_pkt->data[2], tmp_pkt->data[3],  tmp_pkt->data[4]);
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
