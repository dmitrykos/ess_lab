#include "stdio.h"
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
