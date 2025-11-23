#include "pwm.h"

void pwm_assign_and_reset(pwm* pwm_p, const uint16_t valid_new_val) {
    // assign
    *(pwm_p->out_r_p) = valid_new_val;

    // reset
    pwm_p->start_change_delta = 0;
    pwm_p->changing_smoothly = false;
}

void pwm_update(pwm* pwm_p, const uint16_t valid_new_val, const uint16_t cur_change_delta) {
    if (cur_change_delta > PWM_STEP)
        *(pwm_p->out_r_p) += PWM_STEP * (*(pwm_p->out_r_p) < valid_new_val ? 1 : -1);
    else  // this is the last update
        pwm_assign_and_reset(pwm_p, valid_new_val);
}

void pwm_set(pwm* pwm_p, const uint16_t new_val) {
    const uint16_t valid_new_val = limit(new_val, 0, PWM_MAX);
    const uint16_t cur_change_delta = abs(*(pwm_p->out_r_p) - valid_new_val);
    uint32_t polling_period_ms = 0, time_now = 0;

    // set start_change_delta if it hasn't yet
    if (pwm_p->start_change_delta == 0)
        pwm_p->start_change_delta = cur_change_delta;

    if (!(pwm_p->changing_smoothly) || pwm_p->start_change_delta < PWM_MIN_CHANGE_DELTA) {
        pwm_assign_and_reset(pwm_p, valid_new_val);
        return;
    }

    polling_period_ms = PWM_CHANGE_TIME_MS * PWM_STEP / pwm_p->start_change_delta;
    time_now = get_time_ms();

    if (time_now - pwm_p->last_call_time >= polling_period_ms) {
        pwm_p->last_call_time = time_now;
        pwm_update(pwm_p, valid_new_val, cur_change_delta);
        if (pwm_p->start_change_delta == valid_new_val) return;
    }
}
