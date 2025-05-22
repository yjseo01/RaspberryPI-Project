#include "client.h"


void send_http_get(const char* path) {
    int sock;
    struct sockaddr_in serv_addr;
    char request[1024];
    char response[4096];
    int n;

    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    // 3. 서버에 연결
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        close(sock);
        exit(1);
    }

    // 4. GET 요청 문자열 작성
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, SERVER_IP);

    // 5. 요청 전송
    write(sock, request, strlen(request));

    // 6. 응답 수신 및 출력
    while ((n = read(sock, response, sizeof(response) - 1)) > 0) {
        response[n] = '\0';
        printf("%s", response);
    }

    // 7. 소켓 종료
    close(sock);
}

int main()
{
    char path[MAX_BUF];
    int running = 1;

    printf("-----------------HTTP CLIENT----------------\n");
    while (running)
    {
        int choice = 0;
        int value;

        printf("\nmenu:\n");
        printf("  1. LED ON \n");
        printf("  2. LED OFF\n");
        printf("  3. LED Auto Control\n");
        printf("  4. LED brightness\n");
        printf("  4. Music Play\n");
        printf("  5. Music Stop\n");
        printf("  6. Countdown\n");
        printf("  7. Play Pitch\n");
        printf("  8. Exit\n");
        printf("input: ");
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                send_http_get("/ledon");
                break;
            case 2:
                send_http_get("/ledoff");
                break;
            case 3:
                send_http_get("/autoled");
                break;
            case 4:
                send_http_get("/buzzon");
                break;
            case 5:
                send_http_get("/buzzoff");
                break;
            case 6:
                printf("Choose second (1 ~ 9): ");
                scanf("%d", &value);
                snprintf(path, sizeof(path), "/countdown?sec=%d", value);
                send_http_get(path);
                break;
            case 7:
                printf("Choose frequency (e.g. 262 for 도): ");
                scanf("%d", &value);
                snprintf(path, sizeof(path), "/play?freq=%d", value);
                send_http_get(path);
                break;
            case 8:
                running = 0;
                printf("Exiting client...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}