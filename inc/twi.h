#pragma once

#include <avr/io.h>

#include <stdbool.h>
#include <stdint.h>

extern volatile bool twi_ready;

void twi_init(void);
// void twi_start(void);
// void twi_stop(void);
// uint8_t twi_status_code(void);
// uint8_t twi_receive_byte();
bool twi_receive_string(char *buf, const uint8_t amount_of_bytes);
