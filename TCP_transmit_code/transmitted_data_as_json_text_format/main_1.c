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
#include <sys/time.h>
// #include <sys/shm.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "definitions.h"
// #include "BHI260AP.h"

#define PORT 8080
#define HOST "127.0.0.1"
#define DATA_FILE_PATH "ADS_BHI_Data.csv"

int server_socket;
FILE *ads_file;
FILE *bhi_file;

#define SW_PIN 28
#define LED_PIN 30

#define ADS131
#define ADS1298
// #define BHI260AP

#define FILE_NAME_LENGTH 40

typedef union
{
	struct
	{
		unsigned int id;
		int ra;
		int ll;
		int la;
		int v1;
		unsigned int imu_en;
		int x1;
		int y1;
		int z1;
		int x2;
		int y2;
		int z2;
		int x3;
		int y3;
		int z3;
		int x4;
		int y4;
		int z4;
		int as1;
		int as2;
		int as3;
		int as4;
	} _;
	unsigned char uc[84];
} Tx_Data;

int tx_i = 0;

void handle_termination(int signum)
{
	printf("Server is shutting down");
	close(server_socket);
	exit(EXIT_SUCCESS);
}

void sendData(int sock, const char *message)
{
	// Send user input to server
	send(sock, message, strlen(message), 0);
}

int main(int argc, char **argv)
{

	int sock = 0;
	struct sockaddr_in serv_addr;
	char userInput[1024];


	// Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket creation error\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("Invalid address / Address not supported\n");
		return -1;
	}

	// Connect to server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Connection Failed\n");
		return -1;
	}

	int status = 1;
	// recv(client_socket, &status, sizeof(int), 0);

	if (status == 1)
	{
		printf("entered_1");
		// Open and read the Dummy data CSV file at 500 Hz
		FILE *data_file = fopen(DATA_FILE_PATH, "r");
		if (!data_file)
		{
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		printf("entered_2");
		char line1[2048];
		while (fgets(line1, sizeof(line1), data_file) != NULL)
		{
			Tx_Data sample;

			sample._.id = tx_i++;
			// printf("scanning \n");
			sscanf(line1, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &sample._.la, &sample._.ll, &sample._.ra, &sample._.v1, &sample._.as1, &sample._.as2, &sample._.as3, &sample._.as4, &sample._.imu_en,&sample._.x1, &sample._.y1, &sample._.z1, &sample._.x2, &sample._.y2, &sample._.z2, &sample._.x3, &sample._.y3, &sample._.z3, &sample._.x4, &sample._.y4, &sample._.z4);

			char buffer[1024];

			snprintf(buffer, sizeof(buffer), "\n {\"id\" : %d  , \"ecg_1\" : %d , \"ecg_2\" : %d , \"ecg_3\" : %d , \"ecg_4\" : %d , \"imu_en\" : %d  , \"acc_x1\" : %d , \"acc_y1\" : %d , \"acc_z1\" : %d , \"acc_x2\" : %d , \"acc_y2\" : %d , \"acc_z2\" : %d , \"acc_x3\" : %d , \"acc_y3\" : %d , \"acc_z3\" : %d , \"acc_x4\" : %d , \"acc_y4\" : %d , \"acc_z4\" : %d, \"as_1\" : %d, \"as_2\" : %d, \"as_3\" : %d, \"as_4\" : %d }\n",
					 sample._.id, sample._.la, sample._.ll, sample._.ra, sample._.v1,sample._.imu_en,
					 sample._.x1, sample._.y1, sample._.z1, sample._.x2, sample._.y2, sample._.z2, sample._.x3, sample._.y3, sample._.z3,
					 sample._.x4, sample._.y4, sample._.z4, sample._.as1, sample._.as2, sample._.as3, sample._.as4);

			// printf("sending data ...  \n");
			sendData(sock, buffer);
			// printf("Data dent \n");
			usleep(2000); // Introduce a delay to achieve 500 Hz
		}

		int sock = 0;
	}
}