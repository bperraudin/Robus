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



/**
 * This is the minimal include you will need to use Robus in a module
 * application
 */
#include "robus.h"

#include <avr/io.h>
#include <util/delay.h>


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
void rx_cb(msg_t *msg) {
    /*
     * Add your RX code here.
     */
     if (msg->header.cmd == LED)
         if (msg->data[0])
             PORTB |= (1<<PORTB5);
         if (!msg->data[0])
             PORTB &= ~(1<<PORTB5);

}


/**
 * \fn int main(void)
 * \brief Your main module application process.
 *
 * \return integer
 */
int main(void) {
    vm_t *vm1;
    robus_init();
    //Blink debug
    DDRB |= (1<<DDB5); //Set the 6th bit on PORTB (i.e. PB5) to 1 => output
    // while(1)
    // {
    //     PORTB |= (1<<PORTB5);    //Turn 6th bit on PORTB (i.e. PB5) to 1 => on
    //     _delay_ms(1000);        //Delay for 1000ms => 1 sec
    //     PORTB &= ~(1<<PORTB5);    //Turn 6th bit on PORTB (i.e. PB5) to 0 => off
    //     _delay_ms(1000);        //Delay for 1000ms => 1 sec
    // }

    /*
     * Module management with callback
     */
    // creation
    vm1 = robus_module_create(rx_cb, DEV_BOARD, "alias1");
    // reception have to be managed in "rx_cb" callback.

    /*
     * Add your main code here.
     */
     msg_t msg = {.header.target = vm1->id,
                  .header.cmd = LED,
                  .header.target_mode = ID,
                  .header.size = 1};
     while(1)
     {
        //  PORTB |= (1<<PORTB5);    //Turn 6th bit on PORTB (i.e. PB5) to 1 => on
        //  _delay_ms(1000);        //Delay for 1000ms => 1 sec
        //  PORTB &= ~(1<<PORTB5);    //Turn 6th bit on PORTB (i.e. PB5) to 0 => off
        //  _delay_ms(1000);        //Delay for 1000ms => 1 sec
        msg.data[0] = 1;
        robus_send(vm1, &msg);
        _delay_ms(1000);
        msg.data[0] = 0;
        robus_send(vm1, &msg);
        _delay_ms(1000);
     }

    return 0;
}
