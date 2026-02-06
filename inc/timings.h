#pragma once

#include <stdint.h>

#define TIMER_SIZE 1023 // PWM_MAX
#define TIMER_PRESCALER_BITNESS 3

void timer1_init(void);
uint32_t get_ticks(void);
uint32_t get_time_ms(void);
