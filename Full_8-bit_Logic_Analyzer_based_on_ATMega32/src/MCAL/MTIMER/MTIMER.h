#ifndef _MTIMER_H_
#define _MTIMER_H_

#define clks_number 65535
#define tick_time_us 1


void TIMER_Init();
extern uint32_t getTime(void);


#endif