/* Copyright (c) 2019 by Dmitry Kostjuchenko (dmitrykos@neutroncode.com)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef __PWM_DRIVER_H
#define __PWM_DRIVER_H

#include "led_driver.h"

#ifdef __cplusplus
extern C {
#endif

#define PWM_MAX             100
#define PWM_RESOLUTION_USEC 100
#define PWM_HW_TIMER_INIT   TMR4_InitResolutionUsec(PWM_RESOLUTION_USEC)
#define PWM_HW_TIMER_DELAY  TMR_HW_DelayUSec(PWM_RESOLUTION_USEC)

// Basic channel config
typedef enum EPwmChannel
{
	PWM_CH__GREEN = 0,
	PWM_CH__ORANGE,
	PWM_CH__RED,
	PWM_CH__BLUE,
	PWM_CH__MAX
}
EPwmChannel;

void pwm_driver_init(LED_t *ch0, LED_t *ch1, LED_t *ch2, LED_t *ch3);
void pwm_driver_set(uint8_t channel, uint8_t value);
void pwm_driver_update(void);

void pwm_driver2_init(uint8_t led_pin0, uint8_t led_pin1, uint8_t led_pin2, uint8_t led_pin3);
void pwm_driver2_set(uint8_t channel, uint8_t value);
void pwm_driver2_update(void);

typedef struct PWMFade_t
{
	uint8_t ch;
	uint8_t progress;
	uint8_t progress_max;
	uint8_t __pad;
	int32_t time;
	int32_t time_speed;
}
PWMFade_t;

void pwm_driver2_fade_init(PWMFade_t *fade, int32_t time_speed_usec, uint8_t brightness_max);
void pwm_driver2_fade_update(PWMFade_t *fade);

#ifdef __cplusplus
}
#endif

#endif // __PWM_DRIVER_H
