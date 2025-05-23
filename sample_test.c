#include <stdio.h>
#include <wiringPi.h>
#include <softTone.h>
#include <softPwm.h>
#include "librasp.h"

int main()
{
    // wiringPi 초기화
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }

    printf("LED 켜기 테스트\n");
    led_on();
    delay(1000);  // 1초 대기

    printf("LED 끄기 테스트\n");
    led_off();
    delay(1000);

    printf("LED 밝기 조절 테스트 (PWM)\n");
    for (int brightness = 0; brightness <= 255; brightness += 51) {
        set_led_brightness(brightness);
        printf("밝기: %d\n", brightness);
        delay(500);
    }
    set_led_brightness(0);

    printf("비프음 테스트\n");
    beap();

    printf("7-세그먼트 숫자 표시 테스트\n");
    for (int num = 0; num <= 9; num++) {
        printf("숫자: %d 표시\n", num);
        segment_display(num);
        delay(1000);
    }

    // 끝나면 LED 끄기
    led_off();

    return 0;
}
