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

#ifndef __LED_DRIVER_H
#define __LED_DRIVER_H

#include <stdint.h>

#define PORTD ((volatile uint32_t *)0x40020C14)
	
#define LED_START  12
#define LED_COUNT  4
#define LED_GREEN  (LED_START + 0)
#define LED_ORANGE (LED_START + 1)
#define LED_RED    (LED_START + 2)
#define LED_BLUE   (LED_START + 3)

#ifdef __cplusplus
extern C {
#endif

struct LEDstruct
{
	volatile uint32_t *port;
	uint32_t pin;
};
typedef struct LEDstruct LED_t;

#define PORTD ((volatile uint32_t *)0x40020C14)

void led_init(LED_t *led, volatile uint32_t *port, uint32_t pin);
void led_on(LED_t *led);
void led_off(LED_t *led);

#ifdef __cplusplus
}
#endif

#endif
