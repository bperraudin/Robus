/*
 * robus.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: nico
 *  Abstract: User functionalities of the robus communication protocol
 */
#include <robus.h>
#include <string.h>
#include "sys_msg.h"
#include "reception.h"
#include "context.h"
#include "hal.h"
#include "cmd.h"

// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;

unsigned char transmit(unsigned char* data, unsigned short size);

// Startup and network configuration
void robus_init(RX_CB callback) {
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
    // Save luos callback
    ctx.luos_cb = callback;

    // init detection structure
    reset_detection();

    // Clear message allocation buffer table
    for (int i = 0; i < MSG_BUFFER_SIZE; i++) {
        ctx.alloc_msg[i] = 0;
    }
    // Initialize the start case of the message buffer
    ctx.current_buffer = 0;
    // Initialize the robus module status
    ctx.status.unmap = 0;
    ctx.status.identifier = 0xF;
  // Init hal
    hal_init();
}

void robus_modules_clear(void) {
    // Clear vm table
    memset(ctx.vm_table, 0, sizeof(vm_t) * MAX_VM_NUMBER);
    // Reset the number of created modules
    ctx.vm_number = 0;
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
    // Initialise the module id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    // Save default alias
    for (i=0; i < MAX_ALIAS_SIZE-1; i++) {
        ctx.vm_table[ctx.vm_number].default_alias[i] = alias[i];
        if (ctx.vm_table[ctx.vm_number].default_alias[i] == '\0')
            break;
    }
    ctx.vm_table[ctx.vm_number].default_alias[i] = '\0';
    // Initialise the module alias to 0
    memset((void *)ctx.vm_table[ctx.vm_number].alias, 0, sizeof(ctx.vm_table[ctx.vm_number].alias));
    if (!read_alias(ctx.vm_number, (char *)ctx.vm_table[ctx.vm_number].alias)) {
        // if no alias saved keep the default one
        for (i=0; i < MAX_ALIAS_SIZE-1; i++) {
            ctx.vm_table[ctx.vm_number].alias[i] = alias[i];
            if (ctx.vm_table[ctx.vm_number].alias[i] == '\0')
                break;
        }
        ctx.vm_table[ctx.vm_number].alias[i] = '\0';
    }
    // Initialize the available message counter.
    ctx.vm_table[ctx.vm_number].message_available = 0;
    // Initialise the number of data available into the buffer.
    ctx.vm_table[ctx.vm_number].data_to_read = 0;
    // Clear the msg allocation table.
    for (i=0; i < MSG_BUFFER_SIZE; i++) {
        ctx.alloc_msg[i] = 0;
    }
    // Initialize dead module detection
    ctx.vm_table[ctx.vm_number].dead_module_spotted = 0;
    // Return the freshly initialized vm pointer.
    return &ctx.vm_table[ctx.vm_number++];
}

static void wait_tx_unlock(void) {
    volatile int timeout = 0;
    while(ctx.tx_lock && (timeout < 64000)) {
        timeout++;
    }
}

unsigned char robus_send_sys(vm_t* vm, msg_t *msg) {
    // Compute the full message size based on the header size info.
    unsigned short full_size = sizeof(header_t) + msg->header.size;
    unsigned short crc_val = 0;
    unsigned char nbr_nak_retry = 0;
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
    ctx.vm_last_send = vm;
    ack_restart :
    nbr_nak_retry++;
    hal_disable_irq();
    ctx.ack = FALSE;
    hal_enable_irq();
    // Send message
    while (transmit(msg->stream, full_size)) {
        // There is a collision
        hal_disable_irq();
        // switch reception in header mode
        ctx.data_cb = get_header;
        hal_enable_irq();
        // wait timeout of collided packet
        wait_tx_unlock();
        // timer proportional to ID
        for (volatile unsigned int tempo = 0; tempo < (COLLISION_TIMER * (vm->id -1)); tempo++);
    }
    // Check if ACK needed
    if (msg->header.target_mode == IDACK) {
        // Check if it is a localhost message
        if (module_concerned(&msg->header)) {
            send_ack();
            ctx.ack = 0;
        } else {
            // ACK needed, change the state of state machine for wait a ACK
            ctx.data_cb = catch_ack;
            volatile int time_out = 0;
            while (!ctx.ack & (time_out < 60)){
                time_out++;
            }
            status_t status;
            status.unmap = vm->msg_pt->ack;
            if ((!ctx.ack) | (status.rx_error) | (status.identifier != 0xF)) {
                if (ctx.ack && status.identifier != 0xF) {
                    // This is probably a part of another message
                    // Send it to header
                    ctx.data_cb = get_header;
                    get_header(&vm->msg_pt->ack);
                }
                if (nbr_nak_retry < 10) {
                    timeout();
                    goto ack_restart;
                } else {
                    // Set the dead module ID into the VM
                    vm->dead_module_spotted = msg->header.target;
                }
            }
            ctx.ack = 0;
        }
    }
    // localhost management
    if (module_concerned(&msg->header)) {
        msg_complete(msg);
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

void save_alias(vm_t* vm, char* alias) {
    int i;
    // Get vm number
    for (i=0; i<MAX_VM_NUMBER; i++) {
        if (vm == &ctx.vm_table[i]) break;
    }
    write_alias(i, alias);
}

unsigned char transmit(unsigned char* data, unsigned short size) {
    const int col_check_data_num = 5;
    // wait tx unlock
    wait_tx_unlock();
    hal_disable_irq();
    // re-lock the transmission
    ctx.tx_lock = TRUE;
    // switch reception in collision detection mode
    ctx.data_cb = get_collision;
    ctx.tx_data = data;
    hal_enable_irq();
    // Enable TX
    hal_enable_tx();
    // Try to detect a collision during the 4 first octets
    if (hal_transmit(data, col_check_data_num)) {
        hal_disable_tx();
        return 1;
    }
    // No collision occure, stop collision detection mode and continue to transmit
    hal_disable_irq();
    ctx.data_cb = get_header;
    hal_enable_irq();
    hal_disable_rx();
    hal_transmit(data + col_check_data_num, size-col_check_data_num);
    hal_wait_transmit_end();
    // Force Usart Timeout
    timeout();
    // disable TX and Enable RX
    hal_enable_rx();
    hal_disable_tx();
    return 0;
}
