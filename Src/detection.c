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
        // Check if every line have been poked and poke it if not
        for (int branch = 0; branch < NO_BRANCH; branch++){
            if (ctx.detection.branches[branch] == 0){
                // this branch have not been detected
                if (poke(branch)) {
                    //we get someone, go back to let the detection continue.
                    return;
                }
            }
        }
        // if it is finished reset all lines
        if (ctx.detection_mode != MASTER_DETECT && ctx.detection.detection_end) {
            for (int branch = 0; branch < NO_BRANCH; branch++) {
                reset_PTP(branch);
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
        // enable activ branch to get the next ID and save it into this branch number.
        ctx.detection.activ_branch = branch;
        return 1;
    } else {
        // Nobodies reply to our poke
        // Save branch as empty
        ctx.detection.branches[branch] = 0xFFFF;
    }
    return 0;
}

/**
 * \fn void poke_next_branch(void)
 * \brief find the next branch to poke and poke it
 */
void poke_next_branch(void){
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++) {
        if (ctx.detection.branches[branch] == 0) {
            // this branch have not been poked
            if (poke(branch)) {
                return;
            } else {
                // nobody is here
                ctx.detection.branches[branch] = 0xFFFF;
            }
        }
    }
    // no more branch need to be poked
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++) {
        reset_PTP(branch);
    }
    reset_detection();
    return;
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

    for (unsigned char branch = 0 ; branch < NO_BRANCH; branch++) {
        ctx.detection_mode = MASTER_DETECT;
        if (poke(branch)) {
            // Someone reply to our poke!
            // loop while the line is released
            int module_number = 0;
            while ((ctx.detection.keepline != NO_BRANCH) & (module_number < 1024)) {
                if (set_extern_id(vm, IDACK, DEFAULTID, newid++)) {
                    // set extern id fail
                    // remove this id and stop topology detection
                    newid--;
                    break;
                }
                module_number++;
                for (volatile unsigned int i = 0; i < (TIMERVAL * 4); i++);
            }
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
  ctx.detection.activ_branch = NO_BRANCH;
}
