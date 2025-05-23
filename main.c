#include "daemon_server.h"

#define PORT 8080
#define BACKLOG 10
#define BUF_SZ 1024

int a2dVal;
void* lib_handler;
volatile sig_atomic_t is_cleaned_up = 0;

void cleanup_on_exit();
void handle_sigterm(int sig);

int main()
{
    lib_handler = handle_library();
    if (!lib_handler) 
    {
        syslog(LOG_ERR, "Failed to load librasp.so");
        exit(EXIT_FAILURE);
    }

    p_init_rasp();
    printf("------SERVER START------\n");

    // daemon
    struct sigaction sa;
    struct rlimit rl;
    int fd0, fd1, fd2, i;
    pid_t pid;

    // socket
    int serv_sock;
    struct sockaddr_in serv_addr;

    umask(0);
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        perror("getrlimit");
    }

    if ((pid = fork()) < 0)
    {
        perror("fork");
    }
    else if (pid != 0)
    {
        return 0;
    }

    setsid();

    //sa.sa_handler = SIG_IGN;
    sa.sa_handler = handle_sigterm;
    atexit(cleanup_on_exit);
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("sigaction: Can't ignore SIGHUP");
    }

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    if (chdir("/") < 0)
    {
        perror("cd()");
    }

    if(rl.rlim_max == RLIM_INFINITY) 
    {
        rl.rlim_max = 1024;
    }

    for(i = 0; i < rl.rlim_max; i++) 
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog("daemon_server", LOG_CONS, LOG_DAEMON);
    if(fd0 != 0 || fd1 != 1 || fd2 != 2) 
    {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        return -1;
    }

    syslog(LOG_INFO, "------------Daemon Process------------");

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        //perror("socket");
        syslog(LOG_ERR, "socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        //perror("bind");
        syslog(LOG_ERR, "bind");
        return -1;
    }

    if (listen(serv_sock, BACKLOG) == -1)
    {
        //perror("listen");
        syslog(LOG_ERR, "listen");
        return -1;
    }

    // acd
    int acd_fd;
    int a2dChannel = 0;

    if ((acd_fd = wiringPiI2CSetupInterface("/dev/i2c-1", 0x48)) < 0) {
        // printf("wiringPiI2CSetupInterface failed\n");
        syslog(LOG_ERR, "wiringPiI2CSetupInterface");
        exit(1);
    }
    wiringPiI2CWrite(acd_fd, 0x00 | a2dChannel);  // AIN0 선택

    while (1)
    {
        pthread_t handle_thd;
        char clnt_ip[INET_ADDRSTRLEN];
        int clnt_sock;
        struct sockaddr_in clnt_addr;
        socklen_t adr_sz = sizeof(clnt_addr);
        
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &adr_sz);
        if (clnt_sock < 0) {
            //perror("accept");
            syslog(LOG_ERR, "accept");
            continue;
        }

        thd_arg_t* t_arg = malloc(sizeof(thd_arg_t));
        t_arg->clnt_sock = clnt_sock;
        t_arg->acd_fd = acd_fd;
        
        inet_ntop(AF_INET, &clnt_addr.sin_addr, clnt_ip, sizeof(clnt_ip));
        //printf("Client IP: %s:%d\n", clnt_ip, ntohs(clnt_addr.sin_port));
        syslog(LOG_INFO, "Client IP: %s:%d", clnt_ip, ntohs(clnt_addr.sin_port));
        
        pthread_create(&handle_thd, NULL, handle_thd_func, (void*)t_arg);
        pthread_detach(handle_thd);
    }

    return 0;
}

void cleanup_on_exit()
{
    if (is_cleaned_up) 
        return;
    is_cleaned_up = 1;

    syslog(LOG_INFO, "cleaning up...");
    dlclose(lib_handler);
    closelog();
}

void handle_sigterm(int sig) {
    syslog(LOG_INFO, "SIGTERM detacted...");
    cleanup_on_exit();
    exit(0);
}
