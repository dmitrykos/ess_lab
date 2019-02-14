#ifndef __LED_DRIVER_H
#define __LED_DRIVER_H

#include <stdint.h>

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
