/* ------------------------------ HEADER FILES ------------------------------ */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

// #include "usart.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "extint.h"
#include "twi.h"
#include "button.h"
#include "rtc.h"
#include "timings.h"
#include "utils.h"

/* ------------------------------ DEFINES ------------------------------ */
// #define BCD_DATETIME_SIZE		2
// #define FORMATTED_DATETIME_SIZE (3 * BCD_DATETIME_SIZE)
// #define TWI_ERROR_MESSAGE_PREFIX	   "twierr"

#define WHITE_ON	true
#define YELLOW_ON	false


int main(void) {
	/* ------------------------------ LOCAL VARIABLES ------------------------------ */
	Pwm white_led_pwm = {
		.data_direction_r = &DDRB,
		.output_compare_r = &OCR1A,
		.pin = WHITE_LED_PIN,
		.change_smoothly = false,
		.last_call_time = 0,
		.start_change_delta = 0
	};
	Pwm yellow_led_pwm = {
		.data_direction_r = &DDRB,
		.output_compare_r = &OCR1B,
		.pin = YELLOW_LED_PIN,
		.change_smoothly = false,
		.last_call_time = 0,
		.start_change_delta = 0
	};

	Button button = {
		.port_r = &PORTD,
		.pin_address_r = &PIND,
		.pin = LEFT_BUTTON_PIN,
		.was_pressed = false,
		.last_call_time = 0,
		.passed_debounce_amount = 0
	};

	bool led_light_mode = WHITE_ON;
	uint16_t max_brightness_level = PWM_MAX, min_brightness_level = 0;

//	uint8_t bcd_datetime[BCD_DATETIME_SIZE];
//	char formatted_datetime[FORMATTED_DATETIME_SIZE];
//	bool need_to_read_datetime = false, need_to_transmit_datetime = false;

	/* ------------------------------ INITIALIZATION ------------------------------ */
	gpio_output_init(white_led_pwm.data_direction_r, white_led_pwm.pin);
	gpio_output_init(yellow_led_pwm.data_direction_r, yellow_led_pwm.pin);
	gpio_input_init(button.port_r, button.pin);

	timer1_init();

	adc_init();
	adc_set_pin(PHOTORESISTOR_PIN);

	//usart_init();

	twi_init();
	
	extint0_init();

	sei(); // allow interrupts;

	// init RTC with default values
	if (button_is_pressed(&button))
		rtc_init();

	/* ------------------------------ PROGRAM LOOP ------------------------------ */
	while (true) {
		// get RTC datetime
//		if (need_to_read_datetime) {
//			const int16_t twi_exit_code = twi_receive_bytes(bcd_datetime, 0x0E, BCD_DATETIME_SIZE);
//
//			// success
//			if (twi_exit_code == TWI_SUCCESS) {
//				rtc_format_datetime(formatted_datetime, bcd_datetime, BCD_DATETIME_SIZE);
//
//				need_to_read_datetime = false;
//				need_to_transmit_datetime = true;
//			}
//
//			// error
//			else {
//				// TODO: replace sprintf with other function (e.g. memcpy) to reduce program size
//				sprintf(formatted_datetime, "%s%d", TWI_ERROR_MESSAGE_PREFIX, twi_exit_code);
//
//				need_to_read_datetime = false;
//				need_to_transmit_datetime = true;
//			}
//		}
//
//		// transmit data with USART
//		if (need_to_transmit_datetime && usart_transmit_string(formatted_datetime))
//			need_to_transmit_datetime = false;
//
//		// receive data with USART
//		if (usart_rx_complete) {
//			min_brightness_level = map(UDR0, 0, UINT8_MAX, 0, PWM_MAX);
//			usart_rx_complete = false;
//		}
		
		// toggle LEDs when RTC alarm is ready
		if (alarm_ready) {
			uint8_t rtc_status[1];
			if (twi_receive_bytes(rtc_status, 0x0F, 1) == TWI_SUCCESS) {
				bool morning_alarm_flag = rtc_status[0] & (1 << 0);
				bool evening_alarm_flag = rtc_status[0] & (1 << 1);
				
				white_led_pwm.change_smoothly = true;
				yellow_led_pwm.change_smoothly = true;
				if (morning_alarm_flag)
					led_light_mode = WHITE_ON;
				else if (evening_alarm_flag)
					led_light_mode = YELLOW_ON;
				
				rtc_status[0] ^= (evening_alarm_flag << 1 | morning_alarm_flag); // inverse alarm flags
				if (twi_transmit_bytes(rtc_status, 0x0F, 1) == TWI_SUCCESS)
					alarm_ready = false;
			}
		}

		// update ADC value
		if (adc_complete) {
			// use 8 high ADC bits only due to the inaccuracy of 1-2 low bits
			max_brightness_level = pwm_gamma_correct(ADCH);
			adc_complete = false;
		}

		// update the button
		button_poll(&button);

		if (button_clicked(&button) /*&& !white_led_pwm.change_smoothly && !yellow_led_pwm.change_smoothly*/) {
			led_light_mode = !led_light_mode;
			white_led_pwm.change_smoothly = true;
			yellow_led_pwm.change_smoothly = true;

			// twi_ready = true;
			// need_to_read_datetime = true;
			// need_to_write_datetime = true;
		}

		// manage the LEDs
		switch (led_light_mode) {
			case WHITE_ON:
				pwm_set(&white_led_pwm, max_brightness_level);	// turn on
				pwm_set(&yellow_led_pwm, min_brightness_level); // turn off
				break;
			case YELLOW_ON:
				pwm_set(&white_led_pwm, min_brightness_level);
				pwm_set(&yellow_led_pwm, max_brightness_level);
				break;
		}
	}
	
	return 0;
}
