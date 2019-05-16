#include "INA219.h"

/* The following multipliers are used to convert raw current and power values to mA and mW, taking into account the current config settings */
uint32_t ina219_current_divider_mA;
uint32_t ina219_power_multiplier_mW;
uint32_t ina219_calValue;

/* Sends a single command byte over I2C */
void ina219_write_register(uint8_t device_addr, uint8_t reg, uint16_t value)
{
    uint8_t i2c_temp[2];
    i2c_temp[0] = value>>8;
    i2c_temp[1] = value;

    i2c_write_mem(device_addr, reg, i2c_temp, 2);
}

/* Reads a 16 bit values over I2C */
void ina219_read_register(uint8_t devive_addr, uint8_t reg, uint16_t *value)
{
    uint8_t i2c_temp[2];

    i2c_read_mem(devive_addr, reg, i2c_temp, 2);
    *value = ((uint16_t)i2c_temp[0]<<8 )|(uint16_t)i2c_temp[1];
}

/* Calibration 0x05 register */
void ina219_calibration_16v_3a(uint8_t devive_addr)
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
	ina219_current_divider_mA = 10;  	 // Current LSB = 100uA per bit (1000/100 = 10)
	ina219_power_multiplier_mW = 2;     // Power LSB = 1mW per bit (2/1)

	// Set Calibration register to 'Cal' calculated above
	ina219_write_register(devive_addr, INA219_REG_CALIBRATION, ina219_calValue);

	// Set Config register to take into account the settings above
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
					  INA219_CONFIG_GAIN_8_320MV |
					  INA219_CONFIG_BADCRES_12BIT |
					  INA219_CONFIG_SADCRES_12BIT_1S_532US |
					  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	ina219_write_register(devive_addr, INA219_REG_CONFIG, config);
}


/* public: */
void ina219_init()
{
	/* Set Calibration 16V 3A */
	ina219_calibration_16v_3a(INA219_LEFT_ADDR);
	ina219_calibration_16v_3a(INA219_RIGHT_ADDR);
}

int16_t ina219_get_bus_voltage_mV(uint8_t devive_addr)
{
  uint16_t value;
  ina219_read_register(devive_addr, INA219_REG_BUSVOLTAGE, &value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

int16_t ina219_get_shunt_voltage_uV(uint8_t devive_addr)
{
  uint16_t value;
  ina219_read_register(devive_addr, INA219_REG_SHUNTVOLTAGE, &value);

  return (int16_t)(value * 10);
}

int16_t ina219_get_current_mA(uint8_t devive_addr)
{
  uint16_t value;

  ina219_write_register(devive_addr, INA219_REG_CALIBRATION, ina219_calValue);
  ina219_read_register(devive_addr, INA219_REG_CURRENT, &value);

  return (int16_t)(value / ina219_current_divider_mA);
}

int16_t ina219_get_power_mW(uint8_t devive_addr)
{
  uint16_t value;

  ina219_write_register(devive_addr, INA219_REG_CALIBRATION, ina219_calValue);
  ina219_read_register(devive_addr, INA219_REG_POWER, &value);

  return (int16_t)(value * ina219_power_multiplier_mW);
}
