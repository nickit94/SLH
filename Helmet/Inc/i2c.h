#ifndef I2C_H_
#define I2C_H_

#include "main.h"

#define ADDR 0x40

#define I2C_WRITE_MODE   	0
#define I2C_READ_MODE      	1
#define I2C_ACK_ENABLE  	1
#define I2C_ACK_DISABLE 	0
#define I2C_TIMEOUT			20000
#define I2C_ADDR(addr, rw)	((addr << 1) | rw)

#define I2C_FLAG_SMBALERT               0x00018000U
#define I2C_FLAG_TIMEOUT                0x00014000U
#define I2C_FLAG_PECERR                 0x00011000U
#define I2C_FLAG_OVR                    0x00010800U
#define I2C_FLAG_AF                     0x00010400U
#define I2C_FLAG_ARLO                   0x00010200U
#define I2C_FLAG_BERR                   0x00010100U
#define I2C_FLAG_TXE                    0x00010080U
#define I2C_FLAG_RXNE                   0x00010040U
#define I2C_FLAG_STOPF                  0x00010010U
#define I2C_FLAG_ADD10                  0x00010008U
#define I2C_FLAG_BTF                    0x00010004U
#define I2C_FLAG_ADDR                   0x00010002U
#define I2C_FLAG_SB                     0x00010001U
#define I2C_FLAG_DUALF                  0x00100080U
#define I2C_FLAG_SMBHOST                0x00100040U
#define I2C_FLAG_SMBDEFAULT             0x00100020U
#define I2C_FLAG_GENCALL                0x00100010U
#define I2C_FLAG_TRA                    0x00100004U
#define I2C_FLAG_BUSY                   0x00100002U
#define I2C_FLAG_MSL                    0x00100001U

void i2c_init();
uint8_t i2c_is_device_connected(uint8_t address);

uint8_t i2c_read_byte(uint8_t address, uint8_t reg);
uint8_t i2c_read_byte_no_register(uint8_t address);
void i2c_read_mem(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);
void i2c_read_mem_no_register(uint8_t address, uint8_t* data, uint16_t count);

void i2c_write_byte(uint8_t address, uint8_t reg, uint8_t data);
void i2c_write_byte_no_register(uint8_t address, uint8_t data);
void i2c_write_mem(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);
void i2c_write_mem_no_register(uint8_t address, uint8_t* data, uint16_t count);

#endif
