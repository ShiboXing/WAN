// #include <vector>
// #include <iostream>
// #include <stdio.h>
// #include <unordered_map>
// #include <algorithm>
#include "utils/file_helper.h"
//#include "utils/sendto_dbg.h"
#include "utils/net_include.h"

static void Usage(int argc, char *argv[]);
static void Print_help();
static char *Server_IP;
static int Port;
static FILE *pd;
using namespace std;
long long W_SIZE; // for linker

int main(int argc, char *argv[])
{
    struct sockaddr_in host;
    struct hostent h_ent, *p_h_ent;

    struct timeval trans_start = {0, 0};
    struct timeval last_record = {0, 0};
    struct timeval trans_curr;
    struct timeval diff_time;

    double total_trans = 0;
    double success_trans = 0;
    double last_record_bytes = 0;

    int sock;
    int ret;
    int bytes_sent;
    int mess_len;
    char mess_buf[MAX_PKT_SIZE];
    char *data_mess_ptr = &mess_buf[sizeof(mess_len)];

    /* Parse commandline args */
    Usage(argc, argv);
    printf("Sending to %s at port %d\n", Server_IP, Port);
    pd = fopen("./ncp_pd/pd1.txt", "rb");
    /* Open socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("tcp_client: socket error\n");
        exit(1);
    }

    /* Set up server address to send to */
    p_h_ent = gethostbyname(Server_IP);
    if (p_h_ent == NULL)
    {
        printf("tcp_client: gethostbyname error.\n");
        exit(1);
    }
    memcpy(&h_ent, p_h_ent, sizeof(h_ent));
    memcpy(&host.sin_addr, h_ent.h_addr_list[0], sizeof(host.sin_addr));
    host.sin_family = AF_INET;
    host.sin_port = htons(Port);

    /* Connect to server */
    ret = connect(sock, (struct sockaddr *)&host, sizeof(host));
    if (ret < 0)
    {
        perror("tcp_client: could not connect to server\n");
        exit(1);
    }
    gettimeofday(&trans_start, NULL);
    last_record.tv_sec = trans_start.tv_sec;
    for (;;)
    {
        int tmp_bytes = fread(data_mess_ptr, 1, sizeof(mess_buf) - sizeof(mess_len), pd);
        mess_len = tmp_bytes + sizeof(mess_len);
        /* Put message length into beginning of message buffer */
        memcpy(mess_buf, &mess_len, sizeof(mess_len));

        /* Send message */
        bytes_sent = 0;
        while (bytes_sent < mess_len)
        {
            ret = send(sock, &mess_buf[bytes_sent], mess_len - bytes_sent, 0);

            if (ret < 0)
            {
                perror("tcp_client: error in sending\n");
                exit(1);
            }
            bytes_sent += ret;
        }

        total_trans += bytes_sent;
        success_trans += strlen(data_mess_ptr);
        if (total_trans - last_record_bytes >= 10 * MEGABYTES)
        {
            gettimeofday(&trans_curr, NULL);
            timersub(&trans_curr, &last_record, &diff_time);
            double trans_data = (double)(total_trans - last_record_bytes);
            print_statistics(diff_time, trans_data, (double)success_trans / MEGABYTES);
            last_record.tv_sec = trans_curr.tv_sec;
            last_record.tv_usec = trans_curr.tv_usec;
            last_record_bytes = total_trans;
        }

        if (feof(pd))
        {
            fclose(pd);
            break;
        }
    }

    gettimeofday(&trans_curr, NULL);
    timersub(&trans_curr, &trans_start, &diff_time);

    print_statistics_finish(diff_time, total_trans, (double)success_trans / MEGABYTES, true);
    return 0;
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
}

static void Print_help()
{
    printf("Usage: udp_client <server_ip>:<port>\n");
    exit(0);
}