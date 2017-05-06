/*
 * robus.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: nico
 *  Abstract: User functionalities of the robus communication protocol
 */
#include "robus.h"
#include "sys_msg.h"
#include "reception.h"
#include "context.h"
#include "hal.h"
#include "cmd.h"

// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;

// Startup and network configuration
void robus_init(void) {
    // Init the number of created  virtual module.
    ctx.vm_number = 0;
    // Initialize the reception state machine
    ctx.data_cb = get_header;
    // Set default module id. This id is a void id used if no module is created.
    ctx.id = DEFAULTID;
    // VOID Module type
    ctx.type = NULLBOARD;
    // no transmission lock
    ctx.tx_lock = FALSE;

    // init detection structure
    // No detection already done
    ctx.detection.detection_end = FALSE;
    // init PTP line detection status
    ctx.detection.keepline = NO_BRANCH;
    ctx.detection.expect = POKE;
    ctx.detection_mode = NO_DETECT;

    // Clear message allocation buffer table
    for (int i = 0; i < MSG_BUFFER_SIZE; i++) {
        ctx.alloc_msg[i] = 0;
    }
    // Initialize the start case of the message buffer
    ctx.current_buffer = 0;
    // Initialize the robus modul status
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
    // Link the VM to his callback
    ctx.vm_table[ctx.vm_number].rx_cb = rx_cb;
    // Set the module type
    ctx.vm_table[ctx.vm_number].type = type;
    // Initialise the module type, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    // Initialise the module alias, TODO the alias could be stored in EEprom...
    for (i=0; i < MAX_ALIAS_SIZE-1; i++) {
        ctx.vm_table[ctx.vm_number].alias[i] = alias[i];
        if (ctx.vm_table[ctx.vm_number].alias[i] == '\0')
            break;
    }
    ctx.vm_table[ctx.vm_number].alias[i] = '\0';
    // Initialize the available message counter.
    ctx.vm_table[ctx.vm_number].message_available = 0;
    // Initialise the number of data available into the buffer.
    ctx.vm_table[ctx.vm_number].data_to_read = 0;
    // Clear the msg allocation table.
    for (i=0; i < MSG_BUFFER_SIZE; i++) {
        ctx.alloc_msg[i] = 0;
    }
    // Return the freshly initialized vm pointer.
    return &ctx.vm_table[ctx.vm_number++];
}


unsigned char robus_send_sys(vm_t* vm, msg_t *msg) {
    // Compute the full message size based on the header size info.
    unsigned short full_size = sizeof(header_t) + msg->header.size;
    unsigned short crc_val = 0;
    // Set protocol revision and source ID on the message
    msg->header.protocol = PROTOCOL_REVISION;
    msg->header.source = vm->id;
    // compute the CRC
    crc_val = crc(msg->stream, full_size);
    // Add the CRC to the total size of the message
    full_size += 2;
    // Write the CRC into the message.
    msg->data[msg->header.size] = (unsigned char)crc_val;
    msg->data[msg->header.size + 1] = (unsigned char)(crc_val >> 8);
    // wait tx unlock
    while(ctx.tx_lock) {
        hal_delay_ms(1);
    }
    // Send message
    if (hal_transmit(msg->stream, full_size))
        return 1;
    // Check if ACK needed
    if (msg->header.target_mode == IDACK) {
        // ACK needed, change the state of state machine for wait a ACK
        ctx.data_cb = catch_ack;
        // Clear the ack value
        vm->msg_pt->ack = 0;
        // TODO
        // We could use the waituntil from the Xevel lib
        // In the same time if we don't have to do anything else we just can wait here...
        while (!vm->msg_pt->ack);
    }
    return 0;
}

unsigned char robus_send(vm_t* vm, msg_t *msg) {
    msg->header.cmd += PROTOCOL_CMD_NB;
    unsigned char ret = robus_send_sys(vm, msg);
    msg->header.cmd -= PROTOCOL_CMD_NB;
    return ret;
}

unsigned char robus_read(vm_t* vm) {
    unsigned char data = 0;
    if (vm->message_available > 1) {
        ctx.status.rx_error = TRUE;
    }
    if (vm->message_available) {
        if (vm->data_to_read > 0) {
            data = vm->msg_pt->data[vm->msg_pt->header.size - vm->data_to_read--];
            if (vm->data_to_read == 0) {
                vm->message_available = 0;
            }
        }
        else {
            vm->message_available = 0;
        }
    }
    return data;
}
