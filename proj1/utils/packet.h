#ifndef PACKET_H
#define PACKET_H
#define PKT_DT_SIZE 1355
#define MAX_PKT_SIZE 1400
#define TIMEOUT_S 0
#define TIMEOUT_MS 10000 // microseconds
#define RECORD_TIME 10    //seconds;
#define MEGABYTES 1000000 // 1 megabytes = 1000000bytes
#define MEGABITS 8        //1 megabytes = 8 megabits
#define S_CACHE ".sender_cache/"

extern long long W_SIZE;

struct net_pkt
{
    char data[PKT_DT_SIZE];
    char d_fname[20];
    long long seq;
    long long w_size;
    int dt_size;
    int pid;
    bool is_end;
} __attribute__((packed));

struct ack_pkt
{
    long long cum_seq;
    int data_size;
    bool is_nack;
} __attribute__((packed));

#endif
