#include <stdlib.h>
#include <math.h>

#include "utils.h"
#include "timings.h"
#include "pwm.h"
#include "adc.h"

#define GAMMA 0.45f

uint16_t pwm_gamma_correct(const uint8_t brightness_level) {
	return mapf(powf(mapf(brightness_level, 0, ADC_MAX, 0, 1), GAMMA), 0, 1, 0, PWM_MAX);
}

static void pwm_assign_and_reset(Pwm* my_pwm, const uint16_t valid_new_val) {
	// assign
	*(my_pwm->output_compare_r) = valid_new_val;

	// reset
	my_pwm->start_change_delta = 0;
	my_pwm->change_smoothly = false;
}

static void pwm_update(Pwm* my_pwm, const uint16_t valid_new_val, const uint16_t cur_change_delta) {
	if (cur_change_delta > PWM_STEP)
		*(my_pwm->output_compare_r) += PWM_STEP * (*(my_pwm->output_compare_r) < valid_new_val ? 1 : -1);
	else // this is the last update
		pwm_assign_and_reset(my_pwm, valid_new_val);
}

void pwm_set(Pwm* my_pwm, const uint16_t new_val) {
	const uint16_t valid_new_val = limit(new_val, 0, PWM_MAX);
	const uint16_t cur_change_delta = labs((int32_t) (*(my_pwm->output_compare_r)) - (int32_t) valid_new_val);
	uint32_t polling_period_ms = 0, time_now = 0;

	// set start_change_delta if it hasn't yet
	if (my_pwm->start_change_delta == 0)
		my_pwm->start_change_delta = cur_change_delta;

	if (!(my_pwm->change_smoothly) || my_pwm->start_change_delta < PWM_MIN_CHANGE_DELTA) {
		pwm_assign_and_reset(my_pwm, valid_new_val);
		return;
	}

	polling_period_ms = PWM_CHANGE_TIME_MS * PWM_STEP / my_pwm->start_change_delta;
	time_now = get_time_ms();

	if (time_now - my_pwm->last_call_time >= polling_period_ms) {
		my_pwm->last_call_time = time_now;
		pwm_update(my_pwm, valid_new_val, cur_change_delta);
		if (my_pwm->start_change_delta == valid_new_val) return;
	}
}
