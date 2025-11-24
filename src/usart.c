#include "usart.h"

#define BAUD_RATE 9600U
#define MY_UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)

volatile uint8_t usart_rx_data = 0;
volatile bool usart_rx_complete = false;
volatile bool usart_data_register_empty = false;

void usart_init(void) {
    // set baud rate
    UBRR0H = (MY_UBRR_VALUE >> 8);
    UBRR0L = MY_UBRR_VALUE;
    
    UCSR0B |= (
        (1 << RXCIE0) | // enable RX complete interrupt
        (1 << UDRIE0) | // enable data register empty interrupt
        (1 << TXEN0) |  // enable transmitter
        (1 << RXEN0)    // enable receiver
    );

    UCSR0C |= (
        (3 << UCSZ00) | // character size = 8 bit
        (0 << USBS0)    // use 1 stop bit
    );
}

// void usart_transmit(const uint8_t data) {
//     usart_data_register_empty = false;
//     UDR0 = data;
// }

// uint8_t usart_receive(void) {
//     usart_rx_data = false;
//     return UDR0;
// }
