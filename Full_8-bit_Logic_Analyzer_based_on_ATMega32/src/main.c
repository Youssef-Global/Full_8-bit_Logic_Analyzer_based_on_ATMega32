/*
 * Logic_Analyzer
 * main.c
 *
 * Created: 5/12/2023 22:19:29 PM
 * Author : Youssef Ashraf
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "APP/logicAnalyzer.h"


int main(void)
{

	LOGIC_Init();
	sei();

    while (1) 
    {
		LOGIC_MainFunction();
    }
	
  return 0;
}