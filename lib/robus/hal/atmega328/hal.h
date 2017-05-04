#ifndef _HAL_H_
#define _HAL_H_

#include "context.h"
#include "detection.h"

void hal_init(void);

unsigned char hal_transmit(unsigned char* data, unsigned short size);

void send_poke(branch_t branch);
void hal_timeout(int factor);

void set_PTP(branch_t branch);
void reset_PTP(branch_t branch);
void reverse_detection(branch_t branch);

#endif /* _HAL_H_ */
