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
// #include <sys/ipc.h>
// #include <sys/shm.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "mraa/gpio.h"
#include "definitions.h"
#include "spi.h"
#include "ADS1298.h"
#include "ADS131M08.h"
//#include "BHI260AP.h"

#define PORT 12345
#define DUMMY_DATA_FILE_PATH "paxxy_data.csv"
int server_socket;
FILE* file;

#define SW_PIN 28
#define LED_PIN 30

#define ADS131
#define ADS1298
//#define BHI260AP

#define FILE_NAME_LENGTH 40

unsigned char sw_pressed=NO;
static mraa_gpio_context sw, led;
int start_position = 0;

typedef struct{
    int id;
    float ra;
    float ll;
    float la;
    float v1;
    int as1;
    int as2;
    int as3;
    int as4;

}DataObject;

void handle_termination(int signum){
    printf("Server is shutting down");
    close(server_socket);
    exit(EXIT_SUCCESS);
}

void switch_int_handler(void* args)
{
    sw_pressed = YES;
}

// void write_to_shared_memory(int shmid, char* array){
// 	char* shmaddr = shmat(shmid, NULL, 0); //Attach the shared memory segment

// 	if(shmaddr == (char *) - 1){
// 		perror("shmat");
// 		exit(FAILED);
// 	}

// 	strncpy(shmaddr, array, SHM_SIZE);

// 	if(shmdt(shmaddr == -1)){
// 		perror("shmdt");
// 		exit(FAILED);
// 	}
// }

int main(int argc, char **argv)
{
	signal(SIGINT, handle_termination);

	// Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

	// Set up server address structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

	// Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

	// Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

	// Accept a connection
    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

	int status = 2;
    // recv(client_socket, &status, sizeof(int), 0);
	
	if(status == 1)
    {
        // Open and read the Dummy data CSV file at 500 Hz
        FILE* file = fopen(DUMMY_DATA_FILE_PATH, "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        char line[2048];
        while (fgets(line, sizeof(line), file) != NULL) {
            DataObject data;
            sscanf(line, "%d,%f,%f,%f,%f,%d,%d,%d,%d", &data.id,&data.ra,&data.ll,&data.la,&data.v1,&data.as1,&data.as2,&data.as3,&data.as4); 
            // Send data to the Python client
            //send(client_socket, &data,sizeof(DataObject), 0);
			usleep(2000);
            // Introduce a delay to achieve 500 Hz
            // if(data.id%3000==0){
            //     printf("****************************************************");
			// 	sleep(6);
				
            // }
            // 1 second divided by 500u
        }
    }

	else if(status == 2){

	unsigned char ADS131_read_started=NO, ADS1298_read_started=NO;
	unsigned char data_log_started=NO, init_completed=NO;
	int read_status, log_i=1;
	
	unsigned char file_name[FILE_NAME_LENGTH] = {0};
	int file_i = 0, ADS1298_ri=0, ADS131_ri=0;

	FILE *fi_file, *ADS_data_file=NULL, *BHI_data_file;
	int count=0, count2=0;
	
	mraa_result_t status = MRAA_SUCCESS;

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
#endif

//#ifdef BHI260AP
//	if(SUCCEEDED == bhi_spi_init())
//	{
//		printf("OK initialization of BHI SPI\n");
//	}
//	
//	BHI260AP_init_gpio();
//	BHI260AP_init_device();
//	printf("OK initialization of BHI260AP\n");
//#endif

#ifdef ADS1298	
	ADS1298_init_gpio();
#endif

#ifdef ADS131
	ADS131_init_gpio();
#endif

#ifdef ADS1298	
	if(SUCCEEDED == ADS1298_init_device())
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
	if(SUCCEEDED == ADS131_init_device())
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
	
	
//#ifdef BHI260AP
//	BHI_data_file = fopen("BHI260AP_Data.csv","w");
//#endif

	
	while(1)	//(count<30000)
	{
#ifdef ADS131
		
		read_status = ADS131_data_read_task();
		if(FAILED < read_status)
		{
			ADS131_read_started = YES;
			ADS131_ri = read_status;
#ifndef ADS1298
//			count++;
#endif
		}
#endif

		
		if(YES==data_log_started)
		{
			if(ADS1298_ri >= log_i && ADS131_ri >=log_i)
			{
				fprintf(ADS_data_file,"%d,",log_i);
				ECGValues ecgvalues =  ADS1298_print_data_2_file(ADS_data_file, log_i);
				AcousticVal acousvalues = ADS131_print_data_2_file(ADS_data_file, log_i);
			
				DataObject data;
				data.id=log_i;
				data.as1 = acousvalues.mic1;
				data.as2 = acousvalues.mic2;
				data.as3 = acousvalues.mic3;
				data.as4 = acousvalues.mic4;
				data.la = ecgvalues.ecg1;
				data.ll = ecgvalues.ecg2;
				data.ra = ecgvalues.ecg3;
				data.v1 = ecgvalues.ecg4;

				//send data to the Python client
				send(client_socket, &data, sizeof(DataObject), 0);

				// if(data.id%3000==0){
				// 	char line[1024];
				// 	fseek(ADS_data_file, start_position, SEEK_SET);

				// 	for(int i=0; i<=3000; i++){
				// 		fgets(line, sizeof(line), ADS_data_file);
				// 		DataObject data;
				// 		sscanf(line, "%d,%f,%f,%f,%f,%d,%d,%d,%d", &data.id,&data.ra,&data.ll,&data.la,&data.v1,&data.as1,&data.as2,&data.as3,&data.as4);
				// 		send(client_socket, &data, sizeof(DataObject), 0);
				// 	}
				// 	start_position = ftell(ADS_data_file);
				// }
				
				log_i++;
			}
		}
	
#ifdef ADS1298
		read_status = ADS1298_data_read_task();
		if(FAILED < read_status)
		{
			ADS1298_read_started = YES;
			ADS1298_ri = read_status;
//			count++;
		}
#endif

		if(YES==data_log_started)
		{
			if(ADS1298_ri >= log_i && ADS131_ri >=log_i)
			{
				fprintf(ADS_data_file,"%d,",log_i);
				ECGValues ecgvalues =  ADS1298_print_data_2_file(ADS_data_file, log_i);
				AcousticVal acousvalues = ADS131_print_data_2_file(ADS_data_file, log_i);
			
				DataObject data;
				data.id=log_i;
				data.as1 = acousvalues.mic1;
				data.as2 = acousvalues.mic2;
				data.as3 = acousvalues.mic3;
				data.as4 = acousvalues.mic4;
				data.la = ecgvalues.ecg1;
				data.ll = ecgvalues.ecg2;
				data.ra = ecgvalues.ecg3;
				data.v1 = ecgvalues.ecg4;

				//send data to the Python client
				send(client_socket, &data, sizeof(DataObject), 0);

				// if(data.id%3000==0){
				// 	char line[1024];
				// 	fseek(ADS_data_file, start_position, SEEK_SET);

				// 	for(int i=0; i<=3000; i++){
				// 		fgets(line, sizeof(line), ADS_data_file);
				// 		DataObject data;
				// 		sscanf(line, "%d,%f,%f,%f,%f,%d,%d,%d,%d", &data.id,&data.ra,&data.ll,&data.la,&data.v1,&data.as1,&data.as2,&data.as3,&data.as4);
				// 		send(client_socket, &data, sizeof(DataObject), 0);
				// 	}
				// 	start_position = ftell(ADS_data_file);
				// }
				
				log_i++;
			}
		}
						
//#ifdef BHI260AP
//		if(count2>=25)		//5ms
//		{
//			if(SUCCEEDED == BHI260AP_data_read_task())
//			{
//				BHI260AP_print_data_2_file(BHI_data_file);
//	#ifndef ADS1298
//				count++;
//	#else
//		#ifndef ADS131
//				count++;
//		#endif
//	#endif
//				
//			}
//			count2=0;
//		}
//
//		count2++;
//#endif

		if(NO==init_completed)
		{
			if(YES==ADS131_read_started && YES==ADS1298_read_started)
			{
				init_completed = YES;
			}	
		}
		if(YES==sw_pressed)
		{
			if(NO==data_log_started)
			{
				usleep(200000);
				if(YES==init_completed)
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
					
					snprintf(file_name, FILE_NAME_LENGTH, "ADS_Data_%d.csv", file_i);
										
#if defined(ADS131) || defined(ADS1298)	
					ADS_data_file = fopen(file_name,"w");
#endif
					
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
						ADS1298_set_data_log_started(YES);
						ADS131_set_data_log_started(YES);
						log_i = 1;
						data_log_started = YES;
						mraa_gpio_write(led, 1);
						printf("Logging started\n");

					}
					else
					{
						printf("Error: opening data file (%s) for logging\n", file_name);
						return FAILED;
					}
				}
				else
				{
					printf("Not init ADS1298 %d, ADS131 %d\n",ADS1298_read_started, ADS131_read_started );
				}
			}
			else
			{
				ADS1298_set_data_log_started(NO);
				ADS131_set_data_log_started(NO);
				
				data_log_started = NO;
				mraa_gpio_write(led, 0);
				fclose(ADS_data_file);
				printf("Logging stopped\n");
    			close(client_socket);
    			close(server_socket);
				usleep(200000);
			}
			sw_pressed=NO;
		}
		usleep(50);
	}

#if defined(ADS131) || defined(ADS1298)	
	if(NULL!=ADS_data_file)
		fclose(ADS_data_file);
#endif


	//#ifdef BHI260AP
	//	fclose(BHI_data_file);
	//#endif
	return 0;
	
	
	
	
err_exit:
    
    mraa_result_print(status);
	/* deinitialize mraa for the platform (not needed most of the times) */
	mraa_deinit();
	exit(FAILED);
	
	}
}

