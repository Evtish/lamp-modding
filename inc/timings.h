#pragma once

#include <stdint.h>

#define TIMER_SIZE 1023 // PWM_MAX
#define TIMER_PRESCALER_BITNESS 3

extern volatile uint32_t timer_amount_of_overflows;

void timer1_init(void);
uint32_t get_ticks(void);
uint32_t get_time_ms(void);
