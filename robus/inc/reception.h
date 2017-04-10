/*
 * reception.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: reception state machine header.
 */
 #ifndef _RECEPTION_H_
#define _RECEPTION_H_

#include "context.h"

unsigned short crc(unsigned char* data, unsigned short size);

// Callbacks reception
void get_header(volatile unsigned char *data);
void get_data(volatile unsigned char *data);
void msg_complete(void);

// Callbacks send
void catch_ack(volatile unsigned char *data);


#endif /* _RECEPTION_H_ */
