#include "hal.h"

#include "context.h"

#define USART_BAUDRATE 9600ul
#define DE 12
#define RE 14
#include "HardwareSerial.h"
#include <arduino.h>

void hal_init(void) {
    Serial.begin(USART_BAUDRATE);
    Serial.swap();
    pinMode(DE, OUTPUT);
    pinMode(RE, OUTPUT);
    digitalWrite(DE, 0);
    digitalWrite(RE, 0);
}

unsigned char hal_transmit(unsigned char *data, unsigned short size) {
int plop = Serial.availableForWrite();
digitalWrite(DE, 1);
digitalWrite(RE, 1);
for (unsigned short i = 0; i<size; i++) {
    Serial.write(data[i]); // Send data
    while(Serial.availableForWrite() != plop);
}
delay(1);
digitalWrite(DE, 0);
digitalWrite(RE, 0);

    return 0;
}

void send_poke(branch_t branch) {

}

void hal_timeout(int factor) {

}

void set_PTP(branch_t branch) {

}

void reset_PTP(branch_t branch) {

}
