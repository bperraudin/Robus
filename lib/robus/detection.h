/*
 * detection.h
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
 #ifndef _DETECTION_H_
#define _DETECTION_H_

typedef enum {
    NO_DETECT,
    BRANCHA,
    BRANCHB
}branch_t;

typedef struct __attribute__((__packed__)){
    branch_t keepline, /*!< last keepline status on PTP lines . */
    unsigned char detection_end; /*!< All Virtual Module have ID. */
    unsigned char vm_number; /*!< Virtual Module number. */
}detection_t;

unsigned char topology_detection(vm_t* vm);

unsigned char poke (branch_t branch);
void ptp_detect(branch_t branch);

#endif /* _DETECTION_H_ */
