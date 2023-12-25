#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdlib.h>


#include <util/delay.h>

#include "logicAnalyzer.h"
#include <avr/interrupt.h>
#include "uart.h"
#include "STD_MACROS.h"

#define _CMD_START_CNT 1
#define _CMD_END_CNT   1
#define _CMD_SPACING   1
#define _CMD_PINS_ST   1
#define _CMD_TIME_SNAP 4

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


static logic_port_state = 0;
static logic_port_pre_state;
static states_t currentState = SAMPLING;
static uint8_t  pin_states[_SAMPLES_NUM];
static uint32_t time_snap[_SAMPLES_NUM];
static uint8_t samples_num = _SAMPLES_NUM;


////////////////////// Timer1 Functions //////////////////////
volatile static uint32_t timerOVFs  = 0;

void TIMER_Init()
{
	// Setting Prescaler to 8
	CLR_BIT(TCCR1B,CS10); //CS10
	SET_BIT(TCCR1B,CS11); //CS11
	CLR_BIT(TCCR1B,CS12); //CS12
	

	// Setting Timer1 to Normal Mode
	CLR_BIT(TCCR1A,WGM10); //WGM10
	CLR_BIT(TCCR1A,WGM11); //WGM11
	CLR_BIT(TCCR1B,WGM12); //WGM12
	CLR_BIT(TCCR1B,WGM13); //WGM13
	
	
	// Setting Timer ov interrupt on
	SET_BIT(TIMSK,TOIE1);

	// Starting Timer
	TCNT1 = 0;
	timerOVFs = 0;
	sei();
}

ISR(TIMER1_OVF_vect)
{
	timerOVFs++;
}

static uint32_t getTime(void)
{
	// TODO: Place your code here, to compute the elapsed time.
	// Max Clocks (Ticks) Number in 16bit timer = 2^16 - 1 = 65536 - 1 = 65535 tick
	// Frequency = F_CPU/prescaler = 8*10^6/8 Hz
	// Tick Time = 1/Frequency = 8 us
	// Elapced Time = ((No of Overflows * Max Ticks) + Timer Counter Value) * Tick Time
	uint32_t elapcedTime = 0;
	uint32_t timerValue = 0;
	timerValue  = (TCNT1L)&0x00FF;
	timerValue |= TCNT1H<<8;
	elapcedTime = ((timerOVFs * clks_number) + timerValue) * tick_time_us;
	return elapcedTime;
}
//////////////////////End of Timer1 Functions//////////////////////
//////////////////////////////////////////////////////////////////

////////////////////// Logic Analyzer Functions //////////////////////
void LOGIC_Init(void)
{
	TIMER_Init();

    /* Init UART driver. */
    UART_cfg my_uart_cfg;
    
    /* Set USART mode. */
    my_uart_cfg.UBRRL_cfg = (UBBR_VALUE)&0x00FF;
    my_uart_cfg.UBRRH_cfg = (((UBBR_VALUE)&0xFF00)>>8);
    
    my_uart_cfg.UCSRA_cfg = 0;
    my_uart_cfg.UCSRB_cfg = (1<<RXEN)  | (1<<TXEN) | (1<<TXCIE) | (1<<RXCIE);
    my_uart_cfg.UCSRC_cfg = (1<<URSEL) | (3<<UCSZ0);
    
    UART_Init(&my_uart_cfg);
    
    // TODO: Place your code here for timer1 initialization to normal mode 
    // and keep track to time elapsed.
    
 
    /* Start with getting which wave to generate. */ 
    currentState = MONITOR;    
}

static uint8_t _go_signal_buf = 100;
 
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
                // TODO: Place your code here to reset the timer value and update the samples number.
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
