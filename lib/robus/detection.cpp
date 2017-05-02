/*
 * detection.c
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
#include "robus.h"
#include "sys_msg.h"
#include "hal.h"


unsigned char poke(branch_t branch) {
    if (branch == BRANCH_A) {
        send_poke(BRANCH_A);
    }
    if (branch == BRANCH_B) {
        send_poke(BRANCH_B);
    }

    // Wait a keepline
    // TODO: improve the way we deal with timeout
    for (int i=0; i < 10; i++) {
        if (ctx.detection.keepline == branch) {
            return 1;
        }
        hal_timeout(1);
    }

    return 0;
}

void poke_detected(branch_t branch) {
    set_PTP(branch);
    ctx.detection.keepline = branch;
}

void ptp_detected(branch_t branch) {
    ctx.detection.keepline = branch;
}

unsigned char topology_detection(vm_t* vm) {
    unsigned short newid = 1;

    ctx.detection_mode = MASTER_DETECT;

    // start by parsing internal vm
    for (unsigned char i=0; i < ctx.vm_number; i++) {
        ctx.vm_table[i].id = newid++;
    }

    ctx.detection.detected_vm = ctx.vm_number;
    ctx.detection.detection_end = TRUE;

    if (poke(BRANCH_A)) { // Someone reply to our poke!
        // loop while PTP_A is enabled
        while (ctx.detection.keepline != NO_BRANCH) {
            set_extern_id(vm, BROADCAST, BROADCAST_VAL, newid++);
            hal_timeout(15);
        }
    }

    if (poke(BRANCH_B)) {
        while (ctx.detection.keepline != NO_BRANCH) {
            set_extern_id(vm, BROADCAST, BROADCAST_VAL, newid++);
            hal_timeout(15);
        }
    }

    ctx.detection_mode = NO_DETECT;

    return 0;
}

void reset_detection(void) {
  ctx.detection.keepline = NO_BRANCH;
  ctx.detection.detection_end = 0;
  ctx.detection.detected_vm = 0;

  ctx.detection_mode = NO_DETECT;
}

void ptp_released(branch_t branch) {
    ctx.detection.keepline = NO_BRANCH;

    if (ctx.detection_mode != MASTER_DETECT && ctx.detection.detection_end) {
        if (branch == BRANCH_A) {
            reset_PTP(BRANCH_B);
        }
        else if (branch == BRANCH_B) {
            reset_PTP(BRANCH_A);
        }
    }
}
