#include "hal.h"
#include "reception.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define USART_BAUDRATE 57600ul
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

volatile unsigned int hal_millis = 0;
#define TIMEOUT_VAL 1

 ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
 {
    if (hal_millis == TIMEOUT_VAL && ctx.tx_lock) {
        timeout();
        hal_millis = 0;
    }
    if (hal_millis < 0xFFFF) hal_millis++;
}

/**
 * \fn ISR(USART_RX_vect)
 * \brief reception interrupt
 */
ISR(USART_RX_vect)
{
    TCNT0 = 0;
    hal_millis = 0;
    ctx.data_cb(&UDR0); // send reception byte to state machine
}

/**
 * \fn ISR(INT0_vect)
 * \brief PTPB interrupt
 */
ISR (INT0_vect) {
    ptp_handler(BRANCH_B);
}

/**
 * \fn ISR(INT1_vect)
 * \brief PTPA interrupt
 */
ISR (INT1_vect) {
    ptp_handler(BRANCH_A);
}

void reverse_detection(branch_t branch) {
    if (branch == BRANCH_A) {
        EICRA |= (1 << ISC10); // reverse the detection edge
    }
    if (branch == BRANCH_B) {
        EICRA |= (1 << ISC00); // reverse the detection edge
    }
}

void set_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        EIMSK &= ~(1 << INT1); // Turns off INT1
        EICRA &= ~(1 << ISC11) & ~(1 << ISC10); // Clean edge/state detection
        EIFR |= (1 << INTF1); //reset event flag
        PTPA_SETUP_PORT |= (1 << PTPA_SETUP_PIN);     // set the PTPA pin as output
        PTPA_PORT &= ~(1<<PTPA_PIN); // Set the PTPA pin
    }
    else if (branch == BRANCH_B) {
        EIMSK &= ~(1 << INT0); // Turns off INT0
        EICRA &= ~(1 << ISC01) & ~(1 << ISC00); // Clean edge/state detection
        EIFR |= (1 << INTF0); //reset event flag
        PTPB_SETUP_PORT |= (1 << PTPB_SETUP_PIN);     // set the PTPB pin
        PTPB_PORT &= ~(1<<PTPB_PIN); // Set the PTPB pin
    }
}

void reset_PTP(branch_t branch) {
    sei();
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

void hal_delay_ms(int factor) {
    // TODO: do something clever here...
    for(int i=0; i<factor; i++)_delay_ms(1);
}

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
    cli();
    TCCR0A |= (1 << WGM01); // Set the Timer Mode to CTC
    OCR0A = 0xF9; // Set the value that you want to count to
    TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect
     TCCR0B |= (1 << CS01) | (1 << CS00); // set prescaler to 64 and start the timer

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
    cli();
    ENABLE_TX;
    DISABLE_RX;

    for (unsigned short i = 0; i<size; i++) {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Wait transmit buffer ready
        UDR0 = data[i]; // Send data
        UCSR0A |= (1<<TXC0); // Clear end transmission flag
        hal_millis = 0;
    }
    loop_until_bit_is_set(UCSR0A, TXC0); // wait transmission end
    //PORTD &= ~(1<<PORTD3) & ~(1<<PORTD2); // disable TX and enable RX
    ENABLE_RX;
    DISABLE_TX;
    sei();
    hal_millis = 0;
    return 0;
}

unsigned char get_PTP(branch_t branch) {
  if (branch == BRANCH_A) {
      return (PTPA_READ == 0);
  }
  else if (branch == BRANCH_B) {
      return (PTPB_READ == 0);
  }
}

inline unsigned short eeprom_compute_addr(int id){
    return id * (MAX_ALIAS_SIZE +1);
}

void write_alias(unsigned short id, char* alias) {
    // const unsigned short addr = eeprom_compute_addr(id);
    // if (alias[0] == '\0') {
    //     // there is no data in the name erase it
    //     eeprom_update_byte((void*)addr, 0XFF);
    // } else {
    //     eeprom_update_byte((void*)addr, 0XAA);
    //     eeprom_update_block((void*)alias, (const void*)addr+1, MAX_ALIAS_SIZE);
    // }
}

char read_alias(unsigned short id, char* alias) {
    // const unsigned short addr = eeprom_compute_addr(id);
    // if (!(eeprom_read_byte((void*)addr) == 0xAA)) {
        return 0;
    // }
    // eeprom_read_block((void*)alias, (const void*)addr+1, MAX_ALIAS_SIZE);
    // return 1;
}