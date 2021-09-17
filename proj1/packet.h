#include <stdio.h>
#define PKT_DT_SIZE 1379
#define PKT_SIZE 1400

struct net_pkt {
    char data[PKT_DT_SIZE];
    long long seq;
    int dt_size;
    bool is_end;
} __attribute__((packed));  

