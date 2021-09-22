#include <netinet/in.h>
struct ncp_addr
{
    sockaddr_in from_addr;
    long w_size;
    long pid; 
    bool blked = false;
};