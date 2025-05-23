#include <dlfcn.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <softTone.h>
#include <softPwm.h>

#include "librasp.h"

// 함수 포인터 extern 선언
extern void (*p_init_rasp)();
extern void (*p_led_on)();
extern void (*p_led_off)();
extern void (*p_set_led_brightness)(int);
extern void (*p_beep)();
extern void (*p_segment_display)(int);
extern void (*p_segment_off)();

void* handle_library();