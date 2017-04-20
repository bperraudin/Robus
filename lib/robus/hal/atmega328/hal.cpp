#include "hal.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define USART_BAUDRATE 9600ul
#define UBRR_VALUE (F_CPU/(USART_BAUDRATE<<4))-1

/**
 * \fn ISR(USART_RX_vect)
 * \brief reception interrupt
 */
ISR(USART_RX_vect)
{
    ctx.data_cb(&UDR0); // send reception byte to state machine
}

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
    // Set baud rate
    //UBRR0= UBRR_VALUE;
    UBRR0H = (uint8_t)(UBRR_VALUE>>8);
    UBRR0L = (uint8_t)(UBRR_VALUE);
    // interrupt on receive

    // 9600-8-E-1
    // That is, baudrate of 9600bps
    // 8 databits
    // Even parity
    // 1 stopbit
    UCSR0B = (1 << RXEN0) | (1 << RXCIE0) | (1<<TXEN0); // enable TX RX and interrupts
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // init DE port
    DDRD |= (1<<DDD3);
    // init DE port
    DDRD |= (1<<DDD2);
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
    PORTD |= (1<<PORTD3) | (1<<PORTD2); // Disable RX and enable TX

    for (unsigned short i = 0; i<size; i++) {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Wait transmit buffer ready
        UDR0 = data[i]; // Send data
        UCSR0A |= (1<<TXC0); // Clear end transmission flag
    }
    loop_until_bit_is_set(UCSR0A, TXC0); // wait transmission end
    PORTD &= ~(1<<PORTD3) & ~(1<<PORTD2); // disable TX and enable RX
    return 0;
}
