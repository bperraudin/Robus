/*
 * This file contain default configuration of the project.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "mod_list.h"

#define DEFAULTID 0x00
#define PROTOCOL_REVISION 0
#define BROADCAST_VAL 0x0FFF
#define MAX_VM_NUMBER 5
#define MSG_BUFFER_SIZE 5


#ifndef MODULETYPE
    #define MODULETYPE DEV_BOARD
#endif

#ifndef MAINCLOCK
    #define MAINCLOCK 16000000
#endif

#ifndef MAX_TRIES
    #define MAX_TRIES 5
#endif


#endif /* _CONFIG_H_ */
