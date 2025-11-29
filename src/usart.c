#include "usart.h"

#define BAUD_RATE 9600U
#define UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)

volatile uint8_t usart_rx_data = 0;
volatile bool usart_rx_complete = false;
volatile bool usart_data_register_empty = false;

void usart_init(void) {
    // set baud rate
    UBRR0H = (UBRR_VALUE >> 8);
    UBRR0L = UBRR_VALUE;
    
    UCSR0B |= (
        (1 << RXCIE0) | // enable RX complete interrupt
        (1 << TXEN0) |  // enable transmitter
        (1 << RXEN0)    // enable receiver
    );

    UCSR0C |= (
        (3 << UCSZ00) | // character size = 8 bit
        (0 << USBS0)    // use 1 stop bit
    );
}

// returns if the data was successfully transmitted
bool usart_transmit_byte(const uint8_t data) {
    UCSR0B |= (1 << UDRIE0); // enable data register empty interrupt
    if (usart_data_register_empty) {
        UDR0 = data;
        UCSR0B &= ~(1 << UDRIE0); // disable data register empty interrupt
        usart_data_register_empty = false;
        return true;
    }
    return false;
}

// returns if the data was successfully transmitted
bool usart_transmit_string(const char *data) {
    static size_t i = 0;
    size_t data_len = strlen(data);
    
    if (usart_transmit_byte(data[i]))
        i++;

    if (i >= data_len) {
        i = 0;
        return true;
    }
    return false;
}

// uint8_t usart_receive(void) {
//     usart_rx_data = false;
//     return UDR0;
// }
