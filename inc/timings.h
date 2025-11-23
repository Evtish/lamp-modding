#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define TIMER_SIZE 1023  // PWM_MAX
#define TIMER_PRESCALER_BITNESS 3

extern volatile uint32_t timer_overflow_amount_now;

void timer1_init(void);
uint32_t get_time_ms(void);
