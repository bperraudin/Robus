#ifndef _HAL_H_
#define _HAL_H_

#include "context.h"

void hal_init(void);

unsigned char hal_transmit(unsigned char* data, unsigned short size);

void set_PTPA(void);
void reset_PTPA(void);

void set_PTPB(void);
void reset_PTPB(void);

#endif /* _HAL_H_ */
