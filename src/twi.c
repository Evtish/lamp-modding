#include "twi.h"

#define TWSR_MASK 0xF8

#define SLAVE_ADDRESS 0b1101000 // DS3231
// #define SLA_W (SLAVE_ADDRESS << 1)
#define SLA_R ((SLAVE_ADDRESS << 1) | 1)

#define CODE_START              0x08
#define CODE_SLA_R_ACK          0x40
#define CODE_RECEIVE_DATA_ACK   0x50
#define CODE_RECEIVE_DATA_NACK  0x58

// #define ERROR_MESSAGE "TWI: failed to receive data"

volatile bool twi_ready = false;

void twi_init(void) {
    TWBR = 12; // SCL freq = 400 kHz

    TWCR |= (
        (1 << TWEN) |   // enable TWI
        (1 << TWEA) |   // enable acknowledge bit (for receiver mode)
        (1 << TWIE)     // enable interrupts
    );
}

// writing a one to TWINT clears the flag (the TWI will not start any operation as long as the TWINT bit in TWCR is set)
void twi_start(void) { TWCR |= (1 << TWINT) | (1 << TWSTA); }

void twi_stop(void) { TWCR |= (1 << TWINT) | (1 << TWSTO); }

uint8_t twi_status_code(void) { return (TWSR & TWSR_MASK); }

// returns if data was successfully received
bool twi_receive(char *buf, const uint8_t amount_of_bytes) {
    static uint8_t step = 0, i = 0;

    switch (step) {
    case 0:
        twi_start();
        step++;
    break;

    case 1:
        if (twi_ready) {
            if (twi_status_code() == CODE_START) {
                TWDR = SLA_R;
                step++;
            }
            else {
                step = 0;
                buf[0] = '0' + step;
            }
            TWCR |= (1 << TWINT);
            twi_ready = false;
        }
    break;
    
    case 2:
        if (twi_ready) {
            if (twi_status_code() == CODE_SLA_R_ACK) {
                step++;
            }
            else {
                step = 0;
                buf[0] = '0' + step;
            }
            TWCR |= (1 << TWINT);
            twi_ready = false;
        }
    break;

    case 3:
        if (twi_ready) {
            if (twi_status_code() == CODE_RECEIVE_DATA_ACK) {
                buf[i] = '0' + TWDR;
                i++;
                if (i >= amount_of_bytes - 1) TWCR &= ~(1 << TWEA);
            }
            else if (twi_status_code() == CODE_RECEIVE_DATA_NACK) {
                buf[i] = '0' + TWDR;
                i = 0, step = 0;
                twi_stop();
            }
            else {
                i = 0, step = 0;
                buf[0] = '0' + step;
            }
            TWCR |= (1 << TWINT);
            twi_ready = false;
        }
    break;
    }

    return (step == 0);
}
