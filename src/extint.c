#include <avr/io.h>

#include "extint.h"

void extint0_init(void) {
	EICRA |= ~((1 << ISC01) | (1 << ISC00)); // The low level of INT0 generates an interrupt request
	EIMSK |= (1 << INT0); // Enable INT0
}
