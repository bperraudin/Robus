#include "hal.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define USART_BAUDRATE 9600ul
#define UBRR_VALUE (MAINCLOCK/(USART_BAUDRATE<<4))-1

ISR(USART_RX_vect)
{
    ctx.data_cb(&UDR0);
}

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
    // Set baud rate
    UBRR0= UBRR_VALUE;
    // interrupt on receive

// 9600-8-E-1
    // That is, baudrate of 9600bps
    // 8 databits
    // Even parity
    // 1 stopbit
    UCSR0B = (1 << RXEN0) | (1 << RXCIE0); // And enable interrupts
    UCSR0C = (1 << UPM01) | (1 << UCSZ01) | (1 << UCSZ00);

    sei();
}

/**
 * \fn unsigned char hal_transmit(unsigned char* data)
 * \brief write a data byte
 *
 * \param data *data bytes to send
 * \param size size of data to send in byte
 *
 * \return error
 */
unsigned char hal_transmit(unsigned char* data, unsigned short size) {
    for (unsigned short i = 0; i<size; i++) {
        UDR0 = data[i];
        while(!(UCSR0A&(1<<UDRE0))){};
    }
    return 0;
}
