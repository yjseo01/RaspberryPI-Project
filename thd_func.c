#include "thd_func.h"

RequestQueue cmd_queue;

void* worker_thd_func(void* arg)
{
    while (1)
    {
        if (is_empty(&cmd_queue))
            continue;

        Request* req = dequeue(&cmd_queue);

        switch (req->cmd)
        {
        case CMD_LED_ON:
            printf("led_on\n");
            break;
        case CMD_LED_OFF:
            printf("led_off\n");
            break;
        case CMD_BUZZER_ON:
            printf("buzzor_on\n");
            break;
        case CMD_BUZZER_OFF:
            printf("buzzor_off\n");
            break;
        case CMD_AUTO_LED:
            printf("led set\n");
            break;
        case CMD_COUNTDOWN:
            printf("countdown\n");
            break;
        default:
            break;
        }
        
        free(req);
    }
}

// 수정 필요요
void* handle_thd_func(void* arg)
{
    int sock = *((int*)arg);
    FILE* clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], path[BUFSIZ];
    char* ret;

    clnt_read = fdopen(sock, "r");
    clnt_write = fdopen(dup(sock), "w");

    if (!clnt_read || !clnt_write) {
        perror("fdopen");
        close(sock);
        pthread_exit(NULL);
    }

    if (fgets(reg_line, sizeof(reg_line), clnt_read) == NULL) {
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(NULL);
    }

    fputs(reg_line, stdout);

    // HTTP method / path 파싱
    ret = strtok(reg_line, " ");
    strcpy(method, ret ? ret : "");
    ret = strtok(NULL, " ");
    strcpy(path, ret ? ret : "");

    // index.html
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        FILE* fp = fopen("./index.html", "r");

        if (fp == NULL) {
            fprintf(clnt_write, "HTTP/1.1 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n\r\n"
                                "index.html not found\n");
        } else {
            fprintf(clnt_write, "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n\r\n");
            char buf[1024];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
                fwrite(buf, 1, n, clnt_write);
            }
            fclose(fp);
        }
        fflush(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(NULL);
    }

    if (strcmp(method, "GET") != 0) {
        fprintf(clnt_write, "HTTP/1.1 405 Method Not Allowed\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Only GET supported\n");
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(NULL);
    }

    // CommandType으로 파싱
    CommandType cmd = CMD_UNKNOWN;
    int brightness = -1;
    int sec = -1;

    if (strcmp(path, "/led/on") == 0) {
        cmd = CMD_LED_ON;
    } else if (strcmp(path, "/led/off") == 0) {
        cmd = CMD_LED_OFF;
    } else if (strncmp(path, "/led/brightness/", strlen("/led/brightness/")) == 0) {
        cmd = CMD_LED_BRIGHTNESS;
        brightness = atoi(path + strlen("/led/brightness/"));  // 추가로 저장 필요 시 여기서 값 전달
    } else if (strcmp(path, "/buzzon") == 0) {
        cmd = CMD_BUZZER_ON;
    } else if (strcmp(path, "/buzzoff") == 0) {
        cmd = CMD_BUZZER_OFF;
    } else if (strcmp(path, "/autoled") == 0) {
        cmd = CMD_AUTO_LED;
    } else if (strncmp(path, "/countdown?sec=", strlen("/countdown?sec=")) == 0) {
        cmd = CMD_COUNTDOWN;
        sec = atoi(path + strlen("/countdown?sec=")); // 마찬가지로 값 넘기고 싶으면 구조체 확장
    }

    // 큐에 명령 넣기
    if (cmd != CMD_UNKNOWN) {
        Request* req = (Request*)malloc(sizeof(Request));
        if (brightness != -1)
            req->arg = brightness;
        else 
            req->arg = sec;

        enqueue(&cmd_queue, cmd);
        fprintf(clnt_write, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Command received\n");
    } else {
        fprintf(clnt_write, "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Unknown command\n");
    }

    fclose(clnt_read);
    fclose(clnt_write);
    pthread_exit(NULL);
}