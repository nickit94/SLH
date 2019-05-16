#ifndef SW_TIMER_H_
#define SW_TIMER_H_

#include <main.h>

void swTimerSet(swtimer_t* pTimer, uint32_t Time, uint32_t Period);
void swTimerReset(swtimer_t* pTimer);
uint8_t swTimerCheck(swtimer_t* pTimer);


#endif /* SW_TIMER_H_ */
