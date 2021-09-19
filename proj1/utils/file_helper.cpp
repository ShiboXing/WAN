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