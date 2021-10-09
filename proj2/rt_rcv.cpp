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

static char *svr_ip;
static int svr_port;
static int app_port;
static int loss_perc;
static int cum_seq = 0;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[]) {
    int soc, host_num;                
    struct sockaddr_in send_addr;
    struct hostent h_ent;
    struct hostent *p_h_ent;
    fd_set read_mask, tmp_mask;
    struct timeval timeout;
    {
        
    
    for (;;) {
        /* good style to re-initialize */
        tmp_mask = read_mask;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int num = select(FD_SETSIZE, &tmp_mask, NULL, NULL, &timeout);
        if (num > 0) {
            if (FD_ISSET(soc, &tmp_mask)) {
                struct net_pkt* tmp_pkt = (net_pkt*) malloc(sizeof(net_pkt));
                if (recvfrom(soc, tmp_pkt, sizeof(net_pkt), 0, NULL, NULL) == 0) continue;
                printf("first 5 chars %d%d%d%d%d\n", tmp_pkt->data[0], tmp_pkt->data[1], tmp_pkt->data[2], tmp_pkt->data[3], tmp_pkt->data[4]);
            }
        } else {
            struct ack_pkt* tmp_pkt = (ack_pkt*) malloc(sizeof(ack_pkt));
            tmp_pkt->seq = cum_seq;
            tmp_pkt->is_nack = false;
            sendto_dbg(soc, (char*)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
            request_sent = true;
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
    svr_ip = strtok(argv[2], ":");
    if (svr_ip == NULL) {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    port_str = strtok(NULL, ":");
    if (port_str == NULL) {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    svr_port = atoi(port_str);
    port_str = argv[3];
    if (port_str == NULL) {
        printf("Error: incorrect target IP and port format\n");
        Print_help();
    }
    app_port = atoi(port_str);
}

static void Print_help() {
    printf("Usage: rt_rcv <loss_rate_percent> <server_ip>:<server_port> <app_port>\n");
    exit(0);
}
