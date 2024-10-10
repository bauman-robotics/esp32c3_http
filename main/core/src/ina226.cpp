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

void ina226_Set_Coeff_Default();
void ina226_Calc_Coeff(); 

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
	i2c_write_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CAL_REG, var.ina226.calibr.CALIBR_VAL);	// 1A, 0.100Ohm Resistor
	uint16_t reg_data = 0;
	printf("Manufacturer ID:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_MANUFACTURER_ID));
	printf("Die ID Register:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_DIE_ID));
	printf("Die INA226_CAL_REG:     %d\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CAL_REG));	
	reg_data = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CFG_REG);

	printf("Configuration Register: 0x%04X\r\n",reg_data);
	if (reg_data == INA226_CFG_REG_VAL) {
		return 1;
	} else {
		return 0;
	}
	ESP_LOGI(TAG, "var.ina226.is_init = %d", (int)var.ina226.is_init);
}
//==============================================================================

float ina226_voltage(uint8_t i2c_master_port){
	
	float fBusVoltage = 0;	
	//ESP_LOGI(TAG, "ina226_voltage__start");
	var.ina226.value_is_valid = 1;
	uint16_t iBusVoltage_filtred; 
	uint16_t iBusVoltage = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_BUS_VOLT_REG);

	if (iBusVoltage == 65535) {
		var.ina226.value_is_valid = 0;
	}

	if (var.filter.enabled) {

		iBusVoltage_filtred = Filter_SMA_V(iBusVoltage);
	} else {
		iBusVoltage_filtred = iBusVoltage;
	}

	var.ina226.voltage_i = iBusVoltage_filtred;

	//Bus Voltage Register (02h) is a fixed 1.25 mV/bit
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
	var.ina226.value_is_valid = 1;
	iCurrent = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CURRENT_REG);

	if (iCurrent == 65535) {
		var.ina226.value_is_valid = 0;
	}	

	if (var.filter.enabled) {

		iCurrent_filtred = Filter_SMA_I(iCurrent);
	} else {
		iCurrent_filtred = iCurrent;
	}

	var.ina226.current_i = iCurrent_filtred;  // only for logs 

	fCurrent = (float)iCurrent_filtred * var.ina226.calibr.LSB_mA; // mA
	
	#ifdef DEBUG_LOG
		printf("Current = %.3fA\r\n", fCurrent);
	#endif	

	var.ina226.current_f = fCurrent;  // for Debug 

	return (fCurrent);
}
//==============================================================================

float ina226_power(uint8_t i2c_master_port)
{
	unsigned int iPower = 0;
	float fPower = 0;
	uint16_t iPower_filtred = 0; 

	iPower = i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_POWER_REG);


	if (var.filter.enabled) {

		iPower_filtred = Filter_SMA_P(iPower);
	} else {
		iPower_filtred = iPower;
	}


	// The Power Register LSB is internally programmed to equal 25 times the programmed value of the Current_LSB
	float lsb_Pow_mW = var.ina226.calibr.LSB_mA * 25; 
	fPower = (float)iPower_filtred * lsb_Pow_mW; //* 0.0125;* var.ina226.calibr.LSB_mA
	var.ina226.power_i = iPower_filtred;  // for debug 
	//printf("Power = %.2fW\r\n", fPower);
	return (fPower);
}
//==============================================================================

float Get_Power() {
	float result;
	result = ina226_power(I2C_CONTROLLER_0);

	return result;
}
//==============================================================================

float Get_Voltage() {
	float result;
	result = ina226_voltage(I2C_CONTROLLER_0);
	return result;	
}
//==============================================================================

float Get_Current() {
	float result;
	result = ina226_current(I2C_CONTROLLER_0);
	return result;		
}
//==============================================================================

void ina226_Set_Coeff_Default() {
	var.ina226.calibr.I_lim_mA = MAX_EXPECTED_CURRENT_mA;
	var.ina226.calibr.R_shunt_Om =(float)INA226_R_SHUNT_Om;
	ina226_Calc_Coeff();
	var.ina226.is_init = 0;
}
//==============================================================================

void ina226_Calc_Coeff() {

	var.ina226.calibr.LSB_mA         = (float)var.ina226.calibr.I_lim_mA / 32768; // 2**15 = 32768
	var.ina226.calibr.CALIBR_VAL    = (uint16_t)( 5.12 / (var.ina226.calibr.LSB_mA  * var.ina226.calibr.R_shunt_Om));
}
//==============================================================================

void ina226_Calibr_Logs() {

	ESP_LOGI(TAG, "I_lim_mA      = %d",    var.ina226.calibr.I_lim_mA);
	//ESP_LOGI(TAG, "R_shunt_Om    = %0.1f", var.ina226.calibr.R_shunt_Om);
	ESP_LOGI(TAG, "LSB_mA        = %0.5f", var.ina226.calibr.LSB_mA);
	ESP_LOGI(TAG, "CALIBR_VAL    = %d",    var.ina226.calibr.CALIBR_VAL);
	//ESP_LOGI(TAG, "Voltage_LSB   = %s",  " is a fixed 1.25 mV/bit");
}


