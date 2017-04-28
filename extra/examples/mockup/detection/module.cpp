#ifndef UNIT_TEST

#include <hal.h>
#include <robus.h>
#include <config.h>
#include <detection.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>


typedef enum {
    IDENTIFY_CMD,
    LEAVE_CMD,
    MOTOR_PROTOCOL_NB,
} module_register_t;

typedef enum {
  MODULE_TYPE,
} module_type_t;


int leave = 0;


vm_t *vm;

msg_t leave_msg;
msg_t identify_msg;



void rx_cb(msg_t *msg) {
  if (msg->header.cmd == IDENTIFY_CMD) {
    printf("I'm the module %s with id %d \n", vm->alias, vm->id);

  } else if (msg->header.cmd == LEAVE_CMD) {
    leave = 1;

  }
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s alias plug\n", argv[0]);
    return 1;
  }

  const char *alias = argv[1];
  const char *side = argv[2];

  robus_init();
  vm = robus_module_create(rx_cb, MODULE_TYPE, alias);

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

  leave_msg.header.cmd = LEAVE_CMD;
  leave_msg.header.target = BROADCAST_VAL;
  leave_msg.header.target_mode = BROADCAST;

  identify_msg.header.cmd = IDENTIFY_CMD;
  identify_msg.header.target = BROADCAST_VAL;
  identify_msg.header.target_mode = BROADCAST;


  if (strcmp(side, "none") == 0) {
    usleep(1000 * 1000 * 2);

    printf("Launch topology detection\n");
    topology_detection(vm);

    printf("Ask for identification\n");
    robus_send(vm, &identify_msg);
    usleep(1000 * 1000 * 2);

    printf("Broadcasting leave msg!\n");
    robus_send(vm, &leave_msg);
  }

  while (!leave) {
    usleep(100000);
  }

  return 0;
}

#endif /* UNIT_TEST */
