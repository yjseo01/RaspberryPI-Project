#include "ctrl_func.h"

int led_state = LED_OFF;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void cmd_led_on()
{
    pthread_mutex_lock(&mtx);
    if (led_state == LED_OFF)
    {
        p_led_on();
        //printf("[CMD] LED ON\n");
        syslog(LOG_INFO, "[CMD] LED ON");
        led_state = LED_ON;
    }
    pthread_mutex_unlock(&mtx);
}

void cmd_led_off()
{
    pthread_mutex_lock(&mtx);
    if (led_state == LED_ON)
    {
        p_led_off();
        //printf("[CMD] LED OFF\n");
        syslog(LOG_INFO, "[CMD] LED OFF");
        led_state = LED_OFF;
    }
    pthread_mutex_unlock(&mtx);
}

void cmd_led_set_brightness(int brightness)
{
    pthread_mutex_lock(&mtx);
    //printf("[CMD] brightness: %d\n", brightness);
    syslog(LOG_INFO, "[CMD] brightness: %d", brightness);
    p_set_led_brightness(brightness);
    pthread_mutex_unlock(&mtx);
}

void cmd_countdown(int sec)
{
    //printf("[CMD] sec: %d\n", sec);
    syslog(LOG_INFO, "[CMD] sec: %d", sec);

    if (sec < 0 || sec > 9)
        return;

    pthread_mutex_lock(&mtx);
    for (int i = sec; i > 0; i--)
    {
        p_segment_display(i);
        //printf("[CMD] 7 segment: %d\n", i);
        syslog(LOG_INFO, "[CMD] 7 segment: %d", i);
        sleep(1);
    }

    p_segment_off();
    p_beep();
    pthread_mutex_unlock(&mtx);
}

int cmd_get_brightness(int fd)
{
    return wiringPiI2CRead(fd);
}