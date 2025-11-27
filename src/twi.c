#include "twi.h"

#define TWSR_MASK 0xF8

#define SCL_FREQUENCY_HZ    400000UL
#define TWBR_VALUE          ((F_CPU / SCL_FREQUENCY_HZ - 16) / 2)

#define SLAVE_ADDRESS   0b1101000 // DS3231
#define SLA_W           (SLAVE_ADDRESS << 1)
#define SLA_R           ((SLAVE_ADDRESS << 1) | 1)

#define CODE_START              0x08
#define CODE_REPEATED_START     0x10
#define CODE_SLA_W_ACK          0x18
#define CODE_TRANSMIT_DATA_ACK  0x28
#define CODE_SLA_R_ACK          0x40
#define CODE_RECEIVE_DATA_ACK   0x50
#define CODE_RECEIVE_DATA_NACK  0x58

#define ERROR_MESSAGE "vobla"

volatile bool twi_ready = true;

void twi_init(void) {
    TWBR = TWBR_VALUE;

    TWCR |= (
        (1 << TWEN) |   // enable TWI
        (1 << TWEA) |   // enable acknowledge bit (for receiver mode)
        (1 << TWIE)     // enable interrupts
    );
}

// writing a one to TWINT clears the flag (the TWI will not start any operation as long as the TWINT bit in TWCR is set)
void twi_start(void) {
    TWCR |= (1 << TWSTA) | (1 << TWINT);
}

void twi_stop(void) {
    TWCR |= (1 << TWSTO) | (1 << TWINT);
}

uint8_t twi_status_code(void) { return (TWSR & TWSR_MASK); }

// returns if data was successfully received
bool twi_receive_string(char *buf, const uint8_t start_address, const uint8_t amount_of_bytes) {
    static uint8_t step = 0, i = 0;
    uint8_t code = 0;

    if (twi_ready) {
        switch (step) {
        case 0:
            twi_start();
            step++;
        break;

        case 1:
            code = twi_status_code();
            buf[step - 1] = '0' + code;
            // buf[0] = '0' + 1;
            if (code == CODE_START) {
                TWDR = SLA_W;
                TWCR |= (1 << TWINT);
                step++;
            }
            else {
                // buf[step - 1] = '0' + code;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                step = 0;
                twi_stop();
            }
            TWCR &= ~(1 << TWSTA);
        break;
        
        case 2:
            code = twi_status_code();
            buf[step - 1] = '0' + code;
            // buf[0]++;
            if (code == CODE_SLA_W_ACK) {
                step++;
                // if (amount_of_bytes > 1) TWCR |= (1 << TWEA); // enable acknowledge bit
                TWDR = start_address;
                TWCR |= (1 << TWINT);
            }
            else {
                // buf[step - 1] = '0' + code;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                step = 0;
                twi_stop();
            }
        break;

        case 3:
            code = twi_status_code();
            buf[step - 1] = '0' + code;
            // buf[0]++;
            if (code == CODE_TRANSMIT_DATA_ACK) {
                twi_start();
                step++;
                TWCR |= (1 << TWINT);
            }
            else {
                // buf[step - 1] = '0' + code;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                step = 0;
                // TWCR &= ~(1 << TWEA); // disable acknowledge bit
                twi_stop();
            }
        break;

        case 4:
            code = twi_status_code();
            buf[step - 1] = '0' + code;
            // buf[0]++;
            if (code == CODE_REPEATED_START) {
                TWDR = SLA_R;
                TWCR |= (1 << TWINT);
                step++;
            }
            else {
                // buf[step - 1] = '0' + code;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                step = 0;
                twi_stop();
            }
            TWCR &= ~(1 << TWSTA);
        break;

        case 5:
            code = twi_status_code();
            buf[step - 1] = '0' + code;
            // buf[0]++;
            if (code == CODE_SLA_R_ACK) {
                step++;
                // if (amount_of_bytes > 1) TWCR |= (1 << TWEA); // enable acknowledge bit
                TWCR |= (1 << TWINT);
            }
            else {
                // buf[step - 1] = '0' + code;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                step = 0;
                twi_stop();
            }
        break;

        case 6:
            code = twi_status_code();
            // buf[step - 1] = '0' + code;
            if (code == CODE_RECEIVE_DATA_ACK) {
                buf[i++] = '0' + TWDR;
                if (i >= amount_of_bytes - 1) TWCR &= ~(1 << TWEA);
                TWCR |= (1 << TWINT);
            }
            else if (code == CODE_RECEIVE_DATA_NACK) {
                buf[i] = '0' + TWDR;
                i = 0, step = 0;
                TWCR |= (1 << TWEA);
                twi_stop();
            }
            else {
                buf[step - 1] = '0' + code;
                // buf[0]++;
                // buf[0] = '0' + step;
                // strcpy(buf, ERROR_MESSAGE);
                i = 0, step = 0;
                // TWCR &= ~(1 << TWEA); // disable acknowledge bit
                twi_stop();
            }
        break;
        }
    }
    twi_ready = false;

    return (step == 0);
}

// uint8_t twi_receive_byte() {
//     static uint8_t step = 0;
//     uint8_t res = 0;

//     switch (step) {
//     case 0:
//         twi_start();
//         step++;
//     break;

//     case 1:
//         if (twi_ready) {
//             if (twi_status_code() == CODE_START) {
//                 TWDR = SLA_R;
//                 step++;
//             }
//             else {
//                 res = '0' + step;
//                 step = 0;
//                 twi_stop();
//             }
//             TWCR |= (1 << TWINT);
//             twi_ready = false;
//         }
//     break;
    
//     case 2:
//         if (twi_ready) {
//             if (twi_status_code() == CODE_SLA_R_ACK) {
//                 step++;
//             }
//             else {
//                 res = '0' + step;
//                 step = 0;
//                 twi_stop();
//             }
//             TWCR |= (1 << TWINT);
//             twi_ready = false;
//         }
//     break;

//     case 3:
//         if (twi_ready) {
//             res = '0' + twi_status_code();
//             if (twi_status_code() == CODE_RECEIVE_DATA_NACK) {
//                 // res = '0' + TWDR;
//                 step = 0;
//                 twi_stop();
//             }
//             else {
//                 // res = '0' + step;
//                 step = 0;
//                 twi_stop();
//             }
//             TWCR |= (1 << TWINT);
//             twi_ready = false;
//         }
//     break;
//     }

//     return ((step == 0) ? res : 0);
// }
