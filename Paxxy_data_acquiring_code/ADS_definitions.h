/*
 * ADS_definitions.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Thusitha Samarasekara
 */

#ifndef ADS_DEFINITIONS_H_
#define ADS_DEFINITIONS_H_

/* ADS1298 SPI commands */

#define ADS1298_WAKEUP 0x02
#define ADS1298_STANDBY 0x04
#define ADS1298_RESET 0x06
#define ADS1298_START 0x08
#define ADS1298_STOP 0x0A

#define ADS1298_RDATAC 0x10
#define ADS1298_SDATAC 0x11
#define ADS1298_RDATA 0x12

#define ADS1298_RREG 0b00100000
#define ADS1298_WREG 0b01000000

/* ADS1298 addresses */

#define ADS1298_ID 0x00
#define ADS1298_CONFIG1 0x01
#define ADS1298_CONFIG2 0x02
#define ADS1298_CONFIG3 0x03
#define ADS1298_LOFF 0x04

#define ADS1298_CH1SET 0x05
#define ADS1298_CH2SET 0x06
#define ADS1298_CH3SET 0x07
#define ADS1298_CH4SET 0x08
#define ADS1298_CH5SET 0x09
#define ADS1298_CH6SET 0x0A
#define ADS1298_CH7SET 0x0B
#define ADS1298_CH8SET 0x0C
#define ADS1298_RLD_SENSP 0x0D
#define ADS1298_RLD_SENSN 0x0E
#define ADS1298_LOFF_SENSEP 0x0F
#define ADS1298_LOFF_SENSEN 0x10
#define ADS1298_LOFF_FLIP 0x11

#define ADS1298_LOFF_STATP 0x12
#define ADS1298_LOFF_STATN 0x13

#define ADS1298_GPIO 0x14
#define ADS1298_PACE 0x15
#define ADS1298_RESP 0x16
#define ADS1298_CONFIG4 0x17
#define ADS1298_WCT1 0x18
#define ADS1298_WCT2 0x19

/* ADS131M08 SPI commands */

#define ADS131_NULL 0x0000
#define ADS131_RESET 0x0011
#define ADS131_STANDBY 0x0022
#define ADS131_WAKEUP 0x0033

#define ADS131_LOCK 0x0555
#define ADS131_UNLOCK 0x0655

#define ADS131_RREG 0xA000
#define ADS131_WREG 0x6000

/* ADS131M08 addresses */

#define ADS131_ID 0x00
#define ADS131_STATUS 0x01
#define ADS131_MODE 0x02
#define ADS131_CLOCK 0x03
#define ADS131_GAIN1 0x04
#define ADS131_GAIN2 0x05
#define ADS131_CFG 0x06
#define ADS131_THRSHLD_MSB 0x07
#define ADS131_THRSHLD_LSB 0x08

#define ADS131_CH0_CFG 0x09
#define ADS131_CH0_OCAL_MSB 0x0A
#define ADS131_CH0_OCAL_LSB 0x0B
#define ADS131_CH0_GCAL_MSB 0x0C
#define ADS131_CH0_GCAL_LSB 0x0D

#define ADS131_CH1_CFG 0x0E
#define ADS131_CH1_OCAL_MSB 0x0F
#define ADS131_CH1_OCAL_LSB 0x10
#define ADS131_CH1_GCAL_MSB 0x11
#define ADS131_CH1_GCAL_LSB 0x12

#define ADS131_CH2_CFG 0x13
#define ADS131_CH2_OCAL_MSB 0x14
#define ADS131_CH2_OCAL_LSB 0x14
#define ADS131_CH2_GCAL_MSB 0x16
#define ADS131_CH2_GCAL_LSB 0x17

#define ADS131_CH3_CFG 0x18
#define ADS131_CH3_OCAL_MSB 0x19
#define ADS131_CH3_OCAL_LSB 0x1A
#define ADS131_CH3_GCAL_MSB 0x1B
#define ADS131_CH3_GCAL_LSB 0x1C

#define ADS131_CH4_CFG 0x1D
#define ADS131_CH4_OCAL_MSB 0x1E
#define ADS131_CH4_OCAL_LSB 0x1F
#define ADS131_CH4_GCAL_MSB 0x20
#define ADS131_CH4_GCAL_LSB 0x21

#define ADS131_CH5_CFG 0x22
#define ADS131_CH5_OCAL_MSB 0x23
#define ADS131_CH5_OCAL_LSB 0x24
#define ADS131_CH5_GCAL_MSB 0x25
#define ADS131_CH5_GCAL_LSB 0x26

#define ADS131_CH6_CFG 0x27
#define ADS131_CH6_OCAL_MSB 0x28
#define ADS131_CH6_OCAL_LSB 0x29
#define ADS131_CH6_GCAL_MSB 0x2A
#define ADS131_CH6_GCAL_LSB 0x2B

#define ADS131_CH7_CFG 0x2C
#define ADS131_CH7_OCAL_MSB 0x2D
#define ADS131_CH7_OCAL_LSB 0x2E
#define ADS131_CH7_GCAL_MSB 0x2F
#define ADS131_CH7_GCAL_LSB 0x30

#define ADS131_REGMAP_CRC 0x3E

//adc data buffer
struct adc_data
{
	int32_t channel[8];
};

//ADS sensor class 
struct ADS_sensor
{
	mraa_gpio_context cs;
	mraa_gpio_context drdy;
	mraa_gpio_context reset;
	uint8_t id;
	uint8_t data_ready;
	int8_t adc_ri;
	uint8_t initialized;
	struct adc_data adc_buffer[2];
	int adc_count;
	int int_count;
};


#endif /* ADS_DEFINITIONS_H_ */
