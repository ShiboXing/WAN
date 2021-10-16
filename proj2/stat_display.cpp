#include "stat_display.h"

void print_stat(long int duration, unsigned long long int max_seq, double data_bits, int data_pkts)
{
    double rate = data_bits / (1000000 * duration);
    double rate_pkt = data_pkts / (duration);
    printf("The total amount of 'clean' data successfully transferred so far\n");
    printf("%f megabytes,%i packets\n", data_bits / (8 * 1000000), data_pkts);
    printf("The average transfer rate (for clean data) for the whole transfer so far\n");
    printf("%f megabits/sec, %f packets/sec\n", rate, rate_pkt);
    printf("The sequence number of the highest packet sent so far: %llu\n\n", max_seq);
}