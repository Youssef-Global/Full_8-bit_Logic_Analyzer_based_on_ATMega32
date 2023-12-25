#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "MUART.h"

static volatile uint8_t  *tx_buffer;
static volatile uint16_t tx_len;
static volatile uint16_t tx_cnt;

static volatile uint8_t *rx_buffer;
static volatile uint16_t rx_len;
static volatile uint16_t rx_cnt;



ISR(USART_RXC_vect)
{
    uint8_t rx_data;
    
    cli();
    
    /* Read rx_data. */
    rx_data = UDR;
    
    /* Ignore spaces */
    if((rx_cnt < rx_len) && (rx_data != ' '))
    {
        rx_buffer[rx_cnt] = rx_data;
        rx_cnt++;
    }
    else
    {
        /* Do nothing. */
    }
    
    sei();
}

ISR(USART_TXC_vect)
{
    cli();
    
    tx_cnt++;
    
    if(tx_cnt < tx_len)
    {
        /* Send next byte. */
        UDR = tx_buffer[tx_cnt];
    }
    sei();
}


void UART_Init()
{
    /* Set baud rate */
    UBRRL =(UBRR_VALUE)&0x00FF;
    UBRRH = (((UBRR_VALUE)&0xFF00)>>8);
    
    /* Set USART mode */
    UCSRA = 0;
    UCSRB = (1<<RXEN)  | (1<<TXEN) | (1<<TXCIE) | (1<<RXCIE);
    UCSRC = (1<<URSEL) | (3<<UCSZ0);
}

void UART_SendPayload(uint8_t *tx_data, uint16_t len)
{
    tx_buffer = tx_data;
    tx_len    = len;
    tx_cnt    = 0;
    
    /* Wait for UDR is empty. */
    while(0 == (UCSRA & (1 << UDRE)));
    
    /* Send the first byte to trigger the TxC interrupt. */
    UDR = tx_buffer[0];
    
}

void UART_ReceivePayload(uint8_t *rx_data, uint16_t len)
{
    rx_buffer = rx_data;
    rx_len    = len;
    rx_cnt    = 0;
}

uint8_t UART_IsTxComplete(void)
{
    return ( (tx_cnt >= tx_len) ? 1 : 0 );
}

uint8_t UART_IsRxComplete(void)
{
    return ( (rx_cnt >= rx_len) ? 1 : 0 );
}
