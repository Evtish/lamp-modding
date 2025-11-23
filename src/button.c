#include "button.h"

void button_update(button* button_p) {
    bool btn_is_pressed = !(*(button_p->pin_r_p) & (1 << button_p->pin));

    if (btn_is_pressed) {
        if (!button_p->was_pressed)
            button_p->passed_debounce_amount = 1;
        else if (button_p->passed_debounce_amount >= 1)
            button_p->passed_debounce_amount++;
    }
    else
        button_p->passed_debounce_amount = 0;

    button_p->was_pressed = btn_is_pressed;
}

void button_poll(button* button_p) {
    uint32_t time_now = get_time_ms();
    if (time_now - button_p->last_call_time >= BTN_DEBOUNCE_CHECK_PERIOD_MS) {
        button_p->last_call_time = time_now;
        button_update(button_p);
    }
}

bool button_is_clicked(button* button_p) {
    if (button_p->passed_debounce_amount >= BTN_DEBOUNCE_AMOUNT_TO_PASS) {
        button_p->passed_debounce_amount = 0;
        return true;
    }
    return false;
}
