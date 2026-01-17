#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"

volatile bool adc_complete = false;

ISR(ADC_vect) {
	adc_complete = true;
}

void adc_init(void) {
	ADMUX |= (
		(1 << REFS0) |	// set VCC as voltage reference
		(1 << ADLAR)	// adjust ADC result to left
	);

	ADCSRA |= (
		(1 << ADEN) |	// enable ADC
		(1 << ADSC) |	// start ADC conversion
		(1 << ADATE) |	// enable auto trigger
		(1 << ADIE) |	// enable interrupts
		(1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2) // set prescaler == 1024
	);
}

void adc_set_pin(const uint8_t pin) {
	ADMUX |= pin;			// ADC on ADCn (PORTCn) pin
	DIDR0 |= (1 << pin);	// disable digital input buffer to reduce power consumption (PINCn will be always 0)
}
