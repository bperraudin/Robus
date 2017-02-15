/**
 * \file template.c
 * \brief Poppy module application side template.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 22 Avril 2015
 *
 * Please feel free to copy this template and use it at base for your new Poppy
 * module application.
 */



/**
 * This is the minimal include you will need to use poppy_com in a module
 * application
 */
#include "poppyNetwork.h"

/**
 * \enum msg_dir_t
 * \brief Module specific register enumerator.
 *
 * This structure is used to list all the specific module register.
 * The first register should be equal to PROTOCOL_REGISTER_NB, because is the
 * first free register.
 * The last register should be MODULE_REGISTER_NB, for the user space register
 * enumerator...
 */
typedef enum {
    /*
     * Add all you register id here like :
     * FIRST_MODULE_REGISTER = PROTOCOL_REGISTER_NB,
     * SECOND_MODULE_REGISTER,
     * THIRD_MODULE_REGISTER,
     * ...
     */
    MODULE_REGISTER_NB
}module_register_t;

/**
 * \fn void rx_cb(msg_dir_t dir, msg_t *msg)
 * \brief Callback function for Slave mode messages reception.
 *
 * \param dir Message direction. (That will be remove!)
 * \param msg Received message.
 */
void rx_cb(msg_t *msg) {
    /*
     * Add your RX code here.
     */
}


/**
 * \fn int main(void)
 * \brief Your main module application process.
 *
 * \return integer
 */
int main(void) {
    poppyNetwork_init(rx_cb);

    /*
     * Add your main code here.
     */

    return 0;
}
