#include "timings.h"

volatile uint32_t timer_overflow_amount_now = 0;

void timer1_init(void) {
    TCCR1A |= (
        (1 << COM1A1) |  // non-inverting PWM (OCR1A)
        (1 << COM1B1) |  // non-inverting PWM (OCR1B)
        (0 << WGM10) | (1 << WGM11)  // set 10-bit fast PWM mode
    );

    TCCR1B |= (
        (1 << WGM12) | (1 << WGM13) |  // set 10-bit fast PWM mode
        (1 << CS11)  // set prescaler = 8
    );

    ICR1 = TIMER_SIZE;

    TIMSK1 |= (1 << TOIE1);  // enable overflow interrupt
}

uint32_t ticks_to_ms(const uint32_t ticks) {
    return ticks * 1000UL / (F_CPU >> TIMER_PRESCALER_BITNESS);
}

uint32_t get_time_ms(void) {
    return ticks_to_ms(TCNT1 + timer_overflow_amount_now * TIMER_SIZE);
}
