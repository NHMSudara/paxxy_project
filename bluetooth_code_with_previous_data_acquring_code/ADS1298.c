

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

/* ADS1298 SPI commands */

#define WAKEUP 0x02
#define STANDBY 0x04
#define RESET 0x06
#define START 0x08
#define STOP 0x0A

#define RDATAC 0x10
#define SDATAC 0x11
#define RDATA 0x12

#define RREG 0b00100000
#define WREG 0b01000000

/* ADS1298 addresses */

#define ID 0x00
#define CONFIG1 0x01
#define CONFIG2 0x02
#define CONFIG3 0x03
#define LOFF 0x04

#define CH1SET 0x05
#define CH2SET 0x06
#define CH3SET 0x07
#define CH4SET 0x08
#define CH5SET 0x09
#define CH6SET 0x0A
#define CH7SET 0x0B
#define CH8SET 0x0C
#define RLD_SENSP 0x0D
#define RLD_SENSN 0x0E
#define LOFF_SENSEP 0x0F
#define LOFF_SENSEN 0x10
#define LOFF_FLIP 0x11

#define LOFF_STATP 0x12
#define LOFF_STATN 0x13

#define GPIO 0x14
#define PACE 0x15
#define RESP 0x16
#define CONFIG4 0x17
#define WCT1 0x18
#define WCT2 0x19


/* GPIO */

#define CS_PIN 13
#define DRDY_PIN 12		
#define RESET_PIN 22

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
#define WCT1_VALUE 0x0B
#define WCT2_VALUE 0xD4

#define DATA_BUFFER_LEN 3000		//500 samples per second x 6 seconds
#define DATA_BUFFER_WIDTH 8

//#define ENABLE_ADS1298_TEST_SIGNAL

static mraa_gpio_context cs, drdy, reset;

static unsigned char data_ready = NO, data_log_started=NO;

static int32_t ecg_data_buffer[DATA_BUFFER_LEN][DATA_BUFFER_WIDTH] = {0};
static int buff_i=0, sample_i=0;

typedef struct {
    int ecg1;
    int ecg2;
    int ecg3;
    int ecg4;
} ECGValues;

void ADS1298_int_handler(void* args)
{
//    fprintf(stdout, "Data ready\n");
	if(NO==data_ready)
	{
		data_ready = YES;
	}
	else
	{
		if(YES==data_log_started)
			printf("ADS1298 sample miss %d \n", sample_i);
	}
		
}

int ADS1298_init_gpio(void)
{
	mraa_result_t status = MRAA_SUCCESS;

	drdy = mraa_gpio_init(DRDY_PIN);
    cs = mraa_gpio_init(CS_PIN);
    reset = mraa_gpio_init(RESET_PIN);
	
    if (NULL == drdy || NULL == cs || NULL == reset) 
    {
        fprintf(stderr, "Failed to initialize GPIO\n");
        mraa_deinit();
        return FAILED;
    }

    /* set GPIO to input/output */
    status = mraa_gpio_dir(cs, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(cs, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}
	
	status = mraa_gpio_dir(reset, MRAA_GPIO_OUT);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }
    status = mraa_gpio_write(reset, 1);
	if (status != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}
    
    status = mraa_gpio_dir(drdy, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
    }

    /* configure ISR for GPIO */
    status = mraa_gpio_isr(drdy, MRAA_GPIO_EDGE_FALLING, &ADS1298_int_handler, NULL);
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
    
unsigned char ADS1298_spi_write(unsigned char data)
{
	unsigned char rx;
	mraa_gpio_write(cs, 0);
	ads_spi_write(data);	
	mraa_gpio_write(cs, 1);
	return rx;
}


unsigned char *ADS1298_spi_write_buf(unsigned char *data, int len)
{
	unsigned char *rx;
	mraa_gpio_write(cs, 0);
	rx = ads_spi_write_buf(data, len);		
	mraa_gpio_write(cs, 1);
	return rx;
}
    

static int _configure_register(unsigned char register_address, unsigned char register_value)
{
	unsigned char tx_buff[16] = {0};
	unsigned char *rx_buff=NULL;
	
	tx_buff[0] = WREG | (register_address & 0x1F);		
	tx_buff[1] = 0;						
	tx_buff[2] = register_value;				
	rx_buff = ADS1298_spi_write_buf(tx_buff, 3);
	
	usleep(10);
	
	tx_buff[0] = RREG | (register_address & 0x1F);		
	tx_buff[1] = 0;						
	tx_buff[2] = 0;			
	rx_buff = ADS1298_spi_write_buf(tx_buff, 3);
	
	if(register_value != *(rx_buff+2))
	{
		printf("ERROR: Configuration register verification failded. Register address - %x, Configured value - %x, Read value - %x\n", 
					register_address, register_value, *(rx_buff+2));
		return FAILED;
	}
	else
	{
		return SUCCEEDED;
	}
}

static int _configure_multiple_registers(unsigned char register_address, unsigned char register_count, unsigned char register_value)
{
	unsigned char tx_buff[16] = {0};
	unsigned char *rx_buff=NULL;
	int i;
	
	if(register_count<9)
	{
		tx_buff[0] = WREG | (register_address & 0x1F);		
		tx_buff[1] = 0x1F & (register_count-1);
		for(i=0;i<register_count;i++)
		{	
			tx_buff[2+i] = register_value;
		}				
		rx_buff = ADS1298_spi_write_buf(tx_buff, register_count+2);
		
		usleep(10);
		
		tx_buff[0] = RREG | (register_address & 0x1F);
		tx_buff[1] = 0x1F & (register_count-1);		
		for(i=0;i<register_count;i++)
		{	
			tx_buff[2+i] = 0;
		}
	
		rx_buff = ADS1298_spi_write_buf(tx_buff, register_count+2);
		
		for(i=0;i<register_count;i++)
		{
			if(register_value != *(rx_buff+2+i))
			{
				printf("ERROR: Configuration register verification failded. Register address - %x, Configured value - %x, Read value - %x\n", 
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
    
int ADS1298_init_device(void)
{
	unsigned char tx_buff[16] = {0};
	unsigned char *rx_buff=NULL;
	int i;
	
	mraa_gpio_write(reset, 0);
	usleep(500);
	
	mraa_gpio_write(reset, 1);
	usleep(1000);						//Initial power up delay
	ADS1298_spi_write(SDATAC);			//Stop continuous mode 
	
	usleep(10);

	mraa_gpio_write(reset, 1);
	usleep(1000);						//Initial power up delay
	ADS1298_spi_write(STOP);			//Stop conversion
	
	usleep(10);
	
	if(FAILED == _configure_register(CONFIG1, CONFIG1_VALUE))
		return FAILED;
		
	//Configure test signals
#ifdef ENABLE_ADS1298_TEST_SIGNAL
	
	if(FAILED == _configure_register(CONFIG2, 0x14))
		return FAILED;
	
	if(FAILED == _configure_multiple_registers(CH1SET, 8, 0x05))
		return FAILED;
	
#else

	if(FAILED == _configure_register(CONFIG2, CONFIG2_VALUE))
		return FAILED;
	if(FAILED == _configure_register(CONFIG3, CONFIG3_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF, LOFF_VALUE))
		return FAILED;
	
	if(FAILED == _configure_multiple_registers(CH1SET, 8, CHxSET_VALUE))
		return FAILED;
		
	if(FAILED == _configure_register(RLD_SENSP, RLD_SENSP_VALUE))
		return FAILED;
	if(FAILED == _configure_register(RLD_SENSN, RLD_SENSN_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF_SENSEP, LOFF_SENSEP_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF_SENSEN, LOFF_SENSEN_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF_FLIP, LOFF_FLIP_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF_STATP, LOFF_STATP_VALUE))
		return FAILED;
	if(FAILED == _configure_register(LOFF_STATN, LOFF_STATN_VALUE))
		return FAILED;
	if(FAILED == _configure_register(GPIO, GPIO_VALUE))
		return FAILED;
	if(FAILED == _configure_register(PACE, PACE_VALUE))
		return FAILED;
	if(FAILED == _configure_register(RESP, RESP_VALUE))
		return FAILED;
	if(FAILED == _configure_register(CONFIG4, CONFIG4_VALUE))
		return FAILED;
	if(FAILED == _configure_register(WCT1, WCT1_VALUE))
		return FAILED;
	if(FAILED == _configure_register(WCT2, WCT2_VALUE))
		return FAILED;
		
#endif
	
	ADS1298_spi_write(START);			//Start conversion
	
	usleep(10);
	
	ADS1298_spi_write(RDATAC);			//Start continous data read
	
	return SUCCEEDED;
}
    
void ADS1298_set_data_log_started(unsigned char state)
{
	data_log_started = state;
	if(NO==state)
	{
		buff_i = 0;
	}
}	

unsigned char *ADS1298_read_data(void)
{
	unsigned char tx_buff[28] = {0};
	unsigned char *rx_buff;
	
	rx_buff = ADS1298_spi_write_buf(tx_buff,27);
		 
	return rx_buff; 
}
//mraa_result_t 	mraa_spi_transfer_buf (mraa_spi_context dev, uint8_t *data, uint8_t *rxbuf, int length)

int ADS1298_data_read_task(void)
{
	union int24_values temp;
	struct timeval now;
	
	unsigned char *rx;
	int i,j;
	
	if(YES == data_ready)
	{
		data_ready = NO;
		rx = ADS1298_read_data();
		gettimeofday(&now, NULL);
		
		temp.i32=0;
		temp.ui8[2] = *(rx);
		temp.ui8[1] = *(rx+1);
		temp.ui8[0] = *(rx+2);
		//temp.ui8[0] = 0;
		
//		printf("ADS1298 %li %03li - %d - %x\n", now.tv_sec, now.tv_usec, buff_i, (0x00FFFFFF & temp.i24));
		
		//Do some status bit validation
		
		for(i=1;i<=DATA_BUFFER_WIDTH;i++)
		{
			temp.i32 = 0;
			temp.ui8[2] = *(rx+(i*3));
			temp.ui8[1] = *(rx+(i*3)+1);
			temp.ui8[0] = *(rx+(i*3)+2);
			ecg_data_buffer[buff_i][i-1] = (int32_t) (temp.i24);
		}
		if(YES==data_log_started)
		{
			buff_i++;
			sample_i++;
			if(buff_i>=DATA_BUFFER_LEN)
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

ECGValues ADS1298_print_data_2_file(FILE *data_file, int buffer_index)
{
	ECGValues values;
	//int i, j;
	int i;
	
	i = (buffer_index % DATA_BUFFER_LEN);
		
	if(0==i)
	{
		i = DATA_BUFFER_LEN-1;
	}
	else
	{
		i--;
	}
	if(i>=0 && i<DATA_BUFFER_LEN)
	{
		// for(j=0;j<DATA_BUFFER_WIDTH;j++)
		// {
		// 	fprintf(data_file,"%d,",ecg_data_buffer[i][j]);
		// }
		values.ecg1 = ecg_data_buffer[i][3];
        values.ecg2 = ecg_data_buffer[i][4];
        values.ecg3 = ecg_data_buffer[i][5];
        values.ecg4 = ecg_data_buffer[i][7];
		fprintf(data_file,"%d,",ecg_data_buffer[i][3]);
		fprintf(data_file,"%d,",ecg_data_buffer[i][4]);
		fprintf(data_file,"%d,",ecg_data_buffer[i][5]);
		fprintf(data_file,"%d,",ecg_data_buffer[i][7]);
	}

	return values;
}