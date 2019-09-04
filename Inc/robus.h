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

#include "robus_struct.h"

/**
 * \fn void robus_init(RX_CB)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param Luos function pointer into the rx callback interrupt.
 */
void robus_init(RX_CB callback);

/**
 * \fn void robus_modules_clear(void)
 * \brief Completely reset the list of virtual modules.
 *
 */
void robus_modules_clear(void);

/**
 * \fn vm_t* robus_module_create(unsigned char type, unsigned char *alias)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param type module type.
 * \param alias string (15 caracters max).
 *
 * \return virtual module pointer.
 *
 */
vm_t* robus_module_create(unsigned char type, const char *alias);

/**
 * \fn unsigned char robus_send(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send.
 *
 * \return sent or not
 */
unsigned char robus_send(vm_t* vm, msg_t *msg);

/**
 * \fn unsigned char robus_send_sys(vm_t* vm, msg_t *msg)
 * \brief  Send Luos management messages.
 *
 * \param virtual module who send.
 * \param msg Message to send.
 *
 * \return sent or not
 */
unsigned char robus_send_sys(vm_t* vm, msg_t *msg);

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
