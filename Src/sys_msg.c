/*
 * sys_msg.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: protocol system message management.
 */
#include <robus.h>
#include "sys_msg.h"
#include "hal.h"
#include <string.h>

/* Specific system mesages :
 * These messages don't follow generic rules of this protocol, there are
 * protocols level messages.
 * Please use it with caution
 */

void send_ack(void) {
    hal_enable_tx();
    hal_disable_rx();
    hal_transmit(&ctx.status.unmap, 1);
    hal_wait_transmit_end();
    hal_enable_rx();
    hal_disable_tx();
    ctx.status.unmap = 0x0F;
}

unsigned char reset_network_detection(vm_t* vm) {
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++){
        reset_PTP(branch);
        ctx.detection.branches[branch] = 0;
    }
    reset_detection();
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    if (robus_send_sys(vm, &msg))
        return 1;

    // Reinit VM id
    for (int i = 0; i< ctx.vm_number; i++) {
        ctx.vm_table[i].id = DEFAULTID;
    }
    return 0;
}

unsigned char set_extern_id(vm_t* vm, target_mode_t target_mode, unsigned short target,
                            unsigned short newid) {
    msg_t msg;

    msg.header.target = target;
    msg.header.target_mode = target_mode;
    msg.header.cmd = WRITE_ID;
    msg.header.size = 2;
    msg.data[1] = newid;
    msg.data[0] = (newid <<8);

    if (robus_send_sys(vm, &msg))
        return 1;
    return 0;
}

unsigned char set_network_baudrate(vm_t* vm, unsigned int baudrate) {
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = SET_BAUDRATE;
    msg.header.size = sizeof(baudrate);
    memcpy(msg.data, &baudrate, sizeof(baudrate));

    if (robus_send_sys(vm, &msg))
        return 1;
    return 0;
}
