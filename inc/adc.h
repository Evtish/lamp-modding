#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_MAX 255  // use ADCH only

extern volatile bool adc_complete;

void adc_init(void);
void adc_set_pin(const uint8_t pin);
