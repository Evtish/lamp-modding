#pragma once

#include <stdint.h>

#define array_len(array) sizeof((array)) / sizeof((array)[0])

int16_t limit(const int16_t val, const int16_t low, const int16_t high);
float limitf(const float val, const float low, const float high);
int16_t map(const int16_t var, const int16_t low1, const int16_t high1, const int16_t low2, const int16_t high2);
float mapf(const float var, const float low1, const float high1, const float low2, const float high2);
