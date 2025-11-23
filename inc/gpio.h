#pragma once

#include <stdint.h>

#define PHOTORESISTOR_PIN PORTC3  // ADC3

#define WHITE_LED_PIN PORTB1
#define YELLOW_LED_PIN PORTB2

#define RIGHT_BUTTON_PIN PORTD2
#define LEFT_BUTTON_PIN PORTD3

void gpio_output_init(volatile uint8_t* ddr_p, const uint8_t pin);
void gpio_input_init(volatile uint8_t* port_p, const uint8_t pin);
