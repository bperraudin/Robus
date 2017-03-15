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

#endif /* _HAL_H_ */
