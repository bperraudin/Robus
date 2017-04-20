#include <robus.h>
#include <sys_msg.h>

#include <Arduino.h>

void hal_loop();

typedef enum {
    TEST_CMD = PROTOCOL_CMD_NB,
    NO_OVERLAP_TARGET_CMD,
    MODULE_PROTOCOL_NB
} module_register_t;


void rx_cb(msg_t *msg) {
    Serial.println("Module got a message!");
}

msg_t msg;
vm_t *vm;

void setup() {
    Serial.begin(115200);

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

int i = 0;

void loop() {
    if (i == 1000) {
      robus_send(vm, &msg);
      i = 0;
    }
    i++;

    hal_loop();

    delay(10);
}
