// #include <vector>
// #include <iostream>
// #include <stdio.h>
// #include <unordered_map>
// #include <algorithm>
#include "utils/file_helper.h"
#include "utils/net_include.h"
// #include "utils/sendto_dbg.h"

static void Usage(int argc, char *argv[]);
static void Print_help();
static void Clear_sock(int index);

static int Port;
static int Recv_socks[10];
static int Valid[10];
static fd_set Read_mask;
FILE *payload;

int main(int argc, char *argv[])
{
    struct sockaddr_in name;
    int listen_sock;
    fd_set mask;
    int i, j, num;
    int mess_len;
    int data_len;
    int bytes_read;
    int ret;
    char mess_buf[MAX_PKT_SIZE];
    long on = 1;
    long long W_SIZE = 0;

    double total_trans = 0;
    double success_trans = 0;
    double last_record_bytes = 0;

    struct timeval last_recv_time = {0, 0};
    struct timeval now;
    struct timeval last_record_time = {0, 0};
    struct timeval trans_start = {0, 0};
    struct timeval diff_time;

    bool rcv_start = false;

    /* Parse commandline args */
    Usage(argc, argv);
    printf("Listening for connections on port %d\n", Port);
    payload = fopen("./rcv_payload/payload1.txt", "wb");

    /* Open socket to listen for connections */
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0)
    {
        perror("tcp_server: socket");
        exit(1);
    }

    /* Allow binding to same local address multiple times */
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("Net_server: setsockopt error \n");
        exit(1);
    }

    /* Bind to listen on given port */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(Port);
    if (bind(listen_sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("tcp_server: bind");
        exit(1);
    }

    /* Start listening */
    if (listen(listen_sock, 4) < 0)
    {
        perror("tcp_server: listen");
        exit(1);
    }

    /* Init mask of file descriptors we're interested in */
    FD_ZERO(&Read_mask);
    FD_SET(listen_sock, &Read_mask);

    i = 0;
    for (;;)
    {
        /* (Re)set mask */
        mask = Read_mask;

        num = select(FD_SETSIZE, &mask, NULL, NULL, NULL);
        if (num > 0)
        {
            /* Accept a new connection */
            if (FD_ISSET(listen_sock, &mask))
            {
                if (i == 10)
                {
                    printf("Already at max connections!\n");
                    continue;
                }

                Recv_socks[i] = accept(listen_sock, 0, 0);
                FD_SET(Recv_socks[i], &Read_mask);
                Valid[i] = 1;
                i++;
            }

            /* Check kor incoming messages on existing connections */
            for (j = 0; j < i; j++)
            {
                if (Valid[j] && FD_ISSET(Recv_socks[j], &mask))
                {
                    /* Read message length */
                    if (!rcv_start)
                    {
                        gettimeofday(&last_record_time, NULL);
                        trans_start.tv_sec = last_record_time.tv_sec;
                        rcv_start = true;
                    }
                    bytes_read = 0;
                    while (bytes_read < sizeof(mess_len))
                    {
                        ret = recv(Recv_socks[j], &(((char *)&mess_len)[bytes_read]), sizeof(mess_len) - bytes_read, 0);
                        if (ret <= 0)
                        {
                            gettimeofday(&now, NULL);
                            timersub(&now, &trans_start, &diff_time);
                            print_statistics_finish(diff_time, total_trans, (double)success_trans / MEGABYTES, false);
                            last_record_time.tv_sec = 0;
                            last_record_time.tv_usec = 0;
                            last_record_bytes = 0;
                            rcv_start = false;

                            Clear_sock(j);
                            fflush(payload);
                            break;
                        }
                        bytes_read += ret;
                    }
                    if (bytes_read < sizeof(mess_len))
                        continue; /* socket was closed mid-read */

                    /* Read full message */
                    data_len = mess_len - sizeof(mess_len);
                    bytes_read = 0;
                    while (bytes_read < data_len)
                    {
                        ret = recv(Recv_socks[j], &mess_buf[bytes_read], data_len - bytes_read, 0);
                        if (ret <= 0)
                        {
                            gettimeofday(&now, NULL);
                            timersub(&now, &trans_start, &diff_time);
                            print_statistics_finish(diff_time, total_trans, (double)success_trans / MEGABYTES, false);
                            last_record_time.tv_sec = 0;
                            last_record_time.tv_usec = 0;
                            last_record_bytes = 0;
                            rcv_start = false;

                            Clear_sock(j);
                            fflush(payload);
                            break;
                        }
                    }
                    if (bytes_read < data_len)
                        continue; /* socket was closed mid-read */
                    fwrite(mess_buf, 1, sizeof(mess_buf), payload);

                    total_trans += bytes_read;
                    success_trans += data_len;

                    if (total_trans - last_record_bytes >= 10 * MEGABYTES)
                    {
                        gettimeofday(&now, NULL);
                        timersub(&now, &last_record_time, &diff_time);
                        double trans_data = (double)(total_trans - last_record_bytes);
                        print_statistics(diff_time, trans_data, (double)success_trans / MEGABYTES);
                        last_record_time.tv_sec = now.tv_sec;
                        last_record_bytes = total_trans;
                    }
                    //mess_buf[data_len] = '\0';
                    //printf("socket is %d ", j);
                    // printf("len is : %d  message is : %s \n ", mess_len, mess_buf);
                    // printf("---------------- \n");
                }
            }
        }
    }
    fclose(payload);
    return 0;
}

/* Close socket and clear from list of valid sockets */
static void Clear_sock(int index)
{
    printf("closing sock %d\n", index);
    FD_CLR(Recv_socks[index], &Read_mask);
    close(Recv_socks[index]);
    Valid[index] = 0;
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
    printf("Usage: tcp_server <port>\n");
    exit(0);
}
