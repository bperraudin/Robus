#include "hal.h"
#include "reception.h"
#include <stdio.h>

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

void send_poke(branch_t branch) {

}

void hal_delay_ms(int factor) {

}

void set_PTP(branch_t branch) {

}

void reset_PTP(branch_t branch) {

}

void reverse_detection(branch_t branch) {
  
}
