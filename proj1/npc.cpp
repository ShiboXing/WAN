#include "net_include.h"
#include "packet.h"
#include "sendto_dbg.h"
#include <vector>
#include <iostream>
#include <stdio.h>

using namespace std;

static void Usage(int argc, char *argv[]);
static void Print_help();

// transfer functions
void init_send(FILE* payload, int sock);

static char *Server_IP;
static int Port;

static vector<int> buff;
static vector<int> window;
static struct sockaddr_in send_addr;
static struct sockaddr_in from_addr;

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
    int win_size;


    // set-up 
    win_size = 6;

    // read file
    FILE* payload = fopen("./npc_payload/astrill-setup-win.exe", "rb");
    if (payload==NULL) {
        fputs("open payload error",stderr); 
        exit (1);
    }
    rewind(payload);

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
            init_send(payload, sock);
        }
    }

    // wrap up
    fclose(payload);
    return 0;
}

void init_send(FILE* payload, int sock) {
    // read to packet
    struct net_pkt pkt;
    char tmp[PKT_DT_SIZE];
    fread(tmp, sizeof(tmp), 1, payload);
    pkt.data = tmp;
    // send packet
    sendto_dbg(sock, (char*)&pkt, sizeof(pkt), 0,
        (struct sockaddr *)&send_addr, sizeof(send_addr));

    
    return;
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
