#include "librasp.h"

static int fd = -1;
static int channel = 0;

double notes[] = {
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0,
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
};

int gpiopins[4] = {21, 20, 16, 12};

int number[10][4] = {
    {0, 0, 0, 0}, // 0: A B C D
    {1, 0, 0, 0}, // 1
    {0, 1, 0, 0}, // 2
    {1, 1, 0, 0}, // 3
    {0, 0, 1, 0}, // 4
    {1, 0, 1, 0}, // 5
    {0, 1, 1, 0}, // 6
    {1, 1, 1, 0}, // 7
    {0, 0, 0, 1}, // 8
    {1, 0, 0, 1}  // 9
};

void init_rasp()
{
    syslog(LOG_DEBUG, "init_rasp");
    if (wiringPiSetupGpio() == -1)
    {
        //perror("wiringPiSetupGpio0");
        syslog(LOG_ERR, "wiringPiSetupGpio0");
        exit(1);
    }

    pinMode(LED_PIN, OUTPUT);
    if (softPwmCreate(LED_PIN, 0, 255) != 0) 
    {
        //perror("softPwmCreate fail");
        syslog(LOG_ERR, "softPwmCreate");
        exit(1);
    }
    softToneCreate(BUZZER_GPIO);
    // 핀 모드 설정
    for (int i = 0; i < 4; i++) {
        pinMode(gpiopins[i], OUTPUT);
    }
}

// led
void led_on()
{
    softPwmCreate(LED_PIN, 0, 255);
    digitalWrite(LED_PIN, HIGH);
    softPwmWrite(LED_PIN, 255);
}

void led_off()
{
    softPwmCreate(LED_PIN, 0, 255);
    digitalWrite(LED_PIN, LOW);
    softPwmWrite(LED_PIN, 0);
}

void set_led_brightness(int brightness)
{
    softPwmCreate(LED_PIN, 0, 255);
    syslog(LOG_DEBUG, "set_led_brightness %d", brightness);
    if (brightness < 0)
        brightness = 0;
    if (brightness > 255)
        brightness = 255;

    softPwmWrite(LED_PIN, brightness);
}

// 
/*
void musicPlay() {
    int i;
    int total = sizeof(notes) / sizeof(double);
    
    for (i = 0; i < total; ++i) {
        softToneWrite(BUZZER_GPIO, (int)notes[i]);  // float → int 변환
        //printf("%d ", (int)notes[i]);

        delay(280);
    }
}
*/

void beep()
{
    softToneWrite(BUZZER_GPIO, 1000);
    delay(500);
    softToneWrite(BUZZER_GPIO, 0);
}

/*
void musicStop()
{
    softToneWrite(BUZZER_GPIO, 0);
}
*/

//
void segment_display(int num)
{
    int i;
    if (num < 0 || num > 9) {
        //printf("Invalid number for 7-segment display: %d\n", num);
        return;
    }

    // 숫자 출력
    for (i = 0; i < 4; i++) {
        digitalWrite(gpiopins[i], number[num][i] ? HIGH : LOW);
    }
}

void segment_off()
{
    int i;
    // 세그먼트 끄기 (모든 핀 HIGH로 설정)
    for (i = 0; i < 4; i++) {
        digitalWrite(gpiopins[i], HIGH);
    }
}
