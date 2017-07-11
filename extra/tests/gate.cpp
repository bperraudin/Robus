/*
 * tests.c
 *
 * Created: 11/02/2015 12:40:48
 *  Author: Nicolas Rabault
 */

 #ifndef UNIT_TEST

 #include <robus.h>

 #include <Arduino.h>

 #define LOG_BAUDRATE 57600

 void mySerialEvent();

 void rx_cb(vm_t *vm, msg_t *msg) {

 }


 void setup() {

     // Setup pullH and pullL
     pinMode(17, OUTPUT);
     digitalWrite(17, HIGH);
     pinMode(16, OUTPUT);
     digitalWrite(16, LOW);

     // Setup serial speed with RPI
     Serial.begin(LOG_BAUDRATE);

     robus_init();
     vm_t* my_vm = robus_module_create(rx_cb, 1, "unit_test");

 }

 void loop() {

 }

 void mySerialEvent() {
     while (Serial.available()) {
         // TODO
     }
 }

#endif /* UNIT_TEST */
