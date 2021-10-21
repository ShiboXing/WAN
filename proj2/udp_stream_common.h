#ifndef CS2520_EX2_UDP_STREAM
#define CS2520_EX2_UDP_STREAM
#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define BOLDGREEN "\033[1m\033[32m"
#define BOLDRED "\033[1m\033[31m"
#define TARGET_RATE 8 /* rate to send at, in Mbps */
#define MAX_DATA_LEN 1300
#define MAX_PKT_LEN 1400
#define REPORT_SEC 5 /* print status every REPORT_SEC seconds */
#define W_SIZE_PER_SECOND 1785
#define TIMEOUT_MS 80000
#define TIMEOUT_S 0
#define W_SIZE 60
#define LATENCY 1000 // miliseconds

#include <chrono>
using namespace std;
typedef chrono::time_point<chrono::steady_clock> Tp;
typedef chrono::steady_clock Time;
typedef chrono::milliseconds MS;
typedef chrono::duration<float> Fsec;

struct stream_pkt
{
    int32_t seq;
    int32_t ts_sec;
    int32_t ts_usec;
    char data[MAX_DATA_LEN];
};

struct net_pkt
{
    Tp senderTS;
    long long unsigned int seq;
    int dt_size;
    char data[1380];
};

struct ack_pkt
{
    long long unsigned int seq;
    bool is_nack;
};

#endif
