#ifndef UNIT_TEST

#include <robus.h>
#include <hal.h>
#include <config.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>


typedef enum {
    HELLO_CMD,
    MOTOR_PROTOCOL_NB,
} module_register_t;

typedef enum {
  MODULE_TYPE,
} module_type_t;


int received_msg = 0;

void rx_cb(msg_t *msg) {
  if (msg->header.cmd == HELLO_CMD) {
    received_msg++;
  }
}

void say_goodbye(void) {
  printf("Got %d msg\n", received_msg);
}

msg_t hello_msg;
vm_t *vm;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s alias plug\n", argv[0]);
    return 1;
  }

  const char *alias = argv[1];
  const char *side = argv[2];

  robus_init();
  vm = robus_module_create(rx_cb, MODULE_TYPE, alias);

  hello_msg.header.cmd = HELLO_CMD;
  hello_msg.header.target = BROADCAST_VAL;
  hello_msg.header.target_mode = BROADCAST;

  if (strcmp(side, "left") == 0) {
      plug(LEFT_SIDE);
  } else if (strcmp(side, "right") == 0) {
      plug(RIGHT_SIDE);
  }
  else if (strcmp(side, "none") == 0) {
      plug(NO_SIDE);
  } else {
      printf("Unknown side %s (left, right, none)\n", side);
      return 1;
  }

  atexit(say_goodbye);

  for (;;) {
    robus_send(vm, &hello_msg);
    usleep(10000);
  }

  return 0;
}

#endif /* UNIT_TEST */
