#include "twi.h"

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
#define CODE_SLA_R_ACK          0x40
#define CODE_RECEIVE_DATA_ACK   0x50
#define CODE_RECEIVE_DATA_NACK  0x58

volatile bool twi_ready = true;

// void twi_reset_twcr() {
//     // TWCR |= (
//     //     (1 << TWINT) |  // clear TWINT flag
//     //     (1 << TWEN) |   // enable TWI
//     //     (1 << TWIE)     // enable interrupts
//     // );
//     // TWCR &= ~(1 << TWSTA);
//     TWCR |= (1 << TWINT);
// }

void twi_init(void) {
    TWBR = TWBR_VALUE;

    TWSR &= ~((1 << TWPS1) | (1 << TWPS0)); // prescaler = 1

    TWCR = (
        (1 << TWEN) |   // enable TWI
        (1 << TWEA) |   // enable acknowledge bit
        (1 << TWIE)     // enable interrupts
    );
}

// writing a one to TWINT clears the flag (the TWI will not start any operation as long as the TWINT bit in TWCR is set)
void twi_start(void) {
    // TWCR |= (1 << TWSTA);
    // twi_reset_twcr();
    TWCR |= (1 << TWSTA) | (1 << TWINT);
}

void twi_stop(void) {
    // TWCR |= (1 << TWSTO);
    // twi_reset_twcr();
    TWCR |= (1 << TWSTO) | (1 << TWINT);
}

// uint8_t twi_status_code(void) { return (TWSR & TWSR_STATUS_CODE_MASK); }

void twi_conversation_error(int16_t *exit_code, uint8_t *step, const uint8_t status_code) {
    twi_stop();
    *exit_code = status_code * 10 + *step;
    *step = 0;
}

/*returns the exit code:
-1: in progress
0:  successfully received
1:  error (status code + number of step)*/
int16_t twi_receive_bytes(char *buf, const uint8_t start_address, const uint8_t amount_of_bytes) {
    if (amount_of_bytes == 0) return 0;

    static uint8_t step = 0, i = 0;
    uint8_t status_code = TWSR_STATUS_CODE;
    int16_t exit_code = -1;

    switch (step) {
        case 0:
            twi_start();
            step++;
        break;

        case 1:
            if (status_code == CODE_START) {
                TWDR = SLA_W;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
            TWCR &= ~(1 << TWSTA);
        break;
        
        case 2:
            if (status_code == CODE_SLA_W_ACK) {
                TWDR = start_address;
                TWCR |= (1 << TWINT);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;

        case 3:
            if (status_code == CODE_TRANSMIT_DATA_ACK) {
                twi_start();
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;

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
                // if (i >= amount_of_bytes - 1)   twi_reset_twcr();
                // else                            twi_reset_twcr();
                TWCR |= (1 << TWINT);
                if (i >= amount_of_bytes - 1) TWCR &= ~(1 << TWEA);
                step++;
            }
            else twi_conversation_error(&exit_code, &step, status_code);
        break;

        case 6:
            switch (status_code) {
                case CODE_RECEIVE_DATA_ACK:
                    buf[i++] = TWDR;
                    // if (i >= amount_of_bytes - 1)   twi_reset_twcr();
                    // else                            twi_reset_twcr();
                    TWCR |= (1 << TWINT);
                    if (i >= amount_of_bytes - 1) TWCR &= ~(1 << TWEA);
                break;
                case CODE_RECEIVE_DATA_NACK:
                    buf[i] = TWDR;
                    twi_stop();
                    i = 0, step = 0;
                    exit_code = 0;
                break;
                default:
                    i = 0;
                    twi_conversation_error(&exit_code, &step, status_code);
                break;
            }
        break;
    }

    return exit_code;
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
