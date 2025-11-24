#pragma once

#include "utils.h"
#include "timings.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define PWM_MAX 1023

#define PWM_CHANGE_TIME_MS 1000UL   // YOU CAN EDIT THE VALUE (the more the time the longer the animation)
#define PWM_STEP 1                  // YOU CAN EDIT THE VALUE (from 1 to PWM_MAX, the less the step the smoother the animation)
#define PWM_MIN_CHANGE_DELTA 7

typedef struct {
    uint32_t last_call_time;
    uint16_t start_change_delta;
    volatile uint8_t* data_direction_r;     // DDRx
    volatile uint16_t* output_compare_r;    // OCRnx
    uint8_t pin;                            // Pxn
    bool change_smoothly;
} pwm;

void pwm_set(pwm* my_pwm, const uint16_t new_val);
