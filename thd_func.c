#include "thd_func.h"

pthread_t g_buzzer_tid;
atomic_bool g_buzzer_running = false;
pthread_mutex_t acdmtx2;

void* handle_thd_func(void* arg)
{
    thd_arg_t* t_arg = (thd_arg_t*)arg;
    int clnt_sock = t_arg->clnt_sock;
    int acd_fd = t_arg->acd_fd;
    
    pthread_mutex_init(&acdmtx2, NULL);
    syslog(LOG_DEBUG, "handle_thd");

    int sock = *((int*)arg);
    FILE* clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], path[BUFSIZ];
    char* ret;
    pthread_t tid;

    clnt_read = fdopen(sock, "r");
    clnt_write = fdopen(dup(sock), "w");

    if (!clnt_read || !clnt_write) {
        //perror("fdopen");
        syslog(LOG_ERR, "fdopen");
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
            syslog(LOG_DEBUG, "[DEBUG] 404 not found");
            fprintf(clnt_write, "HTTP/1.1 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n\r\n"
                                "index.html not found\n");
        } else {
            syslog(LOG_DEBUG, "[DEBUG] 200 ok");
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
        syslog(LOG_DEBUG, "[DEBUG] 405 method not allowed");
        fprintf(clnt_write, "HTTP/1.1 405 Method Not Allowed\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Only GET supported\n");
        fclose(clnt_read);
        fclose(clnt_write);
        pthread_exit(NULL);
    }

    int brightness = 0;
    int sec = 0;
    int note = 0;

    if (strcmp(path, "/ledon") == 0) {
        cmd_led_on();
    } else if (strcmp(path, "/ledoff") == 0) {
        cmd_led_off();

    } else if (strncmp(path, "/ledbrightness?value=", strlen("/ledbrightness?value=")) == 0) {
        brightness = atoi(path + strlen("/ledbrightness?value="));
        cmd_led_set_brightness(brightness);
        if (led_state == LED_OFF)
        {
            led_state = LED_ON;
        }

    } else if (strcmp(path, "/buzzon") == 0) {
        if (!atomic_load(&g_buzzer_running)) 
        {
            pthread_create(&g_buzzer_tid, NULL, handle_buzzer_on, NULL);
            pthread_detach(g_buzzer_tid);
        }           
    } else if (strcmp(path, "/buzzoff") == 0) {
        syslog(LOG_INFO, "[CMD] buzzer off");
        if (atomic_load(&g_buzzer_running)) 
        {
            pthread_cancel(g_buzzer_tid);
        }
    } else if (strcmp(path, "/autoled") == 0) {
        pthread_mutex_lock(&acdmtx2);
        syslog(LOG_INFO, "[CMD] led auto set");

        int a2dVal = cmd_get_brightness(acd_fd);
        if (a2dVal > THRESHOLD)
        {
            //printf("[ADC] bright(a2dVal = %d)\n", a2dVal);
            syslog(LOG_INFO, "[ADC] bright(a2dVal = %d)", a2dVal);
            cmd_led_off();
        }
        else
        {
            //printf("[ADC] dark(a2dVal = %d)\n", a2dVal);
            syslog(LOG_INFO, "[ADC] dark(a2dVal = %d)", a2dVal);
            cmd_led_on();
        }
        pthread_mutex_unlock(&acdmtx2);

    } else if (strncmp(path, "/countdown?sec=", strlen("/countdown?sec=")) == 0) {
        sec = atoi(path + strlen("/countdown?sec=")); 
        pthread_create(&tid, NULL, handle_countdown, (void*)(intptr_t)sec);
        pthread_detach(tid);
    } else if (strncmp(path, "/play?freq=", strlen("/play?freq=")) == 0) {
        note = atoi(path + strlen("/play?freq=")); 
        syslog(LOG_INFO, "[CMD] play pitch");
        softToneCreate(BUZZER_GPIO);
        softToneWrite(BUZZER_GPIO, note);
        delay(500);
        softToneWrite(BUZZER_GPIO, 0);
    }
    else
    {
        syslog(LOG_DEBUG, "[DEBUG] 404 not found");
        fprintf(clnt_write, "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Unknown command\n");

        fclose(clnt_read);
        fclose(clnt_write);
        pthread_mutex_destroy(&acdmtx2);
        pthread_exit(NULL);
    }

    fprintf(clnt_write, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Command received\n");

    fclose(clnt_read);
    fclose(clnt_write);
    free(arg);
    pthread_mutex_destroy(&acdmtx2);
    pthread_exit(NULL);
}

// thread handler
void* handle_buzzer_on(void* arg)
{
    syslog(LOG_DEBUG, "[DEBUG] buzzer on");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(cleanup_buzzer, NULL);

    atomic_store(&g_buzzer_running, true);

    double notes[] = {
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0,
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
};

    int i;
    int total = sizeof(notes) / sizeof(double);
    softToneCreate(BUZZER_GPIO);  // 부저용 GPIO 설정
    for (i = 0; i < total; ++i) {
        softToneWrite(BUZZER_GPIO, (int)notes[i]);  // float → int 변환
        //printf("%d ", (int)notes[i]);
        syslog(LOG_INFO, "%d ", (int)notes[i]);
        delay(280);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

void* handle_countdown(void* arg)
{
    syslog(LOG_DEBUG, "[DEBUG] countdown");
    int sec = (int)(intptr_t)arg;;
    cmd_countdown(sec);
    return NULL;
}

void cleanup_buzzer(void* arg)
{
    softToneWrite(BUZZER_GPIO, 0);
    atomic_store(&g_buzzer_running, false);
}