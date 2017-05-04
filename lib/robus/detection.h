/*
 * detection.h
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */

#ifndef _DETECTION_H_
#define _DETECTION_H_

#include "robus.h"

typedef enum {
    BRANCH_A,
    BRANCH_B,
    NO_BRANCH
}branch_t;

typedef enum {
    NO_DETECT,
    MASTER_DETECT
}detection_mode_t;

typedef enum {
    POKE,
    KEEPLINE
}expected_detection_t;

typedef struct __attribute__((__packed__)){
    branch_t keepline; /*!< last keepline status on PTP lines . */
    unsigned char detection_end; /*!< All Virtual Module have ID. */
    unsigned char detected_vm; /*!< Virtual Module number. */
    expected_detection_t expect;
} detection_t;

unsigned char topology_detection(vm_t* vm);
void reset_detection(void);

unsigned char poke(branch_t branch);
void poke_detected(branch_t branch);

void ptp_detected(branch_t branch);
void ptp_released(branch_t branch);
void ptp_handler(branch_t branch);

#endif /* _DETECTION_H_ */
