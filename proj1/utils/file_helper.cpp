#include "file_helper.h"
#include "packet.h"

void fetch_next(FILE *f, FILE *f_end, struct net_pkt *pkt)
{

    // set pkt params
    pkt->w_size = W_SIZE;
    if (ftell(f_end) - ftell(f) < PKT_DT_SIZE)
    { // bound check for last packet
        pkt->dt_size = ftell(f_end) - ftell(f);
    }
    else
    {
        pkt->dt_size = PKT_DT_SIZE;
    }

    // read from disk
    fread(pkt->data, pkt->dt_size, 1, f);

    // check if last pkt
    if (ftell(f) >= ftell(f_end))
    {
        pkt->is_end = true;
        fclose(f_end);
        fclose(f);
    }
    else
    {
        pkt->is_end = false;
    }
}

void print_statistics_finish(timeval &diff_time, double trans_data, double success_trans, bool isncp)
{
    long int time = diff_time.tv_sec + (diff_time.tv_usec / 1000000.0);
    double rate = ((trans_data / MEGABYTES) * MEGABITS) / time;
    printf("The size of the file transferred %f megabytes\nThe amount of time required for the transfer is %ld seconds\nThe average transfer rate is %f in megabits/sec\n\n", success_trans, time, rate);
    if (isncp)
    {
        printf("The total amount of data sent %f megabytes\n\n", (double)trans_data / MEGABYTES);
    }
}

void print_statistics(timeval &diff_time, double trans_data, double success_trans)
{
    double rate = ((trans_data / MEGABYTES) * MEGABITS) / (diff_time.tv_sec + ((double)diff_time.tv_usec / 1000000.0));
    printf("The total amount of %f megabytes data successfully transferred so far.\nThe average transfer rate of the last 10 megabytes sent/received %f megabits/sec.\n\n", success_trans, rate);
}
