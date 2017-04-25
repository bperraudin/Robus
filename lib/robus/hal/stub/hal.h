#ifndef _HAL_H_
#define _HAL_H_

#include "context.h"

typedef enum {
    // Send start condition
    START,
    // Send data with ACK enable
    DATA,
    // Send data with ACK disable
    DATA_NACK,
    // Send stop condition
    STOP
}com_state_t;

void hal_init(void);

unsigned char hal_transmit(unsigned char* data, unsigned short size);

void send_poke(branch_t branch);
void hal_timeout(int factor);

void set_PTP(branch_t branch);
void reset_PTP(branch_t branch);

#endif /* _HAL_H_ */
