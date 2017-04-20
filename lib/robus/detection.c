/*
 * detection.c
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
#include "robus.h"
#include "sys_msg.h"
#include "hal.h"

unsigned char poke (branch_t branch) {
    if (branch == BRANCHA) {
        // enable PTPA
        set_PTPA();
        // TEMPO?
        // disable PTPA
        reset_PTPA();
        // Wait a keeplineA
    }
    if (branch == BRANCHB) {
        // enable PTPB
        set_PTPB();
        // TEMPO?
        // disable PTPB
        reset_PTPB();
    }
    int tempo=0;
    // Wait a keepline
    while (ctx.detection.keepline == branch || tempo == tempo_val) {tempo++;} // TODO a better timeout

    if (ctx.detection_mode == branch)
        return 0;
    return 1;

}

void ptp_detect(branch_t branch) {
    /*
     * Check if all our virtuals modules are all discovered.
     * if not that was just a poke, send back a keepline.
     * anyway you have to flag keepline on PTPB.
     */
    //tempo
    if (!ctx.detection.detection_end) {
        if (branch == BRANCHA) {
            set_PTPA();
        }
        if (branch == BRANCHB) {
            set_PTPB();
        }
    }
    ctx.detection.keepline = branch
}

unsigned char topology_detection(vm_t* vm) {
    int tempo=0;
    const int tempo_val = 1000;
    unsigned short newid = 1;
    ctx.detection_mode = MASTER_DETECT;
    // start by parsing internal vm
    ctx.detection.detection_end = TRUE;
    // send a poke on PTPA
    set_PTPA();
    reset_PTPA();
    // wait PTPA == 1 (with a timeout)
    tempo=0;
    while (ctx.detection_mode == PTPA_DETECT || tempo == tempo_val) {tempo++;} // TODO a better timeout
    if (ctx.detection_mode == PTPA_DETECT) { // Something reply to our poke!
        // loop while PTPA is enable
        while (ctx.detection_mode != NO_DETECT) {
            // send addresse
            set_extern_id(vm, BROADCAST, BROADCAST_VAL, newid++);
            // Get info from the new detected module
        }
      }
    // send a poke on PTPB
    set_PTPB();
    reset_PTPB();
    // wait PTPB == 1 (with a timeout)
    tempo=0;
    while (ctx.detection_mode == PTPB_DETECT || tempo == tempo_val) {tempo++;} // TODO a better timeout
    if (ctx.detection_mode == PTPB_DETECT) {
    // loop while PTPB == 1
        // send addresse
        // Get info from the new detected module
    }
}
