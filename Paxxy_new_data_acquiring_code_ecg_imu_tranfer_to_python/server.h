#ifndef SERVER_H

    #define SERVER_H

    extern void *read_main_tcp(void *arg);
    extern void *listen_tcp(void *para);
    extern void open_server_sockets(void);
    extern void close_server_sockets(void);
    extern int write_tcp_thread_safe(unsigned char *tx_buff, short len);
    extern int shutdown_tcp_conn_thread_safe();

#endif


    