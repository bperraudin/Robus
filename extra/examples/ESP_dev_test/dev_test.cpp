#ifndef UNIT_TEST

#include <robus.h>
#include <sys_msg.h>

#include <Arduino.h>

void halLoop(void);

typedef enum {
    LED,
    MODULE_PROTOCOL_NB
} module_register_t;


void rx_cb(msg_t *msg) {
    if (msg->header.cmd == LED)
        if (msg->data[0])
            digitalWrite(LED_BUILTIN, LOW);
        if (!msg->data[0])
            digitalWrite(LED_BUILTIN, HIGH);
}

msg_t msg;
vm_t *vm;

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    robus_init();
    vm = robus_module_create(rx_cb, 1, "Larry Skywalker");
    // vm->id = 3;
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    topology_detection(vm);
}

void loop() {
    halLoop();
    msg_t msg;
    msg.header.cmd = LED;
    msg.header.target_mode = ID;
    msg.header.size = 1;
    for (int i = 2; i<5; i++) {
        msg.header.target = i;
        msg.data[0] = 1;
        robus_send(vm, &msg);
        delay(100);halLoop();
        msg.data[0] = 0;
        robus_send(vm, &msg);
        delay(100);halLoop();
    }

}

#endif /* UNIT_TEST */
