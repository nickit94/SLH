#include "sw_timer.h"

void swTimerSet(swtimer_t* pTimer, uint32_t Time, uint32_t Period)
{
	pTimer->time = Time;
	pTimer->period = Period;
	pTimer->prev_time_system = HAL_GetTick();

	if (Time != 0 || Period != 0) 	pTimer->status = 1;
	else 							pTimer->status = 0;
}

void swTimerReset(swtimer_t* pTimer)
{
	pTimer->status = 0;
	pTimer->time = 0;
	pTimer->period = 0;
}

uint8_t swTimerCheck(swtimer_t* pTimer)
{
	if (pTimer->status)
	{
		if ((HAL_GetTick() - pTimer->prev_time_system) >= pTimer->time)
		{
			if (pTimer->period == 0) swTimerReset(pTimer);
			else
			{
				pTimer->time = pTimer->period;
				pTimer->prev_time_system = HAL_GetTick();
			}

			return 1;
		}
	}

  return 0;
}
