
/* standard headers */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* mraa header */
#include "mraa/spi.h"
#include "mraa/gpio.h"

#include "BHI260AP_definitions.h"
#include "definitions.h"
#include "user_define.h"


#define MAX_TX_RX_LENGTH 64
#define FIFO_BUFFER_LENGTH 256

#define EXPECTED_BOOT_STATUS 0x13

#define WAKE_UP_FIFO 1
#define NON_WAKE_UP_FIFO 2
#define STATUS_DEBUG_FIFO 3

static mraa_spi_context bhi_spi;

extern void wake(void);

int8_t bhi_configure_virtual_sensor(struct BHI_sensor *sensor, uint8_t v_sensor_id, float update_rate, int latency);

void bhi_int_handler(void* args)
{
	struct BHI_sensor *sensor = (struct BHI_sensor *)args;

	if(1 == mraa_gpio_read(sensor->drdy))
	{
		if(NO == sensor->data_ready)
		{
			sensor->data_ready = YES;
			wake();
		}
		else
		{
			printf("BHI sensor %d sample missed\n", sensor->id);
		}
//    	printf("Dev Interrupt\n");
	}
}

int BHI_get_drdy_status(struct BHI_sensor *sensor)
{
	return mraa_gpio_read(sensor->drdy);
}

unsigned char *bhi_spi_write_buf(struct BHI_sensor *sensor, unsigned char *data, int len)
{
	unsigned char *rx;

	mraa_gpio_write(sensor->cs, 0);
	rx = mraa_spi_write_buf(bhi_spi, data, len);
	mraa_gpio_write(sensor->cs, 1);

	return rx;
}

int bhi_spi_stop(void)
{
	/* stop spi */
	mraa_spi_stop(bhi_spi);

	return SUCCEEDED;
}


int BHI260AP_init_spi(void)
{
	mraa_result_t status = MRAA_SUCCESS;

	/* initialize mraa for the platform (not needed most of the times) */
	mraa_init();

	/* initialize SPI bus */
	bhi_spi = mraa_spi_init_raw(BHI_SPI_BUS,0);
	if (bhi_spi == NULL)
	{
		fprintf(stderr, "Failed to initialize BHI SPI\n");
		mraa_deinit();
		return FAILED;
	}
	/* set SPI frequency */
	status = mraa_spi_frequency(bhi_spi, BHI_SPI_FREQ);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stdout, "Failed to set BHI SPI frequency to %d Hz\n", BHI_SPI_FREQ);
		goto err_exit;
	}
	/* set big endian mode */
	status = mraa_spi_lsbmode(bhi_spi, 0);
	if (status != MRAA_SUCCESS)
	{
		goto err_exit;
	}
	/* set SPI mode */
	status = mraa_spi_mode(bhi_spi, MRAA_SPI_MODE0);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stdout, "Failed to set SPI mode to 0\n");
		goto err_exit;
	}

//    status = mraa_spi_bit_per_word(spi2, 16);
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


int BHI260AP_init_sensor_gpio(struct BHI_sensor *sensor, uint8_t id, uint8_t cs_pin, uint8_t drdy_pin)
{
	mraa_result_t status = MRAA_SUCCESS;

	sensor->id = id;
	sensor->data_ready = NO;
	sensor->vector_ri = 0;
	sensor->euler_ri = 0;

	sensor->initialized = NO;

	sensor->vector_buffer[0].x = 0;
	sensor->vector_buffer[0].y = 0;
	sensor->vector_buffer[0].z = 0;
	sensor->vector_buffer[1].x = 0;
	sensor->vector_buffer[1].y = 0;
	sensor->vector_buffer[1].z = 0;

	sensor->euler_buffer[0].heading = 0;
	sensor->euler_buffer[0].pitch = 0;
	sensor->euler_buffer[0].roll = 0;
	sensor->euler_buffer[1].heading = 0;
	sensor->euler_buffer[1].pitch = 0;
	sensor->euler_buffer[1].roll = 0;

	sensor->vector_count = 0;
	sensor->euler_count = 0;

	sensor->drdy = mraa_gpio_init(drdy_pin);
    sensor->cs = mraa_gpio_init(cs_pin);
    
    if (NULL == sensor->drdy || NULL == sensor->cs)
    {
        fprintf(stderr, "%d:Failed to initialize GPIO\n", sensor->id);
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
		fprintf(stderr, "%d:Failed to set GPIO output value\n", sensor->id);
		goto err_exit;
	}

    status = mraa_gpio_dir(sensor->drdy, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "%d:Failed to set GPIO direction\n", sensor->id);
		goto err_exit;
    }

    /* configure ISR for GPIO */
    status = mraa_gpio_isr(sensor->drdy, MRAA_GPIO_EDGE_RISING, &bhi_int_handler, sensor);
    if (status != MRAA_SUCCESS) 
    {
		fprintf(stderr, "%d:Failed to configure GPIO interrupt service routine\n", sensor->id);
        goto err_exit;
    }
	
    return SUCCEEDED;
    
err_exit:
    
    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	return FAILED;
}

uint8_t *bhi_read_register(struct BHI_sensor *sensor, uint8_t reg_addr, uint8_t length)
{
	unsigned char tx_buff[MAX_TX_RX_LENGTH] = {0};
	unsigned char *rx_buff=NULL;

	tx_buff[0] = BHI_REG_READ_MASK | reg_addr;
	tx_buff[1] = 0;

	rx_buff = bhi_spi_write_buf(sensor, tx_buff, length+1);

	return (rx_buff+1);
}

uint8_t *bhi_write_register(struct BHI_sensor *sensor, uint8_t reg_addr, uint8_t *data, uint8_t length)
{
	unsigned char tx_buff[MAX_TX_RX_LENGTH] = {0};
	unsigned char *rx_buff=NULL;
	int i;

	if(length>(MAX_TX_RX_LENGTH-1))
	{
		return NULL;
	}

	tx_buff[0] = reg_addr & 0x7F;
	for(i=0;i<length;i++)
	{
		tx_buff[1+i] = *data;
		data++;
	}

	rx_buff = bhi_spi_write_buf(sensor, tx_buff, length+1);
}

int bhi_parse_fifo(struct BHI_sensor *sensor, uint8_t *buffer, uint8_t length)
{
	uint8_t *data = buffer;
	uint8_t rem_length = length;
	int8_t ri;

	union int16_values x,y,z;

	while(rem_length>0)
	{
		if(BHI_IS_SYS_ID(*data))
		{
			switch(*data)
			{
			case BHI_SYS_ID_TS_SMALL_DELTA:
			case BHI_SYS_ID_TS_SMALL_DELTA_WU:
				rem_length -= 2;
				data += 2;
				break;
			case BHI_SYS_ID_TS_LARGE_DELTA:
			case BHI_SYS_ID_TS_LARGE_DELTA_WU:
				rem_length -= 3;
				data += 3;
				break;
			case BHI_SYS_ID_TS_FULL:
			case BHI_SYS_ID_TS_FULL_WU:
				rem_length -= 6;
				data += 6;
				break;
			case BHI_SYS_ID_META_EVENT:
			case BHI_SYS_ID_META_EVENT_WU:
				switch(*(data+1))
				{
				case BHY2_META_EVENT_FLUSH_COMPLETE:
					printf("%d:META EVENT: Flush complete %x\n", sensor->id, *(data+2));
					break;
				case BHY2_META_EVENT_SAMPLE_RATE_CHANGED:
					printf("%d:META EVENT: Sample rate changed %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_POWER_MODE_CHANGED:
					printf("%d:META EVENT: Power mode changed %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_ALGORITHM_EVENTS:
					printf("%d:META EVENT: Algorithm events %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_SENSOR_STATUS:
					printf("%d:META EVENT: Sensor status %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_SENSOR_ERROR:
					printf("%d:META EVENT: Sensor error %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_FIFO_OVERFLOW:
					printf("%d:META EVENT: FIFO overflow %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_DYNAMIC_RANGE_CHANGED:
					printf("%d:META EVENT: Dynamic range changed %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_FIFO_WATERMARK:
					printf("%d:META EVENT: FIFO watermark %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHI_META_EVENT_INITIALIZED:
					sensor->initialized = YES;
					printf("%d:META EVENT: Initialized %x%x\n", sensor->id, *(data+3), *(data+2));
					break;
				case BHY2_META_TRANSFER_CAUSE:
					printf("%d:META EVENT: Transfer cause %x\n", sensor->id, *(data+2));
					break;
				case BHY2_META_EVENT_SENSOR_FRAMEWORK:
					printf("%d:META EVENT: Event driven sensor framework %x-%x\n", sensor->id, *(data+2), *(data+3));
					break;
				case BHY2_META_EVENT_RESET:
					printf("%d:META EVENT: Reset %x\n", sensor->id, *(data+3));
					break;
				case BHI_META_EVENT_SPACER:
					break;
				}
				rem_length -= 4;
				data += 4;
				break;
			case BHI_SYS_ID_DEBUG_MSG:
				rem_length -= 18;
				data += 18;
				break;
			case BHI_SYS_ID_FILLER:
			default:
				rem_length--;
				data++;
				break;
			}
		}
		else
		{
			switch(*data)
			{
			case BHI_SENSOR_ID_LACC_WU:
				x.ui8[0] = *(data+1);
				x.ui8[1] = *(data+2);
				y.ui8[0] = *(data+3);
				y.ui8[1] = *(data+4);
				z.ui8[0] = *(data+5);
				z.ui8[1] = *(data+6);
//				printf("LACC X-%d, Y-%d, Z-%d\n", x.i16, y.i16, z.i16);

				ri = sensor->vector_ri+1;
				if(ri>1)
				{
					ri=0;
				}
				sensor->vector_buffer[ri].x = x.i16;
				sensor->vector_buffer[ri].y = y.i16;
				sensor->vector_buffer[ri].z = z.i16;
				sensor->vector_ri = ri;
				sensor->vector_count++;
				rem_length -= 7;
				data += 7;
				break;
			case BHI_SENSOR_ID_ORI_WU:
				x.ui8[0] = *(data+1);
				x.ui8[1] = *(data+2);
				y.ui8[0] = *(data+3);
				y.ui8[1] = *(data+4);
				z.ui8[0] = *(data+5);
				z.ui8[1] = *(data+6);
//				printf("ORI H-%d, P-%d, R-%d\n", x.i16, y.i16, z.i16);

				ri = sensor->euler_ri+1;
				if(ri>1)
				{
					ri=0;
				}
				sensor->euler_buffer[ri].heading = x.i16;
				sensor->euler_buffer[ri].pitch = y.i16;
				sensor->euler_buffer[ri].roll = z.i16;
				sensor->euler_ri = ri;
				sensor->euler_count++;
				rem_length -= 7;
				data += 7;
				break;
			case BHI_SYS_ID_PADDING:
			default:
				rem_length--;
				data++;
				break;
			}
		}
	}
	return SUCCEEDED;
}

int bhi_read_fifo(struct BHI_sensor *sensor, uint8_t fifo_type, uint8_t *buffer)
{
	uint8_t *rx;
	union int16_values data_length;
	int rem_data_length=0, read_data_length=0;

	if(fifo_type<WAKE_UP_FIFO || fifo_type>STATUS_DEBUG_FIFO)
	{
		printf("%d:Invalid fifo type\n", sensor->id);
		return FAILED;
	}
	if(NULL == buffer)
	{
		printf("%d:Null fifo buffer\n", sensor->id);
		return FAILED;
	}

	rx = bhi_read_register(sensor, fifo_type, 2);
	if(NULL != rx)
	{
		data_length.ui8[0] = *rx;
		data_length.ui8[1] = *(rx+1);
	}
	else
	{
		printf("%d:Buffer read returned null\n", sensor->id);
		return FAILED;
	}
	if(data_length.ui16>0 && data_length.ui16<FIFO_BUFFER_LENGTH)
	{
		if(data_length.ui16>MAX_TX_RX_LENGTH)
		{
			rem_data_length = data_length.ui16;
			read_data_length = 0;
			while(rem_data_length>0)
			{
				if(data_length.ui16>MAX_TX_RX_LENGTH)
				{
					rx = bhi_read_register(sensor, fifo_type, MAX_TX_RX_LENGTH);
					memcpy(buffer+read_data_length, rx, MAX_TX_RX_LENGTH);
					rem_data_length -= MAX_TX_RX_LENGTH;
					read_data_length += MAX_TX_RX_LENGTH;
				}
				else
				{
					rx = bhi_read_register(sensor, fifo_type, rem_data_length);
					memcpy(buffer+read_data_length, rx, rem_data_length);
					rem_data_length = 0;
					read_data_length += rem_data_length;
				}

			}
			return read_data_length;
		}
		else
		{
			rx = bhi_read_register(sensor, fifo_type, data_length.ui16);
			memcpy(buffer, rx, data_length.ui16);
			return data_length.ui16;
		}
	}
	else
	{
		//may have unread data flust the fifo;
		printf("%d:Invalid data length\n", sensor->id);
		return FAILED;
	}
}


int8_t BHI260AP_get_and_process_fifo(struct BHI_sensor *sensor)
{
	union BHI_interrupt_status int_status;
	uint8_t fifo_buffer[FIFO_BUFFER_LENGTH] = {0};
    uint8_t *rx;
    int read_data_length=0;

    rx = bhi_read_register(sensor, BHI_REG_INT_STATUS, 1);

    int_status.ui8 = *rx;

    if(1 == int_status._.host_int_asserted)			//process only if host interrupt has occurred
    {
    	if(0 != int_status._.wake_up_fifo_status)
    	{
    		read_data_length = bhi_read_fifo(sensor, WAKE_UP_FIFO, fifo_buffer);

//    		printf("Wakup FIFO read %d\n", read_data_length);
    		bhi_parse_fifo(sensor, fifo_buffer, read_data_length);
    	}

    	if(0 != int_status._.non_wake_up_fifo_status)
		{
    		read_data_length = bhi_read_fifo(sensor, NON_WAKE_UP_FIFO, fifo_buffer);

//			printf("NON Wakup FIFO read %d\n", read_data_length);
			bhi_parse_fifo(sensor, fifo_buffer, read_data_length);
		}

    	if(0 != int_status._.status)
		{
    		read_data_length = bhi_read_fifo(sensor, STATUS_DEBUG_FIFO, fifo_buffer);

//			printf("Status and debug FIFO read %d\n", read_data_length);
			bhi_parse_fifo(sensor, fifo_buffer, read_data_length);
		}

    	if(0 != int_status._.debug)
		{
    		read_data_length = bhi_read_fifo(sensor, STATUS_DEBUG_FIFO, fifo_buffer);

//			printf("Status and debug FIFO read %d\n", read_data_length);
			bhi_parse_fifo(sensor, fifo_buffer, read_data_length);
		}

    	if(0 != int_status._.reset_fault)
		{
    		printf("%d:interrupt status, Interrupt due to reset or fault\n", sensor->id);
		}

    	if(YES == sensor->initialized)
    	{
			if(0 != (sensor->sensor_config & ENABLE_LACC_VS))
				bhi_configure_virtual_sensor(sensor, BHI_SENSOR_ID_LACC_WU, VS_DATA_UPDATE_RATE, 0);

			if(0 != (sensor->sensor_config & ENABLE_ORI_VS))
				bhi_configure_virtual_sensor(sensor, BHI_SENSOR_ID_ORI_WU, VS_DATA_UPDATE_RATE, 0);

			sensor->data_ready = NO;
			sensor->initialized = VS_CONFIGURED;
			printf("%d:Finish configuration %d\n", sensor->id, sensor->data_ready);

    	}
    	return SUCCEEDED;
    }
    else
    {
    	return FAILED;
    }
}

int8_t bhi_send_command(struct BHI_sensor *sensor, uint16_t command_id, uint8_t *cmd_data, uint8_t data_length)
{
	/* Command id 2 bytes, lsb first
	 * command content length 2 bytes, lsb first
	 * data optional, 4 bytes or more
	 */

	uint8_t data[MAX_TX_RX_LENGTH] = {0};
	union int16_values cmd_id;
	uint8_t i,j, total_length;

	cmd_id.ui16 = command_id;
	data[0] = cmd_id.ui8[0];
	data[1] = cmd_id.ui8[1];
	data[2] = data_length;
	data[3] = 0;
	for(i=0;i<data_length;i++)
	{
		data[4+i] = *(cmd_data+i);
	}
	j = data_length%4;
	if(0!=j)
	{
		j = 4-j;
		total_length = 4 + data_length + j;
		while(j>0)
		{
			data[i] = 0;
			i++;
			j--;
		}
	}
	else
	{
		total_length = 4 + data_length;
	}
	bhi_write_register(sensor, BHI_REG_CHAN_CMD, data, total_length);
	return SUCCEEDED;
}

int8_t bhi_configure_virtual_sensor(struct BHI_sensor *sensor, uint8_t v_sensor_id, float update_rate, int latency)
{
	uint8_t data[8] = {0};
	union float_values data_rate;
	union int24_values latency_value;

	data_rate.f = update_rate;
	latency_value.i32 = latency;

	data[0] = v_sensor_id;
	data[1] = data_rate.ui8[0];
	data[2] = data_rate.ui8[1];
	data[3] = data_rate.ui8[2];
	data[4] = data_rate.ui8[3];
	data[5] = latency_value.ui8[0];
	data[6] = latency_value.ui8[1];
	data[7] = latency_value.ui8[2];

    return bhi_send_command(sensor, BHI_CMD_CONFIG_SENSOR, data, 8);

}

    
int BHI260AP_init_sensor(struct BHI_sensor *sensor, uint8_t conf_sensors)
{
	uint8_t *rx;
	uint8_t boot_status=0;
	int loop_count=0;
	uint8_t data[MAX_TX_RX_LENGTH] = {0};

	sensor->sensor_config = conf_sensors;

	rx = bhi_read_register(sensor, BHI_REG_PRODUCT_ID, 1);
	if(BHI_PRODUCT_ID != *rx)
	{
		printf("%d:Sensor product ID mismatch expected - %x received - %x\n", sensor->id, BHI_PRODUCT_ID, *rx);
		return FAILED;
	}
	
	//Set interrupt control

	data[0] = BHI_ICTL_DISABLE_STATUS_FIFO | BHI_ICTL_DISABLE_DEBUG;
	bhi_write_register(sensor, BHI_REG_HOST_INTERRUPT_CTRL, &data[0], 1);

	rx = bhi_read_register(sensor, BHI_REG_HOST_INTERRUPT_CTRL, 1);
	printf("%d:Interrupt control register %x\n", sensor->id, *rx);

	loop_count = 0;
	while(boot_status != EXPECTED_BOOT_STATUS && loop_count<5)
	{
		//Read boot status
		
		rx = bhi_read_register(sensor, BHI_REG_BOOT_STATUS, 1);
		
		boot_status = *rx;
		printf("%d:Boot status - %x\n", sensor->id, boot_status);
		
		loop_count++;
		usleep(10000);
	}
	if(0 == (boot_status & BHI_BST_HOST_INTERFACE_READY))
	{
		printf("%d:Error: Host interface not ready\n", sensor->id);
		return FAILED;
	}

	if(loop_count>=5)
	{
		printf("%d:Error: Firmware booting error\n", sensor->id);
		return FAILED;
	}


	return SUCCEEDED;
}


//void BHI260AP_print_data_2_file(FILE *data_file)
//{
//	int i = buff_i-1;
//	if(i>=0)
//	{
//		fprintf(data_file,"%d,%d,%d\n",acc_data_buffer[i][0], acc_data_buffer[i][1],
//			acc_data_buffer[i][2]);
//	}
//}
