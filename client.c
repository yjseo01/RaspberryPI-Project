#include "client.h"

void ignore_all_signals_except_int()
{
    for (int sig = 1; sig < 32; sig++)
    {
        if (sig != SIGINT)
        { // SIGINT 제외한 모든 시그널 무시
            signal(sig, SIG_IGN);
        }
    }
}

void send_http_get(const char *path)
{
    int sock;
    struct sockaddr_in serv_addr;
    char request[1024];
    char response[4096];
    ssize_t n;

    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock);
        exit(1);
    }

    // 3. 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
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
    if (write(sock, request, strlen(request)) == -1)
    {
        perror("write");
        close(sock);
        exit(1);
    }

    // 6. 응답 수신 및 출력
    while ((n = read(sock, response, sizeof(response) - 1)) > 0)
    {
        response[n] = '\0';
        printf("%s", response);
    }
    if (n == -1)
    {
        perror("read");
    }

    // 7. 소켓 종료
    close(sock);
}

int note_to_freq(const char *note)
{
    if (strcmp(note, "도") == 0 || strcmp(note, "d") == 0)
        return 262; // C4
    else if (strcmp(note, "레") == 0 || strcmp(note, "r") == 0)
        return 294; // D4
    else if (strcmp(note, "미") == 0 || strcmp(note, "m") == 0)
        return 330; // E4
    else if (strcmp(note, "파") == 0 || strcmp(note, "p") == 0)
        return 349; // F4
    else if (strcmp(note, "솔") == 0 || strcmp(note, "s") == 0)
        return 392; // G4
    else if (strcmp(note, "라") == 0 || strcmp(note, "l") == 0)
        return 440; // A4
    else if (strcmp(note, "시") == 0 || strcmp(note, "sh") == 0)
        return 494; // B4
    else if (strcmp(note, "도2") == 0 || strcmp(note, "D") == 0)
        return 523; // C5
    else
        return -1; // invalid note
}

void clear_stdin_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

int main()
{
    char path[MAX_BUF];
    int running = 1;

    ignore_all_signals_except_int();

    printf("-----------------HTTP CLIENT----------------\n");
    while (running)
    {
        int choice = 0;
        int value;
        char input[16]; // 음 입력용 버퍼
        int freq;

        printf("\nmenu:\n");
        printf("  1. LED ON \n");
        printf("  2. LED OFF\n");
        printf("  3. LED Auto Control\n");
        printf("  4. LED brightness\n");
        printf("  5. Music Play\n");
        printf("  6. Music Stop\n");
        printf("  7. Countdown\n");
        printf("  8. Play Pitch\n");
        printf("  9. Exit\n");
        printf("input: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            clear_stdin_buffer();
            continue;
        }
        clear_stdin_buffer();

        switch (choice)
        {
        case 1:
            send_http_get("/ledon"); // LED ON
            break;
        case 2:
            send_http_get("/ledoff"); // LED OFF
            break;
        case 3:
            send_http_get("/autoled"); // LED Auto Control
            break;
        case 4:
            printf("Choose brightness (0~255): ");
            if (scanf("%d", &value) != 1 || value < 0 || value > 255)
            {
                printf("Invalid brightness.\n");
                clear_stdin_buffer();
                break;
            }
            clear_stdin_buffer();
            snprintf(path, sizeof(path), "/brightness?val=%d", value);
            send_http_get(path);
            break;
        case 5:
            send_http_get("/buzzon"); // Music Play
            break;
        case 6:
            send_http_get("/buzzoff"); // Music Stop
            break;
        case 7:
            printf("Choose second (1 ~ 9): ");
            if (scanf("%d", &value) != 1 || value < 1 || value > 9)
            {
                printf("Invalid value.\n");
                clear_stdin_buffer();
                break;
            }
            clear_stdin_buffer();
            snprintf(path, sizeof(path), "/countdown?sec=%d", value);
            send_http_get(path);
            break;
        case 8:
            printf("음 이름 연속 입력 (drmpslshD): ");
            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = 0; // 개행 제거

                int i = 0;
                while (input[i] != '\0')
                {
                    char note[3] = {
                        0,
                    }; // 음 이름 저장용 (최대 2글자 + NULL)

                    // 두 글자인 음 ("도2", "sh") 확인
                    if ((input[i + 1] == '2') || // 한글 두 글자 음
                        (input[i] == 's' && input[i + 1] == 'h'))
                    { // 영문 두 글자 음
                        note[0] = input[i];
                        note[1] = input[i + 1];
                        i += 2;
                    }
                    else
                    {
                        // 한 글자 음
                        note[0] = input[i];
                        i++;
                    }

                    freq = note_to_freq(note);
                    if (freq == -1)
                    {
                        printf("잘못된 음 입력: %s\n", note);
                    }
                    else
                    {
                        printf("재생 음: %s (%d Hz)\n", note, freq);
                        snprintf(path, sizeof(path), "/play?freq=%d", freq);
                        send_http_get(path);
                    }
                }
            }
            else
            {
                printf("입력 오류\n");
            }
            break;
        case 9:
            running = 0;
            printf("Exiting client...\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    }

    return 0;
}
