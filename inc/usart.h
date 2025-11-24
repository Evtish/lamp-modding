#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

extern volatile uint8_t usart_rx_data;
extern volatile bool usart_rx_complete;
extern volatile bool usart_data_register_empty;

void usart_init(void);
// void usart_transmit(const uint8_t data);
// uint8_t usart_receive(void);
