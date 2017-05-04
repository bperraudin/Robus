#ifndef UNIT_TEST

#include <Arduino.h>

#include <robus.h>
#include <sys_msg.h>


#define PING_ID 1
#define PONG_ID 2

typedef enum {
    PING_CMD,
    GATE_PROTOCOL_NB,
} module_register_t;

int ping;
int other_id;

vm_t *vm;

#ifndef ESP12E
void led(int on) {
  if (on) {
    PORTB |= (1<<PORTB5);
  }
  else {
    PORTB &= ~(1<<PORTB5);
  }
}
#else
void led(int on) {
  if (on) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
#endif


void rx_cb(msg_t *msg) {
  if (msg->header.cmd == PING_CMD) {
    ping = 1;
  }
}

void setup() {
  #ifdef ESP12E
    pinMode(LED_BUILTIN, OUTPUT);
  #endif

  robus_init();

  #ifdef PING
    const char *alias = "ping";
  #else
    const char *alias = "pong";
  #endif

  vm = robus_module_create(rx_cb, 1, alias);

#ifdef PING
  vm->id = PING_ID;
  ping = 1;
  other_id = PONG_ID;
  delay(2000);

#else
  vm->id = PONG_ID;
  ping = 0;
  other_id = PING_ID;
#endif
}

msg_t ping_msg;

void loop() {
  led(ping);

  if (ping) {
    ping_msg.header.cmd = PING_CMD;
    ping_msg.header.target = other_id;
    ping_msg.header.target_mode = ID;

    robus_send(vm, &ping_msg);
    ping = 0;
  }

  delay(200);
}

#endif
