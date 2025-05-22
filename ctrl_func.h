#ifndef __CTRL_FUNC_H__
#define __CTRL_FUNC_H__

#define LED_ON 1
#define LED_OFF 0

#include "librasp.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

void cmd_led_on();
void cmd_led_off();
void cmd_led_set_brightness(int);
int cmd_get_brightness(int);
void cmd_buzzer_on();
void cmd_buzzer_off();
void cmd_countdown(int);

extern int led_state;
extern pthread_mutex_t mtx;

#endif // __CTRL_FUNC_H__