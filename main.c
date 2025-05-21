#include "daemon_server.h"

#define BACKLOG 10
#define BUF_SZ 1024

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: %s <port> \n", argv[0]);
        exit(1);
    }
    pthread_t worker_thd;

    int serv_sock;
    struct sockaddr_in serv_addr;

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind");
        return -1;
    }

    if (listen(serv_sock, BACKLOG) == -1)
    {
        perror("listen");
        return -1;
    }

    pthread_create(&worker_thd, NULL, worker_thd_func, NULL);

    while (1)
    {
        pthread_t handle_thd;
        char clnt_ip[INET_ADDRSTRLEN];
        int clnt_sock;
        struct sockaddr_in clnt_addr;
        socklen_t adr_sz = sizeof(clnt_addr);
        
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &adr_sz);
        if (clnt_sock < 0) {
            perror("accept");
            continue;
        }
        
        inet_ntop(AF_INET, &clnt_addr.sin_addr, clnt_ip, sizeof(clnt_ip));
        printf("Client IP: %s:%d\n", clnt_ip, ntohs(clnt_addr.sin_port));
        
        int* p_clnt_sock = malloc(sizeof(int));
        *p_clnt_sock = clnt_sock;
        pthread_create(&handle_thd, NULL, handle_thd_func, (void*)p_clnt_sock);
        pthread_detach(handle_thd);
    }

    pthread_join(worker_thd, NULL);

    return 0;
}