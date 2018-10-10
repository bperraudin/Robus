#include "hal.h"
#include "reception.h"

#include "stm32f0xx_ll_usart.h"
#include "usart.h"
#include "gpio.h"
#include "stm32f0xx_hal.h"
#include "main.h"
#include "eeprom.h"
#include <stdio.h>

#define TIMEOUT_VAL 1

uint16_t VirtAddVarTab[NB_OF_VAR] = { 0 };

/**
 * \fn void USART1_IRQHandler(void)
 * \brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
 *
 */
void USART1_IRQHandler(void)
{
	// check if we receive a data
	if((LL_USART_IsActiveFlag_RXNE(USART1) != RESET) && (LL_USART_IsEnabledIT_RXNE(USART1) != RESET))
	{
		uint8_t data = LL_USART_ReceiveData8(USART1);
		ctx.data_cb(&data); // send reception byte to state machine
	}
	// Check if a timeout on reception occure
	if((LL_USART_IsActiveFlag_RTO(USART1) != RESET) && (LL_USART_IsEnabledIT_RTO(USART1) != RESET))
	{
		if (ctx.tx_lock) {
				timeout();
		} else {
			//ERROR
		}
		LL_USART_ClearFlag_RTO(USART1);
		LL_USART_SetRxTimeout(USART1, TIMEOUT_VAL * (8 + 1 + 1));
	}
}

/**
 * \fn HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
 * \brief PTP interrupt management
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==ROBUS_PTPA_Pin){
		ptp_handler(BRANCH_A);
		return;
	}
	if(GPIO_Pin==ROBUS_PTPB_Pin){
		ptp_handler(BRANCH_B);
		return;
	}
}

void reverse_detection(branch_t branch) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode=GPIO_MODE_IT_RISING;// reverse the detection edge
	if (branch == BRANCH_A) {
		GPIO_InitStruct.Pin=ROBUS_PTPA_Pin;
		HAL_GPIO_Init(ROBUS_PTPA_GPIO_Port,&GPIO_InitStruct);
	}
	else if (branch == BRANCH_B) {
		GPIO_InitStruct.Pin=ROBUS_PTPB_Pin;
		HAL_GPIO_Init(ROBUS_PTPB_GPIO_Port,&GPIO_InitStruct);
	}
}

void set_PTP(branch_t branch) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;// Clean edge/state detection and set the PTP pin as output
	if (branch == BRANCH_A) {
		GPIO_InitStruct.Pin=ROBUS_PTPA_Pin;
		HAL_GPIO_Init(ROBUS_PTPA_GPIO_Port,&GPIO_InitStruct);
		HAL_GPIO_WritePin(ROBUS_PTPA_GPIO_Port,ROBUS_PTPA_Pin,GPIO_PIN_RESET);// Reset the PTPA pin
	}
	else if (branch == BRANCH_B) {
		GPIO_InitStruct.Pin=ROBUS_PTPB_Pin;
		HAL_GPIO_Init(ROBUS_PTPB_GPIO_Port,&GPIO_InitStruct);
		HAL_GPIO_WritePin(ROBUS_PTPB_GPIO_Port,ROBUS_PTPB_Pin,GPIO_PIN_RESET);// Reset the PTPB pin
	}
}

void reset_PTP(branch_t branch) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode=GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	if (branch == BRANCH_A) {
		// set the PTPA pin as input pull-up IRQ triggered on falling edge event
		__HAL_GPIO_EXTI_CLEAR_IT(ROBUS_PTPA_Pin);
		GPIO_InitStruct.Pin=ROBUS_PTPA_Pin;
		HAL_GPIO_Init(ROBUS_PTPA_GPIO_Port,&GPIO_InitStruct);
	}
	else if (branch == BRANCH_B) {
		// set the PTPB pin as input pull-up IRQ triggered on falling edge event
		__HAL_GPIO_EXTI_CLEAR_IT(ROBUS_PTPB_Pin);
		GPIO_InitStruct.Pin=ROBUS_PTPB_Pin;
		HAL_GPIO_Init(ROBUS_PTPB_GPIO_Port,&GPIO_InitStruct);
	}
}

/*void hal_delay_ms(int factor) { // Do we really need that?
    // TODO: do something clever here...
	HAL_Delay(factor);
}*/

/**
 * \fn void hal_init(void)
 * \brief hardware configuration (clock, communication, DMA...)
 */
void hal_init(void) {
	// Serial init
	// Enable Reception interrupt
	LL_USART_EnableIT_RXNE(USART1);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	// Enable Reception timeout interrupt
	// the timeout expressed in nb of bits duration
	LL_USART_EnableRxTimeout(USART1);
	LL_USART_EnableIT_RTO(USART1);
	LL_USART_SetRxTimeout(USART1, TIMEOUT_VAL * (8 + 1 + 1));
	// Setup data direction
	HAL_GPIO_WritePin(ROBUS_DE_GPIO_Port,ROBUS_DE_Pin,GPIO_PIN_RESET); 	// Disable emitter | Enable Receiver only - Hardware DE impossible
	// Setup pull ups pins
	HAL_GPIO_WritePin(RS485_LVL_UP_GPIO_Port,RS485_LVL_UP_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(RS485_LVL_DOWN_GPIO_Port,RS485_LVL_DOWN_Pin,GPIO_PIN_RESET);

	// Setup PTP lines
	reset_PTP(BRANCH_A);
	reset_PTP(BRANCH_B);

	// Unlock the Flash Program Erase controller
	HAL_FLASH_Unlock();
	// EEPROM Init
	for (uint16_t i = 0; i < NB_OF_VAR; i++) {
		VirtAddVarTab[i] = i;
	}
	EE_Init();
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
    // Disable RX and enable TX
	HAL_GPIO_WritePin(ROBUS_DE_GPIO_Port,ROBUS_DE_Pin,GPIO_PIN_SET);	// Enable TX
	HAL_GPIO_WritePin(ROBUS_RE_GPIO_Port,ROBUS_RE_Pin,GPIO_PIN_SET); 	// Disable RX

	for (unsigned short i = 0; i < size; i++) {
		while (!LL_USART_IsActiveFlag_TXE(USART1));
		LL_USART_TransmitData8(USART1,*(data+i));
	}
	while(!LL_USART_IsActiveFlag_TC(USART1));

	// Force start Usart Timeout
	timeout();

    // disable TX and enable RX
	HAL_GPIO_WritePin(ROBUS_DE_GPIO_Port,ROBUS_DE_Pin,GPIO_PIN_RESET);	// Disable TX
	HAL_GPIO_WritePin(ROBUS_RE_GPIO_Port,ROBUS_RE_Pin,GPIO_PIN_RESET); 	// Enable RX
    return 0;
}

unsigned char get_PTP(branch_t branch) {

	if (branch == BRANCH_A) {
		return (!HAL_GPIO_ReadPin(ROBUS_PTPA_GPIO_Port,ROBUS_PTPA_Pin));
	}
	else if (branch == BRANCH_B) {
		return (!HAL_GPIO_ReadPin(ROBUS_PTPB_GPIO_Port,ROBUS_PTPB_Pin));
	}
	return 0;
}

// ******** Alias management ****************
void write_alias(unsigned short id, char* alias) {
	// Check name integrity
	if (((alias[0] > 'A') & (alias[0] < 'Z')) | ((alias[0] > 'a') & (alias[0] < 'z')) | (alias[0] == '\0')) {
		const uint16_t addr = id * (MAX_ALIAS_SIZE +1);
		for (uint8_t i=0; i<MAX_ALIAS_SIZE; i++) {
			// here we save an uint8_t on an uint16_t
			EE_WriteVariable(addr + i, (uint16_t)alias[i]);
		}
	}
}

char read_alias(unsigned short id, char* alias) {
     const uint16_t addr = id * (MAX_ALIAS_SIZE +1);
     uint16_t data;
     EE_ReadVariable(addr, &data);
     // Check name integrity
     if (((((char)data < 'A') | ((char)data > 'Z'))
             & (((char)data < 'a') | ((char)data > 'z')))
             | ((char)data == '\0')) {
         return 0;
     } else {
         alias[0] = (char)data;
     }
     for (uint8_t i=1; i<MAX_ALIAS_SIZE; i++) {
        EE_ReadVariable(addr + i, &data);
        alias[i] = (char)data;
     }
     return 1;
}