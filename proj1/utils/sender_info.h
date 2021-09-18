#include <netinet/in.h>
struct npc_addr
{
    sockaddr_in from_addr;
    long w_size;
    long pid; 
    bool blked = false;
};