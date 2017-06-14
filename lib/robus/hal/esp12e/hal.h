#ifndef HAL_ESP12E
#define HAL_ESP12E

#include "context.h"

void hal_init(void);

unsigned char hal_transmit(unsigned char *data, unsigned short size);

void hal_delay_ms(int factor);

void set_PTP(branch_t branch);
void reset_PTP(branch_t branch);
unsigned char get_PTP(branch_t branch);
void reverse_detection(branch_t branch);


#endif /* HAL_ESP12E */
