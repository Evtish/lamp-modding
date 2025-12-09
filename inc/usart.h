#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile bool usart_rx_complete;
extern volatile bool usart_data_register_empty;

void usart_init(void);
bool usart_transmit_byte(const uint8_t data);
bool usart_transmit_string(const char *data);
