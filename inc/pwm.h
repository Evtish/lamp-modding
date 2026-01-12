#pragma once

#include <stdbool.h>
#include <stdint.h>

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
} Pwm;

void pwm_set(Pwm* my_pwm, const uint16_t new_val);
