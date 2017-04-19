#include <robus.h>
#include <sys_msg.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

vm_t *vm;
int id = 1;


typedef enum {
    TEST_CMD = PROTOCOL_CMD_NB,
    NO_OVERLAP_TARGET_CMD,
    MODULE_PROTOCOL_NB
} module_register_t;

void rx_cb(msg_t *msg) {
    printf("Module %d got a message\n", id);
}

// All modules will send a message to this module
// This module will wait and receive them
#define MAGIC_ID 42

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s id\n", argv[0]);
        return 1;
    }

    id = atoi(argv[1]);

    robus_init();

    vm = robus_module_create(rx_cb, 1, "test module of the death!");
    vm->id = id;

    if (id != MAGIC_ID) {
      msg_t msg = {.header.cmd = TEST_CMD,
                   .header.target = MAGIC_ID,
                   .header.target_mode = ID,
                   .header.size = 2,
                   .data[0] = 0xCA,
                   .data[1] = 0xFE};

      robus_send(vm, &msg);
    }

    sleep(5);

    return 0;
}
