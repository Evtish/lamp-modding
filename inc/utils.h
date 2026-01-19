#pragma once

#include <stdint.h>

#define array_len(array) sizeof((array)) / sizeof((array)[0])

int16_t limit(const int16_t val, const int16_t low, const int16_t high);
int16_t map(const int16_t var, const int16_t low1, const int16_t high1, const int16_t low2, const int16_t high2);
