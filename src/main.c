/* ------------------------------ HEADER FILES ------------------------------ */
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "usart.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "twi.h"

#include "modes.h"
#include "button.h"
#include "timings.h"
#include "utils.h"

/* ------------------------------ DEFINES ------------------------------ */
#define RTC_BUFFER_SIZE 6
#define TWI_ERROR_MESSAGE "вобла"

/* ------------------------------ INTERRUPTS ------------------------------ */
ISR(ADC_vect) {
    adc_complete = true;
}

ISR(TIMER1_OVF_vect) {
    timer_amount_of_overflows++;
}

ISR(USART_RX_vect) {
    usart_rx_complete = true;
}
ISR(USART_UDRE_vect) {
    usart_data_register_empty = true;
}

ISR(TWI_vect) {
    twi_ready = true;
}

int main(void) {
    /* ------------------------------ LOCAL VARIABLES ------------------------------ */
    pwm white_led_pwm = {
        .data_direction_r = &DDRB,
        .output_compare_r = &OCR1A,
        .pin = WHITE_LED_PIN,
        .change_smoothly = false,
        .last_call_time = 0,
        .start_change_delta = 0
    };
    pwm yellow_led_pwm = {
        .data_direction_r = &DDRB,
        .output_compare_r = &OCR1B,
        .pin = YELLOW_LED_PIN,
        .change_smoothly = false,
        .last_call_time = 0,
        .start_change_delta = 0
    };

    button left_button = {
        .port_r = &PORTD,
        .pin_address_r = &PIND,
        .pin = LEFT_BUTTON_PIN,
        .was_pressed = false,
        .last_call_time = 0,
        .passed_debounce_amount = 0
    };

    light_mode led_light_mode = WHITE_ON;
    uint16_t max_brightness_level = PWM_MAX, min_brightness_level = 0;

    char raw_datetime[RTC_BUFFER_SIZE], formatted_seconds[16];
    bool need_to_read_datetime = false, need_to_transmit_datetime = false;

    /* ------------------------------ INITIALIZATION ------------------------------ */
    gpio_output_init(white_led_pwm.data_direction_r, white_led_pwm.pin);
    gpio_output_init(yellow_led_pwm.data_direction_r, yellow_led_pwm.pin);
    gpio_input_init(left_button.port_r, left_button.pin);

    timer1_init();

    adc_init();
    adc_set_pin(PHOTORESISTOR_PIN);

    usart_init();

    twi_init();

    sei();  // allow interrupts;

    /* ------------------------------ PROGRAM LOOP ------------------------------ */
    while (true) {
        // receive data from RTC
        if (need_to_read_datetime && twi_ready) {
            int16_t twi_exit_code = twi_receive_bytes(raw_datetime, 0x00, 6);
            switch (twi_exit_code) {
                // in progress
                case -1: break;
                // success
                case 0:
                    formatted_seconds[0] = '0' + (raw_datetime[0] >> 4);
                    formatted_seconds[1] = '0' + (raw_datetime[0] & 0x0F);
                    formatted_seconds[2] = '\0';

                    need_to_read_datetime = false;
                    need_to_transmit_datetime = true;
                break;
                // error
                default:
                    sprintf(formatted_seconds, "%s%d", TWI_ERROR_MESSAGE, twi_exit_code);

                    need_to_read_datetime = false;
                    need_to_transmit_datetime = true;
                break;
            }
            twi_ready = false;
        }

        // transmit data with USART
        if (need_to_transmit_datetime && usart_transmit_string(formatted_seconds))
            need_to_transmit_datetime = false;

        // receive data with USART
        if (usart_rx_complete) {
            min_brightness_level = map(UDR0, 0, UINT8_MAX, 0, PWM_MAX);
            usart_rx_complete = false;
        }

        // update ADC value
        if (adc_complete) {
            max_brightness_level = map(ADCH, 0, ADC_MAX, 0, PWM_MAX);  // use 8 high ADC bits only due to the inaccuracy of 1-2 low bits
            adc_complete = false;
        }

        // update the button
        button_poll(&left_button);

        if (button_clicked(&left_button) /*&& !white_led_pwm.change_smoothly && !yellow_led_pwm.change_smoothly*/) {
            led_light_mode = (led_light_mode == WHITE_ON) ? YELLOW_ON : WHITE_ON;
            white_led_pwm.change_smoothly = true;
            yellow_led_pwm.change_smoothly = true;

            twi_ready = true;
            need_to_read_datetime = true;
        }

        // manage the LEDs
        switch (led_light_mode) {
            case WHITE_ON:
                pwm_set(&white_led_pwm, max_brightness_level);  // turn on
                pwm_set(&yellow_led_pwm, min_brightness_level);  // turn off
            break;
            case YELLOW_ON:
                pwm_set(&white_led_pwm, min_brightness_level);
                pwm_set(&yellow_led_pwm, max_brightness_level);
            break;
        }
    }
    
    return 0;
}
