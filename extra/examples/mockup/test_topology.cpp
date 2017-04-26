#ifndef UNIT_TEST

#include <robus.h>
#include <detection.h>
#include <hal.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int stop = 0;

typedef enum {
    LEAVE_CMD,
    IDENTIFY_CMD,
    MODULE_PROTOCOL_NB
} module_register_t;


vm_t *vm;


void rx_cb(msg_t *msg) {
    if (msg->header.cmd == IDENTIFY_CMD) {
        printf("%s has the id(s)", vm->alias);

        for (int i=0; i < ctx.vm_number; i++) {
            printf(" %d", ctx.vm_table[i].id);
        }
        printf("\n");
    }

    else if (msg->header.cmd == LEAVE_CMD) {
        stop = 1;
    }
}


int main(int argc, char const *argv[]) {
    if (argc != 4) {
        printf("Usage: %s alias #vm plug-left/right/none\n", argv[0]);
        return 1;
    }

    const char *alias = argv[1];
    const int number_vm = atoi(argv[2]);
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

    vm = robus_module_create(rx_cb, 1, alias);

    char name[256];
    for (int i=1; i < number_vm; i++) {
        snprintf(name, 256, "%s_vm_%d", alias, i);
        robus_module_create(0, 1, name);
    }

    if (strcmp(side, "none") == 0) {
        // Wait for all modules to be connected
        sleep(10);

        printf("Master > Start topology detection...\n");
        topology_detection(vm);
        printf("Done!\n");

        // Send identify to everyone
        msg_t identify_msg;
        identify_msg.header.cmd = IDENTIFY_CMD;
        identify_msg.header.target = BROADCAST_VAL;
        identify_msg.header.target_mode = BROADCAST;

        printf("Master > Broadcasting identify signal!\n");
        robus_send(vm, &identify_msg);

        sleep(5);

        // Send leave message to everyone
        msg_t leave_msg;
        leave_msg.header.cmd = LEAVE_CMD;
        leave_msg.header.target = BROADCAST_VAL;
        leave_msg.header.target_mode = BROADCAST;

        printf("Master > Broadcasting leave signal!\n");
        robus_send(vm, &leave_msg);
    }

    while (!stop) {
        sleep(1);
    }

    return 0;
}

#endif /* UNIT_TEST */
