#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SEONOR_H__

#include <wiringPi.h>
#include <softTone.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LED_PIN 18
#define BUZZER_GPIO 25
#define LIGHT_SENSOR_PIN 4

// LED
void led_on();
void led_off();
void set_led_brightness(int level);

// Buzzor
void beep();
void musicPlay();
void musicStop();

// 7 segment
void segment_display(int num);
void segment_off();

#endif // __LIGHT_SENSOR_H__