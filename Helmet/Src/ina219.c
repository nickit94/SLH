#include "INA219.h"

/* The following multipliers are used to convert raw current and power values to mA and mW, taking into account the current config settings */
uint32_t ina219_currentDivider_mA;
uint32_t ina219_powerMultiplier_mW;
uint32_t ina219_calValue;

/* Sends a single command byte over I2C */
void wireWriteRegister (uint8_t reg, uint16_t value)
{
    uint8_t i2c_temp[2];
    i2c_temp[0] = value>>8;
    i2c_temp[1] = value;

    i2c_write_mem(INA219_ADDRESS, reg, i2c_temp, 2);

    //HAL_I2C_Mem_Write(&hi2c1, INA219_ADDRESS << 1, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)i2c_temp, 2, HAL_MAX_DELAY);
    HAL_Delay(1);
}

/* Reads a 16 bit values over I2C */
void wireReadRegister(uint8_t reg, uint16_t *value)
{
    uint8_t i2c_temp[2];

    i2c_read_mem(INA219_ADDRESS, reg, i2c_temp, 2);

    //HAL_I2C_Mem_Read(&hi2c1, INA219_ADDRESS << 1, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)i2c_temp, 2, HAL_MAX_DELAY);
    HAL_Delay(1);
    *value = ((uint16_t)i2c_temp[0]<<8 )|(uint16_t)i2c_temp[1];
}

/**************************************************************************/
/*!
    @brief  Configures to INA219 to be able to measure up to 32V and 3A
            of current.  Each unit of current corresponds to 100uA, and
            each unit of power corresponds to 2mW. Counter overflow
            occurs at 3.2A.

    @note   These calculations assume a 0.1 ohm resistor is present
*/
/**************************************************************************/
void setCalibration_16V_3A()
{
	// VBUS_MAX = 16V             (Assumes 32V, can also be set to 16V)
	// VSHUNT_MAX = 0.32          (Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
	// RSHUNT = 0.1               (Resistor value in ohms)

	// 1. Determine max possible current
	// MaxPossible_I = VSHUNT_MAX / RSHUNT
	// MaxPossible_I = 3.2A

	// 2. Determine max expected current
	// MaxExpected_I = 3.0A

	// 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
	// MinimumLSB = MaxExpected_I/32768
	// MinimumLSB = 0.000092              (92uA per bit)
	// MaximumLSB = MaxExpected_I/4096
	// MaximumLSB = 0,000732              (732uA per bit)

	// 4. Choose an LSB between the min and max values
	//    (Preferrably a roundish number close to MinLSB)
	// CurrentLSB = 0.0001 (100uA per bit)

	// 5. Compute the calibration register
	// Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
	// Cal = 4096 (0x1000)

	  ina219_calValue = 4096;

	// 6. Calculate the power LSB
	// PowerLSB = 20 * CurrentLSB
	// PowerLSB = 0.002 (2mW per bit)

	// 7. Compute the maximum current and shunt voltage values before overflow
	//
	// Max_Current = Current_LSB * 32767
	// Max_Current = 3.2767A before overflow
	//
	// If Max_Current > Max_Possible_I then
	//    Max_Current_Before_Overflow = MaxPossible_I
	// Else
	//    Max_Current_Before_Overflow = Max_Current
	// End If
	//
	// Max_Current_Before_Overflow = 3.2
	//
	// Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
	// Max_ShuntVoltage = 0.32V
	//
	// If Max_ShuntVoltage >= VSHUNT_MAX
	//    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
	// Else
	//    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
	// End If

	// 8. Compute the Maximum Power
	// MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
	// MaximumPower = 3.2 * 32V
	// MaximumPower = 102.4W

	// Set multipliers to convert raw current/power values
	ina219_currentDivider_mA = 10;  // Current LSB = 100uA per bit (1000/100 = 10)
	ina219_powerMultiplier_mW = 2;     // Power LSB = 1mW per bit (2/1)

	// Set Calibration register to 'Cal' calculated above
	wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

	// Set Config register to take into account the settings above
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
					  INA219_CONFIG_GAIN_8_320MV |
					  INA219_CONFIG_BADCRES_12BIT |
					  INA219_CONFIG_SADCRES_12BIT_1S_532US |
					  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	wireWriteRegister(INA219_REG_CONFIG, config);
}

/********************************************************************/

/* Gets the raw bus voltage (16-bit signed integer, so +-32767)
 * return the raw bus voltage reading */
int16_t getBusVoltage_raw()
{
  uint16_t value;
  wireReadRegister(INA219_REG_BUSVOLTAGE, &value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

/* Gets the raw shunt voltage (16-bit signed integer, so +-32767)
 * return the raw shunt voltage reading */
int16_t getShuntVoltage_raw()
{
  uint16_t value;
  wireReadRegister(INA219_REG_SHUNTVOLTAGE, &value);

  return (int16_t)value;
}

/* Gets the raw current value (16-bit signed integer, so +-32767)
 * return the raw current reading! */
int16_t getCurrent_raw()
{
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

  // Now we can safely read the CURRENT register!
  wireReadRegister(INA219_REG_CURRENT, &value);

  return (int16_t)value;
}

/* Gets the raw power value (16-bit signed integer, so +-32767)
 * return raw power reading! */
int16_t getPower_raw()
{
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

  // Now we can safely read the POWER register!
  wireReadRegister(INA219_REG_POWER, &value);

  return (int16_t)value;
}

/* Gets the shunt voltage in mV (so +-327mV)
 * return the shunt voltage converted to millivolts */
float getShuntVoltage_mV()
{
  int16_t value;
  value = getShuntVoltage_raw();

  return value * 0.01;
}

/* Gets the shunt voltage in volts
 * return the bus voltage converted to volts */
float getBusVoltage_V()
{
  int16_t value = getBusVoltage_raw();

  return value * 0.001;
}

/* Gets the current value in mA, taking into account the config settings and current LSB
 * return the current reading convereted to milliamps */
float getCurrent_mA()
{
  float valueDec = getCurrent_raw();
  valueDec /= ina219_currentDivider_mA;

  return valueDec;
}

/* Gets the power value in mW, taking into account the config settings and current LSB
 * Return power reading converted to milliwatts */
float getPower_mW()
{
  float valueDec = getPower_raw();
  valueDec *= ina219_powerMultiplier_mW;

  return valueDec;
}


/* Описание:
 *
 * Датчик можно использовать и без предварительных калибровок. По умолчанию доступно получение напряжения на шунте и напряжение на шине.
 * Но если вбить калибровочные значения, то можно получать значение тока и мощности.
 *
 *
 *  */





