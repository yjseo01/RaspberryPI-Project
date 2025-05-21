#include "daemon_server.h"

void* led_thd_func(void* arg)
{
    while(1)
    {
        
    }
}

void* handle_thd_func(void* arg)
{
    int sock = *((int*)arg);
    FILE* clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], path[BUFSIZ];
    char filename[BUFSIZ], *ret;

    clnt_read = fdopen(sock, "r");
    clnt_write = fdopen(dup(sock), "w");

    if (!clnt_read || !clnt_write) {
        perror("fdopen");
        if (clnt_read) fclose(clnt_read);
        if (clnt_write) fclose(clnt_write);
        close(sock);
        pthread_exit(NULL);
    }

    if (fgets(reg_line, sizeof(reg_line), clnt_read) == NULL) {
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(0);
    }

    fputs(reg_line, stdout);

    // request parsing
    ret = strtok(reg_line, " ");
    strcpy(method, ret ? ret: "");
    ret = strtok(NULL, " ");
    strcpy(path, ret ? ret : "");

    // 정적 파일 요청 처리
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        FILE* fp = fopen("/home/yjseo/lnsp/project/RaspberryPI-Project/index.html", "r");

        if (fp == NULL) {
            fprintf(clnt_write,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n\r\n"
                "index.html not found\n");
        } else {
            fprintf(clnt_write,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n");

            char file_buf[1024];
            size_t n;
            while ((n = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
                fwrite(file_buf, 1, n, clnt_write);
            }
            fclose(fp);
        }
        fflush(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(NULL);
    }

    // method
    if (strcmp(method, "GET") != 0)
    {
        fprintf(clnt_write,
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Only GET supported\n");
        fflush(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(0);
    }

    // led control
    if (strcmp(path, "/led/on") == 0)
    {
        printf("ledon!\n");
        fprintf(clnt_write,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "LED ON\n");
    }
    else if (strcmp(path, "/led/off") == 0)
    {
        printf("ledoff!\n");
        fprintf(clnt_write,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "LED OFF\n");
    }
    else if (strncmp(path, "/led/brightness/", strlen("/led/brightness/")) == 0)
    {
        char* brightness_str = path + strlen("/led/brightness/");
        int brightness = atoi(brightness_str);
        printf("LED brightness: %d\n", brightness);
        
        fprintf(clnt_write,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "LED Brightness: %d\n", brightness);
    }
    else
    {
        fprintf(clnt_write,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Unknown command\n");
    }

    fclose(clnt_read);
    fclose(clnt_write);
    pthread_exit(0);

    return (void*)NULL;
}