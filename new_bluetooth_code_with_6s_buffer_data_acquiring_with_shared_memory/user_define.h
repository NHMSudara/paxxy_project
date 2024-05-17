#ifndef USER_DEFINE_H__
#define USER_DEFINE_H__

#define DEBUG_EN

//#define ADS131
#define ADS1298
#define BHI260AP

//#define ENABLE_ADS1298_TEST_SIGNAL
//#define ENABLE_ADS131_TEST_SIGNAL

#define VS_DATA_UPDATE_RATE 50.0		//virtual sensor data update rate

#define MAX_BHI_ACC_SENSOR_COUNT 4
#define BHI_SENSOR1
//#define BHI_SENSOR2
//#define BHI_SENSOR3
//#define BHI_SENSOR4
//#define BHI_SENSOR5

#define ECG_DATA_TYPE 8
#define ACC_DATA_TYPE 256

#define SW_PIN 28
#define LED_PIN 30


/* SPI declaration */
#define BHI_SPI_BUS 2

/* SPI frequency in Hz */
#define BHI_SPI_FREQ 500000			//500k


#define BHI_SENSOR1_CS_PIN 7
#define BHI_SENSOR1_INT_PIN 11

#define BHI_SENSOR2_CS_PIN 32
#define BHI_SENSOR2_INT_PIN 34

#define BHI_SENSOR3_CS_PIN 43
#define BHI_SENSOR3_INT_PIN 44

#define BHI_SENSOR4_CS_PIN 45
#define BHI_SENSOR4_INT_PIN 46

#define BHI_SENSOR5_CS_PIN 5
#define BHI_SENSOR5_INT_PIN 18



/* SPI declaration */
#define ADS_SPI_BUS 1

/* SPI frequency in Hz */
#define ADS_SPI_FREQ 1000000		//1 MHz



/* ADS1298_GPIO */

#define ADS1298_CS_PIN 13
#define ADS1298_DRDY_PIN 12
#define ADS1298_RESET_PIN 22


/* ADS131M08 GPIO */

#define ADS131_CS_PIN 15
#define ADS131_DRDY_PIN 16
#define ADS131_RESET_PIN 3

/* ADS1298 register configurations */

#define CONFIG1_VALUE 0x86
#define CONFIG2_VALUE 0x10
#define CONFIG3_VALUE 0xDC

#define LOFF_VALUE 0x03

#define CHxSET_VALUE 0x60

#define RLD_SENSP_VALUE 0x86
#define RLD_SENSN_VALUE 0x06
#define LOFF_SENSEP_VALUE 0x00
#define LOFF_SENSEN_VALUE 0x00
#define LOFF_FLIP_VALUE 0x00

#define LOFF_STATP_VALUE 0x00
#define LOFF_STATN_VALUE 0x00

#define GPIO_VALUE 0x00
#define PACE_VALUE 0x00
#define RESP_VALUE 0xF0
#define CONFIG4_VALUE 0x00
#define WCT1_VALUE 0x02
#define WCT2_VALUE 0x23

#endif

#define SERVER_TCP_PORT 6789
