#ifndef CS2520_EX2_UDP_STREAM
#define CS2520_EX2_UDP_STREAM
#define RESET   "\033[0m"
#define YELLOW  "\033[33m"
#define BOLDGREEN   "\033[1m\033[32m"
#define TARGET_RATE 8 /* rate to send at, in Mbps */
#define MAX_DATA_LEN 1300
#define REPORT_SEC 5 /* print status every REPORT_SEC seconds */
#define W_SIZE_PER_SECOND 1785
#define TIMEOUT_MS 10000
#define TIMEOUT_S 0
#define W_SIZE 20
#define LATENCY 1000 // miliseconds

#include <chrono>
using namespace std;
typedef chrono::time_point<chrono::steady_clock> Tp;
typedef chrono::steady_clock Time;
typedef chrono::milliseconds MS;
typedef chrono::duration<float> Fsec;

struct stream_pkt {
    int32_t seq;
    int32_t ts_sec;
    int32_t ts_usec;
    char data[MAX_DATA_LEN];
};

struct net_pkt {
    Tp senderTS;
    unsigned long long int seq;
    bool is_end;
    char data[MAX_DATA_LEN];
} __attribute__((packed));

struct ack_pkt {
    unsigned long long int seq;
    bool is_nack;
} __attribute__ ((packed));

#endif
