#ifndef UNIT_TEST

#include <hal.h>
#include <robus.h>
#include <config.h>
#include <detection.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <map>


typedef enum {
    LEAVE_CMD,
    PUBLISH_CMD,
    IDENTIFY_CMD,
    INTRODUCTION_CMD,
    MOTOR_PROTOCOL_NB,
} module_register_t;


#define MODULE_TYPE 1
#define GATE_ID 1

int leave = 0;
int identify = 0;

vm_t *vm;

msg_t leave_msg;
msg_t identify_msg;

// route_table[id] = (type, alias)
std::map<int, std::pair<int, std::string> >route_table;

// data[id] = val
std::map<int, int>data;

int random_data = 0;


void rx_cb(msg_t *msg) {
  if (msg->header.cmd == IDENTIFY_CMD) {
    printf("I'm the module \"%s\" with id %d and type %d\n", vm->alias, vm->id, vm->type);
    identify = 1;

  } else if (msg->header.cmd == LEAVE_CMD) {
    leave = 1;

  // Only the gate should receive the introduction CMD
  } else if (msg->header.cmd == INTRODUCTION_CMD && vm->id == GATE_ID) {
    char alias[MAX_ALIAS_SIZE];
    for (int i=0; i < MAX_ALIAS_SIZE; i++) {
      alias[i] = msg->data[i];
    }

    int id = msg->header.source;
    int type = msg->data[MAX_ALIAS_SIZE];

    route_table[id] = std::pair<int, std::string>(type, std::string(alias));

    // Only the gate should receive the publish CMD
  } else if (msg->header.cmd == PUBLISH_CMD && vm->id == GATE_ID) {
      data[msg->header.source] = msg->data[0];
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

  vm = robus_module_create(rx_cb, MODULE_TYPE, alias);

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

    printf("\nAsk for identification\n");
    robus_send(vm, &identify_msg);
    usleep(1000 * 1000 * 2);

    printf("\nRoute table:\n");
    for (auto const &el: route_table) {
      printf("alias \"%s\" - id %d - type %d\n", el.second.second.c_str(), el.first, el.second.first);
    }

    for (int i=0; i < 10; i++) {
      printf("\nGathered updates\n");
      for (auto const &el: data) {
        printf("%s val %d\n", route_table[el.first].second.c_str(), el.second);
      }
      printf("\n");
      usleep(1000 * 100);
    }

    printf("\nBroadcasting leave msg!\n");
    robus_send(vm, &leave_msg);
    usleep(1000 * 1000 * 1);
  }
  else {
    while (!identify) {
      usleep(100000);
    }
    msg_t introduction_msg;
    introduction_msg.header.cmd = INTRODUCTION_CMD;
    introduction_msg.header.target = BROADCAST_VAL;
    introduction_msg.header.target_mode = BROADCAST;
    introduction_msg.header.size = 1 + MAX_ALIAS_SIZE;

    for (int i=0; i < MAX_ALIAS_SIZE; i++) {
      introduction_msg.data[i] = vm->alias[i];
    }
    introduction_msg.data[MAX_ALIAS_SIZE] = vm->type;
    robus_send(vm, &introduction_msg);
  }

  while (!leave) {
    msg_t publish_msg;
    publish_msg.header.cmd = PUBLISH_CMD;
    publish_msg.header.target = BROADCAST_VAL;
    publish_msg.header.target_mode = BROADCAST;
    publish_msg.header.size = 1;
    publish_msg.data[0] = random_data;
    robus_send(vm, &publish_msg);

    random_data += vm->id;

    usleep(1000 * 100);
  }

  return 0;
}

#endif /* UNIT_TEST */
