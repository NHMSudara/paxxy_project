/*
 * server.c
 *
 *  Created on: Feb 27, 2024
 *      Author: Thusitha Samarasekara
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>			/* Used to write variadic function log_status */
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/timeb.h>
#include <errno.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "definitions.h"

/* Definitions */

#define USER_TCP_KEEPIDLE 20
#define USER_TCP_KEEPINTVL 5

#define MAX_TCP_PACKET_SIZE 64

/* Exclusive tcp parameter access control mutex variable */

static int server_socket_fd=0;
static int tcp_port = 6789;

static int tcp_socket;             /* TCP socket identifier */

static pthread_t read_thread;
static pthread_mutex_t data_write;        /*Exclusive TCP data write access control mutex variable */
static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_error_thread_safe(const char *format, ...) {
    // Lock the mutex to ensure thread safety
    pthread_mutex_lock(&error_mutex);

    // Print the error message using variadic arguments
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    // Unlock the mutex after printing the message
    pthread_mutex_unlock(&error_mutex);
}

// Function to log status messages in a thread-safe manner
void log_status_thread_safe(const char *format, ...) {
    // Lock the mutex to ensure thread safety
    pthread_mutex_lock(&log_mutex);

    // Print the status message using variadic arguments
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Unlock the mutex after printing the message
    pthread_mutex_unlock(&log_mutex);
}

void *read_main_tcp (void *arg)
{
	(void)arg;
    /* read_tcp thread function - frontend TCP data receiving thread
     * thread receive frontend TCP commands and execute necessary functions to implement command
     * parameters - new_conn - pointer to TCP connection details structure
     * return - none */

	short rx_count=0;
	unsigned char rx_buffer[MAX_TCP_PACKET_SIZE]={0};

	bzero(rx_buffer,MAX_TCP_PACKET_SIZE);

	/* Read data from TCP socket */
	rx_count = read(tcp_socket,rx_buffer,MAX_TCP_PACKET_SIZE);
	while(rx_count>0)
	{
		int status;
		memcpy(&status, rx_buffer, sizeof(int));
		STATUS = status;
//		main_tcp_message_parse(conn,rx_buffer,rx_count);
		rx_count = read(tcp_socket,rx_buffer,MAX_TCP_PACKET_SIZE); //conn->
	}


	pthread_mutex_lock(&data_write);

	if(close(tcp_socket)<0)
		log_error_thread_safe("Error: Closing socket %d\n", tcp_socket);
	else
		log_status_thread_safe("OK: Closing socket %d\n", tcp_socket);

	pthread_mutex_unlock(&data_write);

	return 0;
}




void *listen_tcp(void *para)
{
    /* listen_tcp thread function - listen to TCP port, accept new connections, validates new connections
     * parameters - para - unused
     * return - none */

    int newsockfd=0;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    pthread_attr_t attr;
    int set_sock_opt=1, set_tcp_opt=1;

    /* Open server socket for main connections */

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_socket_fd,SOL_SOCKET,SO_REUSEADDR,&set_sock_opt,sizeof(set_sock_opt)); /* Release socket immediately when server socket is closed*/


    if (server_socket_fd < 0)
    {
        log_error_thread_safe("Error: Opening TCP server socket\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(tcp_port);
    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        log_error_thread_safe("Error: Binding on %d port\n", tcp_port);
        exit(1);
    }

    listen(server_socket_fd,1); /* Parameter 2 - no of client connections waiting in queue */

    clilen = sizeof(cli_addr);

    while (TRUE)
    {
        /* Accept new connection */

        newsockfd = accept(server_socket_fd,(struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
        {
            log_error_thread_safe("Error: Accepting TCP connection");
        }
		else
        {
			tcp_socket = newsockfd;

			/* Initialize TCP socket parameters */

			pthread_mutex_init(&data_write,NULL);

			setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, &set_tcp_opt,sizeof(set_tcp_opt));

			setsockopt(tcp_socket,SOL_SOCKET,SO_KEEPALIVE,&set_sock_opt,sizeof(set_sock_opt)); /* Enable keepalive*/

			setsockopt(tcp_socket,SOL_TCP,USER_TCP_KEEPIDLE,&set_sock_opt,sizeof(set_sock_opt));
			setsockopt(tcp_socket,SOL_TCP,USER_TCP_KEEPINTVL,&set_sock_opt,sizeof(set_sock_opt));

			setsockopt(tcp_socket,SOL_TCP,TCP_KEEPCNT,&set_sock_opt,sizeof(set_sock_opt));

			/* Create TCP socket reading thread */

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

			pthread_create(&read_thread,&attr,read_main_tcp,NULL);
			pthread_attr_destroy(&attr);

        }
    }
    close(server_socket_fd);
	return 0;
}



void open_server_sockets(void)
{
	/* open_server_sockets function - creates threads that listen on server sockets
	 * parameters -
	 * return - none */

	pthread_t tcp_listener;


	pthread_create(&tcp_listener,NULL,listen_tcp,NULL);
}

void close_server_sockets()
{
	/* close_server_sockets function - close TCP connection server sockets
	 * parameters - none
	 * return - none */

	close(server_socket_fd);
}

int write_tcp_thread_safe(unsigned char *tx_buff, short len)
{
    /* write_tcp_thread_safe function - sends data to TCP connection
     * parameters - tx_buff - pointer to transmit data buffer
     *              len - transmit data length in number of bytes
     * return - 0 - if successful,  -1 if failed */

	int n=0;
	if(len<=MAX_TCP_PACKET_SIZE)
	{
		pthread_mutex_lock(&data_write);

		n = write(tcp_socket,tx_buff,len);

		pthread_mutex_unlock(&data_write);

		if (n < 0)
		{
			log_error_thread_safe("Error: Writing to socket %d\n",tcp_socket);
			return FAILED;
		}
		else
		{
			return SUCCEEDED;
		}
	}
	else
	{
		printf("Error: Writing to socket %d, Packet size (%d) larger than maximum TCP packet size (%d)",
				tcp_socket,len,MAX_TCP_PACKET_SIZE);
		return FAILED;
	}
}


int shutdown_tcp_conn_thread_safe()
{
	/* shutdown_all_tcp_conn_thread_safe function - shutdown all frontend TCP connections
	 * parameters - none
	 * return - 0 - if successful, -1 - if failed */

	pthread_mutex_lock(&data_write);
	if(shutdown(tcp_socket,SHUT_RDWR)<0)
	{
		log_error_thread_safe("Error: Closing socket %d\n",tcp_socket);
		return FAILED;
	}
	pthread_mutex_unlock(&data_write);

	return SUCCEEDED;
}
