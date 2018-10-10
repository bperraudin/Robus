#ifndef HAL_ATMEGA10
#define HAL_ATMEGA10

#include "context.h"

void hal_init(void);

unsigned char hal_transmit(unsigned char *data, unsigned short size);

void hal_delay_ms(int factor);

void set_PTP(branch_t branch);
void reset_PTP(branch_t branch);
unsigned char get_PTP(branch_t branch);
void reverse_detection(branch_t branch);
void write_alias(unsigned short id, char* alias);
char read_alias(unsigned short id, char* alias);


#endif /* HAL_ATMEGA10 */