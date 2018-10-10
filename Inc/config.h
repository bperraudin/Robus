/*
 * config.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: default configuration of the project.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEFAULTID 0x00
#define PROTOCOL_REVISION 0
#define BROADCAST_VAL 0x0FFF
#ifndef MAX_VM_NUMBER
    #define MAX_VM_NUMBER 20
#endif
#ifndef MSG_BUFFER_SIZE
    #define MSG_BUFFER_SIZE 5
#endif


#ifndef MODULETYPE
    #define MODULETYPE DEV_BOARD
#endif

#endif /* _CONFIG_H_ */
