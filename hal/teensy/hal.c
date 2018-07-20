#include "hal.h"
#include <avr/eeprom.h>

#include "context.h"
#include "detection.h"
#include "reception.h"

#define USART_BAUDRATE 57600ul
#define DE 14
#define RE 15
#define PTPA 11
#define PTPB 6


#define TIMEOUT_VAL 1
#include <arduino.h>
#include <TimerOne.h>

volatile unsigned int hal_millis = 0;

void timerCallback() {
    while (Serial2.available()) {
        unsigned char inChar = (char)Serial2.read();
        ctx.data_cb(&inChar); // send reception byte to state machine
        hal_millis = 0;
    }
    if (hal_millis >= TIMEOUT_VAL && ctx.tx_lock) {
        timeout();
        hal_millis = 0;
    }
    if (hal_millis < 0xFFFF) hal_millis++;
}

/**
 * \fn ptpb_handler()
 * \brief PTPB interrupt
 */
void ptpb_handler() {
    ptp_handler(BRANCH_B);
}

/**
 * \fn ptpa_handler()
 * \brief PTPA interrupt
 */
void ptpa_handler() {
    ptp_handler(BRANCH_A);
}

void reverse_detection(branch_t branch) {
    if (branch == BRANCH_A) {
        detachInterrupt(digitalPinToInterrupt(PTPA));
        attachInterrupt(digitalPinToInterrupt(PTPA), ptpa_handler, RISING);
    }
    if (branch == BRANCH_B) {
        detachInterrupt(digitalPinToInterrupt(PTPB));
        attachInterrupt(digitalPinToInterrupt(PTPB), ptpb_handler, RISING);
    }
}

void hal_init(void) {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial2.begin(USART_BAUDRATE);
    pinMode(DE, OUTPUT);
    pinMode(RE, OUTPUT);
    digitalWrite(DE, 0);
    digitalWrite(RE, 0);

    // configure ptp lines
    pinMode(PTPA, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PTPA), ptpa_handler, FALLING);
    pinMode(PTPB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PTPB), ptpb_handler, FALLING);

    // configure timer
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerCallback, 1000);
    Timer1.start();

    // manage interrupt priorities
    NVIC_SET_PRIORITY(IRQ_FTM1, 16); // Set timer 1 high priority
}

unsigned char hal_transmit(unsigned char *data, unsigned short size) {
    int plop = Serial2.availableForWrite();
    digitalWrite(DE, 1);
    digitalWrite(RE, 1);
    for (unsigned short i = 0; i<size; i++) {
        Serial2.write(data[i]); // Send data
        while(Serial2.availableForWrite() != plop);
        hal_millis = 0;
    }
    hal_delay_ms(1);
    digitalWrite(DE, 0);
    digitalWrite(RE, 0);
    hal_millis = 0;

    return 0;
}

void hal_delay_ms(int factor) {
    delay(factor);
}

void set_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        detachInterrupt(digitalPinToInterrupt(PTPA)); // Turns off PTPA interruption
        pinMode(PTPA, OUTPUT); // set the PTPA pin as output
        digitalWrite(PTPA, 0); // Set the PTPA pin
    }
    else if (branch == BRANCH_B) {
        detachInterrupt(digitalPinToInterrupt(PTPB)); // Turns off PTPB interruption
        pinMode(PTPB, OUTPUT); // set the PTPB pin as output
        digitalWrite(PTPB, 0); // Set the PTPB pin
    }
}

void reset_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        detachInterrupt(digitalPinToInterrupt(PTPA)); // Turns off PTPA interruption
        pinMode(PTPA, INPUT_PULLUP); // set the PTPA pin as input
        attachInterrupt(digitalPinToInterrupt(PTPA), ptpa_handler, FALLING); // Turns on PTPA interruption on falling edge
    }
    else if (branch == BRANCH_B) {
        detachInterrupt(digitalPinToInterrupt(PTPB)); // Turns off PTPB interruption
        pinMode(PTPB, INPUT_PULLUP); // set the PTPB pin as input
        attachInterrupt(digitalPinToInterrupt(PTPB), ptpb_handler, FALLING); // Turns on PTPB interruption on falling edge
    }
}

unsigned char get_PTP(branch_t branch) {
  if (branch == BRANCH_A) {
      return (digitalRead(PTPA) == 0);
  }
  else if (branch == BRANCH_B) {
      return (digitalRead(PTPB) == 0);
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
