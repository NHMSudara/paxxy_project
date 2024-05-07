/*
 * server.h
 *
 *  Created on: Apr 30, 2024
 *      Author: Thusitha Samarasekara
 */

#ifndef SERVER_H_
#define SERVER_H_

extern void open_server_socket(void);
extern void close_server_sockets(void);
extern int shutdown_tcp_conn_thread_safe(void);

extern int write_tcp_thread_safe(unsigned char *tx_buff, short len);
extern unsigned char is_tcp_client_connected(void);

#endif /* SERVER_H_ */
