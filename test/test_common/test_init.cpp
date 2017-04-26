#include <unity.h>

#ifdef UNIT_TEST

#include <stdio.h>
#include <string.h>

#include <robus.h>

#include "sys_msg.h"
#include "target.h"

int test_value = 0;
unsigned short target_value = 0;
vm_t *vm1, *vm2, *vm3, *vm4;

typedef enum {
    TEST_CMD,
    TARGET_CMD,
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
        case TARGET_CMD:
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


void test_init(void) {
    robus_init();
    TEST_ASSERT_TRUE(1);

    vm1 = robus_module_create(rx_cb, 1, "test module of the death!");
    vm2 = robus_module_create(rx_cb, 2, "another one.");

    TEST_ASSERT_EQUAL(vm1->rx_cb, rx_cb);
    TEST_ASSERT_EQUAL(vm1->type, 1);
    TEST_ASSERT_EQUAL(strcmp(vm1->alias, "test module of "), 0);
    TEST_ASSERT_EQUAL(vm2->rx_cb, rx_cb);
    TEST_ASSERT_EQUAL(vm2->type, 2);
    TEST_ASSERT_EQUAL(strcmp(vm2->alias, "another one."), 0);
}

void add_vm(void) {
    vm3 = robus_module_create(rx_cb_bis, vm2->type, "separate cb");

    TEST_ASSERT_EQUAL(vm3->rx_cb, rx_cb_bis);
    TEST_ASSERT_EQUAL(vm3->type, vm2->type);
    TEST_ASSERT_EQUAL(strcmp(vm3->alias, "separate cb"), 0);
}

void add_nocb_vm(void) {
    vm4 = robus_module_create(0, vm1->type, "no cb");

    TEST_ASSERT_EQUAL(vm4->rx_cb, 0);
    TEST_ASSERT_EQUAL(vm4->type, vm1->type);
    TEST_ASSERT_EQUAL(strcmp(vm4->alias, "no cb"), 0);
}

void set_id_brdcst(void) {
    // ID Broadcast is reserved for topology detection;
    // printf("\nSet ID with BROADCAST mode :\n");
    // if (test(!set_extern_id(vm1, BROADCAST, BROADCAST_VAL, 0x000A))) return 1;
    // if (test(vm1->id == 0x000A)) return 1;
    // if (test(vm2->id == 0x000A)) return 1;
    // if (test(vm3->id == 0x000A)) return 1;
    // if (test(vm4->id == 0x000A)) return 1;
    vm1->id = 0x000A;
    vm2->id = 0x000A;
    vm3->id = 0x000A;
    vm4->id = 0x000E;
}

void set_id_type(void) {
    TEST_ASSERT_FALSE(set_extern_id(vm1, TYPE, vm2->type, 0x000B));
    TEST_ASSERT_EQUAL(vm2->id, 0x000B);
    TEST_ASSERT_EQUAL(vm3->id, 0x000B);
    TEST_ASSERT_EQUAL(vm1->id, 0x000A);
    TEST_ASSERT_EQUAL(vm4->id, 0x000E);

    vm3->id = 0x000D;
}

void set_id(void) {
    TEST_ASSERT_FALSE(set_extern_id(vm2, ID, vm1->id, 0x000C));
    TEST_ASSERT_EQUAL(vm1->id, 0x000C);
    TEST_ASSERT_EQUAL(vm2->id, 0x000B);
}

void set_id_ack(void) {
    TEST_ASSERT_FALSE(set_extern_id(vm1, IDACK, ctx.type, 0x000D));
    TEST_ASSERT_EQUAL(vm1->id, 0x000D);
    TEST_ASSERT_TRUE(vm1->msg_pt->ack);
}

void write_id_mode(void) {
    msg_t msg = {.header.cmd = TEST_CMD,
                 .header.target = vm1->id,
                 .header.target_mode = ID,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};

    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
    TEST_ASSERT_FALSE(robus_send(vm2, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
    msg.header.target = vm2->id;
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
    TEST_ASSERT_FALSE(robus_send(vm2, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
}

void write_id_mode_no_cb(void) {
    msg_t msg = {.header.cmd = TEST_CMD,
                 .header.target = vm4->id,
                 .header.target_mode = ID,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};

    test_value = 0x0000;
    TEST_ASSERT_FALSE(robus_send(vm2, &msg));
    TEST_ASSERT_EQUAL(test_value, 0x0000);
    TEST_ASSERT_EQUAL(vm4->data_to_read, msg.header.size);
    TEST_ASSERT_TRUE(vm4->message_available > 0);
    int the_value = 0;
    unsigned char i = 0;
    while (vm4->message_available) {
        the_value = (the_value << 8) | robus_read(vm4);
        i++;
    }
    TEST_ASSERT_EQUAL(i, vm4->msg_pt->header.size);
    TEST_ASSERT_TRUE(ctx.status.rx_error);
    TEST_ASSERT_FALSE(vm4->message_available);
    TEST_ASSERT_EQUAL(the_value, 0xCAFE);
}

void write_broadcast_mode(void) {
    msg_t msg = {.header.cmd = TEST_CMD,
                 .header.target = BROADCAST_VAL,
                 .header.target_mode = BROADCAST,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};

    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
}

void add_multicast(void) {
    add_multicast_target(vm1, 0x000E);
    TEST_ASSERT_NOT_EQUAL(vm1->multicast_target_bank[vm1->max_multicast_target], 0x000E);

    msg_t msg = {.header.cmd = TARGET_CMD,
                 .header.target = 0x000E,
                 .header.target_mode = MULTICAST,
                 .header.size = 2,
                 .data[0] = 0xCA,
                 .data[1] = 0xFE};

    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(target_value, 0x000E);
    add_multicast_target(vm1, 0x00AE);
    msg.header.target = 0x00AE;
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(target_value, 0x00AE);
    msg.header.target = 0x000E;
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(target_value, 0x000E);
    target_value = 0x0000;

    msg.header.cmd = TEST_CMD;
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xCAFE);
    test_value = 0x0000;
    add_multicast_target(vm2, 0x000E);
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xFECA);
    test_value = 0x0000;

    add_multicast_target(vm3, 0x00AE);
    msg.header.target = 0x00AE;
    TEST_ASSERT_FALSE(robus_send(vm1, &msg));
    TEST_ASSERT_EQUAL(test_value, 0xEFAC);
    test_value = 0x0000;
}



void process() {
    UNITY_BEGIN();
    
    RUN_TEST(test_init);
    RUN_TEST(add_vm);
    RUN_TEST(add_nocb_vm);
    RUN_TEST(set_id_brdcst);
    RUN_TEST(set_id_type);
    RUN_TEST(set_id);
    RUN_TEST(write_id_mode);
    RUN_TEST(write_id_mode_no_cb);
    RUN_TEST(write_broadcast_mode);
    RUN_TEST(add_multicast);

    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>

void setup() {
  // NOTE!!! Wait for >2 secs
   // if board doesn't support software reset via Serial.DTR/RTS
   delay(2000);

   process();
}

void loop() {
}

#else /* ARDUINO */

int main(int argc, char const *argv[]) {
    process();

    return 0;
}

#endif /* ARDUINO */

#endif /* UNIT_TEST */
