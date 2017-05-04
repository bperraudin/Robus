#ifndef UNIT_TEST

#include <robus.h>
#include <sys_msg.h>

#include <Arduino.h>

void halLoop(void);

typedef enum {
    TEST_CMD,
    NO_OVERLAP_TARGET_CMD,
    MODULE_PROTOCOL_NB
} module_register_t;


void rx_cb(msg_t *msg) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1);
    digitalWrite(LED_BUILTIN, HIGH);
}

msg_t msg;
vm_t *vm;

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    robus_init();
    vm = robus_module_create(rx_cb, 1, "Larry Skywalker");
    vm->id = 42;

    msg.header.cmd = TEST_CMD;
    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.size = 2;
    msg.data[0] = 1;
    msg.data[1] = 2;
}

void loop() {
    halLoop();
    digitalWrite(LED_BUILTIN, LOW);
    robus_send(vm, &msg);
    delay(1000);
}

#endif /* UNIT_TEST */
