#pragma once

#include <avr/io.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern volatile uint8_t usart_rx_data;
extern volatile bool usart_rx_complete;
extern volatile bool usart_data_register_empty;

void usart_init(void);
bool usart_transmit_byte(const uint8_t data);
bool usart_transmit_string(const char *data);
// uint8_t usart_receive(void);
