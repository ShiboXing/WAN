#include <vector>
#include <iostream>
#include <stdio.h>
#include <unordered_map>

#include "utils/net_include.h"
#include "utils/sendto_dbg.h"
#include "utils/file_helper.h"

using namespace std;

static void Usage(int argc, char *argv[]);
static void Print_help();
static void fill_win();

static char *Server_IP;
static int Port;

static struct sockaddr_in send_addr;
static struct sockaddr_in from_addr;
static FILE* payload;
static FILE* payload_end;

// window variables
static vector<net_pkt> buff;
static vector<net_pkt> window;
unordered_map<int, char> umap; // labels for packets. unacked = 'u', sent = 's'
static int pkt_cnt = 0;


int main(int argc, char *argv[])
{
    socklen_t from_len;
    struct hostent h_ent;
    struct hostent *p_h_ent;
    struct timeval timeout;
    int host_num;
    int from_ip;
    int sock;
    fd_set mask;
    fd_set read_mask;
    char mess_buf[MAX_MESS_LEN];
    int bytes;
    int num;

    // read file
    payload = fopen("./npc_payload/payload.txt", "rb");
    payload_end = fopen("./npc_payload/payload.txt", "rb");
    fseek(payload_end,0,SEEK_END); // get end pointer

    /* Parse commandline args */
    Usage(argc, argv);
    printf("Sending to %s at port %d\n", Server_IP, Port);
    
    /* Open socket for sending */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("udp_client: socket");
        exit(1);
    }

    /* Convert string IP address (or hostname) to format we need */
    p_h_ent = gethostbyname(Server_IP);
    if (p_h_ent == NULL)
    {
        printf("udp_client: gethostbyname error.\n");
        exit(1);
    }
    
    memcpy(&h_ent, p_h_ent, sizeof(h_ent));
    memcpy(&host_num, h_ent.h_addr_list[0], sizeof(host_num));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = host_num;
    send_addr.sin_port = htons(Port);
    /* Set up mask for file descriptors we want to read from */
    FD_ZERO(&read_mask);
    FD_SET(sock, &read_mask);

    for (;;)
    {
        /* (Re)set mask */
        mask = read_mask;
        timeout.tv_usec = 100;

        num = select(FD_SETSIZE, &mask, NULL, NULL, &timeout);
        // receive ACKS, NACKS
        if (num > 0)
        {
            if (FD_ISSET(sock, &mask))
            {
                from_len = sizeof(from_addr);
                bytes = recvfrom(sock, mess_buf, sizeof(mess_buf), 0,
                                 (struct sockaddr *)&from_addr,
                                 &from_len);
                mess_buf[bytes] = '\0'; /* ensure string termination for nice printing to screen */
                from_ip = from_addr.sin_addr.s_addr;

                printf("Received from (%d.%d.%d.%d): %s\n",
                       (htonl(from_ip) & 0xff000000) >> 24,
                       (htonl(from_ip) & 0x00ff0000) >> 16,
                       (htonl(from_ip) & 0x0000ff00) >> 8,
                       (htonl(from_ip) & 0x000000ff),
                       mess_buf);
            }
        } else {
            struct net_pkt pkt = fetch_next(payload, payload_end);
            
            // send packet
            sendto_dbg(sock, (char*)&pkt, sizeof(pkt), 0,
                (struct sockaddr *)&send_addr, sizeof(send_addr));

            if (pkt.is_end) { // finish transfering but don't close socket
                return 0;
            }
        }
    }

    // wrap up
    return 0;
}

void fill_win() {
    while (window.size() < WIN_SIZE) {
        struct net_pkt pkt = fetch_next(payload, payload_end);
        pkt.seq = pkt_cnt++;
        window.push_back(pkt);
        umap[pkt.seq] = 'u';
    }

}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        Print_help();
    }

    Server_IP = strtok(argv[1], ":");
    if (Server_IP == NULL)
    {
        printf("Error: no server IP provided\n");
        Print_help();
    }
    Port = atoi(strtok(NULL, ":"));

    /* set loss rate */
    sendto_dbg_init(0);
}

static void Print_help()
{
    printf("Usage: udp_client <server_ip>:<port>\n");
    exit(0);
}
