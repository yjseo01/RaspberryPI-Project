#include "thd_func.h"

pthread_t g_buzzer_tid;
atomic_bool g_buzzer_running = false;
RequestQueue cmd_queue;

void* worker_thd_func(void* arg)
{
    printf("[WORKER THREAD]\n");
    // adc 실행
    int fd;
    int a2dChannel = 0;
    int a2dVal;
    int threshold = 180;
    int cnt = 0;

    printf("[ADC/DAC(YL-40) Module start........]\n");
    if ((fd = wiringPiI2CSetupInterface("/dev/i2c-1", 0x48)) < 0) {
        printf("wiringPiI2CSetupInterface failed\n");
        pthread_exit(NULL);
    }

    while (1)
    {
        wiringPiI2CWrite(fd, 0x00 | a2dChannel);  // AIN0 선택
        a2dVal = wiringPiI2CRead(fd);             // 실제값

        if (is_empty(&cmd_queue))
            continue;

        Request* req = dequeue(&cmd_queue);
        pthread_t tid;

        //
        switch (req->cmd)
        {
        case CMD_LED_ON:
            cmd_led_on();
            break;
        case CMD_LED_OFF:
            cmd_led_off();
            break;
        case CMD_LED_BRIGHTNESS:
            cmd_led_set_brightness(req->arg);
            break;
        case CMD_BUZZER_ON:
            // cmd_buzzer_on();
            if (!atomic_load(&g_buzzer_running)) 
            {
                pthread_create(&g_buzzer_tid, NULL, handle_buzzer_on, NULL);
                pthread_detach(g_buzzer_tid);
            }           
            break;
        case CMD_BUZZER_OFF:
            // buzzer thread에 signal 보내기
            printf("[CMD] buzzer off\n");
            if (atomic_load(&g_buzzer_running)) 
            {
                pthread_cancel(g_buzzer_tid);
            }
            break;
        case CMD_AUTO_LED:
            printf("[CMD] led auto set\n");
            if (a2dVal > threshold) 
            {
                printf("[ADC]: bright(a2dVal = %d)\n", a2dVal);
                cmd_led_off();
            } 
            else 
            {
                printf("[ADC]: dark(a2dVal = %d)\n", a2dVal);
                cmd_led_on();
            }

            break;
        case CMD_COUNTDOWN:
            // cmd_countdown(req->arg);
            pthread_create(&tid, NULL, handle_countdown, (void*)(intptr_t)req->arg);
            pthread_join(tid, NULL);
            break;
        case CMD_PLAY_PITCH:
            printf("[CMD] play pitch\n");
            softToneCreate(BUZZER_GPIO);
            softToneWrite(BUZZER_GPIO, req->arg);
            delay(500);
            softToneWrite(BUZZER_GPIO, 0);
        default:
            break;
        }
        
        free(req);
    }
}

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
    int brightness = 0;
    int sec = 0;
    int note = 0;

    if (strcmp(path, "/ledon") == 0) {
        cmd = CMD_LED_ON;
    } else if (strcmp(path, "/ledoff") == 0) {
        cmd = CMD_LED_OFF;
    } else if (strncmp(path, "/ledbrightness?value=", strlen("/ledbrightness?value=")) == 0) {
        cmd = CMD_LED_BRIGHTNESS;
        brightness = atoi(path + strlen("/ledbrightness?value=")); 
    } else if (strcmp(path, "/buzzon") == 0) {
        cmd = CMD_BUZZER_ON;
    } else if (strcmp(path, "/buzzoff") == 0) {
        cmd = CMD_BUZZER_OFF;
    } else if (strcmp(path, "/autoled") == 0) {
        cmd = CMD_AUTO_LED;
    } else if (strncmp(path, "/countdown?sec=", strlen("/countdown?sec=")) == 0) {
        cmd = CMD_COUNTDOWN;
        sec = atoi(path + strlen("/countdown?sec=")); 
    } else if (strncmp(path, "/play?freq=", strlen("/play?freq=")) == 0) {
        cmd = CMD_PLAY_PITCH;
        note = atoi(path + strlen("/play?freq=")); 
    }

    // 큐에 명령 넣기
    if (cmd != CMD_UNKNOWN) {
        int arg = 0;
        Request* req = (Request*)malloc(sizeof(Request));
        if (brightness != 0 && sec == 0 && note == 0)
            arg = brightness;
        else if (sec != 0 && brightness == 0 && note == 0)
            arg = sec;
        else if (note != 0 && brightness == 0 && sec == 0)
            arg = note;

        enqueue(&cmd_queue, cmd, arg);
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

// thread handler
void* handle_buzzer_on(void* arg)
{
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
        printf("%d ", (int)notes[i]);
        delay(280);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

void* handle_countdown(void* arg)
{
    int sec = (int)(intptr_t)arg;
    printf("[DEBUG] handle_countdown: %d\n", sec);
    cmd_countdown(sec);
    return NULL;
}

void cleanup_buzzer(void* arg)
{
    softToneWrite(BUZZER_GPIO, 0);
    atomic_store(&g_buzzer_running, false);
}