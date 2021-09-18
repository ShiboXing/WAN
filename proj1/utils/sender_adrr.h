#include <netinet/in.h>
struct npc_addr
{
    sockaddr_in from_addr;
    unsigned short int window_size;
    bool is = false;
};