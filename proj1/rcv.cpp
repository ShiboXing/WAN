#include <vector>
#include <iostream>
#include <stdio.h>

#include "utils/packet.h"
#include "utils/net_include.h"
#include "utils/sender_adrr.h"

static void Usage(int argc, char *argv[]);
static void Print_help();
static int Cmp_time(struct timeval t1, struct timeval t2);
void save_sender_information(npc_addr *npc, int &from_ip);
void init_receive(FILE *payload, char *buf);
void test_result();
static const struct timeval Zero_time = {0, 0};

static int Port;

FILE *payload;

int main(int argc, char *argv[])
{
    npc_addr npc;
    struct sockaddr_in name;
    struct sockaddr_in from_addr;
    int from_ip;
    socklen_t from_len;
    int sock;
    fd_set mask;
    fd_set read_mask;
    int bytes;
    int num;
    char mess_buf[sizeof(net_pkt)]; // if this not work, comment it, and use char mess_buf instead
    //char mess_buf[MAX_MESS_LEN];
    struct timeval timeout;
    struct timeval last_recv_time = {0, 0};
    struct timeval now;
    struct timeval diff_time;
    std::vector<int> buff;
    /* Parse commandline args */
    Usage(argc, argv);
    printf("Listening for messages on port %d\n", Port);

    // open destination
    payload = fopen("./rcv_payload/payload.txt", "wb");

    /* Open socket for receiving */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("udp_server: socket");
        exit(1);
    }

    /* Bind receive socket to listen for incoming messages on specified port */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(Port);

    if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("udp_server: bind");
        exit(1);
    }

    /* Set up mask for file descriptors we want to read from */
    FD_ZERO(&read_mask);
    FD_SET(sock, &read_mask);

    for (;;)
    {
        /* (Re)set mask and timeout */
        mask = read_mask;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        /* Wait for message or timeout */
        num = select(FD_SETSIZE, &mask, NULL, NULL, NULL);
        if (num > 0)
        {
            if (FD_ISSET(sock, &mask))
            {
                from_len = sizeof(from_addr);
                memset(mess_buf, 0, PKT_DT_SIZE);
                bytes = recvfrom(sock, mess_buf, sizeof(net_pkt), 0,
                                 (struct sockaddr *)&from_addr,
                                 &from_len);
                if (bytes == 0)
                    continue; // didn't receive anything

                init_receive(payload, mess_buf);
                from_ip = from_addr.sin_addr.s_addr;

                /* Record time we received this msg */
                gettimeofday(&last_recv_time, NULL);

                // printf("Received from (%d.%d.%d.%d)\n",
                //        (htonl(from_ip) & 0xff000000) >> 24,
                //        (htonl(from_ip) & 0x00ff0000) >> 16,
                //        (htonl(from_ip) & 0x0000ff00) >> 8,
                //        (htonl(from_ip) & 0x000000ff));

                //std::string ip_location = convertToString(format_ip, sizeof(format_ip) / sizeof(char));

                npc.from_addr = from_addr;
                npc.is = true;

                /* Echo message back to sender */
                // sendto(sock, feedback, bytes, 0, (struct sockaddr *)&from_addr,
                //        sizeof(from_addr));
            }
        }
        else
        {
            if (npc.is)
            {
                npc_addr *p = &npc;

                save_sender_information(p, from_ip);
            }

            printf("timeout...nothing received for 10 seconds.\n");
            gettimeofday(&now, NULL);
            if (Cmp_time(last_recv_time, Zero_time) > 0)
            {
                timersub(&now, &last_recv_time, &diff_time);
                printf("last msg received %lf seconds ago.\n\n",
                       diff_time.tv_sec + (diff_time.tv_usec / 1000000.0));
            }
        }
    }

    fclose(payload);
    return 0;
}

// std::string convertToString(char *format_ip, int size)
// {
//     int i;
//     std::string s = "";
//     for (i = 0; i < size; i++)
//     {
//         s = format_ip[i];
//     }
//     return s;
// }
void save_sender_information(npc_addr *p, int &from_ip)
{
    char format_ip[100];
    std::sprintf(format_ip, "./%d.%d.%d.%d",
                 (htonl(from_ip) & 0xff000000) >> 24,
                 (htonl(from_ip) & 0x00ff0000) >> 16,
                 (htonl(from_ip) & 0x0000ff00) >> 8,
                 (htonl(from_ip) & 0x000000ff));

    FILE *sender_file = fopen(format_ip, "wb");

    char data[sizeof(p)];
    memcpy(data, p, sizeof(p));
    fwrite(data, 1, sizeof(data), sender_file);
    fclose(sender_file);

    //test_result();
}

void init_receive(FILE *payload, char *buf)
{
    struct net_pkt *pkt = (struct net_pkt *)buf;
    fwrite((const char *)pkt->data, 1, pkt->dt_size, payload);
    if (pkt->is_end)
    {
        fflush(payload);
    }
    return;
}

/* Read commandline arguments */
static void Usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        Print_help();
    }

    if (sscanf(argv[1], "%d", &Port) != 1)
    {
        Print_help();
    }
}

static void Print_help()
{
    printf("Usage: udp_server <port>\n");
    exit(0);
}

/* Returns 1 if t1 > t2, -1 if t1 < t2, 0 if equal */
static int Cmp_time(struct timeval t1, struct timeval t2)
{
    if (t1.tv_sec > t2.tv_sec)
        return 1;
    else if (t1.tv_sec < t2.tv_sec)
        return -1;
    else if (t1.tv_usec > t2.tv_usec)
        return 1;
    else if (t1.tv_usec < t2.tv_usec)
        return -1;
    else
        return 0;
}
// void test_result()
// {
//     FILE *pFile;
//     FILE *payload_end;
//     long lSize;
//     char *buffer;
//     size_t result;

//     pFile = fopen("./127.0.0.1", "rb");
//     if (pFile == NULL)
//     {
//         fputs("File error", stderr);
//         exit(1);
//     }

//     // obtain file size:
//     fseek(pFile, 0, SEEK_END);
//     lSize = ftell(pFile);
//     rewind(pFile);

//     // allocate memory to contain the whole file:
//     buffer = (char *)malloc(sizeof(char) * lSize);
//     if (buffer == NULL)
//     {
//         fputs("Memory error", stderr);
//         exit(2);
//     }

//     // copy the file into the buffer:
//     result = std::fread(buffer, 1, lSize, pFile);
//     if (result != lSize)
//     {
//         fputs("Reading error", stderr);
//         exit(3);
//     }

//     /* the whole file is now loaded in the memory buffer. */
//     struct npc_addr *result_0 = (struct npc_addr *)buffer;
//     // terminate
//     int from_ip_1 = result_0->from_addr.sin_addr.s_addr;
//     printf("Received from (%d.%d.%d.%d)\n",
//            (htonl(from_ip_1) & 0xff000000) >> 24,
//            (htonl(from_ip_1) & 0x00ff0000) >> 16,
//            (htonl(from_ip_1) & 0x0000ff00) >> 8,
//            (htonl(from_ip_1) & 0x000000ff));
//     fclose(pFile);
//     free(buffer);
// }