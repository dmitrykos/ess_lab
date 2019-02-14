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

#include "hw_helper.h"
#include "pwm_driver.h"

typedef struct pwm_state_base
{
	uint32_t counter;
	uint32_t ch0_compare;
	uint32_t ch1_compare;
	uint32_t ch2_compare;
	uint32_t ch3_compare;
}
pwm_state_base;

static void pwm_driver_base_init(pwm_state_base *base)
{
	base->counter = 0;

	base->ch0_compare = 0;
	base->ch1_compare = 0;
	base->ch2_compare = 0;
	base->ch3_compare = 0;
}

static void pwm_driver_base_set(pwm_state_base *base, uint8_t channel, uint8_t value)
{
	if (channel > 3)
		return;

	if (value > 100)
		value = 100;

	switch (channel)
	{
	case 0: base->ch0_compare = value; break;
	case 1: base->ch1_compare = value; break;
	case 2: base->ch2_compare = value; break;
	case 3: base->ch3_compare = value; break;
	default:
		break;
	}
}

struct pwm_state
{
	pwm_state_base base;
	
	LED_t *ch0;
	LED_t *ch1;
	LED_t *ch2;
	LED_t *ch3;
};
static struct pwm_state state;

void pwm_driver_init(LED_t *ch0, LED_t *ch1, LED_t *ch2, LED_t *ch3)
{
	pwm_driver_base_init(&state.base);	
	state.ch0 = ch0;
	state.ch1 = ch1;
	state.ch2 = ch2;
	state.ch3 = ch3;
	
	led_off(ch0);
	led_off(ch1);
	led_off(ch2);
	led_off(ch3);	
}

void pwm_driver_set(uint8_t channel, uint8_t value)
{
	pwm_driver_base_set(&state.base, channel, value);
}

void pwm_driver_update(void)
{
	if (state.base.ch0_compare > state.base.counter)
		led_on(state.ch0);
	else
		led_off(state.ch0);
	
	if (state.base.ch1_compare > state.base.counter)
		led_on(state.ch1);
	else
		led_off(state.ch1);
	
	if (state.base.ch2_compare > state.base.counter)
		led_on(state.ch2);
	else
		led_off(state.ch2);
	
	if (state.base.ch3_compare > state.base.counter)
		led_on(state.ch3);
	else
		led_off(state.ch3);	
	
	if (state.base.counter++ > PWM_MAX)
		state.base.counter = 0;
}

struct pwm_state2
{
	pwm_state_base base;
	
	LED_t ch0;
	LED_t ch1;
	LED_t ch2;
	LED_t ch3;
};
static struct pwm_state2 state2;

void pwm_driver2_init(uint8_t led_pin0, uint8_t led_pin1, uint8_t led_pin2, uint8_t led_pin3)
{
	pwm_driver_base_init(&state2.base);
	
	led_init(&state2.ch0, PORTD, led_pin0);
	led_init(&state2.ch1, PORTD, led_pin1);
	led_init(&state2.ch2, PORTD, led_pin2);
	led_init(&state2.ch3, PORTD, led_pin3);
}

void pwm_driver2_set(uint8_t channel, uint8_t value)
{
	pwm_driver_base_set(&state2.base, channel, value);
}

void pwm_driver2_update(void)
{
	if (state2.base.ch0_compare > state2.base.counter)
		led_on(&state2.ch0);
	else
		led_off(&state2.ch0);
	
	if (state2.base.ch1_compare > state2.base.counter)
		led_on(&state2.ch1);
	else
		led_off(&state2.ch1);
	
	if (state2.base.ch2_compare > state2.base.counter)
		led_on(&state2.ch2);
	else
		led_off(&state2.ch2);
	
	if (state2.base.ch3_compare > state2.base.counter)
		led_on(&state2.ch3);
	else
		led_off(&state2.ch3);	
	
	if (state2.base.counter++ > PWM_MAX)
		state2.base.counter = 0;
}

void pwm_driver2_fade_init(PWMFade_t *fade, int32_t time_speed_usec, uint8_t brightness_max)
{
	memset(fade, 0, sizeof(*fade));
	fade->progress_max = brightness_max;
	fade->time_speed   = time_speed_usec;
}

void pwm_driver2_fade_update(PWMFade_t *fade)
{
	if ((fade->time += PWM_RESOLUTION_USEC) < fade->time_speed)
		return;
	
	if (++fade->progress >= fade->progress_max)
	{
		pwm_driver2_set(fade->ch, 0);
		
		if (++fade->ch >= PWM_CH__MAX)
			fade->ch = 0;
		
		fade->progress = 0;
	}
		
	pwm_driver2_set(fade->ch, fade->progress_max - fade->progress);
	pwm_driver2_set((fade->ch + 1) % PWM_CH__MAX, fade->progress);
	
	fade->time = 0;
}
