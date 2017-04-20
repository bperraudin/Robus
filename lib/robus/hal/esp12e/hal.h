#ifndef HAL_ESP12E
#define HAL_ESP12E

void hal_init(void);

unsigned char hal_transmit(unsigned char *data, unsigned short size);

#endif /* HAL_ESP12E */
