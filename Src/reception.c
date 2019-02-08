/*
 * reception.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: reception state machine
 */
//#define DEBUG
#include <string.h>
#include "reception.h"
#include "hal.h"
#include "cmd.h"
#include "target.h"
#include "sys_msg.h"
#include "time.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#define CURRENTMSG ctx.msg[ctx.current_buffer]
#define CURRENTMODULE ctx.vm_table[ctx.alloc_msg[ctx.current_buffer]]

unsigned char keep = FALSE;
unsigned char concernedmodules[MAX_VM_NUMBER] = {FALSE};
unsigned short data_count = 0;
// static timeout_t timeout; //TODO timeout management

unsigned char module_concerned(header_t* header) {
    unsigned char concerned = FALSE;
    // Find if we are concerned by this message.
    switch (header->target_mode) {
        case ID:
        case IDACK:
            // Default id
            if(header->target == ctx.id) {
                concerned = TRUE;
                ctx.alloc_msg[ctx.current_buffer] = 0;
                ctx.data_cb = get_data;
                break;
            }
            // Check all VM id
            for (int i = 0; i < ctx.vm_number; i++) {
                concerned = (header->target == ctx.vm_table[i].id);
                if (concerned) {
                    ctx.alloc_msg[ctx.current_buffer] = i;
                    ctx.data_cb = get_data;
                    break;
                }
            }
        break;
        case TYPE:
            //check default type
            if(header->target == ctx.type) {
                concerned = TRUE;
                concernedmodules[0] = TRUE;
                ctx.data_cb = get_data;
                break;
            }
            // Check all VM type
            for (int i = 0; i < ctx.vm_number; i++) {
                if (header->target == ctx.vm_table[i].type) {
                    concerned = TRUE;
                    concernedmodules[i] = TRUE;
                    ctx.data_cb = get_data;
                }
            }
        break;
        case BROADCAST:
            concerned = (header->target == BROADCAST_VAL);
            ctx.data_cb = get_data;
            if (concerned) {
                for (int i = 0; i < ctx.vm_number; i++) {
                    concernedmodules[i] = TRUE;
                }
            }
        break;
        case MULTICAST:
            for (int i = 0; i < ctx.vm_number; i++) {
                if (multicast_target_bank(&ctx.vm_table[i], header->target)) { //TODO manage multiple slave concerned
                    concerned = TRUE;
                    concernedmodules[i] = TRUE;
                }
            }
            ctx.data_cb = get_data;
        break;
        default:
            return concerned;
        break;
    }
    return concerned;
}

/**
 * \fn unsigned char crc(unsigned char* data, unsigned char size)
 * \brief generate a CRC
 *
 * \param *data data table
 * \param size data size
 *
 * \return CRC value
 */
unsigned short crc(unsigned char* data, unsigned short size) {
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (size--) {
        x = crc >> 8 ^ *data++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned int)(x << 12))
                         ^ ((unsigned int)(x <<5))
                         ^ ((unsigned int)x);
    }
    return (unsigned short)crc;
}

/**
 * \fn void timeout(flush)
 * \brief manage timeout event
 *
 * \return
 */
void timeout (void) {
    ctx.tx_lock = FALSE;
    flush();
}

/**
 * \fn void flush(flush)
 * \brief reset the reception state machine
 *
 * \return
 */
void flush (void) {
    ctx.data_cb = get_header;
    keep = FALSE;
    data_count = 0;
}

/**
 * \fn void get_header(volatile unsigned char *data)
 * \brief catch a complete header
 *
 * \param *data byte received from serial
 */
void get_header(volatile unsigned char *data) {

    ctx.tx_lock = TRUE;
    // Catch a byte.
    CURRENTMSG.header.unmap[data_count++] = *data;

    // Check if we have all we need.
    if (data_count == (sizeof(header_t))) {

#ifdef DEBUG
        printf("*******header data*******\n");
        printf("protocol : 0x%04x\n", CURRENTMSG.header.protocol);       /*!< Protocol version. */
        printf("target : 0x%04x\n", CURRENTMSG.header.target);        /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
        printf("target_mode : 0x%04x\n", CURRENTMSG.header.target_mode);    /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
        printf("source : 0x%04x\n", CURRENTMSG.header.source);        /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
        printf("cmd : 0x%04x\n", CURRENTMSG.header.cmd);                 /*!< msg definition. */
        printf("size : 0x%04x\n", CURRENTMSG.header.size);                /*!< Size of the data field. */
#endif
        // Reset the catcher.
        data_count = 0;
        // Reset the msg allocation
        ctx.alloc_msg[ctx.current_buffer] = 0;

        keep = module_concerned(&CURRENTMSG.header);
    }
}

/**
 * \fn void get_infos(volatile unsigned char *data)
 * \brief catch data field.
 *
 * \param *data byte received from serial
 */
void get_data(volatile unsigned char *data) {
    CURRENTMSG.data[data_count] = *data;

    if (data_count > CURRENTMSG.header.size) {
        if (keep) {
            CURRENTMSG.crc = ((unsigned short)CURRENTMSG.data[CURRENTMSG.header.size]) |
                                              ((unsigned short)CURRENTMSG.data[CURRENTMSG.header.size + 1] << 8);
            if (CURRENTMSG.crc == crc(CURRENTMSG.stream, CURRENTMSG.header.size + sizeof(header_t))) {
                if (CURRENTMSG.header.target_mode == IDACK) {
                    send_ack();
                }
                ctx.data_cb = get_header;
                //TODO a loop if not ID/IDACK
                if (CURRENTMSG.header.target_mode == ID || CURRENTMSG.header.target_mode == IDACK) {
                    msg_complete(&CURRENTMSG);
                }
                else {
                    if(ctx.vm_number == 0) {
                        // no module created, but save this ID in the void module.
                        msg_complete(&CURRENTMSG);
                    }
                    for (int i = 0; i < ctx.vm_number; i++) {
                        if (concernedmodules[i]) {
                            ctx.alloc_msg[ctx.current_buffer] = i;
                            if (msg_complete(&CURRENTMSG))break;
                            concernedmodules[i] = FALSE;
                        }
                    }
                }
                ctx.current_buffer++;
                if (ctx.current_buffer == MSG_BUFFER_SIZE) {
                    ctx.current_buffer = 0;
                }
            } else
                ctx.status.rx_error = TRUE;
        }
        flush ();
        return;
    }
    data_count++;
}


/**
 * \fn void catch_ack(volatile unsigned char *data)
 * \brief catch ack.
 *
 * \param *data byte received from serial
 */
void catch_ack(volatile unsigned char *data) {
    // Check ACK value.
    CURRENTMSG.ack = *data;
    // Set something (the Xevel thread thing??)...
}

/**
 * \fn void msg_complete()
 * \brief the message is now complete, manage it.
 *
 * \param *data byte received from serial
 */
char msg_complete(msg_t* msg) {
    unsigned int baudrate = 0;
    CURRENTMODULE.message_available++;
    if (msg->header.target_mode == ID ||
        msg->header.target_mode == IDACK ||
        msg->header.target_mode == TYPE ||
        msg->header.target_mode == BROADCAST) {
        switch (msg->header.cmd) {
            case WRITE_ID:
                // Get and save a new given ID
                if ((msg->header.target_mode == BROADCAST) &
                    (ctx.detection.keepline != NO_BRANCH) &
                    (ctx.detection_mode != MASTER_DETECT) &
                    (!ctx.detection.detection_end)) {
                    // We are on topology detection mode, and this is our turn
                    // Save id for the next module we have on this board
                    ctx.vm_table[ctx.detection.detected_vm++].id =
                        (((unsigned short)msg->data[1]) |
                        ((unsigned short)msg->data[0] << 8));

                    // Check if that was the last virtual module
                    if (ctx.detection.detected_vm >= ctx.vm_number) {
                        ctx.detection.detection_end = TRUE;
                        if (ctx.detection.keepline == BRANCH_A) {
                            // check if we have a module on the other side
                            if (!poke(BRANCH_B)) {
                                // no mudule on the other side, free the ptp line
                                reset_PTP(BRANCH_A);
                                reset_detection();
                            }
                        }
                        else if (ctx.detection.keepline == BRANCH_B) {
                            // check if we have a module on the other side
                            if (!poke(BRANCH_A)) {
                                // no mudule on the other side, free the ptp line
                                reset_PTP(BRANCH_B);
                                reset_detection();
                            }
                        }
                    }
                    return 1;
                }
                else if (msg->header.target_mode != BROADCAST) {
                    CURRENTMODULE.id = (((unsigned short)msg->data[1]) |
                                       ((unsigned short)msg->data[0] << 8));
                }
            break;
            case WRITE_ALIAS:
                // Make a clean copy with full \0 at the end.
                memset(CURRENTMODULE.alias, '\0', sizeof(CURRENTMODULE.alias));
                if (msg->header.size > 16) msg->header.size = 16;
                if (msg->header.size == 0) {
                    // This is an alias erase instruction, get back to default one
                    write_alias(ctx.alloc_msg[ctx.current_buffer], '\0');
                    memcpy(CURRENTMODULE.alias, CURRENTMODULE.default_alias, MAX_ALIAS_SIZE);
                } else {
                    memcpy(CURRENTMODULE.alias, msg->data, msg->header.size);
                    write_alias(ctx.alloc_msg[ctx.current_buffer], CURRENTMODULE.alias);
                }
            break;
            case RESET_DETECTION:
                reset_PTP(BRANCH_B);
                reset_PTP(BRANCH_A);
                reset_detection();
            break;
            case GET_ID:
            // call something...
            case GET_MODULE_TYPE:
            // call something...
            case GET_STATUS:
            // call something...
            case GET_FIRM_REVISION:
            // call something...
            break;
            case SET_BAUDRATE:
                memcpy(&baudrate, msg->data, msg->header.size);
                set_baudrate(baudrate);
            break;
            default:
                // set VM data
                CURRENTMODULE.msg_pt = msg;
                msg->header.cmd -= PROTOCOL_CMD_NB;

                // Call CM callback
                if (CURRENTMODULE.rx_cb != 0) {
                    CURRENTMODULE.rx_cb(&CURRENTMODULE, CURRENTMODULE.msg_pt);
                    CURRENTMODULE.message_available--;
                }
                else {
                    CURRENTMODULE.data_to_read = msg->header.size;
                }
            break;
        }
   } else {
        // set VM data
        CURRENTMODULE.msg_pt = msg;

        // Call CM callback
        if (CURRENTMODULE.rx_cb != 0) {
            msg->header.cmd -= PROTOCOL_CMD_NB;
            CURRENTMODULE.rx_cb(&CURRENTMODULE, CURRENTMODULE.msg_pt);
            msg->header.cmd += PROTOCOL_CMD_NB;
            CURRENTMODULE.message_available--;
        }
        else {
            CURRENTMODULE.data_to_read = msg->header.size;
        }
    }
    return 0;
}
