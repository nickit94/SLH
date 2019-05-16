#include "ds18b20.h"

/* private:  */
static uint8_t dt[8] = {0};
 int8_t temperature = 0;
static uint8_t cur_state = 0;
static uint8_t count_try = 0;
static swtimer_t timer_wait_data;
static swtimer_t timer_between_requests;

static inline void delay_micro(__IO uint32_t micros)
{
	micros *= (SystemCoreClock / 1000000) / 9;
	while (micros--);
}

uint8_t ds18b20_read_bit()
{
	uint8_t bit = 0;

	DS18B20_PORT->ODR &= ~DS18B20_GPIO_ODR;					// низкий уровень
	delay_micro(2);

	DS18B20_PORT->ODR |= DS18B20_GPIO_ODR;					// высокий уровень
	delay_micro(13);

	bit = (DS18B20_PORT->IDR & DS18B20_GPIO_IDR ? 1 : 0);	// проверяем уровень
	delay_micro(45);

	return bit;
}

uint8_t ds18b20_read_byte()
{
	uint8_t data = 0;

	for(uint8_t i = 0; i <= 7; i++)
	{
		data += ds18b20_read_bit() << i;
	}

	return data;
}

void ds18b20_write_bit(uint8_t bit)
{
	DS18B20_PORT->ODR &= ~DS18B20_GPIO_ODR;
	delay_micro(bit ? 3 : 65);

	DS18B20_PORT->ODR |= DS18B20_GPIO_ODR;
	delay_micro(bit ? 65 : 3);
}

void ds18b20_write_byte(uint8_t dt)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		ds18b20_write_bit(dt >> i & 1);
		delay_micro(5);	//Delay Protection
	}
}

void ds18b20_init_port()
{
	/* PC15 */
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;   	// Разрешить тактирование GPIOC
	GPIOC->CRH &= ~GPIO_CRH_MODE15;  		// Очистить разряды MODE
	GPIOC->CRH &= ~GPIO_CRH_CNF15;   		// Очистить разряды CNF
	GPIOC->CRH |=  GPIO_CRH_MODE15_0;		// Выход, 10MHz
	GPIOC->CRH |=  GPIO_CRH_CNF15_0; 		// Общего назначения, открытый сток
}

uint8_t ds18b20_reset()
{
	uint16_t status;

	DS18B20_PORT->ODR &= ~DS18B20_GPIO_ODR;			//низкий уровень
	delay_micro(485);								//задержка как минимум на 480 микросекунд

	DS18B20_PORT->ODR |= DS18B20_GPIO_ODR;			//высокий уровень
	delay_micro(65);								//задержка как минимум на 60 микросекунд

	status = DS18B20_PORT->IDR & DS18B20_GPIO_IDR;	//проверяем уровень
	delay_micro(500);								//задержка как минимум на 480 микросекунд
													//(на всякий случай подождём побольше, так как могут быть неточности в задержке)
	return (status ? 1 : 0);						//вернём результат
}

uint8_t ds18b20_init(uint8_t mode)
{
	ds18b20_init_port();

	if (ds18b20_reset()) return 1;

	if (mode == DS18B20_SKIP_ROM)
	{
		ds18b20_write_byte(0xCC);	// SKIP ROM
		ds18b20_write_byte(0x4E);	// WRITE SCRATCHPAD
		//ds18b20_write_byte(0x64);	// TH REGISTER 100 градусов
		//ds18b20_write_byte(0x9E);	// TL REGISTER -30 градусов
		ds18b20_write_byte(DS18B20_RESOLUTION_12BIT);	// Resolution 12 bit
	}

	return 0;
}

void ds18b20_send_request_measure_temper(uint8_t mode, uint8_t DevNum)
{
	ds18b20_reset();

	if (mode == DS18B20_SKIP_ROM)
	{
		ds18b20_write_byte(0xCC); 	//SKIP ROM
	}

	ds18b20_write_byte(0x44);		//CONVERT T
}

void ds18b20_read_all_data(uint8_t mode, uint8_t *Data, uint8_t DevNum)
{
	ds18b20_reset();

	if (mode == DS18B20_SKIP_ROM)
	{
		ds18b20_write_byte(0xCC);	//SKIP ROM
	}

	ds18b20_write_byte(0xBE);		//READ SCRATCHPAD

	for(uint8_t i = 0; i < 8; i++)
	{
		Data[i] = ds18b20_read_byte();
	}
}

uint8_t ds18b20_get_sign(uint16_t dt)
{
	return (dt & (1 << 11)) ? 1 : 0;	//Проверим 11-й бит
}

int8_t ds18b20_convert(uint16_t dt)
{
	return (int8_t)((dt & 0x07FF) >> 4); 	// отборосим знаковые и дробные биты
}

/* public: */
void ds18b20_fsm()
{
	switch(cur_state)
	{
	/* Инициализация */
	case 0:
		swTimerSet(&timer_between_requests, DS18B20_TIME_REQUEST, DS18B20_TIME_REQUEST);

		if (!ds18b20_init(DS18B20_SKIP_ROM)) cur_state = 3;
		else 								 cur_state = 5;
	break;

	/* Отправка запроса */
	case 1:
		ds18b20_send_request_measure_temper(DS18B20_SKIP_ROM, 0);
		swTimerSet(&timer_wait_data, 1000, 0);
		cur_state = 2;
	break;

	/* Ожидание и получение ответа */
	case 2:
		if (swTimerCheck(&timer_wait_data))
		{
			ds18b20_read_all_data(DS18B20_SKIP_ROM, dt, 0);
			temperature = ds18b20_convert(((uint16_t)dt[1] << 8) | dt[0]);

			if (temperature > DS18B20_MAX_TEMPERATURE) {}
			cur_state = 3;
		}
	break;

	/* Ожидание между запросами */
	case 3:
		if (swTimerCheck(&timer_between_requests)) cur_state = 1;
	break;

	/* Устройство не отвечает */
	case 5:
		if (count_try == DS18B20_MAX_COUNT_TRY)
		{
			//set_error
			cur_state = 6;
		}

		if (swTimerCheck(&timer_between_requests))
		{
			count_try++;
			cur_state = 0;
		}
	break;

	/* Выключить автомат */
	case 6:
	break;
	}
}

















