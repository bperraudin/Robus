#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "config.h"
#include "robus.h"
#include "cmd.h"

#define TRUE 1
#define FALSE 0

typedef void (*DATA_CB) (volatile unsigned char *data);

typedef struct {
    unsigned char rx_error : 1;
    unsigned char unexpected_state : 1;
    unsigned char warning : 1;
} status_t;

typedef struct {

    // Variables
    DATA_CB data_cb;    /*!< Data management callback. */
    status_t status;    /*!< Status. */
    unsigned short id;       /*!< Module ID. */
    unsigned char type;     /*!< Module type. */

    //Virtual module management
    vm_t vm_table[MAX_VM_NUMBER];       /*!< Virtual Module table. */
    unsigned char vm_number; /*!< Virtual Module number. */

    //msg allocation management
    msg_t msg[MSG_BUFFER_SIZE];          /*!< Message table (one for each virtual module). */
    unsigned char current_buffer;        /*!< current msg buffer used. */
    unsigned char alloc_msg[MSG_BUFFER_SIZE];            /*!< Message allocation table. */

}context_t;

extern context_t ctx;

#endif /* _CONTEXT_H_ */
