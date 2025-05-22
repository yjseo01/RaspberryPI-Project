#include "ctrl_func.h"

int led_state = 0;
pthread_mutex_t mtx;

void cmd_led_on()
{
    pthread_mutex_lock(&mtx);
    if (led_state == LED_OFF)
    {
        led_on();
        printf("[CMD] LED ON\n");
        led_state = LED_ON;
    }
    pthread_mutex_unlock(&mtx);
}

void cmd_led_off()
{
    pthread_mutex_lock(&mtx);
    if (led_state == LED_ON)
    {
        led_off();
        printf("[CMD] LED OFF\n");
        led_state = LED_OFF;
    }
    pthread_mutex_unlock(&mtx);
}

void cmd_led_set_brightness(int brightness)
{
    pthread_mutex_lock(&mtx);
    printf("[CMD] brightness: %d\n", brightness);
    set_led_brightness(brightness);
    pthread_mutex_unlock(&mtx);
}

void cmd_buzzer_on()
{
    pthread_mutex_lock(&mtx);
    printf("[CMD] play music\n");
    musicPlay();
    pthread_mutex_unlock(&mtx);
}

void cmd_buzzer_off()
{
    // buzzer스레드에게 시그널 보내면 될거같음~~
}

void cmd_countdown(int sec)
{
    printf("[CMD] sec: %d\n", sec);

    if (sec < 0 || sec > 9)
        return;

    pthread_mutex_lock(&mtx);
    for (int i = sec; i > 0; i--)
    {
        segment_display(i);
        printf("[CMD] 7 segment: %d\n", i);
        sleep(1);
    }
    segment_off();
    beep();
    pthread_mutex_unlock(&mtx);
}