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

#include "fancy_led.h"

void fancy_led_init(FancyLED_t *led)
{
	led_init(&led->led, PORTD, LED_GREEN);
	led->toggle = 0;
	led->color = 0;
}

void fancy_led_on(FancyLED_t *fled)
{
	led_on(&fled->led);
}

uint8_t fancy_led_step(FancyLED_t *fled)
{
	if (fled->toggle ^= 1)
		led_on(&fled->led);
	else
		led_off(&fled->led);
	
	return fled->toggle;
}

void fancy_led_next(FancyLED_t *fled)
{		
	led_off(&fled->led);
	
	if (++fled->color >= LED_COUNT)
		fled->color = 0;
	
	led_init(&fled->led, PORTD, LED_GREEN + fled->color);
}
