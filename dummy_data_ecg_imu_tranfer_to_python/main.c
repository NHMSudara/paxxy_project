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
#include <string.h>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "mraa/gpio.h"

#include "definitions.h"
#include "ADS.h"

#include "user_define.h"
#include "BHI260AP.h"
#include "server.h"


#define FILE_NAME_LENGTH 40
#define DUMMY_DATA_FILE_PATH "paxxy_data.csv"

unsigned char sw_pressed=NO;
static mraa_gpio_context sw, led;

pthread_cond_t cv;
pthread_mutex_t mp;

uint8_t timer_ticked_ads = NO, timer_ticked_bhi=NO;
int ads_tick_count=0, bhi_tick_count=0, sw_count=0;
uint8_t data_log_started=NO;
int log_i = 0;

typedef struct ecgData
{
	/* data */
    int id;
    int ra;
    int ll;
    int la;
    int v1;

}ECGdata;



void handle_termination(int signum){
    printf("Server socket is clossing");
    close_server_sockets();
    exit(EXIT_SUCCESS);
}

void wake(void)
{
	pthread_mutex_lock(&mp);
	pthread_cond_signal(&cv);
	pthread_mutex_unlock(&mp);
}

void send_data_object(ECGdata *data) {
    unsigned char serialized_data[sizeof(ECGdata)]; // Allocate buffer for serialized data

    // Serialize the data object into a byte array
    memcpy(serialized_data, data, sizeof(ECGdata));

    // Calculate the size of the serialized data
    size_t data_size = sizeof(serialized_data);

    // Send the serialized data over TCP
    if (write_tcp_thread_safe(serialized_data, data_size) == FAILED) {
        printf("Failed to send data object\n");
    } else {
        printf("Data object sent successfully\n");
    }
}

void switch_int_handler(void* args)
{
	if(NO == sw_pressed)
		sw_pressed = YES;
}

void timer_callback(int signum)
{
//    struct timeval now;
//    gettimeofday(&now, NULL);
//    printf("Signal %d caught on %li.%03li\n", signum, now.tv_sec, now.tv_usec / 1000);

	//printf("%d-%d %d-%d\n", acount, ADS1298_ri, bcount, ADS131_ri);
//	print_status();
	timer_ticked_ads = YES;
	ads_tick_count++;
	if(0==(ads_tick_count%10))
	{
		timer_ticked_bhi = YES;
		bhi_tick_count++;
	}
	sw_count++;					//Switch debouncing count
}

#ifdef BHI260AP
void log_bhi_data(FILE *data_file, struct BHI_sensor *sensor1, struct BHI_sensor *sensor2, struct BHI_sensor *sensor3,
						struct BHI_sensor *sensor4, struct BHI_sensor *sensor5)
{
	if(YES==data_log_started)
	{
		if(YES==timer_ticked_bhi)
		{
#ifdef BHI_SENSOR1
			fprintf(data_file,"%d,%d,%d,", sensor1->vector_buffer[sensor1->vector_ri].x, sensor1->vector_buffer[sensor1->vector_ri].y,
								sensor1->vector_buffer[sensor1->vector_ri].z);
			
#endif
#ifdef BHI_SENSOR2
			fprintf(data_file,"%d,%d,%d,", sensor2->vector_buffer[sensor2->vector_ri].x, sensor2->vector_buffer[sensor2->vector_ri].y,
								sensor2->vector_buffer[sensor2->vector_ri].z);
#endif
#ifdef BHI_SENSOR3
			fprintf(data_file,"%d,%d,%d", sensor3->vector_buffer[sensor3->vector_ri].x, sensor3->vector_buffer[sensor3->vector_ri].y,
								sensor3->vector_buffer[sensor3->vector_ri].z);
#endif
#ifdef BHI_SENSOR4
			fprintf(data_file,"%d,%d,%d,", sensor4->vector_buffer[sensor4->vector_ri].x, sensor4->vector_buffer[sensor4->vector_ri].y,
								sensor4->vector_buffer[sensor4->vector_ri].z);
								//sensor4->euler_buffer[sensor4->euler_ri].heading, sensor4->euler_buffer[sensor4->euler_ri].pitch,
								//sensor4->euler_buffer[sensor4->euler_ri].roll)
#endif
#ifdef BHI_SENSOR5
			fprintf(data_file,"%d,%d,%d,", sensor5->vector_buffer[sensor5->vector_ri].x, sensor5->vector_buffer[sensor5->vector_ri].y,
								sensor5->vector_buffer[sensor5->vector_ri].z);
#endif
			fprintf(data_file,"\n");
			fflush(data_file);

			if(bhi_tick_count>=50)
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
				//printf("Sensor 4 LACC %d, ORI %d, ", sensor4->vector_count, sensor4->euler_count);
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
			timer_ticked_bhi = NO;
		}
	}
}
#endif

#if defined(ADS1298) || defined(ADS131)
void log_ads_data(FILE *data_file, struct ADS_sensor *ads1298, struct ADS_sensor *ads131) 
{
	if(YES==data_log_started)
	{
		if(YES==timer_ticked_ads)
		{
			fprintf(data_file,"%d,%d,%d,%d\n", ads1298->adc_buffer[ads1298->adc_ri].channel[3], ads1298->adc_buffer[ads1298->adc_ri].channel[4],
					ads1298->adc_buffer[ads1298->adc_ri].channel[5], ads1298->adc_buffer[ads1298->adc_ri].channel[7]);

			fflush(data_file);

			log_i ++;
			ECGdata ecgData;
			ecgData.id = log_i;
			ecgData.ra = ads1298->adc_buffer[ads1298->adc_ri].channel[3];
			ecgData.ll = ads1298->adc_buffer[ads1298->adc_ri].channel[4];
			ecgData.la = ads1298->adc_buffer[ads1298->adc_ri].channel[5];
			ecgData.v1 = ads1298->adc_buffer[ads1298->adc_ri].channel[7];
			
			send_data_object(&ecgData);

			if(ads_tick_count>=500)
			{
				ads_tick_count = 0;
				//printf("ADS1298 %d - %d, ADS131 %d - %d\n", ads1298->int_count, ads1298->adc_count, ads131->int_count, ads131->adc_count);
				printf("ADS1298 %d - %d\n", ads1298->int_count, ads1298->adc_count);
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

 

int main(int argc, char **argv)
{
	signal(SIGINT, handle_termination);

	//open server socket
	open_server_sockets();


	unsigned char file_name[FILE_NAME_LENGTH] = {0};
	int file_i = 0;
	uint8_t is_configured=NO;
	FILE *ADS_data_file=NULL, *BHI_data_file=NULL;

	FILE *fi_file;
//	int count=0, count2=0;

//	int8_t rslt=BHY2_OK;
#if defined(ADS1298) || defined(ADS131)
	struct ADS_sensor ads131;						//Since both ICs connected to same bus, need to initialize both GPIO if both sensors are connected but we enable on one in the software
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
	if(SUCCEEDED==ads_spi_init())
	{
		printf("OK initialization of ADS SPI\n"); 
	}

	ADS131_init_gpio(&ads131, 2);
	ADS1298_init_gpio(&ads1298, 1);			//Since both ICs connected to same bus, need to initialize both GPIO if both sensors are connected but we enable on one in the software

#endif

#ifdef BHI260AP
	if(SUCCEEDED == BHI260AP_init_spi())
	{
		BHI260AP_init_sensor_gpio(&bhi_sensor1, 1, BHI_SENSOR1_CS_PIN, BHI_SENSOR1_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor2, 2, BHI_SENSOR2_CS_PIN, BHI_SENSOR2_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor3, 3, BHI_SENSOR3_CS_PIN, BHI_SENSOR3_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor4, 4, BHI_SENSOR4_CS_PIN, BHI_SENSOR4_INT_PIN);
		BHI260AP_init_sensor_gpio(&bhi_sensor5, 5, BHI_SENSOR5_CS_PIN, BHI_SENSOR5_INT_PIN);

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
		BHI260AP_init_sensor(&bhi_sensor4, ENABLE_LACC_VS);
		//BHI260AP_init_sensor(&bhi_sensor4, ENABLE_LACC_VS | ENABLE_ORI_VS);
#endif
#ifdef BHI_SENSOR5
		BHI260AP_init_sensor(&bhi_sensor5, ENABLE_LACC_VS);

#endif

	

		while(NO == is_configured)
		{
			is_configured = YES;
#ifdef BHI_SENSOR1
			BHI260AP_get_and_process_fifo(&bhi_sensor1);

			if(VS_CONFIGURED != bhi_sensor1.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR2
			BHI260AP_get_and_process_fifo(&bhi_sensor2);

			if(VS_CONFIGURED != bhi_sensor2.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR3
			BHI260AP_get_and_process_fifo(&bhi_sensor3);

			if(VS_CONFIGURED != bhi_sensor3.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR4
			BHI260AP_get_and_process_fifo(&bhi_sensor4);

			if(VS_CONFIGURED != bhi_sensor4.initialized)
				is_configured = NO;
#endif
#ifdef BHI_SENSOR5
			BHI260AP_get_and_process_fifo(&bhi_sensor5);
			if(VS_CONFIGURED != bhi_sensor5.initialized)
				is_configured = NO;
#endif
			usleep(2000);
		}

		printf("BHI sensors configured\n");
	}
#endif

#ifdef ADS1298	

	if(SUCCEEDED == ADS1298_init_device(&ads1298))
	{
		printf("OK initialization of ADS1298\n");
	}
	else
	{
		printf("FAILED initialization of ADS1298\n");
		exit(FAILED);
	}
#endif

#ifdef ADS131

	if(SUCCEEDED == ADS131_init_device(&ads131))
	{	
		printf("OK initialization of ADS131\n"); 
	}
	else
	{
		printf("FAILED initialization of ADS131\n");
		exit(FAILED);
	}
#endif

	for(int i=0; i<=2; i++){
		mraa_gpio_write(led, 1);
		sleep(1);
		mraa_gpio_write(led, 0);
		sleep(1);
	}

	while(1)	//(count<30000)
	{

#ifdef ADS1298

		if(YES == ads1298.data_ready)
		{
			ads1298.data_ready = NO;
			ADS1298_get_and_process_data(&ads1298);
		}
		log_ads_data(ADS_data_file, &ads1298, &ads131); 
#endif

#ifdef ADS131
		if(YES == ads131.data_ready)
		{
			ads131.data_ready = NO;
			ADS131_get_and_process_data(&ads131);
		}
		log_ads_data(ADS_data_file, &ads1298, &ads131);
#endif

#ifdef BHI260AP
#ifdef BHI_SENSOR1
		if(YES == bhi_sensor1.data_ready)
		{
			bhi_sensor1.data_ready = NO;
			BHI260AP_get_and_process_fifo(&bhi_sensor1);
		}
#endif
#ifdef BHI_SENSOR2
		if(YES == bhi_sensor2.data_ready)
		{
			bhi_sensor2.data_ready = NO;
			BHI260AP_get_and_process_fifo(&bhi_sensor2);
		}
#endif
#ifdef BHI_SENSOR3
		if(YES == bhi_sensor3.data_ready)
		{
			bhi_sensor3.data_ready = NO;
			BHI260AP_get_and_process_fifo(&bhi_sensor3);
		}
#endif
#ifdef BHI_SENSOR4
		if(YES == bhi_sensor4.data_ready)
		{
			bhi_sensor4.data_ready = NO;
			BHI260AP_get_and_process_fifo(&bhi_sensor4);
		}
#endif
#ifdef BHI_SENSOR5
		if(YES == bhi_sensor5.data_ready)
		{
			bhi_sensor5.data_ready = NO;
			BHI260AP_get_and_process_fifo(&bhi_sensor5);
		}
#endif
		log_bhi_data(BHI_data_file, &bhi_sensor1, &bhi_sensor2, &bhi_sensor3, &bhi_sensor4, &bhi_sensor5);
#if defined(ADS1298) || defined(ADS131)
		log_ads_data(ADS_data_file, &ads1298, &ads131); 
#endif
#endif


		if(YES==sw_pressed)
		{
			if(NO==data_log_started)
			{
				fi_file = fopen("fi.ini","r");
				if(NULL==fi_file)
				{
					file_i=1;
				}
				else
				{
					fgets(file_name, FILE_NAME_LENGTH, fi_file);
					file_i = atoi(file_name);
					file_i++;
					fclose(fi_file);
				}
#if defined(ADS1298) || defined(ADS131)
				snprintf(file_name, FILE_NAME_LENGTH, "ADS_Data_%d.csv", file_i);
				ADS_data_file = fopen(file_name,"w");

				if(NULL!=ADS_data_file)
				{
					fi_file = fopen("fi.ini","w");
					if(NULL==fi_file)
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
				BHI_data_file = fopen(file_name,"w");

				if(NULL!=BHI_data_file)
				{
					fi_file = fopen("fi.ini","w");
					if(NULL==fi_file)
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
			sw_pressed=PROCESSING;
			sw_count = 0;
//			printf("Switch pressed\n");
		}
		
		if(sw_count>500)
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

#if defined(ADS1298) || defined(ADS131)	
	if(NULL!=ADS_data_file)
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

