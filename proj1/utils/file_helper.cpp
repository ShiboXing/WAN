#include "file_helper.h"
#include "packet.h"

void fetch_next(FILE* f, FILE* f_end, struct net_pkt *pkt) {
    
    // set pkt params
    pkt->w_size = W_SIZE;
    pkt->pid = PID;
    if (ftell(f_end) - ftell(f) < PKT_DT_SIZE) { // bound check for last packet
        pkt->dt_size = ftell(f_end) - ftell(f);
    } else {
        pkt->dt_size = PKT_DT_SIZE;
    }

    // read from disk
    fread(pkt->data, pkt->dt_size, 1, f);

    // check if last pkt
    if (ftell(f) >= ftell(f_end)) {
        pkt->is_end = true;
        fclose(f_end);
        fclose(f);
    } else {
        pkt->is_end = false;
    }
}

void print_statistics_finish(timeval &diff_time, double trans_data, double success_trans)
{
    long int time = diff_time.tv_sec + (diff_time.tv_usec / 1000000.0);
    double rate = ((trans_data / MEGABYTES) * MEGABITS) / time;
    printf("the size of the file transferred %f megabytes\nThe amount of time required for the transfer is %ld seconds\nthe average transfer rate is %f in megabits/sec\n\n", success_trans, time, rate);
};

void print_statistics(timeval &diff_time, double trans_data, double success_trans)
{
    double rate = ((trans_data / MEGABYTES) * MEGABITS) / (long int)(diff_time.tv_sec + (diff_time.tv_usec / 1000000.0));
    printf("The total amount of %f megabytes data successfully transferred so far.\nThe average transfer rate of the last 10 megabytes sent/received %f megabits/sec.\n\n", success_trans, rate);
}
