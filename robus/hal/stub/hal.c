#include "hal.h"
#include "reception.h"
#include <stdio.h>

// Global variables
context_t ctx;


/**
 * \fn unsigned char hal_transmit(unsigned char* data)
 * \brief write a data byte
 *
 * \param data *data bytes to send
 * \param size size of data to send in byte
 *
 * \return error
 */
unsigned char hal_transmit(unsigned char* data, unsigned short size) {
   for (unsigned short i = 0; i < size; i++)
   { 
        //printf("0x%02x ", *data);
        ctx.data_cb(data++);
   }
    //printf("\n");
    return 0;
}

void hal_init(void) {
    // I2C configuration
}

/**
 * \fn void id_update(unsigned short id)
 * \brief update the slave address
 *
 * \param id new address
 *
 */
void id_update(unsigned short id) {
    ctx.id = id;
    /*
     * TORO(NR) : Write this ID on EEPROM and use it as default at
     *            reboot (do this after debug)
    */
}

/**
 * \fn void alias_update(unsigned char id[16])
 * \brief update the slave address
 *
 * \param alias new address
 *
 */
/*void alias_update(unsigned char alias[16]) {
    for (unsigned char i=0; i < 16; i++)
        ctx.alias[i] = alias[i];
    // Write your alias on EEprom
}*/

