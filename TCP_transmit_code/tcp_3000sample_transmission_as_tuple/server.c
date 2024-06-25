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
#include "user_define.h"

/* Definitions */

#define USER_TCP_KEEPIDLE 20
#define USER_TCP_KEEPINTVL 5

#define MAX_TCP_PACKET_SIZE 90

/* Exclusive tcp parameter access control mutex variable */

static int server_socket_fd=0;
static int tcp_port=SERVER_TCP_PORT;

static int tcp_socket;             /* TCP socket identifier */

static pthread_t read_thread;
static pthread_mutex_t data_write;        /*Exclusive TCP data write access control mutex variable */

static unsigned char is_connected = NO;

void *read_tcp (void *para)
{
    /* read_tcp thread function - TCP data receiving thread
     * parameters - none
     * return - none */

	short rx_count=0;
	unsigned char rx_buffer[MAX_TCP_PACKET_SIZE]={0};

	bzero(rx_buffer,MAX_TCP_PACKET_SIZE);

	/* Read data from TCP socket */
	rx_count = read(tcp_socket,rx_buffer,MAX_TCP_PACKET_SIZE);
	while(rx_count>0)
	{
//		main_tcp_message_parse(conn,rx_buffer,rx_count);
		rx_count = read(tcp_socket,rx_buffer,MAX_TCP_PACKET_SIZE);
	}


	pthread_mutex_lock(&data_write);

	if(close(tcp_socket)<0)
		printf("Error: Closing socket %d\n", tcp_socket);
	else
		printf("OK: Closing socket %d\n", tcp_socket);

	is_connected = NO;

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
        printf("Error: Opening TCP server socket\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(tcp_port);
    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("Error: Binding on %d port\n", tcp_port);
        exit(1);
    }

    listen(server_socket_fd,1); /* Parameter 2 - no of client connections waiting in queue */

    clilen = sizeof(cli_addr);

    while (1)
    {
        /* Accept new connection */

        newsockfd = accept(server_socket_fd,(struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
        {
            printf("Error: Accepting TCP connection");
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

			pthread_create(&read_thread,&attr,read_tcp,NULL);
			pthread_attr_destroy(&attr);

			is_connected = YES;;
        }
    }
    close(server_socket_fd);
	return 0;
}



void open_server_socket(void)
{
	/* open_server_socket function - creates threads that listen on server socket
	 * parameters -
	 * return - none */

	pthread_t tcp_listener;


	pthread_create(&tcp_listener,NULL,listen_tcp,NULL);
}

void close_server_sockets(void)
{
	/* close_server_sockets function - close TCP connection server sockets
	 * parameters - none
	 * return - none */

	close(server_socket_fd);

	is_connected = NO;
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
			printf("Error: Writing to socket %d\n",tcp_socket);
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


int shutdown_tcp_conn_thread_safe(void)
{
	/* shutdown_all_tcp_conn_thread_safe function - shutdown all TCP connections
	 * parameters - none
	 * return - 0 - if successful, -1 - if failed */

	pthread_mutex_lock(&data_write);
	if(shutdown(tcp_socket,SHUT_RDWR)<0)
	{
		printf("Error: Closing socket %d\n",tcp_socket);
		return FAILED;
	}

	is_connected = NO;

	pthread_mutex_unlock(&data_write);

	return SUCCEEDED;
}

unsigned char is_tcp_client_connected(void)
{
	return is_connected;
}
