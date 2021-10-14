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

using namespace std;

static char *svr_ip;
static int svr_port;
static int app_port;
static int loss_perc;
static unsigned int delta = numeric_limits<unsigned int>::max();
static unsigned long long int cum_seq = 0;
static map<unsigned long long int, chrono::steady_clock::time_point> timetable;
static map<unsigned long long int, char*> window;

static void Usage(int argc, char *argv[]);
static void Print_help();

int main(int argc, char *argv[]) {
    int soc, host_num;     
    struct sockaddr_in send_addr;
    struct hostent h_ent;
    struct hostent *p_h_ent;
    fd_set read_mask, tmp_mask;    
    
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_S;
    timeout.tv_usec = TIMEOUT_MS;

    {
        Usage(argc, argv);
        soc = socket(AF_INET, SOCK_DGRAM, 0);
        if (soc < 0) {
            perror("error opening sending socket");
            exit(1);
        } 
        p_h_ent = gethostbyname(svr_ip);
        memcpy(&h_ent, p_h_ent, sizeof(h_ent));
        memcpy(&host_num, h_ent.h_addr_list[0], sizeof(host_num));
        send_addr.sin_family = AF_INET; 
        send_addr.sin_addr.s_addr = host_num;
        send_addr.sin_port = htons(svr_port);
        FD_ZERO(&read_mask);
        FD_ZERO(&tmp_mask);
        FD_SET(soc, &read_mask); 
        printf(RESET "Successfully initialized with:\n");
        printf("\tloss rate = %d\n", loss_perc);
        printf("\tTarget IP = %s\n", svr_ip);
        printf("\tsvr Port = %d\n", svr_port);
        printf("\tapp Port = %d\n", app_port);
    }
    for (;;) {
        /* good style to re-initialize */
        tmp_mask = read_mask;
        int num = select(FD_SETSIZE, &tmp_mask, NULL, NULL, &timeout);
        if (num > 0) { // ON RECEIVED: SEND ACKS
            if (FD_ISSET(soc, &tmp_mask)) {
                struct ack_pkt* tmp_pkt = (ack_pkt*) malloc(sizeof(ack_pkt));
                struct net_pkt* data_pkt = (net_pkt*) malloc(sizeof(net_pkt));
                if (recvfrom(soc, data_pkt, sizeof(net_pkt), 0, NULL, NULL) == 0) continue;
                /* phase I */
                if (cum_seq == 0 && delta == numeric_limits<unsigned int>::max()) { 
                    delta = chrono::duration_cast<MS>(Time::now() - data_pkt->senderTS).count();
                    cum_seq = 1;
                    cout << BOLDGREEN << "delta acquired: " << delta << " miliseconds\n" << RESET;
                    continue;
                } 
                /* phase II */
                else if (data_pkt->seq == cum_seq) { // SEQUENTIAL     
                    tmp_pkt->is_nack = false;
                    tmp_pkt->seq = cum_seq;
                    sendto_dbg(soc, (char*)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr)); 
                    printf(YELLOW "[ACK] seq %llu\n" RESET, tmp_pkt->seq);
                    cum_seq = tmp_pkt->seq + 1;
                    unsigned long long int i = cum_seq;
                    for (; i < cum_seq + W_SIZE; i++) {
                        if (window.find(i) == window.end()) {
                            break;
                        }
                        tmp_pkt->seq = i;
                        sendto_dbg(soc, (char*)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr)); 
                        printf(YELLOW "[ACK] seq %llu\n" RESET, tmp_pkt->seq);
                    }
                    cum_seq = i; // update cum_seq, (outside for in case the entire window is filled, painful)
                } else if (data_pkt->seq > cum_seq) { // GAPPED
                    printf(YELLOW "[BUFFERED] seq %llu\n" RESET, data_pkt->seq);
                } 
                timetable[data_pkt->seq] = data_pkt->senderTS;
                window[data_pkt->seq] = (char*)malloc(sizeof(data_pkt->data));
                memcpy(window[data_pkt->seq], data_pkt->data, sizeof(data_pkt->data));
            }
        } else { // SEND NACKS 
            for (unsigned long long int i = cum_seq; i < cum_seq + W_SIZE; i++) {
                if (window.find(i) == window.end()) {
                    struct ack_pkt* tmp_pkt = (ack_pkt*) malloc(sizeof(ack_pkt));
                    tmp_pkt->seq = i;
                    tmp_pkt->is_nack = true;
                    sendto_dbg(soc, (char*)tmp_pkt, sizeof(*tmp_pkt), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
                }
            }
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
