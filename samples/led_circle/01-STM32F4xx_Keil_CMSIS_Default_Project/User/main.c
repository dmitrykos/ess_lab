MIT License

Copyright (c) 2019

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

/**
 *	STM32F4 Discovery Skeleton Project
 *
 *	@ide		Keil uVision 5
 *	@conf		PLL parameters are set in "Options for Target" -> "C/C++" -> "Defines"
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.5.0 or greater required
 */
/* Include core modules */
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
/* Include helper library */
#include "ess_helper.h"
#include "led_driver.h"
#include "pwm_driver.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_conf.h"
// -- For debug functionality
// -- Set core clock to 168MHz in debug options
// -- Enable Trace
// -- Enable Port 7-0
#include <stdio.h>
#include <time.h>
// led driver
#define PORTD ((volatile uint32_t*)0x40020C14)
int light_control = 0;
int itm_debug(int c){
  return(ITM_SendChar(c));
}

int fputc(int ch, FILE *f) {
	/* Do your stuff here */
	/* Send your custom byte */
	
	
	/* If everything is OK, you have to return character written */
	return itm_debug(ch);
	/* If character is not correct, you can return EOF (-1) to stop writing */
	//return -1;
}
void delay_msec(uint32_t delay)
{
	// internal loop counter
	uint32_t k;
	// dummy variable to prevent compiler optimization
	volatile uint32_t internal_fake;
	while (delay-- >0)
	{
		for (k = 0; k <21000;k++)
		{
		internal_fake--;
		}
	}
}




int main(void) {
	LED_t led_green;
	LED_t led_orange;
	LED_t led_blue;
	LED_t led_red;
	uint16_t shadow;
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t m;
	uint32_t counter;
	uint32_t roundtime;
	uint8_t who_am_I;
	/* Initialize system */
	SystemInit();
	/* Initialize peripherals on board */
	ess_helper_init();
	led_init(&led_green,PORTD,12);
	led_init(&led_orange,PORTD,13);
	led_init(&led_red,PORTD,14);
	led_init(&led_blue,PORTD,15);
	pwm_driver_init(&led_orange,&led_green,&led_blue,&led_red);
	i=100;
	j=200;
	k=300;
	m=400;
	// Main loop 
	while (1) {
		
		if(i==400) i=0;
		if(j==400) j=0;
		if(k==400) k=0;
		if(m==400) m=0;
		pwm_driver_set(0,i>=100?(i<200?200-i:0):i);
		pwm_driver_set(1,j>=100?(j<200?200-j:0):j);
		pwm_driver_set(2,k>=100?(k<200?200-k:0):k);
		pwm_driver_set(3,m>=100?(m<200?200-m:0):m);
		for(roundtime=0;roundtime<50;roundtime++){
			for(counter=0;counter<100;counter++)
			{
				pwm_driver_update();
			}
		}
		i++;
		j++;
		k++;
		m++;
	}
	
}
