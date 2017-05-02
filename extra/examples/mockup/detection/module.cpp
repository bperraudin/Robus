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
  GATE_TYPE,
  MOTOR_TYPE,
  LED_TYPE,
  POTARD_TYPE,
  BUTTON_TYPE,
} module_type_t;

typedef enum {
    RESET_CMD,
    LEAVE_CMD,
    PUBLISH_CMD,
    IDENTIFY_CMD,
    INTRODUCTION_CMD,
    MOTOR_PROTOCOL_NB,
} module_register_t;


#define MAX_JSON_STRING 512

#define MSG_TIMEOUT (1000 * 1000)
#define LOOP_TIMEOUT (1000 * 100)
#define ALL_CONNECTED_TIMEOUT (1000 * 1000 * 2)

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

  } else if (msg->header.cmd == RESET_CMD) {
    reset_detection();

  }
}

std::string type_from_enum(module_type_t type) {
  if (type == BUTTON_TYPE) {
    return "button";

  } else if (type == LED_TYPE) {
    return "led";

  } else if (type == MOTOR_TYPE) {
    return "motor";
  }
  return "unknown";
}

void publish_info_as_json() {
  std::string json;

  json += "{";
  json += "\"modules\":[";

  for (auto const &el: route_table) {
    json += "{";

    json += "\"type\": \"" + type_from_enum((module_type_t)el.second.first) + "\", ";
    json += "\"id\": " + std::to_string(el.first) + ", ";
    json += "\"alias\": \"" + el.second.second + "\"";

    if (el.second.first == BUTTON_TYPE) {
      json += ", \"value\": " + std::to_string(data[el.first]);
    }

    json += "}, ";
  }

  // HACK: remove last " ,"
  json.pop_back(); json.pop_back();

  json += "]";
  json += "}";

  printf("%s\n", json.c_str());
}

void reset_table() {
  msg_t reset_msg;

  reset_msg.header.cmd = RESET_CMD;
  reset_msg.header.target = BROADCAST_VAL;
  reset_msg.header.target_mode = BROADCAST;

  robus_send(vm, &reset_msg);

  route_table.clear();
  data.clear();

  usleep(MSG_TIMEOUT);
}


int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s type alias plug\n", argv[0]);
    return 1;
  }

  const char *type = argv[1];
  const char *alias = argv[2];
  const char *side = argv[3];

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

  unsigned char mod_type;
  if (strcmp(type, "gate") == 0) {
    mod_type = GATE_TYPE;
  } else if (strcmp(type, "motor") == 0) {
    mod_type = MOTOR_TYPE;
  } else if (strcmp(type, "led") == 0) {
    mod_type = LED_TYPE;
  } else if (strcmp(type, "button") == 0) {
    mod_type = BUTTON_TYPE;
  }

  vm = robus_module_create(rx_cb, mod_type, alias);

  leave_msg.header.cmd = LEAVE_CMD;
  leave_msg.header.target = BROADCAST_VAL;
  leave_msg.header.target_mode = BROADCAST;

  identify_msg.header.cmd = IDENTIFY_CMD;
  identify_msg.header.target = BROADCAST_VAL;
  identify_msg.header.target_mode = BROADCAST;


  if (strcmp(side, "none") == 0) {
    usleep(ALL_CONNECTED_TIMEOUT);

    for (int j= 0; j < 10; j++) {
      printf("\nCleaning route table\n");
      reset_table();

      printf("Launch topology detection\n");
      topology_detection(vm);

      printf("\nAsk for identification\n");
      robus_send(vm, &identify_msg);
      usleep(MSG_TIMEOUT);

      printf("\nRoute table:\n");
      for (auto const &el: route_table) {
        printf("alias \"%s\" - id %d - type %d\n", el.second.second.c_str(), el.first, el.second.first);
      }

      printf("\nGathered updates\n");
      for (int i=0; i < 10; i++) {
        publish_info_as_json();
        usleep(LOOP_TIMEOUT);
      }
    }

    printf("\nBroadcasting leave msg!\n");
    robus_send(vm, &leave_msg);
    usleep(MSG_TIMEOUT);
  }
  else {
    while (!leave) {
      if (identify) {
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
        identify = 0;
      }

      if (vm->type == BUTTON_TYPE) {
        msg_t publish_msg;
        publish_msg.header.cmd = PUBLISH_CMD;
        publish_msg.header.target = BROADCAST_VAL;
        publish_msg.header.target_mode = BROADCAST;
        publish_msg.header.size = 1;
        publish_msg.data[0] = random_data;
        robus_send(vm, &publish_msg);

        random_data += vm->id;
      }

      usleep(LOOP_TIMEOUT);
    }
  }

  return 0;
}

#endif /* UNIT_TEST */
