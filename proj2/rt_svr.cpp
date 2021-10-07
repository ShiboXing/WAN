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

static int svr_port;
static int app_port;
static int Num_msgs;
static int loss_perc;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[]) {
    int socket;                
    struct sockaddr_in name; /* address to bind to receive */
    fd_set read_mask, tmp_mask;
    int rcvd_bytes;
    struct stream_pkt rcv_pkt;
    struct timeval start_ts, now;
    long int duration;
    double rate;
    int rcvd_count;
    int last_rcvd;
    int out_of_order_count;
    int num;
    double max_oneway;
    double oneway;
    struct timeval next_report_time;
    struct timeval timeout;
    struct timeval *to_ptr;
    Usage(argc, argv);

    return 0;
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[]) {
    char *port_str;

    if (argc != 4) {
        Print_help();
    }
    loss_perc = atoi(argv[1]);
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
