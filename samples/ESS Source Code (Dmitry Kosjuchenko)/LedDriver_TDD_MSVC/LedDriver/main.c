#include "Windows.h"
#include "assert.h"
#include "led_driver.h"

#define LED_GREEN (12)

int main(int argc, char *argv[])
{
	LED_t led;
	uint32_t port = 0x11111111;
	
	led_init(&led, &port, LED_GREEN);
	led_on(&led);
	led_off(&led);

	assert(port == 0x11110111 && "invalid PORT value for led GREEN (BIT 12)");
}