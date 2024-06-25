/*
 * main.c
 *
 * Copyright 2023 Thusitha Samarasekara <thusitha@Thusitha-PC>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

/* standard headers */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h> // Include this header to use strtok

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "mraa/gpio.h"

#include "definitions.h"
#include "ADS.h"

#include "user_define.h"

#include "BHI260AP.h"
// #include "server.h"
#include "shared_memory.h"

#define FILE_NAME_LENGTH 40

#define ECG_DATA_BUFFER_LEN 3000 // 500 samples per second x 6 seconds
#define ACC_DATA_BUFFER_LEN 300	 // 50 sapmples per second x 6 seconds

// struct ecg_data
//{
//	int ra;
//	int ll;
//	int la;
//	int v1;
// };
//
// struct acc_data
//{
//	int x;
//	int y;
//	int z;
// };
//
// typedef union
//{
//	struct
//	{
//		int type;
//		int count;
//	}_;
//	unsigned char uc[8];
// }HeaderDataObject;
//
// typedef union
//{
//	struct
//	{
//		int id;
//		int ra;
//		int ll;
//		int la;
//		int v1;
//	}_;
//	unsigned char uc[20];
// }ECGDataObject;
//
// typedef union
//{
//	struct
//	{
//		int id;
//		struct
//		{
//			int x;
//			int y;
//			int z;
//		}sensor[4];
//	}_;
//	unsigned char uc[48];
// }ACCDataObject;

struct shared_memory_data
{
	struct
	{
		unsigned int ecg_data_batch;
		unsigned int acc_data_batch;
		struct
		{
			int ra; // E1
			int v1; // E2
			int la; // E3
			int ll; // E4
		} ecg_data[ECG_DATA_BUFFER_LEN];
		struct
		{
			struct
			{
				int x;
				int y;
				int z;
			} sensor[4];
		} acc_data[ACC_DATA_BUFFER_LEN];
	} buffer[2];
};

// struct ecg_data ecg_data_buffer[2][ECG_DATA_BUFFER_LEN] = {0};
//
// struct acc_data acc_data_buffer[2][MAX_BHI_ACC_SENSOR_COUNT][ACC_DATA_BUFFER_LEN] = {0};
//

//
// unsigned char ecg_wr_i=0, acc_wr_i=0;
// unsigned char ecg_rd_i=0, acc_rd_i=0;
// unsigned char ecg_tx_ready = NO, acc_tx_ready = NO;

struct shared_memory_data *sm_buffer;
int sm_status = FAILED;

int acc_buffer_i = 0, ecg_buffer_i = 0;
int acc_data_i = 0, ecg_data_i = 0;
unsigned int ecg_batch_i = 1, acc_batch_i = 1;
unsigned int batch_number = 1;

unsigned char ecg_buffer_filled = NO;
unsigned char acc_buffer_filled = NO;

unsigned char sw_pressed = NO;
static mraa_gpio_context sw, led;

pthread_cond_t cv;
pthread_mutex_t mp;

uint8_t timer_ticked_ads = NO, timer_ticked_bhi = NO;
int ads_tick_count = 0, bhi_tick_count = 0, sw_count = 0;
uint8_t data_log_started = NO;

void wake(void)
{
	pthread_mutex_lock(&mp);
	pthread_cond_signal(&cv);
	pthread_mutex_unlock(&mp);
}

void switch_int_handler(void *args)
{
	if (NO == sw_pressed)
		sw_pressed = YES;
}

void timer_callback(int signum)
{
	//    struct timeval now;
	//    gettimeofday(&now, NULL);
	//    printf("Signal %d caught on %li.%03li\n", signum, now.tv_sec, now.tv_usec / 1000);

	// printf("%d-%d %d-%d\n", acount, ADS1298_ri, bcount, ADS131_ri);
	//	print_status();
	timer_ticked_ads = YES;
	ads_tick_count++;
	if (0 == (ads_tick_count % 10))
	{
		timer_ticked_bhi = YES;
		bhi_tick_count++;
	}
	sw_count++; // Switch debouncing count
}

#ifdef BHI260AP
void log_bhi_data(FILE *data_file, struct BHI_sensor *sensor1, struct BHI_sensor *sensor2, struct BHI_sensor *sensor3,
				  struct BHI_sensor *sensor4, struct BHI_sensor *sensor5)
{
	//	HeaderDataObject header;
	//	ACCDataObject data;
	int i;

	if (YES == data_log_started)
	{
		if (YES == timer_ticked_bhi)
		{
#ifdef BHI_SENSOR1
			fprintf(data_file, "%d,%d,%d,", sensor1->vector_buffer[sensor1->vector_ri].x, sensor1->vector_buffer[sensor1->vector_ri].y,
					sensor1->vector_buffer[sensor1->vector_ri].z);
			if (SUCCEEDED == sm_status)
			{
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].x = sensor1->vector_buffer[sensor1->vector_ri].x;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].y = sensor1->vector_buffer[sensor1->vector_ri].y;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].z = sensor1->vector_buffer[sensor1->vector_ri].z;
			}

//			acc_data_buffer[acc_wr_i][0][acc_data_i].x = sensor1->vector_buffer[sensor1->vector_ri].x;
//			acc_data_buffer[acc_wr_i][0][acc_data_i].y = sensor1->vector_buffer[sensor1->vector_ri].y;
//			acc_data_buffer[acc_wr_i][0][acc_data_i].z = sensor1->vector_buffer[sensor1->vector_ri].z;
#endif
#ifdef BHI_SENSOR2
			fprintf(data_file, "%d,%d,%d,", sensor2->vector_buffer[sensor2->vector_ri].x, sensor2->vector_buffer[sensor2->vector_ri].y,
					sensor2->vector_buffer[sensor2->vector_ri].z);
			if (SUCCEEDED == sm_status)
			{
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].x = sensor1->vector_buffer[sensor2->vector_ri].x;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].y = sensor1->vector_buffer[sensor2->vector_ri].y;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].z = sensor1->vector_buffer[sensor2->vector_ri].z;
			}
			//			acc_data_buffer[acc_wr_i][1][acc_data_i].x = sensor2->vector_buffer[sensor2->vector_ri].x;
			//			acc_data_buffer[acc_wr_i][1][acc_data_i].y = sensor2->vector_buffer[sensor2->vector_ri].y;
			//			acc_data_buffer[acc_wr_i][1][acc_data_i].z = sensor2->vector_buffer[sensor2->vector_ri].z;

#endif
#ifdef BHI_SENSOR3
			fprintf(data_file, "%d,%d,%d,", sensor3->vector_buffer[sensor3->vector_ri].x, sensor3->vector_buffer[sensor3->vector_ri].y,
					sensor3->vector_buffer[sensor3->vector_ri].z);
			if (SUCCEEDED == sm_status)
			{
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].x = sensor1->vector_buffer[sensor3->vector_ri].x;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].y = sensor1->vector_buffer[sensor3->vector_ri].y;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].z = sensor1->vector_buffer[sensor3->vector_ri].z;
			}
//			acc_data_buffer[acc_wr_i][2][acc_data_i].x = sensor3->vector_buffer[sensor3->vector_ri].x;
//			acc_data_buffer[acc_wr_i][2][acc_data_i].y = sensor3->vector_buffer[sensor3->vector_ri].y;
//			acc_data_buffer[acc_wr_i][2][acc_data_i].z = sensor3->vector_buffer[sensor3->vector_ri].z;
#endif
#ifdef BHI_SENSOR4
			/*fprintf(data_file, "%d,%d,%d,%d,%d,%d,", sensor4->vector_buffer[sensor4->vector_ri].x, sensor4->vector_buffer[sensor4->vector_ri].y,
					sensor4->vector_buffer[sensor4->vector_ri].z, sensor4->euler_buffer[sensor4->euler_ri].heading, sensor4->euler_buffer[sensor4->euler_ri].pitch,
					sensor4->euler_buffer[sensor4->euler_ri].roll);*/
			fprintf(data_file, "%d,%d,%d,", sensor4->vector_buffer[sensor4->vector_ri].x, sensor4->vector_buffer[sensor4->vector_ri].y,
					sensor4->vector_buffer[sensor4->vector_ri].z);
			if (SUCCEEDED == sm_status)
			{
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].x = sensor1->vector_buffer[sensor4->vector_ri].x;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].y = sensor1->vector_buffer[sensor4->vector_ri].y;
				sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].z = sensor1->vector_buffer[sensor4->vector_ri].z;
			}
//			acc_data_buffer[acc_wr_i][3][acc_data_i].x = sensor4->vector_buffer[sensor4->vector_ri].x;
//			acc_data_buffer[acc_wr_i][3][acc_data_i].y = sensor4->vector_buffer[sensor4->vector_ri].y;
//			acc_data_buffer[acc_wr_i][3][acc_data_i].z = sensor4->vector_buffer[sensor4->vector_ri].z;
#endif
#ifdef BHI_SENSOR5
			fprintf(data_file, "%d,%d,%d,", sensor4->vector_buffer[sensor4->vector_ri].x, sensor4->vector_buffer[sensor4->vector_ri].y,
					sensor4->vector_buffer[sensor4->vector_ri].z);
#endif
			fprintf(data_file, "\n");
			fflush(data_file);

			if (bhi_tick_count >= 50)
			{
				bhi_tick_count = 0;
#ifdef BHI_SENSOR1
				printf("Sensor 1 LACC %d, ", sensor1->vector_count);
				sensor1->vector_count = 0;
				sensor1->euler_count = 0;
#endif
#ifdef BHI_SENSOR2
				printf("Sensor 2 LACC %d, ", sensor2->vector_count);
				sensor2->vector_count = 0;
				sensor2->euler_count = 0;
#endif
#ifdef BHI_SENSOR3
				printf("Sensor 3 LACC %d, ", sensor3->vector_count);
				sensor3->vector_count = 0;
				sensor3->euler_count = 0;
#endif
#ifdef BHI_SENSOR4
				printf("Sensor 4 LACC %d, ", sensor4->vector_count);
				sensor4->vector_count = 0;
				sensor4->euler_count = 0;
#endif
#ifdef BHI_SENSOR5
				printf("Sensor 5 LACC %d, ", sensor5->vector_count);
				sensor5->vector_count = 0;
				sensor5->euler_count = 0;
#endif
				printf("\n");
			}

			if (SUCCEEDED == sm_status)
			{
				acc_data_i++;

				if (acc_data_i >= ACC_DATA_BUFFER_LEN)
				{
					acc_buffer_filled = YES;
					// send BHI data
					/*printf("ACC batch - %d\n", acc_batch_i);
					sm_buffer->buffer[acc_buffer_i].acc_data_batch = acc_batch_i++;
					acc_buffer_i++;
					if (acc_buffer_i > 1)
					{
						acc_buffer_i = 0;
					}
					acc_data_i = 0;*/
					//				if(YES==is_tcp_client_connected())
					//				{
					//					header._.type = ACC_DATA_TYPE;
					//					header._.count = ACC_DATA_BUFFER_LEN;
					//					write_tcp_thread_safe(header.uc, 8);
					//
					//					for(i = 0; i<ACC_DATA_BUFFER_LEN; i++)
					//					{
					//						data._.id = i++;
					//						data._.sensor[0].x = acc_data_buffer[0][i].x;
					//						data._.sensor[0].y = acc_data_buffer[0][i].y;
					//						data._.sensor[0].z = acc_data_buffer[0][i].z;
					//
					//						data._.sensor[1].x = acc_data_buffer[1][i].x;
					//						data._.sensor[1].y = acc_data_buffer[1][i].y;
					//						data._.sensor[1].z = acc_data_buffer[1][i].z;
					//
					//						data._.sensor[2].x = acc_data_buffer[2][i].x;
					//						data._.sensor[2].y = acc_data_buffer[2][i].y;
					//						data._.sensor[2].z = acc_data_buffer[2][i].z;
					//
					//						data._.sensor[3].x = acc_data_buffer[3][i].x;
					//						data._.sensor[3].y = acc_data_buffer[3][i].y;
					//						data._.sensor[3].z = acc_data_buffer[3][i].z;
					//						write_tcp_thread_safe(data.uc, 48);
					//					}
					//				}
				}
			}
			timer_ticked_bhi = NO;
		}
	}
}
#endif

#if defined(ADS1298) || defined(ADS131)
void log_ads_data(FILE *data_file, struct ADS_sensor *ads1298, struct ADS_sensor *ads131)
{
	//	HeaderDataObject header;
	//	ECGDataObject data;
	int i;

	if (YES == data_log_started)
	{
		if (YES == timer_ticked_ads)
		{
			/*fprintf(data_file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", ads1298->adc_buffer[ads1298->adc_ri].channel[0], ads1298->adc_buffer[ads1298->adc_ri].channel[1],
					ads1298->adc_buffer[ads1298->adc_ri].channel[2], ads1298->adc_buffer[ads1298->adc_ri].channel[3], ads1298->adc_buffer[ads1298->adc_ri].channel[4],
					ads1298->adc_buffer[ads1298->adc_ri].channel[5], ads1298->adc_buffer[ads1298->adc_ri].channel[6], ads1298->adc_buffer[ads1298->adc_ri].channel[7],
					ads131->adc_buffer[ads131->adc_ri].channel[0], ads131->adc_buffer[ads131->adc_ri].channel[1], ads131->adc_buffer[ads131->adc_ri].channel[2],
					ads131->adc_buffer[ads131->adc_ri].channel[3]);
			fflush(data_file);*/
			fprintf(data_file, "%d,%d,%d,%d\n", ads1298->adc_buffer[ads1298->adc_ri].channel[3], ads1298->adc_buffer[ads1298->adc_ri].channel[4],
					ads1298->adc_buffer[ads1298->adc_ri].channel[5], ads1298->adc_buffer[ads1298->adc_ri].channel[7]);
			fflush(data_file); // ra/E1 , ll/E4 , la/E3 , v1/E2

			//			ecg_data_buffer[ecg_wr_i][ecg_data_i].la = ads1298->adc_buffer[ads1298->adc_ri].channel[3];
			//			ecg_data_buffer[ecg_wr_i][ecg_data_i].ll = ads1298->adc_buffer[ads1298->adc_ri].channel[4];
			//			ecg_data_buffer[ecg_wr_i][ecg_data_i].ra = ads1298->adc_buffer[ads1298->adc_ri].channel[5];
			//			ecg_data_buffer[ecg_wr_i][ecg_data_i].v1 = ads1298->adc_buffer[ads1298->adc_ri].channel[7];

			if (SUCCEEDED == sm_status)
			{
				sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].ra = ads1298->adc_buffer[ads1298->adc_ri].channel[3];
				sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].v1 = ads1298->adc_buffer[ads1298->adc_ri].channel[7];
				sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].la = ads1298->adc_buffer[ads1298->adc_ri].channel[5];
				sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].ll = ads1298->adc_buffer[ads1298->adc_ri].channel[4];

				ecg_data_i++;

				if (ecg_data_i >= ECG_DATA_BUFFER_LEN)
				{
					ecg_buffer_filled = YES;
					// send ECG data
					/*printf("ECG batch - %d\n", ecg_batch_i);

					sm_buffer->buffer[ecg_buffer_i].ecg_data_batch = ecg_batch_i++;
					ecg_buffer_i++;
					if (ecg_buffer_i > 1)
					{
						ecg_buffer_i = 0;
					}
					ecg_data_i = 0;*/
					//				if(YES==is_tcp_client_connected())
					//				{
					//					header._.type = ECG_DATA_TYPE;
					//					header._.count = ECG_DATA_BUFFER_LEN;
					//					write_tcp_thread_safe(header.uc, 8);
					//
					//					for(i = 0; i<ECG_DATA_BUFFER_LEN; i++)
					//					{
					//						data._.id = i++;
					//						data._.la = ecg_data_buffer[i].la;
					//						data._.ll = ecg_data_buffer[i].ll;
					//						data._.ra = ecg_data_buffer[i].ra;
					//						data._.v1 = ecg_data_buffer[i].v1;
					//						write_tcp_thread_safe(data.uc, 20);
					//					}
					//				}
				}
			}
			if (ads_tick_count >= 500)
			{
				ads_tick_count = 0;
				printf("ADS1298 %d - %d, ADS131 %d - %d\n", ads1298->int_count, ads1298->adc_count, ads131->int_count, ads131->adc_count);
				ads1298->adc_count = 0;
				ads131->adc_count = 0;
				ads1298->int_count = 0;
				ads131->int_count = 0;
			}
			timer_ticked_ads = NO;
		}
	}
}
#endif

int mode_Read_sensor();
int mode_Read_simulator_data();

int main(int argc, char **argv)
{
	int choice;
	int valid_input = 0;

	printf("Program paused. Please enter 1 or 2 to continue in different modes:\n");

	// Loop to ensure valid input is provided
	while (!valid_input)
	{
		printf("Enter 1 for Read_sensor_Mode, or 2 for Read_simulator_data_Mode: ");
		if (scanf("%d", &choice) != 1)
		{
			// Clear the invalid input
			while (getchar() != '\n')
				;
			printf("Invalid input. Please enter a valid number.\n");
			continue;
		}

		// Check if the input is valid
		if (choice == 1 || choice == 2)
		{
			valid_input = 1; // Input is valid, exit the loop
		}
		else
		{
			printf("Invalid choice. Please enter 1 or 2.\n");
		}
	}

	if (choice == 1)
	{
		mode_Read_sensor();
	}
	else if (choice == 2)
	{
		mode_Read_simulator_data();
	}

	return 0;
}

int mode_Read_sensor()
{
	printf("Mode 1 selected. Running Read_sensor_Mode operations...\n");
	unsigned char file_name[FILE_NAME_LENGTH] = {0};
	int file_i = 0;
	uint8_t is_configured = NO;
	FILE *ADS_data_file = NULL, *BHI_data_file = NULL;

	FILE *fi_file;
//	int count=0, count2=0;

//	int8_t rslt=BHY2_OK;
#if defined(ADS1298) || defined(ADS131)
	struct ADS_sensor ads131; // Since both ICs connected to same bus, need to initialize both GPIO if both sensors are connected but we enable on one in the software
	struct ADS_sensor ads1298;
#endif

#ifdef BHI260AP

	struct BHI_sensor bhi_sensor1, bhi_sensor2, bhi_sensor3, bhi_sensor4, bhi_sensor5;
#endif

	mraa_result_t status = MRAA_SUCCESS;

	struct timeval now;
	struct timespec abstime;

	/*timer */
	struct itimerval new_timer;
	struct itimerval old_timer;

	new_timer.it_value.tv_sec = 0;
	new_timer.it_value.tv_usec = 2000;
	new_timer.it_interval.tv_sec = 0;
	new_timer.it_interval.tv_usec = 2000;

	setitimer(ITIMER_REAL, &new_timer, &old_timer);
	signal(SIGALRM, timer_callback);

	/*timer*/
	/* initialize a condition variable to its default value */
	pthread_mutex_init(&mp, NULL);
	pthread_cond_init(&cv, NULL);

	/* Open tcp server port */
	//	open_server_socket();

	/* Open shared memory */
	sm_status = shared_memory_init();
	if (SUCCEEDED == sm_status)
	{
		sm_buffer = (struct shared_memory_data *)shared_memory_get_addr();
		sm_buffer->buffer[0].acc_data_batch = 0;
		sm_buffer->buffer[0].ecg_data_batch = 0;
		sm_buffer->buffer[1].acc_data_batch = 0;
		sm_buffer->buffer[1].ecg_data_batch = 0;
	}

	/* initialize mraa for the platform (not needed most of the times) */
	mraa_init();

	sw = mraa_gpio_init(SW_PIN);
	led = mraa_gpio_init(LED_PIN);

	if (NULL == sw || NULL == led)
	{
		fprintf(stderr, "Failed to initialize switch and led GPIO\n");
		mraa_deinit();
		exit(FAILED);
	}

	/* set GPIO to input/output */
	status = mraa_gpio_dir(led, MRAA_GPIO_OUT);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
	}

	status = mraa_gpio_write(led, 0);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO output value\n");
		goto err_exit;
	}

	status = mraa_gpio_dir(sw, MRAA_GPIO_IN);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO direction\n");
		goto err_exit;
	}

	/* configure ISR for GPIO */
	status = mraa_gpio_isr(sw, MRAA_GPIO_EDGE_FALLING, &switch_int_handler, NULL);
	if (status != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to configure GPIO interrupt service routine\n");
		goto err_exit;
	}

#if defined(ADS1298) || defined(ADS131)
	if (SUCCEEDED == ads_spi_init())
	{
		printf("OK initialization of ADS SPI\n");
	}

	ADS131_init_gpio(&ads131, 2);
	ADS1298_init_gpio(&ads1298, 1); // Since both ICs connected to same bus, need to initialize both GPIO if both sensors are connected but we enable on one in the software

#endif

#ifdef BHI260AP
	if (SUCCEEDED == BHI260AP_init_spi())
	{

		BHI260AP_init_sensor_gpio(&bhi_sensor1, 1, BHI_SENSOR1_CS_PIN, BHI_SENSOR1_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor2, 2, BHI_SENSOR2_CS_PIN, BHI_SENSOR2_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor3, 3, BHI_SENSOR3_CS_PIN, BHI_SENSOR3_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor4, 4, BHI_SENSOR4_CS_PIN, BHI_SENSOR4_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor5, 5, BHI_SENSOR5_CS_PIN, BHI_SENSOR5_INT_PIN);

		// #ifdef BHI_SENSOR1
		//		BHI260AP_init_sensor_gpio(&bhi_sensor1, 1, BHI_SENSOR1_CS_PIN, BHI_SENSOR1_INT_PIN);
		// #endif
		// #ifdef BHI_SENSOR2
		//		BHI260AP_init_sensor_gpio(&bhi_sensor2, 2, BHI_SENSOR2_CS_PIN, BHI_SENSOR2_INT_PIN);
		// #endif
		// #ifdef BHI_SENSOR3
		//		BHI260AP_init_sensor_gpio(&bhi_sensor3, 3, BHI_SENSOR3_CS_PIN, BHI_SENSOR3_INT_PIN);
		// #endif
		// #ifdef BHI_SENSOR4
		//		BHI260AP_init_sensor_gpio(&bhi_sensor4, 4, BHI_SENSOR4_CS_PIN, BHI_SENSOR4_INT_PIN);
		// #endif
		// #ifdef BHI_SENSOR5
		//		BHI260AP_init_sensor_gpio(&bhi_sensor5, 5, BHI_SENSOR5_CS_PIN, BHI_SENSOR5_INT_PIN);
		// #endif

#ifdef BHI_SENSOR1
		BHI260AP_init_sensor(&bhi_sensor1, ENABLE_LACC_VS);
#endif
#ifdef BHI_SENSOR2
		BHI260AP_init_sensor(&bhi_sensor2, ENABLE_LACC_VS);
#endif
#ifdef BHI_SENSOR3
		BHI260AP_init_sensor(&bhi_sensor3, ENABLE_LACC_VS);
#endif
#ifdef BHI_SENSOR4
		//		BHI260AP_init_sensor(&bhi_sensor4, ENABLE_ORI_VS);
		// BHI260AP_init_sensor(&bhi_sensor4, ENABLE_LACC_VS | ENABLE_ORI_VS);
		BHI260AP_init_sensor(&bhi_sensor4, ENABLE_LACC_VS);
#endif
#ifdef BHI_SENSOR5
		// BHI260AP_init_sensor(&bhi_sensor5, ENABLE_ORI_VS);
		BHI260AP_init_sensor(&bhi_sensor5, ENABLE_LACC_VS);

#endif

		while (NO == is_configured)
		{
			is_configured = YES;
#ifdef BHI_SENSOR1
			BHI260AP_get_and_process_fifo(&bhi_sensor1);

			if (VS_CONFIGURED != bhi_sensor1.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR2
			BHI260AP_get_and_process_fifo(&bhi_sensor2);

			if (VS_CONFIGURED != bhi_sensor2.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR3
			BHI260AP_get_and_process_fifo(&bhi_sensor3);

			if (VS_CONFIGURED != bhi_sensor3.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR4
			BHI260AP_get_and_process_fifo(&bhi_sensor4);

			if (VS_CONFIGURED != bhi_sensor4.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR5
			BHI260AP_get_and_process_fifo(&bhi_sensor5);
			if (VS_CONFIGURED != bhi_sensor5.initialized)
				is_configured = NO;
#endif
			usleep(2000);
		}

		printf("BHI sensors configured\n");
	}
#endif

#ifdef ADS1298

	if (SUCCEEDED == ADS1298_init_device(&ads1298))
	{
		printf("OK initialization of ADS1298\n");
	}
	else
	{
		printf("FAILED initialization of ADS1298\n");
		exit(FAILED);
	}
#endif

	while (1) //(count<30000)
	{

#ifdef ADS1298
		if (ecg_buffer_filled == NO)
		{
			if (YES == ads1298.data_ready)
			{
				ads1298.data_ready = NO;
				ADS1298_get_and_process_data(&ads1298);
			}
			log_ads_data(ADS_data_file, &ads1298, &ads131);
		}
#endif

#ifdef BHI260AP
#ifdef BHI_SENSOR1
		if (acc_buffer_filled == NO)
		{
			if (YES == bhi_sensor1.data_ready)
			{
				bhi_sensor1.data_ready = NO;
				BHI260AP_get_and_process_fifo(&bhi_sensor1);
			}
			else if (1 == BHI_get_drdy_status(&bhi_sensor1))
			{
				BHI260AP_get_and_process_fifo(&bhi_sensor1);
			}
		}
#endif

#ifdef ADS1298
		if (ecg_buffer_filled == NO)
		{
			if (YES == ads1298.data_ready)
			{
				ads1298.data_ready = NO;
				ADS1298_get_and_process_data(&ads1298);
			}
			log_ads_data(ADS_data_file, &ads1298, &ads131);
		}
#endif

#ifdef BHI_SENSOR2
		if (acc_buffer_filled == NO)
		{
			if (YES == bhi_sensor2.data_ready)
			{
				bhi_sensor2.data_ready = NO;
				BHI260AP_get_and_process_fifo(&bhi_sensor2);
			}
			else if (1 == BHI_get_drdy_status(&bhi_sensor2))
			{
				BHI260AP_get_and_process_fifo(&bhi_sensor2);
			}
		}
#endif

#ifdef ADS1298
		if (ecg_buffer_filled == NO)
		{
			if (YES == ads1298.data_ready)
			{
				ads1298.data_ready = NO;
				ADS1298_get_and_process_data(&ads1298);
			}
			log_ads_data(ADS_data_file, &ads1298, &ads131);
		}
#endif

#ifdef BHI_SENSOR3
		if (acc_buffer_filled == NO)
		{
			if (YES == bhi_sensor3.data_ready)
			{
				bhi_sensor3.data_ready = NO;
				BHI260AP_get_and_process_fifo(&bhi_sensor3);
			}
			else if (1 == BHI_get_drdy_status(&bhi_sensor3))
			{
				BHI260AP_get_and_process_fifo(&bhi_sensor3);
			}
		}
#endif

#ifdef ADS1298
		if (ecg_buffer_filled == NO)
		{
			if (YES == ads1298.data_ready)
			{
				ads1298.data_ready = NO;
				ADS1298_get_and_process_data(&ads1298);
			}
			log_ads_data(ADS_data_file, &ads1298, &ads131);
		}
#endif

#ifdef BHI_SENSOR4
		if (acc_buffer_filled == NO)
		{
			if (YES == bhi_sensor4.data_ready)
			{
				bhi_sensor4.data_ready = NO;
				BHI260AP_get_and_process_fifo(&bhi_sensor4);
			}
			else if (1 == BHI_get_drdy_status(&bhi_sensor4))
			{
				BHI260AP_get_and_process_fifo(&bhi_sensor4);
			}
		}
#endif

#ifdef ADS1298
		if (ecg_buffer_filled == NO)
		{
			if (YES == ads1298.data_ready)
			{
				ads1298.data_ready = NO;
				ADS1298_get_and_process_data(&ads1298);
			}
			log_ads_data(ADS_data_file, &ads1298, &ads131);
		}
#endif

#ifdef BHI_SENSOR5
		if (acc_buffer_filled == NO)
		{
			if (YES == bhi_sensor5.data_ready)
			{
				bhi_sensor5.data_ready = NO;
				BHI260AP_get_and_process_fifo(&bhi_sensor5);
			}
			else if (1 == BHI_get_drdy_status(&bhi_sensor5))
			{
				BHI260AP_get_and_process_fifo(&bhi_sensor5);
			}
		}

#endif
		log_bhi_data(BHI_data_file, &bhi_sensor1, &bhi_sensor2, &bhi_sensor3, &bhi_sensor4, &bhi_sensor5);
#if defined(ADS1298) || defined(ADS131)
		log_ads_data(ADS_data_file, &ads1298, &ads131);
#endif

#if defined(ADS1298) && defined(BHI260AP)
		if (SUCCEEDED == sm_status)
		{
			if (ecg_buffer_filled == YES && acc_buffer_filled == YES)
			{
				printf("batch - %d\n", batch_number);
				sm_buffer->buffer[acc_buffer_i].acc_data_batch = batch_number;
				acc_buffer_i++;
				if (acc_buffer_i > 1)
				{
					acc_buffer_i = 0;
				}
				acc_data_i = 0;

				// printf("ECG batch - %d\n", ecg_batch_i);

				sm_buffer->buffer[ecg_buffer_i].ecg_data_batch = batch_number;
				ecg_buffer_i++;
				if (ecg_buffer_i > 1)
				{
					ecg_buffer_i = 0;
				}
				ecg_data_i = 0;

				batch_number++;
				acc_buffer_filled = NO;
				ecg_buffer_filled = NO;
			}
		}
#endif
#endif

		if (YES == sw_pressed)
		{
			if (NO == data_log_started)
			{
				fi_file = fopen("fi.ini", "r");
				if (NULL == fi_file)
				{
					file_i = 1;
				}
				else
				{
					fgets(file_name, FILE_NAME_LENGTH, fi_file);
					file_i = atoi(file_name);
					file_i++;
					fclose(fi_file);
				}

#if defined(ADS131) || defined(ADS1298)
				snprintf(file_name, FILE_NAME_LENGTH, "ADS_Data_%d.csv", file_i);
				ADS_data_file = fopen(file_name, "w");

				if (NULL != ADS_data_file)
				{
					fi_file = fopen("fi.ini", "w");
					if (NULL == fi_file)
					{
						printf("Error opening index file\n");
					}
					else
					{
						fprintf(fi_file, "%d", file_i);
						fclose(fi_file);
					}

					ads1298.adc_count = 0;
					ads131.adc_count = 0;
					data_log_started = YES;
					mraa_gpio_write(led, 1);
					printf("ADS Logging started\n");
				}
				else
				{
					printf("Error: opening data file (%s) for logging\n", file_name);
					return FAILED;
				}
#endif
#ifdef BHI260AP
				snprintf(file_name, FILE_NAME_LENGTH, "BHI_Data_%d.csv", file_i);
				BHI_data_file = fopen(file_name, "w");
				if (NULL != BHI_data_file)
				{
					fi_file = fopen("fi.ini", "w");
					if (NULL == fi_file)
					{
						printf("Error opening index file\n");
					}
					else
					{
						fprintf(fi_file, "%d", file_i);
						fclose(fi_file);
					}
					//						BHI_set_logging_started(YES);

					bhi_sensor1.vector_count = 0;
					bhi_sensor1.euler_count = 0;
					bhi_sensor2.vector_count = 0;
					bhi_sensor2.euler_count = 0;
					bhi_sensor3.vector_count = 0;
					bhi_sensor3.euler_count = 0;
					bhi_sensor4.vector_count = 0;
					bhi_sensor4.euler_count = 0;
					bhi_sensor5.vector_count = 0;
					bhi_sensor5.euler_count = 0;

					data_log_started = YES;
					mraa_gpio_write(led, 1);
					printf("BHI Logging started\n");
				}
				else
				{
					printf("Error: opening data file (%s) for logging\n", file_name);
					return FAILED;
				}
#endif
			}
			else
			{
				data_log_started = NO;
				mraa_gpio_write(led, 0);

#if defined(ADS131) || defined(ADS1298)
				fclose(ADS_data_file);
#endif
#ifdef BHI260AP
				fclose(BHI_data_file);
#endif
				printf("Logging stopped\n");
			}
			sw_pressed = PROCESSING;
			sw_count = 0;
			//			printf("Switch pressed\n");
		}

		if (sw_count > 500)
		{
			sw_pressed = NO;
		}

		timespec_get(&abstime, TIME_UTC);
		abstime.tv_nsec += 20000;

		pthread_mutex_lock(&mp);
		pthread_cond_timedwait(&cv, &mp, &abstime);
		pthread_mutex_unlock(&mp);

		//		gettimeofday(&now, NULL);
		//		printf("Sleep1 %li %03li\n", now.tv_sec, now.tv_usec);
		//
		//		usleep(50);
		//		gettimeofday(&now, NULL);
		//		printf("Sleep2 %li %03li\n", now.tv_sec, now.tv_usec);
	}

#if defined(ADS131) || defined(ADS1298)
	if (NULL != ADS_data_file)
		fclose(ADS_data_file);
#endif

#ifdef BHI260AP
	fclose(BHI_data_file);
#endif
	return 0;

err_exit:

	mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	exit(FAILED);
}

/********************************************************************************************************************** */

char line_ADS[1024];
void read_and_send_ADS_file(FILE *file)
{
	char *fields_ADS[4];
	int field_count_ADS;
	if (fgets(line_ADS, sizeof(line_ADS), file) != NULL)
	{
		int field_count_ADS = 0;
		char *token = strtok(line_ADS, ",");
		while (token != NULL && field_count_ADS < 4)
		{
			fields_ADS[field_count_ADS++] = token;
			token = strtok(NULL, ",");
		}
		printf("%s,%s,%s,%s\n", fields_ADS[0], fields_ADS[1], fields_ADS[2], fields_ADS[3]);
		if (SUCCEEDED == sm_status)
		{
			sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].ra = atoi(fields_ADS[0]);
			sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].v1 = atoi(fields_ADS[1]);
			sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].la = atoi(fields_ADS[2]);
			sm_buffer->buffer[ecg_buffer_i].ecg_data[ecg_data_i].ll = atoi(fields_ADS[3]);
			ecg_data_i++;
			if (ecg_data_i >= ECG_DATA_BUFFER_LEN)
			{
				ecg_buffer_filled = YES;
			}
		}
	}
}

char line_BHI[1024];
void read_and_send_BHI_file(FILE *file)
{
	char *fields_BHI[15];
	int field_count_BHI;
	if (fgets(line_BHI, sizeof(line_BHI), file) != NULL)
	{
		int field_count_BHI = 0;
		char *token = strtok(line_BHI, ",");
		while (token != NULL && field_count_BHI < 15)
		{
			fields_BHI[field_count_BHI++] = token;
			token = strtok(NULL, ",");
		}
		printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", fields_BHI[0], fields_BHI[1], fields_BHI[2], fields_BHI[3], fields_BHI[4], fields_BHI[5], fields_BHI[6], fields_BHI[7], fields_BHI[8], fields_BHI[9], fields_BHI[10], fields_BHI[11]);
		if (SUCCEEDED == sm_status)
		{
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].x = atoi(fields_BHI[0]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].y = atoi(fields_BHI[1]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[0].z = atoi(fields_BHI[2]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].x = atoi(fields_BHI[3]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].y = atoi(fields_BHI[4]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[1].z = atoi(fields_BHI[5]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].x = atoi(fields_BHI[6]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].y = atoi(fields_BHI[7]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[2].z = atoi(fields_BHI[8]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].x = atoi(fields_BHI[9]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].y = atoi(fields_BHI[10]);
			sm_buffer->buffer[acc_buffer_i].acc_data[acc_data_i].sensor[3].z = atoi(fields_BHI[11]);
			acc_data_i++;
			if (acc_data_i >= ACC_DATA_BUFFER_LEN)
			{
				acc_buffer_filled = YES;
			}
		}
	}
}

int mode_Read_simulator_data()
{
	printf("Mode 2 selected. Running Read_simulator_data_Mode operations...\n");

	sm_status = shared_memory_init();
	if (SUCCEEDED == sm_status)
	{
		sm_buffer = (struct shared_memory_data *)shared_memory_get_addr();
		sm_buffer->buffer[0].acc_data_batch = 0;
		sm_buffer->buffer[0].ecg_data_batch = 0;
		sm_buffer->buffer[1].acc_data_batch = 0;
		sm_buffer->buffer[1].ecg_data_batch = 0;
	}

	const char *BHI_file = BHI_DATA_FILE_PATH;
	const char *ADS_file = ADS_DATA_FILE_PATH;

	FILE *file_50hz = fopen(BHI_file, "r");
	FILE *file_500hz = fopen(ADS_file, "r");

	if (file_50hz == NULL || file_500hz == NULL)
	{
		perror("Error opening files");
		return EXIT_FAILURE;
	}

	long ADS_counter = 0;

	while (1)
	{
		if (ADS_counter % 10 == 0)
		{
			if (!feof(file_50hz))
			{
				read_and_send_BHI_file(file_50hz);
			}
			else
			{
				printf("BHI_Data file overn\n");
			}
		}

		if (!feof(file_500hz))
		{
			read_and_send_ADS_file(file_500hz);
		}
		else
		{
			printf("ADS_Data file over\n");
		}
		ADS_counter++;
		usleep(2000); // delay 2 ms between each ADS reading for simulation

		if (SUCCEEDED == sm_status)
		{
			if (ecg_buffer_filled == YES && acc_buffer_filled == YES)
			{
				printf("batch - %d\n", batch_number);
				sm_buffer->buffer[acc_buffer_i].acc_data_batch = batch_number;
				acc_buffer_i++;
				if (acc_buffer_i > 1)
				{
					acc_buffer_i = 0;
				}
				acc_data_i = 0;

				// printf("ECG batch - %d\n", ecg_batch_i);

				sm_buffer->buffer[ecg_buffer_i].ecg_data_batch = batch_number;
				ecg_buffer_i++;
				if (ecg_buffer_i > 1)
				{
					ecg_buffer_i = 0;
				}
				ecg_data_i = 0;

				batch_number++;
				acc_buffer_filled = NO;
				ecg_buffer_filled = NO;
			}
		}
	}
}
