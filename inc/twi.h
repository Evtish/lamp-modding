#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile bool twi_ready;

void twi_init(void);
int16_t twi_receive_bytes(char *buf, const uint8_t start_address, const uint8_t amount_of_bytes);
