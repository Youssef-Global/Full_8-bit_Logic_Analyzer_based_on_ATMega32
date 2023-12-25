#ifndef _MUART_H_
#define _MUART_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#define BAUD_RATE 9600
#define UBRR_VALUE (((F_CPU)/(BAUD_RATE*16UL))-1)


extern void 	 UART_Init();
extern void 	 UART_SendPayload(uint8_t *tx_data, uint16_t len);
extern void      UART_ReceivePayload(uint8_t *rx_data, uint16_t len);
extern uint8_t 	 UART_IsDataAvaiable(void);
extern uint8_t 	 UART_IsTxComplete(void);
extern uint8_t 	 UART_IsRxComplete(void);

#endif
