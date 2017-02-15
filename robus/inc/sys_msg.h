#ifndef _SYS_MSG_H_
#define _SYS_MSG_H_

#include "context.h"

unsigned char set_extern_id(vm_t* vm, target_mode_t target_mode, unsigned short target,
                           unsigned short newid);
void send_ack(void);
// unsigned char get_extern_module_type(unsigned short addr, unsigned short *module_type);



#endif /* _SYS_MSG_H_ */
