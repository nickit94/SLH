#include "apa106.h"

/* private: */
static const uint16_t mas_size_led_bufs[APA106_NUM_CHANNELS] = {
		APA106_BUF_CH0_SIZE, APA106_BUF_CH1_SIZE,  APA106_BUF_CH2_SIZE,
		APA106_BUF_CH3_SIZE, APA106_BUF_CH4_SIZE,  APA106_BUF_CH5_SIZE,
		APA106_BUF_CH6_SIZE, APA106_BUF_CH7_SIZE,  APA106_BUF_CH8_SIZE,
		APA106_BUF_CH9_SIZE, APA106_BUF_CH10_SIZE, APA106_BUF_CH11_SIZE,
		APA106_BUF_CH12_SIZE
};
static const uint8_t apa106_array_step_brightness[APA106_MAX_STEP_BRIGHTNESS] = { 1, 2, 3, 4, 6, 8, 12, 17, 25, 50, 100 };
static uint8_t raw_led_buffer[APA106_SIZE_RESET + MAX_Y * 24] = {0};
uint16_t enable_led_channels = 0xFFFF;
uint8_t state_update = 0;
uint8_t cur_led_channel = 0;
swtimer_t st_update;

/* public: */
uint8_t apa106_cur_ind_bright = 5;//3;

/***************************************************************************************************/

void apa106_apply_brightness(uint8_t* red, uint8_t* green, uint8_t* blue)
{
	/* Случай, когда превышение по току */
	if ((*red) + (*green) + (*blue) > APA106_MAX_BRIGHT)
	{
		uint32_t coef_excess_brightness = ((*red) + (*green) + (*blue)) * 1000 / APA106_MAX_BRIGHT;

		(*red)   = ((uint32_t)((*red)   * 1000) / coef_excess_brightness) & 0xFF;
		(*green) = ((uint32_t)((*green) * 1000) / coef_excess_brightness) & 0xFF;
		(*blue)  = ((uint32_t)((*blue)  * 1000) / coef_excess_brightness) & 0xFF;
	}

	/* Применение текущей яркости */
	(*red)   = (((uint16_t)(*red)   * 10 / apa106_array_step_brightness[apa106_cur_ind_bright]) + 9) / 10;	// Округление до следующего целого для случаев, когда после деления знач. < 1
	(*green) = (((uint16_t)(*green) * 10 / apa106_array_step_brightness[apa106_cur_ind_bright]) + 9) / 10;	// Округление до следующего целого для случаев, когда после деления знач. < 1
	(*blue)  = (((uint16_t)(*blue)  * 10 / apa106_array_step_brightness[apa106_cur_ind_bright]) + 9) / 10;	// Округление до следующего целого для случаев, когда после деления знач. < 1
}

void apa106_set_pixel(uint8_t num_channel, uint16_t num_pixel, uint8_t red, uint8_t green, uint8_t blue)
{
	if (num_channel >= APA106_NUM_CHANNELS) return;
	if (num_pixel >= mas_count_leds_in_channel[num_channel]) return;

	for (uint8_t i = 0; i < APA106_SIZE_RESET; i++)
	{
		raw_led_buffer[i] = 0;
	}

	apa106_apply_brightness(&red, &green, &blue);

	/* Четные - правильные
	 * Нечетные - инверсия */

	/* Инверсия */
	if (num_channel % 2)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			if (BIT_IS_SET(red, (7 - i)) == 1)	raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 0)] = APA106_HIGH;
			else 								raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 0)] = APA106_LOW;

			if (BIT_IS_SET(green,(7 - i)) == 1)	raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 8)] = APA106_HIGH;
			else 								raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 8)] = APA106_LOW;

			if (BIT_IS_SET(blue,(7 - i)) == 1)	raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 16)] = APA106_HIGH;
			else 								raw_led_buffer[mas_size_led_bufs[num_channel] - ((num_pixel+1) * 24) + (i + 16)] = APA106_LOW;
		}
	}
	/* Нормальный канал */
	else
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			if (BIT_IS_SET(red, (7 - i)) == 1)	raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 0] = APA106_HIGH;
			else 								raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 0] = APA106_LOW;

			if (BIT_IS_SET(green,(7 - i)) == 1)	raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 8] = APA106_HIGH;
			else 								raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 8] = APA106_LOW;

			if (BIT_IS_SET(blue,(7 - i)) == 1)	raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 16] = APA106_HIGH;
			else 								raw_led_buffer[APA106_SIZE_RESET + num_pixel * 24 + i + 16] = APA106_LOW;
		}
	}
}

void apa106_clear_buf(uint8_t led_channel)
{
	if (led_channel >= APA106_NUM_CHANNELS) return;

	for(uint16_t i = APA106_SIZE_RESET; i < mas_size_led_bufs[led_channel]; i++)
	{
		raw_led_buffer[i] = APA106_LOW;
	}
}

/***************************************************************************************************/
void init_pin(GPIO_TypeDef* GPIOx, uint8_t Pin)
{
	if (Pin < 8)
	{
		GPIOx->CRL &= ~(0x03U <<  (4 * Pin));				// очистить разряды MODE
		GPIOx->CRL &= ~(0x03U << ((4 * Pin) + 2));			// очистить разряды CNF
		GPIOx->CRL |=  (0x1U  <<  (4 * Pin));				// выход, 10MHz
		GPIOx->CRL |=  (0x02U << ((4 * Pin) + 2));			// альтернативная функция, симетричный
	}
	else
	{
		GPIOx->CRH &= ~(0x03U <<  (4 * (Pin - 8)));			// очистить разряды MODE
		GPIOx->CRH &= ~(0x03U << ((4 * (Pin - 8)) + 2));	// очистить разряды CNF
		GPIOx->CRH |=  (0x1U  <<  (4 * (Pin - 8)));			// выход, 10MHz
		GPIOx->CRH |=  (0x02U << ((4 * (Pin - 8)) + 2));	// альтернативная функция, симетричный
	}
}
void deinit_timer(TIM_TypeDef* TIMx)
{
	TIMx->SR = 0; 											// Сбрасываем все флаги прерываний
	TIMx->CR1 &= ~(TIM_CR1_CEN); 							// Останавливаем таймер
	TIMx->DIER = 0;											// Сброс DMA
	TIMx->CCER  &= ~TIM_CCER_CC4E;                          // Выход канала захвата/сравнения выключен
	TIMx->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);      // Сбрасываем все биты OCxM (отключение ШИМ)
	TIMx->CCMR2 &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_OC3M);      // Сбрасываем все биты OCxM (отключение ШИМ)
}
void deinit_dma_channel(DMA_Channel_TypeDef* DMA_Channelx)
{
	DMA1->IFCR = 0xFFFFFFF;								// Очистить все флаги прерываний DMA
	DMA_Channelx->CNDTR = 0;	 						// Устанавливаем количество данных
	DMA_Channelx->CMAR  = (uint32_t)(0); 				// Откуда берем (память - буфер)
	DMA_Channelx->CCR &= ~(DMA_CCR_EN); 				// Отключаем канал DMA
}
void init_send_channel(TIM_TypeDef* TIMx, uint8_t tim_channel, DMA_Channel_TypeDef* DMA_Channelx, uint8_t num_dma_channel, uint8_t led_channel)
{
	/*------------------------------------------ Настройка таймеров и каналов ------------------------------------------*/
	TIMx->CR1 |= TIM_CR1_ARPE; 									// Включен режим предварительной записи регистра автоперезагрузки

	/* Разбиение по каналам */
	switch(tim_channel)
	{
	case 1:
		/* Канал 1 */
		TIMx->CCER  |= TIM_CCER_CC1E;							// Выход канала захвата/сравнения включен
		TIMx->CCMR1 &= ~TIM_CCMR1_OC1M; 						// Сбрасываем все биты OCxM
		TIMx->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;		// Перевести текущий канал в PWM mode 1
		TIMx->CCMR1 |= TIM_CCMR1_OC1PE;							// Включен режим предварительной загрузки регистра сравнения
		TIMx->DIER  |= TIM_DIER_CC1DE;							// Разрешить для текущего канала запрос DMA
		TIMx->DIER  |= TIM_DIER_CC1DE;                          // Разрешить для текущего канала запрос DMA

		DMA_Channelx->CPAR = (uint32_t)(&TIMx->CCR1);			// Куда пишем (переферия - номер таймера, номер канала) ??
		break;

	case 2:
		/* Канал 2 */
		TIMx->CCER  |= TIM_CCER_CC2E;							// Выход канала захвата/сравнения включен
		TIMx->CCMR1 &= ~TIM_CCMR1_OC2M; 						// Сбрасываем все биты OCxM
		TIMx->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;		// Перевести текущий канал в PWM mode 1
		TIMx->CCMR1 |= TIM_CCMR1_OC2PE;							// Включен режим предварительной загрузки регистра сравнения
		TIMx->DIER  |= TIM_DIER_CC2DE;							// Разрешить для текущего канала запрос DMA
		TIMx->DIER  |= TIM_DIER_CC2DE;                          // Разрешить для текущего канала запрос DMA

		DMA_Channelx->CPAR = (uint32_t)(&TIMx->CCR2);			// Куда пишем (переферия - номер таймера, номер канала) ??
		break;

	case 3:
		/* Канал 3 */
		TIMx->CCER  |= TIM_CCER_CC3E;							// Выход канала захвата/сравнения включен
		TIMx->CCMR2 &= ~TIM_CCMR2_OC3M; 						// Сбрасываем все биты OCxM
		TIMx->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;		// Перевести текущий канал в PWM mode 1
		TIMx->CCMR2 |= TIM_CCMR2_OC3PE;							// Включен режим предварительной загрузки регистра сравнения
		TIMx->DIER  |= TIM_DIER_CC3DE;							// Разрешить для текущего канала запрос DMA
		TIMx->DIER  |= TIM_DIER_CC3DE;                          // Разрешить для текущего канала запрос DMA

		DMA_Channelx->CPAR = (uint32_t)(&TIMx->CCR3);			// Куда пишем (переферия - номер таймера, номер канала) ??
		break;

	case 4:
		/* Канал 4 */
		TIMx->CCER  |= TIM_CCER_CC4E;							// Выход канала захвата/сравнения включен
		TIMx->CCMR2 &= ~TIM_CCMR2_OC4M; 						// Сбрасываем все биты OCxM
		TIMx->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;		// Перевести текущий канал в PWM mode 1
		TIMx->CCMR2 |= TIM_CCMR2_OC4PE;							// Включен режим предварительной загрузки регистра сравнения
		TIMx->DIER  |= TIM_DIER_CC4DE;							// Разрешить для текущего канала запрос DMA
		TIMx->DIER  |= TIM_DIER_CC4DE;                          // Разрешить для текущего канала запрос DMA

		DMA_Channelx->CPAR = (uint32_t)(&TIMx->CCR4);			// Куда пишем (переферия - номер таймера, номер канала) ??
		break;
	}

	TIMx->ARR  = APA106_TIMER_AAR;								// Установка периода - 1.25 мс
	TIMx->CCR2 = 0x0000; 										// Устанавливаем ШИМ-регистр таймера в ноль (на шине будет установлен неактивный уровень до момента запуска DMA)
	TIMx->CNT  = 0; 											// Очищаем счетный регистр

	/* ------------------------------------------ Настройка DMA ------------------------------------------ */

	DMA_Channelx->CCR = DMA_CCR_PSIZE_0 | 						// 01: 16 бит переферия
						DMA_CCR_MINC	|						// Инкрементирование указателя памяти
						DMA_CCR_DIR; 							// Напраление чтения из памяти в переферию
	DMA_Channelx->CMAR  = (uint32_t)(raw_led_buffer + 1); 		// Откуда берем (память - буфер)
	DMA_Channelx->CNDTR = mas_size_led_bufs[led_channel]; 		// Устанавливаем количество данных
	DMA_Channelx->CCR  |= DMA_CCR_TCIE; 						// Назначить прерывание завершения передачи


	switch(num_dma_channel)
	{
	case 1:
		NVIC_EnableIRQ(DMA1_Channel1_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CGIF1;		// Очистить все флаги прерываний DMA
		break;
	case 2:
		NVIC_EnableIRQ(DMA1_Channel2_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF2 | DMA_IFCR_CHTIF2 | DMA_IFCR_CTCIF2 | DMA_IFCR_CGIF2;		// Очистить все флаги прерываний DMA
		break;
	case 3:
		NVIC_EnableIRQ(DMA1_Channel3_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3;		// Очистить все флаги прерываний DMA
		break;
	case 4:
		NVIC_EnableIRQ(DMA1_Channel4_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF4 | DMA_IFCR_CHTIF4 | DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4;		// Очистить все флаги прерываний DMA
		break;
	case 5:
		NVIC_EnableIRQ(DMA1_Channel5_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CGIF5;		// Очистить все флаги прерываний DMA
		break;
	case 6:
		NVIC_EnableIRQ(DMA1_Channel6_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF6 | DMA_IFCR_CHTIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CGIF6;		// Очистить все флаги прерываний DMA
		break;
	case 7:
		NVIC_EnableIRQ(DMA1_Channel7_IRQn);														// Разрешение прерывания от DMA по окончанию передачи
		DMA1->IFCR = DMA_IFCR_CTEIF7 | DMA_IFCR_CHTIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CGIF7;		// Очистить все флаги прерываний DMA
	break;
	}

	/* ------------------------------------------ Запуск отправки данных ------------------------------------------ */

	TIMx->CR1 |= TIM_CR1_CEN; 			// Запускаем таймер
	DMA_Channelx->CCR |= DMA_CCR_EN; 	// Включаем канал DMA, тем самым начинаем передачу данных
}

/***************************************************************************************************/
void init_all_gpio()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;    // Разрешить тактирование GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;    // Разрешить тактирование GPIOB

	/* ТАЙМЕР 1: КАНАЛЫ 1, 2, 3, 4 */
	init_pin(GPIOA, 8);		// A8
	init_pin(GPIOA, 9);		// A9
	init_pin(GPIOA, 10);	// A10
	init_pin(GPIOA, 11);	// A11

	/* ТАЙМЕР 2: КАНАЛЫ 2, 3, 4 */
	init_pin(GPIOA, 0);		// A0
	init_pin(GPIOA, 1);		// A1
	init_pin(GPIOA, 2);		// A2
	init_pin(GPIOA, 3);		// A3

	/* ТАЙМЕР 3: КАНАЛЫ 1, 3, 4 */
	init_pin(GPIOA, 6);		// A6
	init_pin(GPIOB, 0);		// B0
	init_pin(GPIOB, 1);		// B1

	/* ТАЙМЕР 4: КАНАЛЫ 1, 2, 3 */
	init_pin(GPIOB, 6);		// B6
	init_pin(GPIOB, 7);		// B7
	//init_pin(GPIOB, 8);	// B8
}
void init_timers()
{
	/* Разрешаем такирование */
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // таймера TIM1
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // таймера TIM2
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // таймера TIM3
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // таймера TIM4

	TIM1->BDTR |= TIM_BDTR_MOE;			// Main output enable - только для таймера 1
}
void init_dma()
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;   // Подключение тактирования к DMA1
}

/***************************************************************************************************/
void apa106_init()
{
	init_all_gpio();
	init_timers();
	init_dma();

	for (uint16_t i = 0; i < (48 + 40 * 24); i++)
		raw_led_buffer[i] = 0;


	for (uint8_t i = 0; i < APA106_NUM_CHANNELS; i++)
	{
		apa106_clear_buf(i);
	}

	apa106_start_update();
}
void apa106_send(uint8_t led_channel)
{
	switch(led_channel)
	{
		/* TIM1_CH1 - PA8 */
		case 0:	init_send_channel(TIM1, 1, DMA1_Channel2, 2, 0);
		break;

		/* TIM1_CH2 - PA9 */
		case 1:	init_send_channel(TIM1, 2, DMA1_Channel3, 3, 1);
		break;

		/* TIM1_CH3 - PA10 */
		case 2:	init_send_channel(TIM1, 3, DMA1_Channel6, 6, 2);
		break;

		/* TIM1_CH4 - PA11 */
		case 3:	init_send_channel(TIM1, 4, DMA1_Channel4, 4, 3);
		break;

		/* TIM4_CH1 - PB6 */
		case 4:	init_send_channel(TIM4, 1, DMA1_Channel1, 1, 4);
		break;

		/* TIM4_CH2 - PB7 */
		case 5:	init_send_channel(TIM4, 2, DMA1_Channel4, 4, 5);
		break;

		/* TIM2_CH1 - PA0 */
		case 6:	init_send_channel(TIM2, 1, DMA1_Channel5, 5, 6);
		break;

		/* TIM2_CH2 - PA1 */
		case 7:	init_send_channel(TIM2, 2, DMA1_Channel7, 7, 7);
		break;

		/* TIM2_CH3 - PA2 */
		case 8: init_send_channel(TIM2, 3, DMA1_Channel1, 1, 8);
		break;

		/* TIM2_CH4 - PA3 */
		case 9: init_send_channel(TIM2, 4, DMA1_Channel7, 7, 9);
		break;

		/* TIM3_CH1 - PA6 */
		case 10: init_send_channel(TIM3, 1, DMA1_Channel6, 6, 10);
		break;

		/* TIM3_CH3 - PB0 */
		case 11: init_send_channel(TIM3, 3, DMA1_Channel2, 2, 11);
		break;

		/* TIM3_CH4 - PB1 */
		case 12: init_send_channel(TIM3, 4, DMA1_Channel3, 3, 12);
		break;
	}
}

/***************************************************************************************************/

void apa106_start_update()
{
	if (!state_update) state_update = 1;
}
void apa106_update()
{
	switch(state_update)
	{
	case 0:
	break;

	case 1:
		if (!BIT_IS_SET(enable_led_channels, cur_led_channel)) apa106_clear_buf(cur_led_channel);

		led_buffer_prepare_for_apa106(cur_led_channel);
		apa106_send(cur_led_channel);

		swTimerSet(&st_update, 3, 0);
		state_update = 2;
	break;

	case 2:
		if (swTimerCheck(&st_update))
		{
			if (++cur_led_channel == APA106_NUM_CHANNELS)
			{
				cur_led_channel = 0;
				state_update = 0;
			}
			state_update = 1;
		}
	break;
	}
}

/*********** interrupts ***********/
void DMA1_Channel1_IRQHandler()
{
	/* TIM2 TIM4 */
	deinit_dma_channel(DMA1_Channel1);
	deinit_timer(TIM2);
	deinit_timer(TIM4);
}
void DMA1_Channel2_IRQHandler()
{
	/* TIM1 TIM3 */
	deinit_dma_channel(DMA1_Channel2);
	deinit_timer(TIM1);
	deinit_timer(TIM3);
}
void DMA1_Channel3_IRQHandler()
{
	/* TIM1 TIM3 */
	deinit_dma_channel(DMA1_Channel3);
	deinit_timer(TIM1);
	deinit_timer(TIM3);
}
void DMA1_Channel4_IRQHandler()
{
	/* TIM1 TIM4 */
	deinit_dma_channel(DMA1_Channel4);
	deinit_timer(TIM1);
	deinit_timer(TIM4);
}
void DMA1_Channel5_IRQHandler()
{
	/* TIM2 TIM4 */
	deinit_dma_channel(DMA1_Channel5);
	deinit_timer(TIM2);
	deinit_timer(TIM4);
}
void DMA1_Channel6_IRQHandler()
{
	/* TIM1 TIM3 */
	deinit_dma_channel(DMA1_Channel6);
	deinit_timer(TIM1);
	deinit_timer(TIM3);
}
void DMA1_Channel7_IRQHandler()
{
	/* TIM2 TIM2 */
	deinit_dma_channel(DMA1_Channel7);
	deinit_timer(TIM2);
}

















