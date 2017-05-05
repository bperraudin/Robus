/**
 * \file template.c
 * \brief Robus module application side template.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 22 Avril 2015
 *
 * Please feel free to copy this template and use it at base for your new Robus module
 * module application.
 */

#ifndef UNIT_TEST
#define MASTER_MODULE

/**
 * This is the minimal include you will need to use Robus in a module
 * application
 */
#include "robus.h"
#include "sys_msg.h"

#include <avr/io.h>
#include <util/delay.h>

volatile int pub = 0;

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
    LED,
    MODULE_REGISTER_NB
}module_register_t;

/**
 * \fn void rx_cb(msg_dir_t dir, msg_t *msg)
 * \brief Callback function for Slave mode messages reception.
 *
 * \param dir Message direction. (That will be remove!)
 * \param msg Received message.
 */

msg_t msg;
vm_t *vm1;

void rx_cb(msg_t *plop) {
    /*
     * Add your RX code here.
     */
     if (plop->header.cmd == LED) {
         if (plop->data[0]) {
             PORTB |= (1<<PORTB5);
             msg.data[0] = 1;
           }
         if (!plop->data[0]) {
             PORTB &= ~(1<<PORTB5);
             msg.data[0] = 0;
           }
      msg.header.target = plop->header.source;
      // #ifndef MASTER_MODULE
      // robus_send(vm1, &msg);
      // #endif
      pub = 1;
    }
}


/**
 * \fn int main(void)
 * \brief Your main module application process.
 *
 * \return integer
 */
int main(void) {
    robus_init();
    //Blink debug
    DDRB |= (1<<DDB5); //Set the 6th bit on PORTB (i.e. PB5) to 1 => output
#ifndef MASTER_MODULE
    DDRD &= ~(1<<DDD6) & ~(1<<DDD7); // Set D6 and D7 to highZ
#endif
#ifdef MASTER_MODULE
    DDRD |= (1<<DDD6) | (1<<DDD7); // Set D6 and D7 to highZ
    PORTD |= (1<<PORTD6); // Set pullA up
    PORTD &= ~(1<<PORTD7); // Set pullB down
#endif

    /*
     * Module management with callback
     */
    // creation
    vm1 = robus_module_create(rx_cb, DEV_BOARD, "alias1");
    // start topology detection
#ifdef MASTER_MODULE
    _delay_ms(1000);
    topology_detection(vm1);
    // vm1->id = 1;
#endif
  // vm1->id = 2;
    /*
     * Add your main code here.
     */

     msg.header.cmd = LED;
     msg.header.target_mode = ID;
     msg.header.size = 1;

     while(1)
     {
    #ifdef MASTER_MODULE
        for (int i = 2; i<5; i++) {
            msg.header.target = i;
            msg.data[0] = 1;
            robus_send(vm1, &msg);
            _delay_ms(10);
            msg.data[0] = 0;
            robus_send(vm1, &msg);
            _delay_ms(10);
        }
    #else
    if (pub) {
      robus_send(vm1, &msg);
      pub = 0;
    }
    #endif
     }

    return 0;
}

#endif /* UNIT_TEST */
