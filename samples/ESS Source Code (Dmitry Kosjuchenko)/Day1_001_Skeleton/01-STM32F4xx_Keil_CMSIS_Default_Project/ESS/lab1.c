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

// Lab 1

#include "stm32f4xx.h"
#include "hw_helper.h"
#include "fancy_led.h"

//#define _TASK_3_BD
//#define _TASK_3_E
//#define _TASK_4

// Task 3 (b-d) (Volatile Bug)
#ifdef _TASK_3_BD

void led_green_init(void)
{
	*(uint32_t *)0x40020C14 = 0;
}

void led_green_on(void) //<<< optimized out by compiler inside the while (1) loop
{
	*(uint32_t *)0x40020C14 = 0x1000; // fix: *(volatile uint32_t *)0x40020C14 = ...
}

void led_green_off(void) //<<< optimized out by compiler inside the while (1) loop
{	
	*(uint32_t *)0x40020C14 = 0x0000; // fix: *(volatile uint32_t *)0x40020C14 = ...
}

#endif

// Task 3 (e)
#ifdef _TASK_3_E

static uint32_t led_green_pin;
#define GET_HW_PIN_MASK(P)  (1 << (P))
#define UPDATE_HW_PIN(P_HW) ((*(uint32_t *)0x40020C14) = (P_HW))

void led_green_init(uint32_t pin)
{
	led_green_pin  = 0;
	led_green_pin |= GET_HW_PIN_MASK(pin);
	UPDATE_HW_PIN(led_green_pin);
}

void led_green_on(void)
{
	led_green_pin |= GET_HW_PIN_MASK(LED_GREEN);
	UPDATE_HW_PIN(led_green_pin);
}

void led_green_off(void)
{	
	led_green_pin &= ~GET_HW_PIN_MASK(LED_GREEN);	
	UPDATE_HW_PIN(led_green_pin);
}

#endif // _TASK_3_E

#if defined(_TASK_3_BD) || defined(_TASK_3_E)

void lab1_task_c()
{
#ifdef _TASK_3_E
	led_green_init(LED_GREEN);
#else
	led_green_init();
#endif
	led_green_on();
	
	while (1)
	{ }
}

void lab1_task_d()
{
#ifdef _TASK_3_E
	led_green_init(LED_GREEN);
#else
	led_green_init();
#endif
	
	while (1)
	{
		led_green_on();
		led_green_off();
	}
}

#endif // #if defined(_TASK_3_BD) || defined(_TASK_3_E)

#ifdef _TASK_4

struct _LEDstruct
{
	volatile uint32_t *port;
	uint32_t pin;
};
typedef struct _LEDstruct _LED_t;

#define _PORTD ((volatile uint32_t *)0x40020C14)

void _led_init(_LED_t *led, volatile uint32_t *port, uint32_t pin)
{
	led->port = port;
	led->pin  = (1 << pin);
	
	(*led->port) = 0;
}

void _led_on(_LED_t *led)
{
	(*led->port) |= led->pin;
}

void _led_off(_LED_t *led)
{
	(*led->port) &= ~led->pin;
}

void lab1_task4_()
{
	_LED_t led;
	
	_led_init(&led, _PORTD, LED_GREEN);
	_led_on(&led);
	
	while (1)
	{ }
}

#endif // _TASK_4

void lab1_task4_delay()
{
	LED_t led;
	uint8_t toggle = 1;
	
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{ 
		//led_on(&led);
		//delay_msec(1000);
		//led_off(&led);
		//delay_msec(1000);
		
		if (toggle ^= 1)
			led_on(&led);
		else
			led_off(&led);			
		
		delay_msec(1000);
	}
}

void lab1_task4_delay_hw()
{
	LED_t led;
	uint8_t toggle = 1;
	
	TMR4_InitResolutionUsec(100);
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{
		if (toggle ^= 1)
			led_on(&led);
		else
			led_off(&led);			
		
		delay_hw_msec(1000);
	}
}

void _lab1_task4_delay_fancy()
{
	LED_t led;
	uint8_t toggle = 1;
	uint8_t color = 0;
	uint8_t next = 0;
	
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{ 		
		if (toggle ^= 1)
			led_on(&led);
		else
			led_off(&led);		
		
		delay_msec(50);
			
		if (++next >= 2)
		{
			led_off(&led);
			
			if (++color >= LED_COUNT)
				color = 0;
			
			next = 0;
			led_init(&led, PORTD, LED_GREEN + color);
		}
	}
}

void lab1_task4_delay_fancy()
{	
	FancyLED_t led;	
	fancy_led_init(&led);

	while (1)
	{
		if (!fancy_led_step(&led))
			fancy_led_next(&led);
		
		delay_msec(50);
	}
}

void lab1_task6_()
{	
	LED_t led;
	uint8_t button;
	uint8_t state = 0;
	uint8_t toggle = 0;
	
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{
		button = GPIOA->IDR & GPIO__BUTTON_0;
		
		// fix hw state jitter which may result in unexpected touch up
		delay_msec(100);
		
		if (button == state)
			continue;
		
		state = button;
		
		if ((button & GPIO__BUTTON_0) != GPIO__BUTTON_0)
			continue;
						
		if (toggle ^= 1)
			led_on(&led);
		else
			led_off(&led);
	}
}

void lab1_task6_fancy()
{
	FancyLED_t led;
	uint8_t button;
	uint8_t state = 0;
	uint8_t toggle = 0;
	
	fancy_led_init(&led);
	
	while (1)
	{
		button = GPIOA->IDR & GPIO__BUTTON_0;
		
		// fix hw state jitter which may result in unexpected touch up
		delay_msec(100);
		
		if (button == state)
			continue;
		
		state = button;
		
		if ((button & GPIO__BUTTON_0) != GPIO__BUTTON_0)
			continue;
				
		if (toggle ^= 1)
			fancy_led_on(&led);
		else
			fancy_led_next(&led);
	}
}

void execute_lab1()
{
	//lab1_task_c();
	//lab1_task_d();
	//lab1_task4_();
	//lab1_task4_delay();
	//lab1_task4_delay_hw();
	//_lab1_task4_delay_fancy();
	//lab1_task4_delay_fancy();
	//lab1_task6_();
	lab1_task6_fancy();
}
