#pragma once

#include "timings.h"
#include <stdint.h>
#include <stdbool.h>

#define BTN_SHORT_CLICK_TIME_MS 50UL  // YOU CAN EDIT THE VALUE (the minimal recommended value is 50)
#define BTN_DEBOUNCE_CHECK_PERIOD_MS 5
#define BTN_DEBOUNCE_AMOUNT_TO_PASS (BTN_SHORT_CLICK_TIME_MS / BTN_DEBOUNCE_CHECK_PERIOD_MS)

typedef struct {
    uint32_t last_call_time;
    uint16_t passed_debounce_amount;
    volatile uint8_t* port_r_p;  // PORTx (pointer to register)
    volatile uint8_t* pin_r_p;  // PINx (pointer to register)
    uint8_t pin;  // Pxn
    bool was_pressed;
} button;

void button_poll(button* button_p);
bool button_is_clicked(button* button_p);
