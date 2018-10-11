/*
 * sys_msg.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: protocol system message management header.
 */
 #ifndef _SYS_MSG_H_
#define _SYS_MSG_H_

#include "context.h"

unsigned char  reset_network_detection(vm_t* vm);
unsigned char set_extern_id(vm_t* vm, target_mode_t target_mode, unsigned short target,
                           unsigned short newid);
void send_ack(void);

// unsigned char get_extern_module_type(unsigned short addr, unsigned short *module_type);



#endif /* _SYS_MSG_H_ */
