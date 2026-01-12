#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BTN_SHORT_CLICK_TIME_MS 50UL // YOU CAN EDIT THE VALUE (the minimal recommended value is 50)
#define BTN_DEBOUNCE_CHECK_PERIOD_MS 5
#define BTN_DEBOUNCE_AMOUNT_TO_PASS (BTN_SHORT_CLICK_TIME_MS / BTN_DEBOUNCE_CHECK_PERIOD_MS)

typedef struct {
    uint32_t last_call_time;
    uint16_t passed_debounce_amount;
    volatile uint8_t* port_r;           // PORTx
    volatile uint8_t* pin_address_r;    // PINx
    uint8_t pin;                        // Pxn
    bool was_pressed;
} Button;

bool button_is_pressed(Button *btn);
void button_poll(Button *btn);
bool button_clicked(Button *btn);
