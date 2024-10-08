/*
 * INA226 - TI Current/Voltage/Power Monitor Code
 * Copyright (C) 2021 Craig Peacock
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "i2c.h"
#include "driver/i2c.h"
#include "ina226.h"
#include "esp_log.h"
#include "variables.h"
#include "filter_sma.h"

//#define portTICK_RATE_MS (portTICK_PERIOD_MS)

//i2c_master_dev_handle_t dev_handle;

static const char *TAG = "ina226";

void i2c_init(uint8_t i2c_master_port, uint8_t sda_io_num, uint8_t scl_io_num);
bool ina226_init(uint8_t i2c_master_port);
float ina226_voltage(uint8_t i2c_master_port);
float ina226_current(uint8_t i2c_master_port);
float ina226_power(uint8_t i2c_master_port);

//==============================================================================

bool ina226_init(uint8_t i2c_master_port)
{
	ESP_LOGI(TAG, "ina226_init__Start");

	// uint16_t CFG_REG_VAL;
	// //CFG_REG_VAL = 0x4127; // Average over 4 Samples
	// //CFG_REG_VAL = 0x4527; // Average over 16 Samples
	// CFG_REG_VAL = 0x4f27; // Average over 1024 Samples

	i2c_write_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CFG_REG, 0x8000);	// Reset
	i2c_write_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CFG_REG, INA226_CFG_REG_VAL);	
	i2c_write_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CAL_REG, 1024);	// 1A, 0.100Ohm Resistor
	uint16_t reg_data = 0;
	printf("Manufacturer ID:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_MANUFACTURER_ID));
	printf("Die ID Register:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_DIE_ID));
	reg_data = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CFG_REG);

	printf("Configuration Register: 0x%04X\r\n",reg_data);
	if (reg_data == INA226_CFG_REG_VAL) {
		return 1;
	} else {
		return 0;
	}
}
//==============================================================================

float ina226_voltage(uint8_t i2c_master_port){
	
	float fBusVoltage = 0;	
	//ESP_LOGI(TAG, "ina226_voltage__start");
	var.ina226.voltage_is_valid = 1;
	uint16_t iBusVoltage_filtred; 
	uint16_t iBusVoltage = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_BUS_VOLT_REG);

	if (iBusVoltage == 65535) {
		var.ina226.voltage_is_valid = 0;
	}

	if (var.filter.enabled) {

		iBusVoltage_filtred = Filter_SMA_V(iBusVoltage);
	} else {
		iBusVoltage_filtred = iBusVoltage;
	}

	var.ina226.voltage_i = iBusVoltage_filtred;

	fBusVoltage = float(iBusVoltage_filtred) * 0.00125;
	var.ina226.voltage_f = fBusVoltage;

	#ifdef DEBUG_LOG
		printf("Bus Voltage = %.2fV, ", fBusVoltage);
	#endif

	return (fBusVoltage);
}
//==============================================================================

float ina226_current(uint8_t i2c_master_port)
{
	unsigned int iCurrent;
	uint16_t iCurrent_filtred; 
	float fCurrent = 0;
	var.ina226.voltage_is_valid = 1;
	iCurrent = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CURRENT_REG);

	if (iCurrent == 65535) {
		var.ina226.voltage_is_valid = 0;
	}	

	if (var.filter.enabled) {

		iCurrent_filtred = Filter_SMA_I(iCurrent);
	} else {
		iCurrent_filtred = iCurrent;
	}

	// Internally Calculated as Current = ((ShuntVoltage * CalibrationRegister) / 2048)
	fCurrent = (float)iCurrent_filtred * 0.05;
	
	#ifdef DEBUG_LOG
		printf("Current = %.3fA\r\n", fCurrent);
	#endif	

	var.ina226.voltage_f = fCurrent;

	return (fCurrent);
}
//==============================================================================

float ina226_power(i2c_port_t i2c_master_port)
{
	unsigned int iPower = 0;
	float fPower = 0;

	iPower = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_POWER_REG);
	// The Power Register LSB is internally programmed to equal 25 times the programmed value of the Current_LSB
	fPower = iPower * 0.0125;

	//printf("Power = %.2fW\r\n", fPower);
	return (fPower);
}
//==============================================================================


void Get_Voltage() {

	ina226_voltage(I2C_CONTROLLER_0);

}


void Get_Current() {

	ina226_current(I2C_CONTROLLER_0);

}