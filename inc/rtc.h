#pragma once

#include <stdint.h>

void rtc_format_datetime(char *formatted_dt, const uint8_t *bcd_dt, const uint8_t bcd_dt_size);
