#include "hal.h"


// Global variables
context_t ctx;

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
    // init all things
}

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
    if /*hardware ready*/ {
        // Put data into a DMA => serial interface or do it with a for
    } else {
        ctx.status.master_write = TRUE;
        return 1;
    }
    return 0;
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
    /*address register*/ = (ctx.id << 1);
    // Write your ID on EEprom
}


/**
 * \fn void alias_update(unsigned char id[16])
 * \brief update the slave address
 *
 * \param alias new address
 *
 */
void alias_update(unsigned char alias[16]) {
    for (unsigned char i=0; i < 16; i++)
        ctx.alias[i] = alias[i];
    // Write your alias on EEprom
}
