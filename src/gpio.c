#include "gpio.h"

// set pin as output
void gpio_output_init(volatile uint8_t* ddr_p, const uint8_t pin) {
	*ddr_p |= (1 << pin);
}

// turn on pull-up resistor
void gpio_input_init(volatile uint8_t* port_p, const uint8_t pin) {
	*port_p |= (1 << pin);
}
