/*
 * robus.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: nico
 *  Abstract: basics functionalities of the robus communication protocol
 */
#include "robus.h"
#include "sys_msg.h"
#include "reception.h"
#include "context.h"
#include "hal.h"


context_t ctx;

// Startup and network configuration
void robus_init(void) {

    //init the virtual module number
    ctx.vm_number = 0;

    // Data callback
    ctx.data_cb = get_header;

    // Module id
    ctx.id = DEFAULTID;
    // Module type
    ctx.type = NULLBOARD;

    // clear alloc_msg
    for (int i = 0; i < MSG_BUFFER_SIZE; i++) {
        ctx.alloc_msg[i] = -1;
    }
    ctx.current_buffer = 0;

    // Status
    ctx.status = (status_t) {.rx_error = FALSE,
                             .unexpected_state = FALSE,
                             .warning = FALSE};
  // Init hal
    hal_init();
}

vm_t* robus_module_create(RX_CB rx_cb, unsigned char type, const char *alias) {
    unsigned char i = 0;

    // module reception callback
    /*if (++ctx.vm_number >= MAX_VM_NUMBER) // TODO Do it using compilation message error
        return 0;
    */
    ctx.vm_table[ctx.vm_number].rx_cb = rx_cb;
    ctx.vm_table[ctx.vm_number].type = type;
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    for (i=0; i < MAX_ALIAS_SIZE-1; i++) {
        ctx.vm_table[ctx.vm_number].alias[i] = alias[i];
        if (ctx.vm_table[ctx.vm_number].alias[i] == '\0')
            break;
    }
    ctx.vm_table[ctx.vm_number].alias[i] = '\0';

    for (i=0; i < MAX_VM_NUMBER; i++) {
        ctx.alloc_msg[i] = 0;
    }
    return &ctx.vm_table[ctx.vm_number++];
}

unsigned char robus_send(vm_t* vm, msg_t *msg) {

    // unsigned char* data = (unsigned char*)msg;
    unsigned short full_size = sizeof(header_t) + msg->header.size;
    unsigned short crc_val = 0;

    msg->header.protocol = PROTOCOL_REVISION;
    msg->header.source = vm->id;
    crc_val = crc(msg->stream, full_size);
    full_size+=2;
    msg->data[msg->header.size] = (unsigned char)crc_val;
    msg->data[msg->header.size + 1] = (unsigned char)(crc_val >> 8);

    // Start and send Target field
    if (hal_transmit(msg->stream, full_size))
        return 1;
    if (msg->header.target_mode == IDACK) {
        ctx.data_cb = catch_ack;
        vm->msg_pt->ack = 0;
        // TODO
        // We could use the waituntil from the Xevel lib
        // In the same time if we don't have to do anything else we just can wait here...
        while (!vm->msg_pt->ack);
    }
    return 0;
}
