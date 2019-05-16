#include "i2c.h"

/* private: */
int16_t i2c_start(uint8_t address, uint8_t rw, uint8_t ack)
{
	/* Generate I2C start pulse */
	I2C1->CR1 |= I2C_CR1_START;

	/* Wait till I2C is busy */
	uint32_t timeout = I2C_TIMEOUT;
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
		if (--timeout == 0x00) return 1;
	}

	if (ack) I2C1->CR1 |= I2C_CR1_ACK;

	/* Send write/read bit */
	if (rw == I2C_WRITE_MODE)
	{
		I2C1->DR = I2C_ADDR(address, 0x00);

		/* Wait till finished */
		timeout = I2C_TIMEOUT;
		while (!(I2C1->SR1 & I2C_SR1_ADDR))
		{
			if (--timeout == 0x00) return 1;
		}
	}
	if (rw == I2C_READ_MODE)
	{
		I2C1->DR = I2C_ADDR(address, 0x01);

		/* Wait till finished */
		timeout = I2C_TIMEOUT;
		while(!(I2C1->SR2 & I2C_SR2_BUSY) && !(I2C1->SR2 & I2C_SR2_MSL) && !(I2C1->SR1 & I2C_SR1_ADDR))
		{
			if (--timeout == 0x00) return 1;
		}
	}

	(void)I2C1->SR2;	/* Read status register to clear ADDR flag */

	/* Return 0, everything ok */
	return 0;
}
uint8_t i2c_stop()
{
	/* Wait till transmitter not empty */
	uint32_t timeout = I2C_TIMEOUT;

	while (((!(I2C1->SR1 & I2C_SR1_TXE)) || (!(I2C1->SR1 & I2C_SR1_BTF))))
	{
		if (--timeout == 0) return 1;
	}

	/* Generate stop */
	I2C1->CR1 |= I2C_CR1_STOP;

	/* Return 0, everything ok */
	return 0;
}

uint8_t i2c_read_ack()
{
	uint8_t data;

	I2C1->CR1 |= I2C_CR1_ACK;	// Enable ACK

	/* Wait till not received */
	uint32_t timeout = I2C_TIMEOUT;
	while (!(I2C1->SR2 & I2C_SR2_BUSY) && !(I2C1->SR2 & I2C_SR2_MSL) && !(I2C1->SR1 & I2C_SR1_RXNE))
	{
		if (--timeout == 0x00) return 1;
	}

	/* Read data */
	data = I2C1->DR;

	return data;
}
uint8_t i2c_read_nack()
{
	uint8_t data;

	I2C1->CR1 &= ~I2C_CR1_ACK;	// Disable ACK
	I2C1->CR1 |= I2C_CR1_STOP;	// Generate stop */

	/* Wait till received */
	uint32_t timeout = I2C_TIMEOUT;
	while (!(I2C1->SR2 & I2C_SR2_BUSY) && !(I2C1->SR2 & I2C_SR2_MSL) && !(I2C1->SR1 & I2C_SR1_RXNE))
	{
		if (--timeout == 0x00) return 1;
	}

	data = I2C1->DR;			// Read data

	return data;
}

void i2c_write_data(uint8_t data)
{
	/* Wait till I2C is not busy anymore */
	uint32_t timeout = I2C_TIMEOUT;
	while (!(I2C1->SR1 & I2C_SR1_TXE))
	{
		if (--timeout == 0x00) break;
	}

	/* Send data */
	I2C1->DR = data;
}

void i2c_errata_fix()
{
	/* 1. Disable I2C */
	I2C1->CR1 &= ~I2C_CR1_PE;

	/*2. */
	//Конфигурирование GPIOB.6
	GPIOB->CRL &= ~GPIO_CRL_MODE6;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF6;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE6_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF6_0;  //общего назначения, открытый сток
	GPIOB->ODR |= GPIO_ODR_ODR6;

	//Конфигурирование GPIOB.7
	GPIOB->CRL &= ~GPIO_CRL_MODE7;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF7;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE7_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF7_0;  //общего назначения, открытый сток
	GPIOB->ODR |= GPIO_ODR_ODR7;

	/* 3. */
	while(!(GPIOB->IDR &(GPIO_IDR_IDR6 | GPIO_IDR_IDR7)));

	/* 4. */
	GPIOB->ODR &= ~GPIO_ODR_ODR6;
	GPIOB->ODR &= ~GPIO_ODR_ODR7;

	/* 5. */
	while(GPIOB->IDR & GPIO_IDR_IDR7);

	/* 6. */
	GPIOB->CRL &= ~GPIO_CRL_MODE6;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF6;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE6_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF6_0;  //общего назначения, открытый сток
	GPIOB->ODR &= ~GPIO_ODR_ODR6;

	/* 7. */
	while(GPIOB->IDR & GPIO_IDR_IDR6);

	/* 8. */
	GPIOB->CRL &= ~GPIO_CRL_MODE6;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF6;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE6_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF6_0;  //общего назначения, открытый сток
	GPIOB->ODR |= GPIO_ODR_ODR6;

	/* 9. */
	while(!(GPIOB->IDR & GPIO_IDR_IDR6));

	/* 10. */
	GPIOB->CRL &= ~GPIO_CRL_MODE7;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF7;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE7_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF7_0;  //общего назначения, открытый сток
	GPIOB->ODR |= GPIO_ODR_ODR7;

	/* 11. */
	while(!(GPIOB->IDR & GPIO_IDR_IDR7));

	/* 12. */
	//GPIOB->CRL &= ~GPIO_CRL_MODE6;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF6;    //очистить разряды CNF
	//GPIOB->CRL |=  GPIO_CRL_MODE6_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF6;    //альтернативная функция, открытый сток

	//GPIOB->CRL &= ~GPIO_CRL_MODE7;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF7;    //очистить разряды CNF
	//GPIOB->CRL |=  GPIO_CRL_MODE7_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF7;    //альтернативная функция, открытый сток

	/* 13. */
	I2C1->CR1 |= I2C_CR1_SWRST;

	/* 14. */
	I2C1->CR1 &= ~I2C_CR1_SWRST;

	/* 15. */
	I2C1->CR1 |= I2C_CR1_PE;
}

/* public: */
void i2c_init()
{
	//SDL -> PB6
	//SDA -> PB7

//	//------------------------Грабли из Errata-------------------------------------------//
//	   GPIOB->CRL &= ~(GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
//	   GPIOB->CRL |= GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1 | GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0;
//	   I2C1->CR1 &= ~I2C_CR1_PE;
//	   GPIOB->ODR |= GPIO_ODR_ODR6 | GPIO_ODR_ODR7;
//	   while(!(GPIOB->IDR &(GPIO_IDR_IDR6 | GPIO_IDR_IDR7)));
//
//	   GPIOB->ODR &= ~(GPIO_ODR_ODR6);
//	   while(GPIOB->IDR & GPIO_IDR_IDR6);
//	   GPIOB->ODR &= ~(GPIO_ODR_ODR7);
//	   while(GPIOB->IDR & GPIO_IDR_IDR7 );
//
//	   GPIOB->ODR |= GPIO_ODR_ODR6;
//	   while(!(GPIOB->IDR & GPIO_IDR_IDR6));
//	   GPIOB->ODR |= GPIO_ODR_ODR7;
//	   while(!(GPIOB->IDR & GPIO_IDR_IDR7));
//
//	   GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
//	   GPIOB->CRL |= GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7;
//	   I2C1->CR1 |= I2C_CR1_SWRST;
//	   I2C1->CR1 &= ~I2C_CR1_SWRST;
//	   I2C1->CR2 &= ~I2C_CR2_FREQ;
//	   I2C1->CR2 |= 8;
//	   I2C1->CCR &= ~I2C_CCR_CCR;
//	   I2C1->CCR = 80;
//	   I2C1->TRISE = 3;
//	   I2C1->CR1 |= I2C_CR1_ACK;
//	   I2C1->CR1 |= I2C_CR1_PE;
//	   I2C1->CR1 |= I2C_CR1_START;
//	//------------------------------------------------------------------------------------//

    // Включаем тактирование портов и модуля I2C
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;   // Разрешить тактирование GPIOB
	RCC->APB1ENR |=	RCC_APB1ENR_I2C1EN;

/*
	//Конфигурирование GPIOB.8
	GPIOB->CRH &= ~GPIO_CRH_MODE8;   //очистить разряды MODE
	GPIOB->CRH &= ~GPIO_CRH_CNF8;    //очистить разряды CNF
	GPIOB->CRH |=  GPIO_CRH_MODE8_0; //выход, 10MHz
	GPIOB->CRH |=  GPIO_CRH_CNF8;    //альтернативная функция, симетричный

	//Конфигурирование GPIOB.9
	GPIOB->CRH &= ~GPIO_CRH_MODE9;   //очистить разряды MODE
	GPIOB->CRH &= ~GPIO_CRH_CNF9;    //очистить разряды CNF
	GPIOB->CRH |=  GPIO_CRH_MODE9_0; //выход, 10MHz
	GPIOB->CRH |=  GPIO_CRH_CNF9;	 //альтернативная функция, открытый сток
*/

	/* SCL - PB6 */
	GPIOB->CRL &= ~GPIO_CRL_MODE6;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF6;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE6_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF6;    //альтернативная функция, открытый сток

	/* SDA - PB7 */
	GPIOB->CRL &= ~GPIO_CRL_MODE7;   //очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF7;    //очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_MODE7_0; //выход, 10MHz
	GPIOB->CRL |=  GPIO_CRL_CNF7;    //альтернативная функция, открытый сток

	/* Remap pins
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;	// Тактирование альтернативных функций
	AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;	// Remap I2C1	*/

	/* I2C */
	I2C1->CR1 &= ~I2C_CR1_SMBUS;				// I2C Mode
	I2C1->CR2 &= ~I2C_CR2_FREQ;					// Указываем частоту тактирования модуля
	I2C1->CR2 = 36;								// 36 MHz (Частота APB1)
	I2C1->CCR &= ~(I2C_CCR_FS | I2C_CCR_DUTY); 	// Конфигурируем I2C, standart mode, 100 KHz duty cycle 1/2
	I2C1->CCR |= 180; 							// Задаем частоту работы модуля SCL. Это кол-во тактов APB1 на полупериод SCL (8МГц/100КГц/2)
	I2C1->TRISE = 37; 	//36					// (1000nS / 31nS)+1 [Standart_Mode = 1000nS, Fast_Mode = 300nS, 1/42MHz = 31nS]s
	I2C1->CR1 |= I2C_CR1_PE; 					// Включаем модуль

	//errata_fix();
}

uint8_t i2c_is_device_connected(uint8_t address)
{
	uint8_t connected = 0;

	/* Try to start, function will return 0 in case device will send ACK */
	if (!i2c_start(address, I2C_WRITE_MODE, I2C_ACK_ENABLE))
	{
		connected = 1;
	}

	/* STOP I2C */
	i2c_stop();

	/* Return status */
	return connected;
}

/* Read */
uint8_t i2c_read_byte(uint8_t address, uint8_t reg)
{
	uint8_t received_data;

	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_DISABLE);
	i2c_write_data(reg);
	i2c_stop();
	i2c_start(address, I2C_READ_MODE, I2C_ACK_DISABLE);

	received_data = i2c_read_nack();

	return received_data;
}
uint8_t i2c_read_byte_no_register(uint8_t address)
{
	uint8_t data;
	i2c_start(address, I2C_READ_MODE, I2C_ACK_ENABLE);
	/* STOP внутри */
	data = i2c_read_nack();

	return data;
}

void i2c_read_mem(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_ENABLE);
	i2c_write_data(reg);
	//i2c_stop();
	i2c_start(address, I2C_READ_MODE, I2C_ACK_ENABLE);

	while (count--)
	{
		if (!count) *data++ = i2c_read_nack(); // Last byte
		else  		*data++ = i2c_read_ack();
	}
}
void i2c_read_mem_no_register(uint8_t address, uint8_t* data, uint16_t count)
{
	i2c_start(address, I2C_READ_MODE, I2C_ACK_ENABLE);

	while (count--)
	{
		if (!count) *data = i2c_read_nack(); // Последний байт
		else 		*data = i2c_read_ack();
	}
}

/* Write */
void i2c_write_byte(uint8_t address, uint8_t reg, uint8_t data)
{
	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_DISABLE);
	i2c_write_data(reg);
	i2c_write_data(data);
	i2c_stop();
}
void i2c_write_byte_no_register(uint8_t address, uint8_t data)
{
	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_DISABLE);
	i2c_write_data(data);
	i2c_stop();
}
void i2c_write_mem(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_DISABLE);
	i2c_write_data(reg);

	while (count--)
	{
		i2c_write_data(*data++);
	}

	i2c_stop();
}
void i2c_write_mem_no_register(uint8_t address, uint8_t* data, uint16_t count)
{
	i2c_start(address, I2C_WRITE_MODE, I2C_ACK_DISABLE);

	while (count--)
	{
		i2c_write_data(*data++);
	}

	i2c_stop();
}

