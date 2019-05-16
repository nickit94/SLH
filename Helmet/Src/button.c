#include "button.h"

button_t but1;

/* private: */
void button_create(button_t* button, GPIO_TypeDef* port, uint16_t pin)
{
	button->port = port;
	button->pin = pin;
	button->state = 0;
	button->time_press = 0;
	button->count_press = 0;
	button->prev_time_system = 0;
	button->state_fsm = 1;
}

void button_update(button_t* button)
{
	switch (button->state_fsm)
	{
	/* Init */
	case 0:
		break;

	/* No Press */
	case 1:
		if ((button->port->IDR & (1 << button->pin)) == 0)
		{
			swTimerSet(&button->timer_debounce, TIME_DEBOUNCE, 0);
			button->state_fsm = 2;
		}

		break;

	/* Press */
	case 2:
		if (swTimerCheck(&button->timer_debounce))
		{
			swTimerReset(&button->timer_debounce);

			if ((button->port->IDR & (1 << button->pin)) == 0)
			{
				button->prev_time_system = HAL_GetTick();
				button->state_fsm = 3;
				button->state = 1;
				button->count_press = button->count_press + 1;
			}
			else
			{
				button->state = 0;
				button->state_fsm = 1;
			}
		}
		break;

	/* Hold */
	case 3:

		/* Всё еще нажата */
		if ((button->port->IDR & (1 << button->pin)) == 0)
		{
			button->time_press = HAL_GetTick() - button->prev_time_system;
		}
		/* Отпустили */
		else
		{
			button->state = 0;

			button->time_press = 0;
			if (button->count_press == 1) swTimerSet(&button->timer_count_press, TIME_COUNT_PRESS, 0);

			button->state_fsm = 1;
		}
		break;
	}

	if (swTimerCheck(&button->timer_count_press))
	{
	    swTimerReset(&button->timer_count_press);
	    button->count_press = 0;
	}
}

/* public: */
void button_init()
{
	/* PA5, вход, подтяжка к плюсу */
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~GPIO_CRL_MODE5;
	GPIOA->CRL &= ~GPIO_CRL_CNF5;
	GPIOA->CRL |=  GPIO_CRL_CNF5_1;
	GPIOA->BSRR =  GPIO_BSRR_BS5;

	button_create(&but1, GPIOA, 5);
}

uint16_t button_get_time_pressed(button_t* button)
{
	return (uint16_t)(HAL_GetTick() - button->prev_time_system);
}

uint16_t button_get_count_press(button_t* button)
{
	return button->count_press;
}

uint8_t button_get_state(button_t* button)
{
	return button->state;
}

uint8_t button_check(button_t* button, uint16_t time_press)
{
	if (button->state && button->time_press >= time_press)
	{
		button->state = 0;
		return 1;
	}

	return 0;
}

void button_fsm()
{
	button_update(&but1);
}

