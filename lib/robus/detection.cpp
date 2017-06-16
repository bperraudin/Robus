/*
 * detection.c
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
#include "robus.h"
#include "sys_msg.h"
#include "hal.h"

/**
 * \fn ptp_handler(branch_t branch)
 * \brief all ptp interrupt handler
 *
 * \param branch branch id
 */
void ptp_handler(branch_t branch) {
    if (ctx.detection.expect == RELEASE) {
        // the line was released
        ctx.detection.keepline = NO_BRANCH;
        ctx.detection.expect = POKE;

        if (ctx.detection_mode != MASTER_DETECT && ctx.detection.detection_end) {
            if (branch == BRANCH_A) {
                reset_PTP(BRANCH_B);
            }
            else if (branch == BRANCH_B) {
                reset_PTP(BRANCH_A);
            }
            reset_detection();
        }
    }
    else if (ctx.detection.expect == POKE) {
        // we receive a poke, pull the line to notify your presence
        set_PTP(branch);
        ctx.detection.keepline = branch;
    }
}

/**
 * \fn unsigned char poke(branch_t branch)
 * \brief detect the next module
 *
 * \param branch branch id
 * \return 1 if there is someone, 0 if not
 */
unsigned char poke(branch_t branch) {
    // pull the ptp line
    set_PTP(branch);
    // wait a little just to be sure everyone can read it
    hal_delay_ms(1);
    // release the ptp line
    reset_PTP(branch);
    // read the line state
    if (get_PTP(branch)) {
        // Someone reply, reverse the detection to wake up on line release
        reverse_detection(branch);
        ctx.detection.expect = RELEASE;
        ctx.detection.keepline = branch;
        return 1;
    }
    return 0;
}

/**
 * \fn unsigned char topology_detection(vm_t* vm)
 * \brief start the detection procedure
 *
 * \param *vm virtual module who start the detection
 * \return return the number of detected module
 */
unsigned char topology_detection(vm_t* vm) {
    unsigned short newid = 1;
    ctx.detection_mode = MASTER_DETECT;

    // start by parsing internal vm
    for (unsigned char i=0; i < ctx.vm_number; i++) {
        ctx.vm_table[i].id = newid++;
    }

    ctx.detection.detected_vm = ctx.vm_number;
    ctx.detection.detection_end = TRUE;

    if (poke(BRANCH_A)) {
        // Someone reply to our poke!
        // loop while PTP_A is released
        while (ctx.detection.keepline != NO_BRANCH) {
            set_extern_id(vm, BROADCAST, BROADCAST_VAL, newid++);
            hal_delay_ms(2);
        }
    }
    if (poke(BRANCH_B)) {
        // Someone reply to our poke!
        // loop while PTP_B is released
        while (ctx.detection.keepline != NO_BRANCH) {
            set_extern_id(vm, BROADCAST, BROADCAST_VAL, newid++);
            hal_delay_ms(2);
        }
    }

    ctx.detection_mode = NO_DETECT;

    return newid - 1;
}

/**
 * \fn void reset_detection(void)
 * \brief reinit the detection state machine
 *
 * \param *vm virtual module who start the detection
 * \return return the number of detected module
 */
void reset_detection(void) {
  ctx.detection.keepline = NO_BRANCH;
  ctx.detection.detection_end = 0;
  ctx.detection.detected_vm = 0;
  ctx.detection_mode = NO_DETECT;
}
