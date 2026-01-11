#include "twi.h"
// #include "timings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define SDA 4
#define SCL 5

#define TWSR_STATUS_CODE_MASK   0xF8
#define TWSR_STATUS_CODE        (TWSR & TWSR_STATUS_CODE_MASK)

#define SCL_FREQUENCY_HZ    400000UL
#define TWBR_VALUE          ((F_CPU / SCL_FREQUENCY_HZ - 16) / 2)

#define SLAVE_ADDRESS   0b1101000 // DS3231
#define SLA_W           (SLAVE_ADDRESS << 1)
#define SLA_R           ((SLAVE_ADDRESS << 1) | 1)

#define CODE_START              0x08
#define CODE_REPEATED_START     0x10
#define CODE_SLA_W_ACK          0x18
#define CODE_TRANSMIT_DATA_ACK  0x28
// #define CODE_TRANSMIT_DATA_NACK 0x30
#define CODE_SLA_R_ACK          0x40
#define CODE_RECEIVE_DATA_ACK   0x50
#define CODE_RECEIVE_DATA_NACK  0x58

volatile bool twi_ready = true;

ISR(TWI_vect) {
    twi_ready = true;
}

void twi_init(void) {
    TWCR &= ~(1 << TWEN); // disable TWI

    DDRC &= ~((1 << SDA) | (1 << SCL)); // configure SDA and SCL as input pins
    PORTC |= (1 << SDA) | (1 << SCL); // enable pull-up resistors
    _delay_us(5);

    if (!(PINC & (1 << SDA))) { // check I2C if is in unknown state
        for(uint8_t i = 0; i < 100 && !(PINC & (1 << SDA)); i++) {
            // SCL low
            DDRC |= (1 << SCL);
            PORTC &= ~(1 << SCL);
            _delay_us(5);
        
            // SCL high
            DDRC &= ~(1 << SCL);
            PORTC |= (1 << SCL);
            _delay_us(5);
            
            // if (PINC & (1 << SDA)) { // check I2C if is in known state
            //     break;
            // }
        }

        // if (ticks_now - prev_ticks >= F_CPU / SCL_FREQUENCY_HZ)
        // SCL high
        DDRC &= ~(1 << SCL);
        PORTC |= (1 << SCL);

        // START (SDA low)
        DDRC |= (1 << SDA);
        PORTC &= ~(1 << SDA);
        _delay_us(5);

        // // STOP (SDA high)
        // DDRC &= ~(1 << SDA);
        // PORTC |= (1 << SDA);
        // _delay_us(5);
    }

    TWBR = TWBR_VALUE;

    TWSR &= ~((1 << TWPS1) | (1 << TWPS0)); // prescaler = 1

    TWCR = (
        (1 << TWEN) |   // enable TWI
        (1 << TWEA) |   // enable acknowledge bit
        (1 << TWIE)     // enable interrupts
    );
}

// writing a one to TWINT clears the flag (the TWI will not start any operation as long as the TWINT bit in TWCR is set)
void twi_start(void) { TWCR |= (1 << TWSTA) | (1 << TWINT); }

// writing a one to TWINT clears the flag (the TWI will not start any operation as long as the TWINT bit in TWCR is set)
void twi_stop(void) { TWCR |= (1 << TWSTO) | (1 << TWINT); }

void twi_conversation_error(uint16_t *exit_code, uint8_t *step, const uint8_t status_code) {
    twi_stop();
    *exit_code = (*step << 8) | status_code;
    *step = 0;
}

/*returns the exit code:
0:      in progress
1:      successfully transmitted
other:  error*/
uint16_t twi_receive_bytes(uint8_t *buf, const uint8_t start_address, const uint8_t amount_of_bytes) {
    if (amount_of_bytes == 0) return 1;

    static uint8_t step = 0, i = 0;
    uint8_t status_code = TWSR_STATUS_CODE;
    uint16_t exit_code = 0;

    switch (step) {
        // transmit START
        case 0:
            twi_start();
            step++;
        break;

        // transmit SLA+W
        case 1:
            if (status_code == CODE_START) {
                TWDR = SLA_W;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
            TWCR &= ~(1 << TWSTA);
        break;
        
        // transmit WORD ADDRESS
        case 2:
            if (status_code == CODE_SLA_W_ACK) {
                TWDR = start_address;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;
        
        // transmit REPEATED START
        case 3:
            if (status_code == CODE_TRANSMIT_DATA_ACK) {
                twi_start();
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;

        // transmit SLA+R
        case 4:
            if (status_code == CODE_REPEATED_START) {
                TWDR = SLA_R;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
            TWCR &= ~(1 << TWSTA);
        break;
        case 5:
            if (status_code == CODE_SLA_R_ACK) {
                if (i >= amount_of_bytes - 1) TWCR &= ~(1 << TWEA);
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;
        
        // receive DATA
        case 6:
            switch (status_code) {
                case CODE_RECEIVE_DATA_ACK:
                    buf[i++] = TWDR;
                    if (i > amount_of_bytes) TWCR &= ~(1 << TWEA);
                    TWCR |= (1 << TWINT);
                break;
                case CODE_RECEIVE_DATA_NACK:
                    buf[i] = TWDR;
                    TWCR |= (1 << TWEA);
                    twi_stop();
                    i = 0, step = 0;
                    exit_code = 1;
                break;
                default:
                    i = 0;
                    TWCR |= (1 << TWEA);
                    twi_conversation_error(&exit_code, &step, status_code);
                break;
            }
        break;

        // // receive a byte of DATA
        // case 6:
        //     if (status_code == CODE_RECEIVE_DATA_ACK) {
        //         buf[i++] = TWDR;
        //         if (i >= amount_of_bytes - 1) {
        //             TWCR &= ~(1 << TWEA);
        //             TWCR |= (1 << TWINT);
        //             step++;
        //         }
        //     }
        //     else {
        //         i = 0;
        //         twi_conversation_error(&exit_code, &step, status_code);
        //     }
        // break;

        // // receive the last byte of DATA
        // case 7:
        //     if (status_code == CODE_RECEIVE_DATA_NACK) {
        //         buf[i] = TWDR;
        //         twi_stop();
        //         i = 0, step = 0;
        //         exit_code = 0;
        //     }
        //     else {
        //         i = 0;
        //         twi_conversation_error(&exit_code, &step, status_code);
        //     }
        // break;
    }

    return exit_code;
}

/*returns the exit code:
0:      in progress
1:      successfully transmitted
other:  error*/
uint16_t twi_transmit_bytes(const uint8_t *buf, const uint8_t start_address, const uint8_t amount_of_bytes) {
    if (amount_of_bytes == 0) return 1;

    static uint8_t step = 0, i = 0;
    uint8_t status_code = TWSR_STATUS_CODE;
    uint16_t exit_code = 0;

    switch (step) {
        // transmit START
        case 0:
            twi_start();
            step++;
        break;

        // transmit SLA+W
        case 1:
            if (status_code == CODE_START) {
                TWDR = SLA_W;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
            TWCR &= ~(1 << TWSTA);
        break;
        
        // transmit WORD ADDRESS
        case 2:
            if (status_code == CODE_SLA_W_ACK) {
                TWDR = start_address;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;
        
        // transmit DATA
        case 3:
            if (status_code == CODE_TRANSMIT_DATA_ACK) {
                if (i < amount_of_bytes) {
                    TWDR = buf[i];
                    i++;
                }
                else {
                    twi_stop();
                    i = 0, step = 0;
                    exit_code = 1;
                }
                TWCR |= (1 << TWINT);
            }
            else {
                i = 0;
                twi_conversation_error(&exit_code, &step, status_code);
            }
        break;

        // // transmit a byte of DATA
        // case 3:
        //     if (status_code == CODE_RECEIVE_DATA_ACK) {
        //         TWDR = buf[i++];
        //         if (i >= amount_of_bytes - 1) {
        //             TWCR &= ~(1 << TWEA);
        //             TWCR |= (1 << TWINT);
        //             step++;
        //         }
        //         else {
        //             i = 0;
        //             twi_conversation_error(&exit_code, &step, status_code);
        //         }
        //     }
        // break;

        // // transmit the last byte of DATA
        // case 4:
        //     if (status_code == CODE_RECEIVE_DATA_NACK) {
        //         TWDR = buf[i];
        //         twi_stop();
        //         i = 0, step = 0;
        //         exit_code = 0;
        //     }
        //     else {
        //         i = 0;
        //         twi_conversation_error(&exit_code, &step, status_code);
        //     }
        // break;
    }

    return exit_code;
}
