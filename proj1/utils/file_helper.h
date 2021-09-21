#include <iostream>
#include "packet.h"

// transfer functions
void fetch_next(FILE *f, FILE *f_end, struct net_pkt *pkt);

void print_statistics_finish(timeval &diff_time, double trans_data, double success_trans, bool isNpc);
void print_statistics(timeval &diff_time, double trans_data, double success_trans);
