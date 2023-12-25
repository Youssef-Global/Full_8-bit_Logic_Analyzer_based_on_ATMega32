
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "../../LIB/STD_MACROS.h"
#include "../MUART/MUART.h"
#include "MTIMER.h"

volatile uint32_t timerOVFs = 0;

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

extern uint32_t getTime(void)
{
	// Computing the elapsed time.
	// Max Clocks (Ticks) Number in 16bit timer = 2^16 - 1 = 65536 - 1 = 65535 tick
	// Frequency = F_CPU/prescaler = 8*10^6/8 = 1 MHz
	// Tick Time = 1/Frequency = 1 us
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
