/*
 * tests.c
 *
 * Created: 11/02/2015 12:40:48
 *  Author: Nicolas Rabault
 */


#include "robus.h"
#include "inc/sys_msg.h"
#include "inc/target.h"
#include "test_mngmnt.h"

#include <stdio.h>
#include <string.h>

int test_value = 0;
unsigned short target_value = 0;
vm_t *vm1, *vm2, *vm3;

typedef enum {
    TARGET_CMD = WRITE_ID,
    TEST_CMD = PROTOCOL_CMD_NB,
    NO_OVERLAP_TARGET_CMD,
    MODULE_PROTOCOL_NB
}module_register_t;

void rx_cb(msg_t *msg) {
    switch (msg->header.cmd) {
        case TEST_CMD :
            if (test_value == 0xCAFE) {
                test_value = ((int)msg->data[1] << 8) | (int)msg->data[0];
            }
            else {
                test_value = ((int)msg->data[0] << 8) | (int)msg->data[1];
            }
        break;
        case TARGET_CMD :
        case NO_OVERLAP_TARGET_CMD:
            target_value = msg->header.target;
        break;
        default :
            test_value = 0;
        break;
    }
}

void rx_cb_bis(msg_t *msg) {
    switch (msg->header.cmd) {
        case TEST_CMD :
                if (test_value != 0xFECA) {
                    test_value = 0xEFAC;
                }
        break;
        default :
            test_value = 0;
        break;
    }
}

/*
 * TEST SEQUENCES
 */

unsigned char test_init(void) {
    printf("\nInitialisation :\n");
    robus_init();
    vm1 = robus_module_create(rx_cb, 1, "test module of the death!");
    vm2 = robus_module_create(rx_cb, 2, "another one.");
    if (test(vm1->rx_cb == rx_cb)) return 1;
    if (test(vm1->type == 1)) return 1;
    if (test(!strncmp(vm1->alias, "test module of ", 15))) return 1;
    if (test(vm2->rx_cb == rx_cb)) return 1;
    if (test(vm2->type == 2)) return 1;
    if (test(!strncmp(vm2->alias, "another one.", 15))) return 1;
    return 0;
}

unsigned char add_vm(void) {
    printf("\nAdd virtual module :\n");
    vm3 = robus_module_create(rx_cb_bis, vm2->type, "separate cb");
    if (test(vm3->rx_cb == rx_cb_bis)) return 1;
    if (test(vm3->type == vm2->type)) return 1;
    if (test(!strncmp(vm3->alias, "separate cb", 15))) return 1;
    return 0;
}

unsigned char set_id_brdcst(void) {
    printf("\nSet ID with BROADCAST mode :\n");
    if (test(!set_extern_id(vm1, BROADCAST, BROADCAST_VAL, 0x000A))) return 1;
    if (test(vm1->id == 0x000A)) return 1;
    if (test(vm2->id == 0x000A)) return 1;
    if (test(vm3->id == 0x000A)) return 1;
    return 0;
}

unsigned char set_id_type(void) {
    printf("\nSet ID in TYPE mode :\n");
    if (test(!set_extern_id(vm1, TYPE, vm2->type, 0x000B))) return 1;
    if (test(vm2->id == 0x000B)) return 1;
    if (test(vm3->id == 0x000B)) return 1;
    if (test(vm1->id == 0x000A)) return 1;
    vm3->id = 0x000D;
    return 0;
}

unsigned char set_id(void) {
    printf("\nSet ID in ID mode :\n");
    if (test(!set_extern_id(vm2, ID, vm1->id, 0x000C))) return 1;
    if (test(vm1->id == 0x000C )) return 1;
    if (test(vm2->id == 0x000B )) return 1;
    return 0;
}

unsigned char set_id_ack(void) {
    // this one can't be tested in stub mode...
    printf("\nSet ID in IDACK mode :\n");
    if (test(!set_extern_id(vm1, IDACK, ctx.type, 0x000D))) return 1;
    if (test(vm1->id == 0x000D)) return 1;
    if (test(vm1->msg_pt->ack)) return 1;
    return 0;
}

unsigned char write_id_mode(void) {
    printf("\nSend something in id_mode message :\n");
    msg_t msg = {.header.cmd = TEST_CMD,
                 .header.target = vm1->id,
                 .header.target_mode = ID,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};

    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xCAFE)) return 1;
    test_value = 0x0000;
    if (test(!robus_send(vm2, &msg))) return 1;
    if (test(test_value == 0xCAFE)) return 1;
    test_value = 0x0000;
    msg.header.target = vm2->id;
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xCAFE)) return 1;
    test_value = 0x0000;
    if (test(!robus_send(vm2, &msg))) return 1;
    if (test(test_value == 0xCAFE)) return 1;
    test_value = 0x0000;

    return 0;
}

unsigned char write_broadcast_mode(void) {
    printf("\nSend something in BROADCAST mode message :\n");
    msg_t msg = {.header.cmd = TEST_CMD,
                 .header.target = BROADCAST_VAL,
                 .header.target_mode = BROADCAST,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xFECA)) return 1;
    test_value = 0x0000;
    return 0;
}

unsigned char add_multicast(void) {
    printf("\nAdd a MULTICAST target :\n");
    add_multicast_target(vm1, 0x000E);
    if (test(vm1->multicast_target_bank[vm1->max_multicast_target] != 0x000E)) return 1;
    msg_t msg = {.header.cmd = TARGET_CMD,
                 .header.target = 0x000E,
                 .header.target_mode = MULTICAST,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(target_value == 0x000E)) return 1;
    add_multicast_target(vm1, 0x00AE);
    msg.header.target = 0x00AE;
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(target_value == 0x00AE)) return 1;
    msg.header.target = 0x000E;
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(target_value == 0x000E)) return 1;
    target_value = 0x0000;

    msg.header.cmd = TEST_CMD;
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xCAFE)) return 1;
    test_value = 0x0000;
    add_multicast_target(vm2, 0x000E);
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xFECA)) return 1;
    test_value = 0x0000;

    add_multicast_target(vm3, 0x00AE);
    msg.header.target = 0x00AE;
    if (test(!robus_send(vm1, &msg))) return 1;
    if (test(test_value == 0xEFAC)) return 1;
    test_value = 0x0000;
    return 0;
}

int main(void) {
    printf("test sequences :\n");

    test_sequences(test_init);
    test_sequences(add_vm);
    test_sequences(set_id_brdcst);
    test_sequences(set_id_type);
    test_sequences(set_id);
    test_sequences(write_id_mode);
    test_sequences(write_broadcast_mode);
    test_sequences(add_multicast);

    return test_end();
}
