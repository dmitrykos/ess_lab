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

#include <stdio.h>
#include "led_driver.h"

#ifdef _WIN32
	#define _TEST_PC
#endif

void led_init(LED_t *led, volatile uint32_t *port, uint32_t pin)
{
	led->port = port;
	led->pin  = (1 << pin);
	
	(*led->port) &= ~led->pin;

#ifdef _TEST_PC
	printf("led = INI | port[%08X] pin[%08X]\n", *led->port, led->pin);
#endif
}

void led_on(LED_t *led)
{
	(*led->port) |= led->pin;

#ifdef _TEST_PC
	printf("led = ON | port[%08X] pin[%08X]\n", *led->port, led->pin);
#endif
}

void led_off(LED_t *led)
{
	(*led->port) &= ~led->pin;

#ifdef _TEST_PC
	printf("led = OFF | port[%08X] pin[%08X]\n", *led->port, led->pin);
#endif
}
