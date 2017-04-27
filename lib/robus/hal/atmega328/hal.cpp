#include "hal.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define USART_BAUDRATE 9600ul
#define UBRR_VALUE (F_CPU/(USART_BAUDRATE<<4))-1

#define RE_PORT PORTD
#define RE_PIN PORTD4
#define RE_SETUP_PORT DDRD
#define RE_SETUP_PIN DDD4

#define DE_PORT PORTD
#define DE_PIN PORTD5
#define DE_SETUP_PORT DDRD
#define DE_SETUP_PIN DDD5

#define PTPA_PORT PORTD
#define PTPA_PIN PORTD3
#define PTPA_READ (PIND & (1<<PD3))
#define PTPA_SETUP_PORT DDRD
#define PTPA_SETUP_PIN DDD3

#define PTPB_PORT PORTD
#define PTPB_PIN PORTD2
#define PTPB_READ (PIND & (1<<PD2))
#define PTPB_SETUP_PORT DDRD
#define PTPB_SETUP_PIN DDD2

#define ENABLE_TX DE_PORT |= (1<<DE_PIN)
#define ENABLE_RX RE_PORT &= ~(1<<RE_PIN)
#define DISABLE_TX DE_PORT &= ~(1<<DE_PIN)
#define DISABLE_RX RE_PORT |= (1<<RE_PIN)

/**
 * \fn ISR(USART_RX_vect)
 * \brief reception interrupt
 */
ISR(USART_RX_vect)
{
    ctx.data_cb(&UDR0); // send reception byte to state machine
}

/**
 * \fn ISR(INT0_vect)
 * \brief PTPB interrupt
 */
ISR (INT0_vect) {
    static char ptp_state = 1;
    if (ptp_state == 0) {
        ptp_released(BRANCH_B);
        reset_PTP(BRANCH_B);
    }
    else {
        hal_timeout(2);
        if (PTPB_READ == 0){
            ptp_detected(BRANCH_B);
            EICRA |= (1 << ISC00); // reverse the detection edge
            ptp_state = 0;
        }
        else{
            poke_detected(BRANCH_B);
        }
    }
}

/**
 * \fn ISR(INT0_vect)
 * \brief PTPA interrupt
 */
ISR (INT1_vect) {
    static char ptp_state = 1;
    if (ptp_state == 0) {
        ptp_released(BRANCH_A);
        reset_PTP(BRANCH_A);
    }
    else {
        hal_timeout(2);
        if (PTPA_READ == 0){
            ptp_detected(BRANCH_A);
            EICRA |= (1 << ISC10); // reverse the detection edge
            ptp_state = 0;
        }
        else{
            poke_detected(BRANCH_A);
        }
    }
}


void set_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        EIMSK &= ~(1 << INT1); // Turns off INT1
        EICRA &= ~(1 << ISC11); // Clean edge/state detection
        EIFR |= (1 << INTF1); //reset event flag
        PTPA_SETUP_PORT |= (1 << PTPA_SETUP_PIN);     // set the PTPA pin as output
        PTPA_PORT &= ~(1<<PTPA_PIN); // Set the PTPA pin
    }
    else if (branch == BRANCH_B) {
        EIMSK &= ~(1 << INT0); // Turns off INT0
        EICRA &= ~(1 << ISC01); // Clean edge/state detection
        EIFR |= (1 << INTF0); //reset event flag
        PTPB_SETUP_PORT |= (1 << PTPB_SETUP_PIN);     // set the PTPB pin
        PTPB_PORT &= ~(1<<PTPB_PIN); // Set the PTPB pin
    }
}

void reset_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        PTPA_SETUP_PORT &= ~(1 << PTPA_SETUP_PIN); // set the PTPA pin as input
        PTPA_PORT |= (1 << PTPA_PIN);    // turn On the Pull-up
        EIFR |= (1 << INTF1); //reset event flag due to Pull-up
        EICRA |= (1 << ISC11); // set to trigger on falling edge event
        EICRA &= ~(1 << ISC10); // set to trigger on falling edge event
        EIMSK |= (1 << INT1); // Turns on INT1
    }
    else if (branch == BRANCH_B) {
        PTPB_SETUP_PORT &= ~(1 << PTPB_SETUP_PIN); // set the PTPB pin as input
        PTPB_PORT |= (1 << PTPB_PIN);    // turn On the Pull-up
        EIFR |= (1 << INTF0); //reset event flag due to Pull-up
        EICRA |= (1 << ISC01); // set to trigger on falling edge event
        EICRA &= ~(1 << ISC00); // set to trigger on falling edge event
        EIMSK |= (1 << INT0); // Turns on INT0
    }
}

void send_poke(branch_t branch) {
    set_PTP(branch);
    hal_timeout(1);
    reset_PTP(branch);
}

void hal_timeout(int factor) {
    // TODO: do something clever here...
    for(int i=0; i<factor; i++)_delay_ms(1);
}

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
    // Set ptp lines
    PTPA_SETUP_PORT &= ~(1 << PTPA_SETUP_PIN);     // Clear the PTPA pin
    PTPB_SETUP_PORT &= ~(1 << PTPB_SETUP_PIN);     // Clear the PTPB pin
    // PTPA and PTPB are now an input
    PTPA_PORT |= (1 << PTPA_PIN);    // turn On the Pull-up
    PTPB_PORT |= (1 << PTPB_PIN);    // turn On the Pull-up
    // PTPA and PTPB are now an input with pull-up enabled
    EIFR |= (1 << INTF1) | (1 << INTF0);
    EICRA |= (1 << ISC11) | (1 << ISC01);    // set to triggers on falling edge event
    EIMSK |= (1 << INT0) | (1 << INT1);     // Turns on INT0 INT1

    // Set baud rate
    //UBRR0= UBRR_VALUE;
    UBRR0H = (uint8_t)((UBRR_VALUE)>>8);
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
    RE_SETUP_PORT |= (1<<DE_SETUP_PIN);
    // init DE port
    DE_SETUP_PORT |= (1<<RE_SETUP_PIN);
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
    //PORTD |= (1<<PORTD3) | (1<<PORTD2); // Disable RX and enable TX
    ENABLE_TX;
    DISABLE_RX;

    for (unsigned short i = 0; i<size; i++) {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Wait transmit buffer ready
        UDR0 = data[i]; // Send data
        UCSR0A |= (1<<TXC0); // Clear end transmission flag
    }
    loop_until_bit_is_set(UCSR0A, TXC0); // wait transmission end
    //PORTD &= ~(1<<PORTD3) & ~(1<<PORTD2); // disable TX and enable RX
    ENABLE_RX;
    DISABLE_TX;
    return 0;
}
