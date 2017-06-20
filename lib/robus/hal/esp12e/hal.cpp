#include "hal.h"

#include "context.h"
#include "detection.h"
#include "reception.h"

#define USART_BAUDRATE 9600ul
#define DE 12
#define RE 14
#define PTPA 4
#define PTPB 2


#define TIMEOUT_VAL 2
#include <arduino.h>
#include "user_interface.h"
os_timer_t myTimer;
volatile unsigned int hal_millis = 0;

// start of timerCallback
void timerCallback(void *pArg) {
    while (Serial.available()) {
        hal_millis = 0;
        unsigned char inChar = (char)Serial.read();
        ctx.data_cb(&inChar); // send reception byte to state machine
    }
    if (hal_millis == TIMEOUT_VAL && ctx.tx_lock) {
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
        detachInterrupt(PTPA);
        attachInterrupt(PTPA, ptpa_handler, RISING );
    }
    if (branch == BRANCH_B) {
        detachInterrupt(PTPB);
        attachInterrupt(PTPB, ptpb_handler, RISING );
    }
}

void hal_init(void) {
    Serial.begin(USART_BAUDRATE);
    Serial1.setDebugOutput(false);
    Serial.setDebugOutput(false);
    Serial.swap();
    pinMode(DE, OUTPUT);
    pinMode(RE, OUTPUT);
    digitalWrite(DE, 0);
    digitalWrite(RE, 0);

    // configure ptp lines
    pinMode(PTPA, INPUT_PULLUP);
    attachInterrupt(PTPA, ptpa_handler, FALLING);
    pinMode(PTPB, INPUT_PULLUP);
    attachInterrupt(PTPB, ptpb_handler, FALLING);

    // configure timer
    os_timer_setfn(&myTimer, timerCallback, NULL);
    os_timer_arm(&myTimer, 1, true);
}

unsigned char hal_transmit(unsigned char *data, unsigned short size) {
int plop = Serial.availableForWrite();
digitalWrite(DE, 1);
digitalWrite(RE, 1);
for (unsigned short i = 0; i<size; i++) {
    Serial.write(data[i]); // Send data
    while(Serial.availableForWrite() != plop);
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
        detachInterrupt(PTPA); // Turns off PTPA interruption
        pinMode(PTPA, OUTPUT); // set the PTPA pin as output
        digitalWrite(PTPA, 0); // Set the PTPA pin
    }
    else if (branch == BRANCH_B) {
        detachInterrupt(PTPB); // Turns off PTPB interruption
        pinMode(PTPB, OUTPUT); // set the PTPB pin as output
        digitalWrite(PTPB, 0); // Set the PTPB pin
    }
}

void reset_PTP(branch_t branch) {

    pinMode(PTPB, INPUT_PULLUP);
    attachInterrupt(PTPB, ptpb_handler, FALLING);
    if (branch == BRANCH_A) {
        detachInterrupt(PTPA); // Turns off PTPA interruption
        pinMode(PTPA, INPUT_PULLUP); // set the PTPA pin as input
        attachInterrupt(PTPA, ptpa_handler, FALLING); // Turns on PTPA interruption on falling edge
    }
    else if (branch == BRANCH_B) {
        detachInterrupt(PTPB); // Turns off PTPB interruption
        pinMode(PTPB, INPUT_PULLUP); // set the PTPB pin as input
        attachInterrupt(PTPB, ptpa_handler, FALLING); // Turns on PTPB interruption on falling edge
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
