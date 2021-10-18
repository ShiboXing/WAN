#include "stat_display.h"

void print_stat(long int duration, int32_t max_seq, double data_bits, int data_pkts, bool isRcv, double avg_delay, double min_delay, double max_delay, int re_pkts)
{
    double rate = data_bits / (1000000 * duration);
    double rate_pkt = data_pkts / (duration);
    printf("The total amount of 'clean' data successfully transferred so far\n");
    printf("%f megabytes,%i packets\n", data_bits / (8 * 1000000), data_pkts);
    printf("The average transfer rate (for clean data) for the whole transfer so far\n");
    printf("%f megabits/sec, %f packets/sec\n", rate, rate_pkt);
    printf("The sequence number of the highest packet sent so far: %llu\n", max_seq);
    if (isRcv)
    {
        printf("The total number of packets lost/dropped so far: %llu\n", max_seq - data_pkts);
        printf("The average delay is %f ms, The minimum delay is %f ms, The maximum delay is %f ms\n", avg_delay, min_delay, max_delay);
    }
    else
    {
        printf("The total number of retransmissions sent so far: %i\n", re_pkts);
    }
    printf("\n");
}