/*
 * detection.c
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
#include <robus.h>
#include "sys_msg.h"
#include "hal.h"

#define TIMERVAL 500

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
        ctx.detection.detection_end = 0;
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
    for (volatile unsigned int i = 0; i < TIMERVAL; i++);
    // release the ptp line
    reset_PTP(branch);
    for (volatile unsigned int i = 0; i < TIMERVAL; i++);
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
    // Reset all detection state of modules on the network
    reset_network_detection(vm);
    ctx.detection_mode = MASTER_DETECT;
    // wait a bit
    for (volatile unsigned int i = 0; i < TIMERVAL; i++);

    // setup sending vm
    vm->id = newid++;

    // Parse internal vm other than the sending one
    for (unsigned char i=0; i < ctx.vm_number; i++) {
        if (&ctx.vm_table[i] != vm) {
            ctx.vm_table[i].id = newid++;
        }
    }

    ctx.detection.detected_vm = ctx.vm_number;
    ctx.detection.detection_end = TRUE;

    if (poke(BRANCH_A)) {
        // Someone reply to our poke!
        // loop while PTP_A is released
        int module_number = 0;
        while (ctx.detection.keepline != NO_BRANCH) {
            if (module_number++ > 500) {
                break;
            }
            if (set_extern_id(vm, IDACK, DEFAULTID, newid++)) {
                break;
            }
            for (volatile unsigned int i = 0; i < (TIMERVAL * 4); i++);
        }
    }
    ctx.detection_mode = MASTER_DETECT;
    if (poke(BRANCH_B)) {
        // Someone reply to our poke!
        // loop while PTP_B is released
        int module_number = 0;
        while (ctx.detection.keepline != NO_BRANCH) {
            if (module_number++ > 500) {
                break;
            }
            if (set_extern_id(vm, IDACK, DEFAULTID, newid++)) {
                break;
            }
            for (volatile unsigned int i = 0; i < (TIMERVAL * 4); i++);
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
  ctx.detection.detected_vm = 0;
  ctx.detection_mode = NO_DETECT;
  ctx.detection.expect = POKE;
}
