#ifndef PACKET_H
#define PACKET_H
#define PKT_DT_SIZE 1371
#define MAX_PKT_SIZE 1400
#define TIMEOUT_S 0
#define TIMEOUT_MS 10000  // microseconds

extern long long W_SIZE;
extern long long PID;

struct net_pkt {
    char data[PKT_DT_SIZE];
    long long seq;
    int dt_size;
    bool is_end;
    long long pid;
    long long w_size;
} __attribute__((packed)); 

struct ack_pkt {
    long long cum_seq;
    bool is_nack;
} __attribute__((packed)); 

#endif
