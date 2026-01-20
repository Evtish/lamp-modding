#pragma once

#include <stdint.h>

extern volatile bool alarm_ready;

uint8_t rtc_init(void);
void rtc_format_datetime(char *formatted_dt, const uint8_t *bcd_dt, const uint8_t bcd_dt_size);
