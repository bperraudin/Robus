#ifndef _HAL_H_
#define _HAL_H_

#include "context.h"

typedef enum {
    NO_SIDE,
    LEFT_SIDE,
    RIGHT_SIDE,
} side_t;

void hal_init(void);

unsigned char hal_transmit(unsigned char* data, unsigned short size);

void send_poke(branch_t branch);
void hal_timeout(int factor);

void set_PTP(branch_t branch);
void reset_PTP(branch_t branch);

// Mockup specific functions

void plug(side_t side);

#endif /* _HAL_H_ */
