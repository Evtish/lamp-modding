/* ------------------------------ HEADER FILES ------------------------------ */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "twi.h"
#include "modes.h"
#include "button.h"
#include "rtc.h"
#include "timings.h"
#include "utils.h"

/* ------------------------------ DEFINES ------------------------------ */
#define BCD_DATETIME_SIZE       7
#define FORMATTED_DATETIME_SIZE (3 * BCD_DATETIME_SIZE)
#define TWI_ERROR_MESSAGE       "twierr"

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

    light_mode led_light_mode = WHITE_ON;
    uint16_t max_brightness_level = PWM_MAX, min_brightness_level = 0;

    uint8_t bcd_datetime[BCD_DATETIME_SIZE];
    char formatted_datetime[FORMATTED_DATETIME_SIZE];
    bool need_to_write_datetime = false, need_to_read_datetime = false, need_to_transmit_datetime = false;

    /* ------------------------------ INITIALIZATION ------------------------------ */
    gpio_output_init(white_led_pwm.data_direction_r, white_led_pwm.pin);
    gpio_output_init(yellow_led_pwm.data_direction_r, yellow_led_pwm.pin);
    gpio_input_init(button.port_r, button.pin);

    timer1_init();

    adc_init();
    adc_set_pin(PHOTORESISTOR_PIN);

    usart_init();

    twi_init();

    sei();  // allow interrupts;

    // while (twi_transmit_bytes((const uint8_t[]) {0b00000111}, 0x0F, 1) == 0);

    // if (button_is_pressed(&left_button))
        // need_to_write_datetime = true;

    /* ------------------------------ PROGRAM LOOP ------------------------------ */
    while (true) {
        // // set RTC datetime
        // if (need_to_write_datetime && twi_ready) {
        //     const uint8_t arb_datetime[] = {48, 89, 33, 2, 9, 146, 37};
        //     const int16_t twi_exit_code = twi_transmit_bytes(arb_datetime, 0x00, 7);
        //     switch (twi_exit_code) {
        //         // in progress
        //         case -1: break;
        //         // success
        //         case 0:
        //             need_to_write_datetime = false;
        //         break;
        //         // error
        //         default:
        //             sprintf(formatted_datetime, "%s%d", TWI_ERROR_MESSAGE, twi_exit_code);

        //             need_to_write_datetime = false;
        //             // need_to_transmit_datetime = true;
        //         break;
        //     }
        //     twi_ready = false;
        // }

        // get RTC datetime
        if (need_to_read_datetime && twi_ready) {
            const uint16_t twi_exit_code = twi_receive_bytes(bcd_datetime, 0x00, BCD_DATETIME_SIZE);
            switch (twi_exit_code) {
                // in progress
                case 0: break;
                // success
                case 1:
                    rtc_format_datetime(formatted_datetime, bcd_datetime, BCD_DATETIME_SIZE);

                    need_to_read_datetime = false;
                    need_to_transmit_datetime = true;
                break;
                // error
                default:
                    // TODO: replace sprintf with other function (e.g. memcpy) to reduce program size
                    sprintf(formatted_datetime, "%s%d", TWI_ERROR_MESSAGE, twi_exit_code);

                    need_to_read_datetime = false;
                    need_to_transmit_datetime = true;
                break;
            }
            twi_ready = false;
        }

        // transmit data with USART
        if (need_to_transmit_datetime && usart_transmit_string(formatted_datetime))
            need_to_transmit_datetime = false;

        // receive data with USART
        if (usart_rx_complete) {
            min_brightness_level = map(UDR0, 0, UINT8_MAX, 0, PWM_MAX);
            usart_rx_complete = false;
        }

        // update ADC value
        if (adc_complete) {
            // use 8 high ADC bits only due to the inaccuracy of 1-2 low bits
            max_brightness_level = map(ADCH, 0, ADC_MAX, 0, PWM_MAX);
            adc_complete = false;
        }

        // update the button
        button_poll(&button);

        if (button_clicked(&button) /*&& !white_led_pwm.change_smoothly && !yellow_led_pwm.change_smoothly*/) {
            led_light_mode = (led_light_mode == WHITE_ON) ? YELLOW_ON : WHITE_ON;
            white_led_pwm.change_smoothly = true;
            yellow_led_pwm.change_smoothly = true;

            // twi_ready = true;
            need_to_read_datetime = true;
            // need_to_write_datetime = true;
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
