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

// Lab 2

#include "hw_helper.h"
#include "pwm_driver.h"

void lab2_task1_a()
{
	LED_t led;
	uint32_t on_time = 10;
	uint32_t off_time = 10 - on_time;	
	
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{
		led_on(&led);
		delay_msec(on_time);
		
		led_off(&led);
		delay_msec(off_time);
	}
}

typedef struct DIMControlPrivate
{
	uint32_t on_time;
	uint32_t off_time;
}
DIMControlPrivate;
typedef struct DIMControlPrivate DIM_t;

void dim_init(DIM_t *dim, uint8_t pct)
{
	enum { DIM_DELAY = 50 };
	
	uint32_t new_time = DIM_DELAY - pct * DIM_DELAY / 100; 
	
	dim->on_time  = new_time;
	dim->off_time = DIM_DELAY - dim->on_time;
}

void dim_step_on(DIM_t *dim)
{
	delay_msec(dim->on_time);
}

void dim_step_off(DIM_t *dim)
{
	delay_msec(dim->off_time);
}

uint32_t dim_total_time(DIM_t *dim)
{
	return dim->off_time + dim->on_time;
}

void lab2_task1_b()
{
	LED_t led;
	DIM_t dim;
	uint32_t time_passed = 0;
	uint8_t dim_pct = 0;
	uint8_t dim_dir = 1;
	
	led_init(&led, PORTD, LED_GREEN);
	dim_init(&dim, dim_pct);
	
	while (1)
	{
		led_on(&led);
		dim_step_on(&dim);
		
		led_off(&led);
		dim_step_off(&dim);
		
		time_passed += dim_total_time(&dim);		
		if (time_passed >= 50)
		{
			if (dim_dir > 0)
			{				
				if (++dim_pct >= 100)
					dim_dir = 0;
			}
			else
			{
				if (--dim_pct == 0)
					dim_dir = 1;
			}
				
			dim_init(&dim, dim_pct);		
			time_passed = 0;
		}
	}
}

void lab2_task1_b2()
{
	LED_t led;
	uint32_t time_passed = 0;
	uint8_t dim_pct = 0;
	uint8_t dim_dir = 1;
	uint8_t dim_cnt = 0;
	
	led_init(&led, PORTD, LED_GREEN);
	
	while (1)
	{
		if (dim_pct > dim_cnt)
			led_on(&led);
		else		
			led_off(&led);
		
		if (dim_cnt++ > 100)
			dim_cnt = 0;
			
		if ((time_passed += 100) >= TIME_MSEC_TO_USEC(40))
		{
			if (dim_dir > 0)
			{				
				if (++dim_pct >= 100)
					dim_dir = 0;
			}
			else
			{
				if (--dim_pct == 0)
					dim_dir = 1;
			}
			
			time_passed = 0;
		}
		
		delay_usec(100);
	}
}

void lab2_task2_a()
{
	LED_t led_green;
	LED_t led_orange;
	LED_t led_blue;
	LED_t led_red;
	
	led_init(&led_green, PORTD, 12);
	led_init(&led_orange, PORTD, 13);
	led_init(&led_blue, PORTD, 15); // bug in the handouts, blue is 15, not 14
	led_init(&led_red, PORTD, 14); // bug in the handouts, red is 14, not 15
	
	pwm_driver_init(&led_green, &led_red, &led_orange, &led_blue);
	
	pwm_driver_set(0, 100);
	pwm_driver_set(1, 50);
	pwm_driver_set(2, 25);
	pwm_driver_set(3, 0);
	
	while (1)
	{
		delay_usec(100);
		pwm_driver_update();
	}
}

void lab2_task2_b()
{
	enum 
	{ 
		RESOLUTION_USEC = 100,
		LED_BIT_START = 12,
		LED_MAX = 4
	};
	
	LED_t led[LED_MAX];
	uint8_t color = 0;
	uint8_t bright = 0;
	uint8_t next = 0;
	uint8_t dir = 1;
	uint32_t time_fade = 0;
	uint32_t i;
	
	for (i = 0; i < LED_MAX; ++i)
		led_init(&led[i], PORTD, LED_BIT_START + i);
		
	pwm_driver_init(&led[0], &led[1], &led[2], &led[3]);
	
	for (i = 0; i < LED_MAX; ++i)
		pwm_driver_set(i, 0);
	
	while (1)
	{
		pwm_driver_update();
		delay_usec(RESOLUTION_USEC);
		
		if ((time_fade += RESOLUTION_USEC) >= TIME_MSEC_TO_USEC(40))
		{
			if (dir > 0)
			{				
				if (++bright >= 100)
					dir = 0;
			}
			else
			{
				if (--bright == 0)
				{
					dir = 1;					
					next = 1;
				}
			}
			
			pwm_driver_set(color, bright);
			
			time_fade = 0;
		}
		
		if (next)
		{
			pwm_driver_set(color, 0);
			
			if (++color >= LED_MAX)							
				color = 0;
						
			next = 0;
			dir = 1;
			bright = 0;
			
			pwm_driver_set(color, bright);
		}
		
		/*if ((time_passed += RESOLUTION_USEC) >= TIME_MSEC_TO_USEC(1000))
		{
			pwm_driver_set(color, 0);
			
			if (++color >= LED_MAX)							
				color = 0;
			
			pwm_driver_set(color, 100);
			
			time_passed = 0;
		}*/
	}
}

void lab2_task2_c(DelayFunc timer, uint32_t time_chunk_usec)
{
	uint8_t color = 0;
	uint8_t bright = 0;
	uint8_t next = 0;
	uint8_t dir = 1;
	uint32_t time_fade = 0;
			
	pwm_driver2_init(LED_START + 0, LED_START + 1, LED_START + 2, LED_START + 3);
		
	while (1)
	{
		pwm_driver2_update();
		timer(time_chunk_usec);
		
		if ((time_fade += time_chunk_usec) >= TIME_MSEC_TO_USEC(40))
		{
			if (dir > 0)
			{				
				if (++bright >= 100)
					dir = 0;
			}
			else
			{
				if (--bright == 0)
				{
					dir = 1;					
					next = 1;
				}
			}
			
			pwm_driver2_set(color, bright);
			
			time_fade = 0;
		}
		
		if (next)
		{
			pwm_driver2_set(color, 0);
			
			if (++color >= LED_COUNT)							
				color = 0;
						
			next = 0;
			dir = 1;
			bright = 0;
			
			pwm_driver2_set(color, bright);
		}
	}
}

void lab2_task4()
{
	// 10 kHZ = 8400 period taking that TMR4_FREQUENCY == TMR4_FREQUENCY
	TMR4_Init(0, 8400/*TMR4_FREQUENCY / 10000*/);
	while (1)
	{
		TMR4_WaitForExpiry();
	}
}

void delay_hw_fixed_task4_b(uint32_t dummy)
{
	(void)dummy;	
	TMR4_WaitForExpiry();
}

void lab2_task4_b()
{
	TMR4_InitResolutionUsec(100);
	
	lab2_task2_c(&delay_hw_fixed_task4_b, 100);
}

static volatile bool_t g_Tmr4IsrRdy = FALSE; // primitive spin lock
static void Tm4IsrCallback(void)
{
	g_Tmr4IsrRdy = TRUE;
}

void delay_hw_fixed_task5(uint32_t dummy)
{
	(void)dummy;
		
	while (!g_Tmr4IsrRdy)
	{ }
	
	g_Tmr4IsrRdy = FALSE;
}

void lab2_task5()
{
	TMR_Init_ISR_ResolutionUsec(TMR_ISR__4, 100, &Tm4IsrCallback);
	
	lab2_task2_c(&delay_hw_fixed_task5, 100);
}

void execute_lab2()
{
	//lab2_task1_a();
	//lab2_task1_b();
	//lab2_task1_b2();
	//lab2_task2_a();
	//lab2_task2_b();
	//lab2_task2_c(&delay_usec);
	//lab2_task4(); //<<< does nothing
	//lab2_task4_b();
	lab2_task5();
}
