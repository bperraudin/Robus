/**
 * \file robus.h
 * \brief Robus communication main include file.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 18 Fevrier 2017
 *
 * Include this file to use the robus communication protocol.
 *
 */

#ifndef _ROBUS_H_
#define _ROBUS_H_

#define MAX_ALIAS_SIZE 16
#define MAX_DATA_MSG_SIZE 128
#define MAX_MULTICAST_ADDRESS 1

#include "robus_struct.h"

enum {
    NULLBOARD,
    DEV_BOARD,
    MOTOR_BOARD
}module_list_t;

/**
 * \fn void robus_init(void)
 * \brief Initialisation of the Robus communication lib.
 *
 */
void robus_init(void);

/**
 * \fn void robus_modules_clear(void)
 * \brief Completely reset the list of virtual modules.
 *
 */
void robus_modules_clear(void);

/**
 * \fn vm_t* robus_module_create(RX_CB rx_cb, unsigned char type, unsigned char *alias)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param rx_cb function pointer into the rx callback.
 * \param type type reference of this module hardware.
 * \param alias string (15 caracters max).
 *
 * \return virtual module object pointer.
 *
 */
vm_t* robus_module_create(RX_CB rx_cb, unsigned char type, const char *alias);

/**
 * \fn unsigned char robus_send(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char robus_send(vm_t* vm, msg_t *msg);

/**
 * \fn unsigned char robus_send_sys(vm_t* vm, msg_t *msg)
 * \brief  Send system message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char robus_send_sys(vm_t* vm, msg_t *msg);

/**
 * \fn unsigned char robus_read(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who receive.
 *
 * \return a data
 */
unsigned char robus_read(vm_t* vm);

/**
 * \fn void save_alias(vm_t* vm, char* alias)
 * \brief  Save Alias in EEprom.
 *
 * \param concerned virtual module.
 * \param name string.
 *
 */
void save_alias(vm_t* vm, char* alias);

#endif /* _ROBUS_H_ */
