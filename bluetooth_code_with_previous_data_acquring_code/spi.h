

#ifndef SPI_H
#define SPI_H

extern int ads_spi_init(void);

extern unsigned char ads_spi_write(unsigned char data);
extern unsigned char *ads_spi_write_buf(unsigned char *data, int len);

extern int ads_spi_stop(void);

extern int bhi_spi_init(void);

extern unsigned char bhi_spi_write(unsigned char data);
extern unsigned char *bhi_spi_write_buf(unsigned char *data, int len);

extern int bhi_spi_stop(void);

#endif