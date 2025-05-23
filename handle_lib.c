#include "handle_lib.h"

// 함수 포인터 선언
void (*p_init_rasp)();
void (*p_led_on)();
void (*p_led_off)();
void (*p_set_led_brightness)(int);
void (*p_beep)();
void (*p_segment_display)(int);
void (*p_segment_off)();

void* handle_library()
{
    // 공유라이브러리 열기
    void* handle = dlopen("./librasp.so", RTLD_LAZY);
    if (handle == NULL)
    {
        syslog(LOG_ERR, "dlopen: %s", dlerror());
        exit(1);
    }

    // 함수 주소 가져오기
    p_init_rasp = dlsym(handle, "init_rasp");
    p_led_on = dlsym(handle, "led_on");
    p_led_off = dlsym(handle, "led_off");
    p_set_led_brightness = dlsym(handle, "set_led_brightness");
    p_beep = dlsym(handle, "beep");
    p_segment_display = dlsym(handle, "segment_display");
    p_segment_off = dlsym(handle, "segment_off");

    if (!p_init_rasp || !p_led_on || !p_led_off || !p_set_led_brightness || 
    !p_segment_display || !p_segment_off || !p_beep)
    {
        syslog(LOG_ERR, "Function pointers not loaded properly");
        exit(EXIT_FAILURE);
    }

    char* err = dlerror();
    if (err) 
    {
        syslog(LOG_ERR, "dlsym error: %s", err);
        exit(1);
    }

    return handle;
}