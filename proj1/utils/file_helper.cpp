#include "file_helper.h"

net_pkt fetch_next(FILE* f, FILE* f_end) {
    // read to packet
    struct net_pkt pkt;
    
    // set packet data size
    if (ftell(f_end) - ftell(f) < PKT_DT_SIZE) { // bound check for last packet
        pkt.dt_size = ftell(f_end) - ftell(f);
    } else {
        pkt.dt_size = PKT_DT_SIZE;
    }

    // read the correct # of bytes
    fread(pkt.data, pkt.dt_size, 1, f);

    // check if last
    if (ftell(f) >= ftell(f_end)) {
        pkt.is_end = true;
        fclose(f_end);
        fclose(f);
    } else {
        pkt.is_end = false;
    }

    return pkt;
}