

/* standard headers */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* mraa header */
#include "mraa/spi.h"
#include "mraa/gpio.h"

#include "definitions.h"
#include "ADS_definitions.h"


#include "user_define.h"



//#define DATA_BUFFER_LEN 3000		//500 samples per second x 6 seconds
//#define DATA_BUFFER_WIDTH 8

//#define ENABLE_ADS1298_TEST_SIGNAL

//static mraa_gpio_context cs, drdy, reset;

//static unsigned char data_ready = NO, data_log_started=NO;
//
//static int32_t ecg_data_buffer[DATA_BUFFER_LEN][DATA_BUFFER_WIDTH] = {0};
//static int buff_i=0, sample_i=0;

extern void wake(void);

static mraa_spi_context ads_spi;



int ads_spi_init(void)
{
	mraa_result_t status = MRAA_SUCCESS;

	/* initialize mraa for the platform (not needed most of the times) */
    mraa_init();

	/* initialize SPI bus */
    ads_spi = mraa_spi_init_raw(ADS_SPI_BUS,0);
    if (ads_spi == NULL)
    {
        fprintf(stderr, "Failed to initialize SPI\n");
        mraa_deinit();
        return FAILED;
    }
    /* set SPI frequency */
    status = mraa_spi_frequency(ads_spi, ADS_SPI_FREQ);
    if (status != MRAA_SUCCESS)
	{
		fprintf(stdout, "Failed to set SPI frequency to %d Hz\n", ADS_SPI_FREQ);
		goto err_exit;
	}
    /* set big endian mode */
    status = mraa_spi_lsbmode(ads_spi, 0);
    if (status != MRAA_SUCCESS)
    {
		goto err_exit;
    }
    /* set SPI mode */
    status = mraa_spi_mode(ads_spi, MRAA_SPI_MODE1);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stdout, "Failed to set SPI mode to 1\n");
        goto err_exit;
	}

//    status = mraa_spi_bit_per_word(ads_spi, 16);
//    if (status != MRAA_SUCCESS)
//    {
//        fprintf(stdout, "Failed to set SPI Device to 16Bit mode\n");
//        goto err_exit;
//    }

    return SUCCEEDED;

err_exit:

    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	return FAILED;
}

void ads1298_int_handler(void* args)
{
	struct ADS_sensor *sensor = (struct ADS_sensor *)args;

//	if(0 == mraa_gpio_read(sensor->drdy))
//	{
		if(NO==sensor->data_ready)
		{
			sensor->data_ready = YES;
			wake();
		}
		else
		{
			if(YES==sensor->initialized)
				printf("ADS1298 sample miss\n");
		}
		sensor->int_count++;
//	}

}

int ADS1298_init_gpio(struct ADS_sensor *sensor, uint8_t id)
{
	mraa_result_t status = MRAA_SUCCESS;
	int i,j;

    sensor->id = id;
    sensor->data_ready = NO;
    sensor->adc_ri = 0;
    sensor->initialized = NO;

    for(i=0;i<2;i++)
    {
    	for(j=0;j<8;j++)
    	{
    		sensor->adc_buffer[i].channel[j] = 0;
    	}
    }
	sensor->adc_count = 0;
	sensor->int_count = 0;

	sensor->drdy = mraa_gpio_init(ADS1298_DRDY_PIN);
	sensor->cs = mraa_gpio_init(ADS1298_CS_PIN);
	sensor->reset = mraa_gpio_init(ADS1298_RESET_PIN);

    if (NULL == sensor->drdy || NULL == sensor->cs || NULL == sensor->reset)
    {
        fprintf(stderr, "Failed to initialize GPIO\n");
        mraa_deinit();
        return FAILED;
    }

    /* set ADS1298_GPIO to input/output */
    status = mraa_gpio_dir(sensor->cs, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set ADS1298_GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(sensor->cs, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set ADS1298_GPIO output value\n");
		goto err_exit;
	}
	
	status = mraa_gpio_dir(sensor->reset, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set ADS1298_GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(sensor->reset, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set ADS1298_GPIO output value\n");
		goto err_exit;
	}

#ifdef ADS1298
    status = mraa_gpio_dir(sensor->drdy, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }

    /* configure ISR for ADS1298_GPIO */
    status = mraa_gpio_isr(sensor->drdy, MRAA_GPIO_EDGE_FALLING, &ads1298_int_handler, sensor);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to configure ADS1298_GPIO interrupt service routine\n");
        goto err_exit;
    }
#endif

    return SUCCEEDED;
    
err_exit:
    
    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	return FAILED;
}
    
uint8_t ads1298_spi_write(struct ADS_sensor *sensor, unsigned char data)
{
	uint8_t rx;
	mraa_gpio_write(sensor->cs, 0);
	rx = mraa_spi_write(ads_spi, data);
	mraa_gpio_write(sensor->cs, 1);
	return rx;
}


uint8_t *ads1298_spi_write_buf(struct ADS_sensor *sensor, unsigned char *data, int len)
{
	uint8_t *rx;
	mraa_gpio_write(sensor->cs, 0);
	rx = mraa_spi_write_buf(ads_spi, data, len);
	mraa_gpio_write(sensor->cs, 1);
	return rx;
}
    

static int ads1298_configure_register(struct ADS_sensor *sensor, unsigned char register_address, unsigned char register_value)
{
	uint8_t tx_buff[16] = {0};
	uint8_t *rx_buff=NULL;
	
	tx_buff[0] = ADS1298_WREG | (register_address & 0x1F);		
	tx_buff[1] = 0;						
	tx_buff[2] = register_value;				
	rx_buff = ads1298_spi_write_buf(sensor, tx_buff, 3);
	
	usleep(10);
	
	tx_buff[0] = ADS1298_RREG | (register_address & 0x1F);		
	tx_buff[1] = 0;						
	tx_buff[2] = 0;			
	rx_buff = ads1298_spi_write_buf(sensor, tx_buff, 3);
	
	if(register_value != *(rx_buff+2))
	{
		printf("ERROR: ADS1298 Configuration register verification failed. Register address - %x, Configured value - %x, Read value - %x\n",
					register_address, register_value, *(rx_buff+2));
		return FAILED;
	}
	else
	{
		return SUCCEEDED;
	}
}

static int ads1298_configure_multiple_registers(struct ADS_sensor *sensor, unsigned char register_address, unsigned char register_count, unsigned char register_value)
{
	unsigned char tx_buff[16] = {0};
	unsigned char *rx_buff=NULL;
	int i;
	
	if(register_count<9)
	{
		tx_buff[0] = ADS1298_WREG | (register_address & 0x1F);		
		tx_buff[1] = 0x1F & (register_count-1);
		for(i=0;i<register_count;i++)
		{	
			tx_buff[2+i] = register_value;
		}				
		rx_buff = ads1298_spi_write_buf(sensor, tx_buff, register_count+2);
		
		usleep(10);
		
		tx_buff[0] = ADS1298_RREG | (register_address & 0x1F);
		tx_buff[1] = 0x1F & (register_count-1);		
		for(i=0;i<register_count;i++)
		{	
			tx_buff[2+i] = 0;
		}
	
		rx_buff = ads1298_spi_write_buf(sensor, tx_buff, register_count+2);
		
		for(i=0;i<register_count;i++)
		{
			if(register_value != *(rx_buff+2+i))
			{
				printf("ERROR: ADS1298 Configuration register verification failded. Register address - %x, Configured value - %x, Read value - %x\n", 
							register_address+i, register_value, *(rx_buff+2+i));
				return FAILED;
			}
		}
		return SUCCEEDED;
	}
	else
	{
		printf("ERROR: Register count must be less than 9\n");
		return FAILED;
	}
}
    
int ADS1298_init_device(struct ADS_sensor *sensor)
{
	unsigned char tx_buff[16] = {0};
	unsigned char *rx_buff=NULL;
	int i;
	
	mraa_gpio_write(sensor->reset, 0);
	usleep(500);
	
	mraa_gpio_write(sensor->reset, 1);
	usleep(1000);						//Initial power up delay
	ads1298_spi_write(sensor, ADS1298_SDATAC);			//Stop continuous mode
	
	usleep(10);

//	mraa_gpio_write(sensor->reset, 1);
//	usleep(1000);						//Initial power up delay
	ads1298_spi_write(sensor, ADS1298_STOP);			//Stop conversion
	
	usleep(10);
	
	if(FAILED == ads1298_configure_register(sensor, ADS1298_CONFIG1, CONFIG1_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_CONFIG3, CONFIG3_VALUE))
		return FAILED;
	//Configure test signals
#ifdef ENABLE_ADS1298_TEST_SIGNAL
	
	if(FAILED == ads1298_configure_register(sensor, ADS1298_CONFIG2, 0x14))
		return FAILED;
	
	if(FAILED == ads1298_configure_multiple_registers(sensor, ADS1298_CH1SET, 8, 0x05))
		return FAILED;
	
#else

	if(FAILED == ads1298_configure_register(sensor, ADS1298_CONFIG2, CONFIG2_VALUE))
		return FAILED;

	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF, LOFF_VALUE))
		return FAILED;
	
	if(FAILED == ads1298_configure_multiple_registers(sensor, ADS1298_CH1SET, 8, CHxSET_VALUE))
		return FAILED;
		
	if(FAILED == ads1298_configure_register(sensor, ADS1298_RLD_SENSP, RLD_SENSP_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_RLD_SENSN, RLD_SENSN_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF_SENSEP, LOFF_SENSEP_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF_SENSEN, LOFF_SENSEN_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF_FLIP, LOFF_FLIP_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF_STATP, LOFF_STATP_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_LOFF_STATN, LOFF_STATN_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_GPIO, GPIO_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_PACE, PACE_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_RESP, RESP_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_CONFIG4, CONFIG4_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_WCT1, WCT1_VALUE))
		return FAILED;
	if(FAILED == ads1298_configure_register(sensor, ADS1298_WCT2, WCT2_VALUE))
		return FAILED;
		
#endif
	
	ads1298_spi_write(sensor, ADS1298_START);			//Start conversion
	
	usleep(10);
	
	ads1298_spi_write(sensor, ADS1298_RDATAC);			//Start continous data read
	
	sensor->initialized = YES;
	return SUCCEEDED;
}
    

unsigned char *ads1298_read_data(struct ADS_sensor *sensor)
{
	unsigned char tx_buff[28] = {0};
	unsigned char *rx_buff;
	
	rx_buff = ads1298_spi_write_buf(sensor, tx_buff,27);
		 
	return rx_buff; 
}
//mraa_result_t 	mraa_spi_transfer_buf (mraa_spi_context dev, uint8_t *data, uint8_t *rxbuf, int length)

int ADS1298_get_and_process_data(struct ADS_sensor *sensor)
{
	union int24_values temp;
	struct timeval now;
	
	unsigned char *rx;
	int i, ri;
	
	rx = ads1298_read_data(sensor);
//		gettimeofday(&now, NULL);

	temp.i32=0;
	temp.ui8[2] = *(rx);
	temp.ui8[1] = *(rx+1);
	temp.ui8[0] = *(rx+2);

	if(0xC0 != (temp.ui8[2] & 0xF0))
		return FAILED;
	//temp.ui8[0] = 0;

//		printf("ADS1298 %li %03li - %d - %x\n", now.tv_sec, now.tv_usec, buff_i, (0x00FFFFFF & temp.i24));

	//Do some status bit validation
	ri = sensor->adc_ri+1;
	if(ri>1)
	{
		ri=0;
	}

	for(i=1;i<9;i++)
	{
		temp.i32 = 0;
		temp.ui8[2] = *(rx+(i*3));
		temp.ui8[1] = *(rx+(i*3)+1);
		temp.ui8[0] = *(rx+(i*3)+2);
		sensor->adc_buffer[ri].channel[i-1] = (int32_t) (temp.i24);
	}
	sensor->adc_ri = ri;
	sensor->adc_count++;

	return SUCCEEDED;
//	else if(0 == mraa_gpio_read(drdy))
//	{
//		rx = ADS1298_read_data();
//		
//		for(i=1;i<=DATA_BUFFER_WIDTH;i++)
//		{
//			temp.i32 = 0;
//			temp.ui8[2] = *(rx+(i*3));
//			temp.ui8[1] = *(rx+(i*3)+1);
//			temp.ui8[0] = *(rx+(i*3)+2);
//			ecg_data_buffer[buff_i][i-1] = temp.i32;
//		}
//		buff_i++;
//	}
}


void ads131_int_handler(void* args)
{
	struct ADS_sensor *sensor = (struct ADS_sensor *)args;

	if(0 == mraa_gpio_read(sensor->drdy))
	{
		if(NO==sensor->data_ready)
		{
			sensor->data_ready = YES;
			wake();
		}
		else
		{
			if(YES==sensor->initialized)
				printf("ADS131 sample miss\n");
		}
		sensor->int_count++;
	}
}

int ADS131_init_gpio(struct ADS_sensor *sensor, uint8_t id)
{
	mraa_result_t status = MRAA_SUCCESS;

	int i,j;

	sensor->id = id;
	sensor->data_ready = NO;
	sensor->adc_ri = 0;
	sensor->initialized = NO;
	for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
			sensor->adc_buffer[i].channel[j] = 0;
		}
	}
	sensor->adc_count = 0;
	sensor->int_count = 0;

	sensor->drdy = mraa_gpio_init(ADS131_DRDY_PIN);
	sensor->cs = mraa_gpio_init(ADS131_CS_PIN);
	sensor->reset = mraa_gpio_init(ADS131_RESET_PIN);

    if (NULL == sensor->drdy || NULL == sensor->cs || NULL == sensor->reset)
    {
        fprintf(stderr, "Failed to initialize GPIO\n");
        mraa_deinit();
        return FAILED;
    }

    /* set GPIO to input/output */
    status = mraa_gpio_dir(sensor->cs, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(sensor->cs, 1);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}

	status = mraa_gpio_dir(sensor->reset, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(sensor->reset, 1);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}

#ifdef ADS131
    status = mraa_gpio_dir(sensor->drdy, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }

    /* configure ISR for GPIO */
    status = mraa_gpio_isr(sensor->drdy, MRAA_GPIO_EDGE_FALLING, &ads131_int_handler, sensor);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stderr, "Failed to configure GPIO interrupt service routine\n");
        goto err_exit;
    }
#endif
    return SUCCEEDED;

err_exit:

    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	return FAILED;
}


uint8_t *ads131_spi_write_buf(struct ADS_sensor *sensor, unsigned char *data, int len)
{
	uint8_t *rx;
	mraa_gpio_write(sensor->cs, 0);
	rx = mraa_spi_write_buf(ads_spi, data, len);
	mraa_gpio_write(sensor->cs, 1);
	return rx;
}

static int ads131_configure_register(struct ADS_sensor *sensor, unsigned char register_address, unsigned short register_value)
{

	uint8_t tx_buff[32] = {0};
	uint8_t *rx_buff=NULL;
	union int16_values rv;

	rv.ui16 = register_value;

	tx_buff[0] = (uint8_t)(ADS131_WREG>>8) | (uint8_t)(register_address>>1);
	tx_buff[1] = (uint8_t)(register_address<<7) & 0x80;
	tx_buff[2] = 0;									//Padding LSB for 24bit word size
	tx_buff[3] = rv.ui8[1];							//Set register value
	tx_buff[4] = rv.ui8[0];
	tx_buff[5] = 0;									//Padding LSB for 24bit word size

	tx_buff[6] = 0;
	tx_buff[7] = 0;
	tx_buff[8] = 0;									//CRC

	rx_buff = ads131_spi_write_buf(sensor, tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word

	usleep(10);

	tx_buff[0] = (uint8_t)(ADS131_RREG>>8) | (uint8_t)(register_address>>1);
	tx_buff[1] = (uint8_t)(register_address<<7) & 0x80;
	tx_buff[2] = 0;									//Padding LSB for 24bit word size
	tx_buff[3] = 0;
	tx_buff[4] = 0;
	tx_buff[5] = 0;									//Padding LSB for 24bit word size

	rx_buff = ads131_spi_write_buf(sensor, tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word

	usleep(10);

	tx_buff[0] = 0;									//NULL command
	tx_buff[1] = 0;
	tx_buff[2] = 0;

	rx_buff = ads131_spi_write_buf(sensor, tx_buff, 30);	//Write SPI frame of 10 words, 24bits per word

	if(rv.ui8[1] != *(rx_buff) && rv.ui8[0] != *(rx_buff+1))
	{
		rv.ui8[1] = *(rx_buff);
		rv.ui8[0] = *(rx_buff+1);

		printf("ERROR: ADS131 Configuration register verification failded. Register address - %x, Configured value - %x, Read value - %x\n",
					register_address, register_value, rv.ui16);
		return FAILED;
	}
	else
	{
		return SUCCEEDED;
	}

}

int ADS131_init_device(struct ADS_sensor *sensor)
{
	uint8_t tx_buff[32] = {0};
	uint8_t *rx_buff=NULL;
	int count=0;

	mraa_gpio_write(sensor->reset, 0);
	usleep(2000);

	mraa_gpio_write(sensor->reset, 1);

	while(0 == mraa_gpio_read(sensor->drdy))			//Wait for DRDY=1 after power to know whether ADC is ready accept commands
	{
		usleep(100);
		count++;
		if(count>100000)					//Wait for 10s
		{
			return FAILED;
		}
	}

	if(FAILED == ads131_configure_register(sensor, ADS131_CLOCK, 0x001A))
		return FAILED;

	//Configure test signals
#ifdef ENABLE_ADS131_TEST_SIGNAL

	if(FAILED == ads131_configure_register(sensor, ADS131_CH0_CFG, 0x0002))
		return FAILED;

	if(FAILED == ads131_configure_register(sensor, ADS131_CH1_CFG, 0x0003))
		return FAILED;

	if(FAILED == ads131_configure_register(sensor, ADS131_CH2_CFG, 0x0002))
		return FAILED;

	if(FAILED == ads131_configure_register(sensor, ADS131_CH3_CFG, 0x0003))
		return FAILED;
#else


#endif
	//Configure MODE reg

	if(FAILED == ads131_configure_register(sensor, ADS131_CLOCK, 0x0F1A))
		return FAILED;

	sensor->initialized = YES;
	return SUCCEEDED;
}



unsigned char *ADS131_read_data(struct ADS_sensor *sensor)
{
	unsigned char tx_buff[32] = {0};
	unsigned char *rx_buff;

	rx_buff = ads131_spi_write_buf(sensor, tx_buff,30);

	return rx_buff;
}
//mraa_result_t 	mraa_spi_transfer_buf (mraa_spi_context dev, uint8_t *data, uint8_t *rxbuf, int length)

int ADS131_get_and_process_data(struct ADS_sensor *sensor)
{
	union int24_values temp;
//	struct timeval now;

	unsigned char *rx;
	int ri,i;

	rx = ADS131_read_data(sensor);
//		gettimeofday(&now, NULL);

	temp.i32=0;
	temp.ui8[2] = *(rx);
	temp.ui8[1] = *(rx+1);
	temp.ui8[0] = *(rx+2);

//		printf("ADS131 %li %03li  - %d - %x \n",now.tv_sec, now.tv_usec, buff_i, temp.i24);

	ri = sensor->adc_ri+1;
	if(ri>1)
	{
		ri=0;
	}

	for(i=1;i<9;i++)
	{
		temp.i32 = 0;
		temp.ui8[2] = *(rx+(i*3));
		temp.ui8[1] = *(rx+(i*3)+1);
		temp.ui8[0] = *(rx+(i*3)+2);
		sensor->adc_buffer[ri].channel[i-1] = (int32_t) (temp.i24);
	}
	sensor->adc_ri = ri;
	sensor->adc_count++;

	return SUCCEEDED;
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
