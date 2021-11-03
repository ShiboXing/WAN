#include "stat_display.h"
#include "udp_stream_common.h"
#include "iostream"

using namespace std;

void print_stat(bool isRcv, double duration, long long unsigned int max_seq,
        long long unsigned int cum_seq, long long unsigned int re_pkts, long long unsigned int lost_pkts,
            double avg_delay, double min_delay, double max_delay)
{

    auto mb = (double)cum_seq * sizeof(net_pkt) / 1000000, bw = (double)cum_seq * sizeof(net_pkt) / 1000000 * 8 / duration,
        pbw = (double)cum_seq / duration;

    cout << "\n";
    cout << "The total amount of 'clean' data successfully transferred so far: " << 
        BOLDGREEN << mb << " megabytes, " << cum_seq << " packets" << RESET << "\n";
    cout << "The average transfer rate (for clean data) for the whole transfer so far " << 
        BOLDGREEN << bw << " megabits/sec, " << pbw << " packets/sec" << RESET "\n";
    cout << "The sequence number of the highest packet sent/receive so far: " << BOLDGREEN << max_seq << RESET << "\n";

    if (isRcv)
    {
        cout << "The total number of packets lost/dropped so far: " << 
            BOLDGREEN << lost_pkts << RESET << "\n";
        cout << "The average delay is " << BOLDGREEN << avg_delay << " ms, " << RESET <<  
            "the minimum delay is " << BOLDGREEN << min_delay << "ms, " << RESET << 
                "the maximum delay is " << BOLDGREEN << max_delay << "ms" << RESET "\n";
    }
    else
        cout << "The total number of retransmissions sent so far: " << BOLDGREEN << re_pkts << RESET << "\n";
}