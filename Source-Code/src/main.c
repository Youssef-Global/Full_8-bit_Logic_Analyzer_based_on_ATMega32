/*
 * Lab5_Logic_Analyzer.c
 *
 * Created: 5/12/2023 22:19:29 PM
 * Author : Global
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "logicAnalyzer.h"


int main(void)
{
    /* Replace with your application code */
	LOGIC_Init();
	sei();
    while (1) 
    {
		LOGIC_MainFunction();
    }
	return 0;
}