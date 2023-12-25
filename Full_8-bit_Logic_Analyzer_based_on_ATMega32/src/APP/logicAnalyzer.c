
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "logicAnalyzer.h"
#include "../MCAL/MTIMER/MTIMER.h"
#include "../MCAL/MUART/MUART.h"
#include "../LIB/STD_MACROS.h"


/* Frame parts sizes */
#define _CMD_START_CNT 1
#define _CMD_END_CNT   1
#define _CMD_PINS_ST   1
#define _CMD_TIME_SNAP 4

/* Useful Indecies defined */
#define FULL_SAMPLE_CNT (_CMD_START_CNT + _CMD_PINS_ST +  _CMD_TIME_SNAP + _CMD_END_CNT)
#define _SAMPLE_PIN  (_CMD_START_CNT)
#define _SAMPLE_TIME (_CMD_START_CNT + _CMD_PINS_ST)
#define MARKER_END   (FULL_SAMPLE_CNT - 1)
#define MARKER_START (0)

// Send the following frame for each sample:
// @ PIN TIME3 TIME2 TIME1 TIME0 ;

#define _SAMPLES_NUM 255
#define LOGIC_DDR  DDRA
#define LOGIC_PORT PINA

typedef enum {MONITOR, SAMPLING, SENDING, IDLE} states_t;


static uint8_t logic_port_state = 0;
static uint8_t logic_port_pre_state;
static states_t currentState = SAMPLING;
static uint8_t  pin_states[_SAMPLES_NUM];
static uint32_t time_snap[_SAMPLES_NUM];
static uint8_t samples_num = _SAMPLES_NUM;
static uint8_t _go_signal_buf = 100;
extern uint32_t timerOVFs;


////////////////////// Logic Analyzer Functions //////////////////////
void LOGIC_Init(void)
{
	TIMER_Init();

    /* Init UART driver. */ 
    UART_Init();
    
    /* Start with getting which wave to generate. */ 
    currentState = MONITOR;    
}

 
void LOGIC_MainFunction(void)
{    
    static volatile uint16_t samples_cnt = 0;
    // Main function must have two states,
    // First state is command parsing and waveform selection.
    // second state is waveform executing.
    switch(currentState)
    {
        case MONITOR:
        {
	        LOGIC_DDR = 0;
	        logic_port_pre_state = logic_port_state;
	        logic_port_state     = LOGIC_PORT;
	        currentState = (logic_port_pre_state != logic_port_state) ? SAMPLING : MONITOR;
	        break;
        }
        case SAMPLING:
        {
            // DO here sampling.
            time_snap[samples_cnt]  = getTime();
			LOGIC_DDR = 0;
            pin_states[samples_cnt] = logic_port_state;
            
            // Increment sample count.
            samples_cnt++;
 
            // Start sending the collected samples_num samples.
            currentState = (samples_cnt >= samples_num) ? SENDING : MONITOR;
            break;
        }
        case SENDING:
        {
			samples_cnt = 0;
            // For samples_num samples send the construct the buffer.
            static uint8_t _sample_buf[FULL_SAMPLE_CNT];
            for(uint8_t i = 0; i < samples_num; ++i)
            {
                // Construct the buffer.
                
                // Add buffer marker
                _sample_buf[MARKER_START] = '@';
 
                // Add pin value.
                _sample_buf[_SAMPLE_PIN]  = pin_states[i];
 
                // Add time snap value.
                _sample_buf[_SAMPLE_TIME + 0] = ((time_snap[i] & 0xFF000000) >> 24);
                _sample_buf[_SAMPLE_TIME + 1] = ((time_snap[i] & 0x00FF0000) >> 16);
                _sample_buf[_SAMPLE_TIME + 2] = ((time_snap[i] & 0x0000FF00) >> 8);
                _sample_buf[_SAMPLE_TIME + 3] = ((time_snap[i] & 0x000000FF) >> 0);
 
                _sample_buf[MARKER_END]   = ';';
 
                // Send sample.
                UART_SendPayload(_sample_buf, FULL_SAMPLE_CNT);
                while (0 == UART_IsTxComplete());
            }
 
            // Trigger receiving for go signal.
            UART_ReceivePayload(&_go_signal_buf, 1);
			while(0 == UART_IsRxComplete());
			TCNT1L = 0;
			TCNT1H = 0;
			timerOVFs = 0;
			samples_num = _go_signal_buf;

        }
        case IDLE:
        {
//          currentState = ((samples_num > 24 && samples_num < 251)) ? MONITOR : IDLE;
			currentState = MONITOR;
			if(currentState == MONITOR)
            {
                // reset the timer value and update the samples number.
				TCNT1L = 0;
				TCNT1H = 0;
				timerOVFs = 0;
				//samples_num = ((volatile uint8_t)atoi(_go_signal_buf));
            }
            break;
        }
        default: {/* Do nothing.*/}
    }
}
