#define PKT_DT_SIZE 1360
#define MAX_PKT_SIZE 1400
#define WIN_SIZE 5

struct net_pkt {
    char data[PKT_DT_SIZE];
    long long seq;
    int dt_size;
    bool is_end;
    long pid;
    long w_size;
} __attribute__((packed)); 


struct ack_pkt {
    long long seq;
    bool is_nack;
} __attribute__((packed)); 
