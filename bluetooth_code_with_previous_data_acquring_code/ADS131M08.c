
/* standard headers */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* mraa header */
#include "mraa/spi.h"
#include "mraa/gpio.h"

#include "definitions.h"
#include "spi.h"

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

/*
	register values
	ID - NN
	STATUS - 
*/

/* GPIO */


#define ADS131_CS_PIN 15		
#define ADS131_DRDY_PIN 18	
#define ADS131_RESET_PIN 3

#define MIC_DATA_BUFFER_LEN 3000		//500 samples per second x 6 seconds
#define MIC_DATA_BUFFER_WIDTH 4

//#define ENABLE_TEST_SIGNAL

static mraa_gpio_context ads131_cs, ads131_drdy, ads131_reset;

static unsigned char mic_data_ready = NO, data_log_started=NO;

static int32_t mic_data_buffer[MIC_DATA_BUFFER_LEN][MIC_DATA_BUFFER_WIDTH] = {0};
static int buff_i=0, sample_i=0;

typedef struct {
    int mic1;
    int mic2;
    int mic3;
    int mic4;
} AcousticVal;

void ADS131_int_handler(void* args)
{
//    fprintf(stdout, "Data ready\n");
	if(NO == mic_data_ready)
	{
		mic_data_ready = YES;
	}
	else
	{
		if(YES== data_log_started)
			printf("ADS131 sample miss %d\n", sample_i);
	}
}

int ADS131_init_gpio(void)
{
	mraa_result_t status = MRAA_SUCCESS;
	


	ads131_drdy = mraa_gpio_init(ADS131_DRDY_PIN);
    ads131_cs = mraa_gpio_init(ADS131_CS_PIN);
    ads131_reset = mraa_gpio_init(ADS131_RESET_PIN);
	
    if (NULL == ads131_drdy || NULL == ads131_cs || NULL == ads131_reset) 
    {
        fprintf(stderr, "Failed to initialize GPIO\n");
        mraa_deinit();
        return FAILED;
    }

    /* set GPIO to input/output */
    status = mraa_gpio_dir(ads131_cs, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(ads131_cs, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}
    
	status = mraa_gpio_dir(ads131_reset, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(ads131_reset, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}
	
    status = mraa_gpio_dir(ads131_drdy, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }

    /* configure ISR for GPIO */
    status = mraa_gpio_isr(ads131_drdy, MRAA_GPIO_EDGE_FALLING, &ADS131_int_handler, NULL);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to configure GPIO interrupt service routine\n");
        goto err_exit;
    }
    
    return SUCCEEDED;
    
err_exit:
    
    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	return FAILED;
}
    

unsigned char *ADS131_spi_write_buf(unsigned char *data, int len)
{
	unsigned char *rx;
	mraa_gpio_write(ads131_cs, 0);
	rx = ads_spi_write_buf(data, len);		
	mraa_gpio_write(ads131_cs, 1);
	return rx;
}
    
static int _configure_register(unsigned char register_address, unsigned short register_value)
{
	uint8_t tx_buff[32] = {0};
	uint8_t *rx_buff=NULL;
	union int16_values rv;
	
	rv.ui16 = register_value;
	
	tx_buff[0] = (uint8_t)(ADS131_WREG>>8) | (uint8_t)(register_address>>1);
	tx_buff[1] = (uint8_t)(register_address<<7);
	tx_buff[2] = 0;									//Padding LSB for 24bit word size
	tx_buff[3] = rv.ui8[1];							//Set register value
	tx_buff[4] = rv.ui8[0];								
	tx_buff[5] = 0;									//Padding LSB for 24bit word size
	
	tx_buff[6] = 0;
	tx_buff[7] = 0;
	tx_buff[8] = 0;									//CRC
	
	rx_buff = ADS131_spi_write_buf(tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word	
	
	usleep(10);

	tx_buff[0] = (uint8_t)(ADS131_RREG>>8) | (uint8_t)(register_address>>1);
	tx_buff[1] = (uint8_t)(register_address<<7);
	tx_buff[2] = 0;									//Padding LSB for 24bit word size
	tx_buff[3] = 0;
	tx_buff[4] = 0;								
	tx_buff[5] = 0;									//Padding LSB for 24bit word size
	
	rx_buff = ADS131_spi_write_buf(tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word	
	
	usleep(10);
	
	tx_buff[0] = 0;									//NULL command
	tx_buff[1] = 0;
	tx_buff[2] = 0;	
	
	rx_buff = ADS131_spi_write_buf(tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word	
	
	if(rv.ui8[1] != *(rx_buff) && rv.ui8[0] != *(rx_buff+1))
	{
		rv.ui8[1] = *(rx_buff);
		rv.ui8[0] = *(rx_buff+1);
		
		printf("ERROR: Configuration register verification failded. Register address - %x, Configured value - %x, Read value - %x\n", 
					register_address, register_value, rv.ui16);
		return FAILED;
	}
	else
	{
		return SUCCEEDED;
	}
	
}	
    
int ADS131_init_device(void)
{
	uint8_t tx_buff[32] = {0};
	uint8_t *rx_buff=NULL;
	int count=0;
	
	mraa_gpio_write(ads131_reset, 0);
	usleep(2000);
	
	mraa_gpio_write(ads131_reset, 1);
	
	while(0 == mraa_gpio_read(ads131_drdy))			//Wait for DRDY=1 after power to know whether ADC is ready accept commands
	{
		usleep(100);
		count++;
		if(count>100000)					//Wait for 10s
		{
			return FAILED;
		}
	}	

	if(FAILED == _configure_register(ADS131_CLOCK, 0x0019)) //0x000E
		return FAILED;
	
	//Configure test signals 
#ifdef ENABLE_TEST_SIGNAL

	if(FAILED == _configure_register(ADS131_CH0_CFG, 0x0002))
		return FAILED;
		
	if(FAILED == _configure_register(ADS131_CH1_CFG, 0x0003))
		return FAILED;
	
	if(FAILED == _configure_register(ADS131_CH2_CFG, 0x0002))
		return FAILED;
		
	if(FAILED == _configure_register(ADS131_CH3_CFG, 0x0003))
		return FAILED;
#else

					
#endif									
	//Configure MODE reg

	if(FAILED == _configure_register(ADS131_MODE, 0x0510))
		return FAILED;

	if(FAILED == _configure_register(ADS131_GAIN1, 0x3333))
		return FAILED;

	if(FAILED == _configure_register(ADS131_GAIN2, 0x3333))
		return FAILED;

	if(FAILED == _configure_register(ADS131_CFG, 0x0600))
		return FAILED;

	if(FAILED == _configure_register(ADS131_CLOCK, 0xFF19)) //0xFF0E
		return FAILED;

	


	return SUCCEEDED;
}

void ADS131_set_data_log_started(unsigned char state)
{
	data_log_started = state;
	if(NO == state)
	{
		buff_i = 0;
	}
}
    
unsigned char *ADS131_read_data(void)
{
	unsigned char tx_buff[32] = {0};
	unsigned char *rx_buff;
	
	rx_buff = ADS131_spi_write_buf(tx_buff,30);
		 
	return rx_buff; 
}
//mraa_result_t 	mraa_spi_transfer_buf (mraa_spi_context dev, uint8_t *data, uint8_t *rxbuf, int length)

int ADS131_data_read_task(void)
{
	union int24_values temp;
	struct timeval now;
	
	unsigned char *rx;
	int i,j;
	
	if(YES == mic_data_ready)
	{
		mic_data_ready = NO;
		
		rx = ADS131_read_data();
		gettimeofday(&now, NULL);
		
		temp.i32=0;
		temp.ui8[2] = *(rx);
		temp.ui8[1] = *(rx+1);
		temp.ui8[0] = *(rx+2);
			
//		printf("ADS131 %li %03li  - %d - %x \n",now.tv_sec, now.tv_usec, buff_i, temp.i24);
		
		for(i=1;i<=MIC_DATA_BUFFER_WIDTH;i++)
		{
			temp.i32 = 0;
			temp.ui8[2] = *(rx+(i*3));
			temp.ui8[1] = *(rx+(i*3)+1);
			temp.ui8[0] = *(rx+(i*3)+2);
			mic_data_buffer[buff_i][i-1] = (int32_t) temp.i24;
		}
		if(YES==data_log_started)
		{
			buff_i++;
			sample_i++;
			if(buff_i>=MIC_DATA_BUFFER_LEN)
			{
				//write data to csv
				buff_i = 0;
			}
		}
		
		return sample_i;
	}
	else
	{
		return FAILED;
	}
//	else if(0 == mraa_gpio_read(ads131_drdy))
//	{
//		rx = ADS131_read_data();
//		
//		for(i=1;i<=MIC_DATA_BUFFER_WIDTH;i++)
//		{
//			temp.i32 = 0;
//			temp.ui8[2] = *(rx+(i*3));
//			temp.ui8[1] = *(rx+(i*3)+1);
//			temp.ui8[0] = *(rx+(i*3)+2);
//			mic_data_buffer[buff_i][i-1] = temp.i32;
//		}
//		buff_i++;
//	}
}

AcousticVal ADS131_print_data_2_file(FILE *data_file, int buffer_index)
{
	AcousticVal accVal;
	int i;
	
	i = (buffer_index % MIC_DATA_BUFFER_LEN);
	
	if(0==i)
	{
		i = MIC_DATA_BUFFER_LEN-1;
	}
	else
	{
		i--;
	}
	
	if(i>=0 && i<MIC_DATA_BUFFER_LEN)
	{
		accVal.mic1 = mic_data_buffer[i][0];
        accVal.mic2 = mic_data_buffer[i][1];
        accVal.mic3 = mic_data_buffer[i][2];
        accVal.mic4 = mic_data_buffer[i][3];
		fprintf(data_file,"%d,%d,%d,%d\n",mic_data_buffer[i][0], mic_data_buffer[i][1],
			mic_data_buffer[i][2], mic_data_buffer[i][3]);
	}

	return accVal;

}