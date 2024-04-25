
/* standard headers */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/* mraa header */
#include "mraa/spi.h"

#include "definitions.h"


/* SPI declaration */
#define ADS_SPI_BUS 1
#define BHI_SPI_BUS 2

/* SPI frequency in Hz */
#define ADS_SPI_FREQ 2000000		//1 MHz
#define BHI_SPI_FREQ 50000			//50 kHz

static mraa_spi_context spi, spi2;



int ads_spi_init(void)
{
	mraa_result_t status = MRAA_SUCCESS;
	
	/* initialize mraa for the platform (not needed most of the times) */
    mraa_init();

	/* initialize SPI bus */
    spi = mraa_spi_init_raw(ADS_SPI_BUS,0);
    if (spi == NULL) 
    {
        fprintf(stderr, "Failed to initialize SPI\n");
        mraa_deinit();
        return FAILED;
    }
    /* set SPI frequency */
    status = mraa_spi_frequency(spi, ADS_SPI_FREQ);
    if (status != MRAA_SUCCESS)
	{
		fprintf(stdout, "Failed to set SPI frequency to %d Hz\n", ADS_SPI_FREQ);
		goto err_exit;
	}
    /* set big endian mode */
    status = mraa_spi_lsbmode(spi, 0);
    if (status != MRAA_SUCCESS) 
    {
		goto err_exit;
    }
    /* set SPI mode */
    status = mraa_spi_mode(spi, MRAA_SPI_MODE1);
    if (status != MRAA_SUCCESS)
    {
		fprintf(stdout, "Failed to set SPI mode to 1\n");
        goto err_exit;
	}    
    
//    status = mraa_spi_bit_per_word(spi, 16);
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

unsigned char ads_spi_write(unsigned char data)
{
	mraa_spi_write(spi,data);
}

unsigned char *ads_spi_write_buf(unsigned char *data, int len)
{
	unsigned char *rx;
	rx = mraa_spi_write_buf(spi, data, len);		
	return rx;
}


int ads_spi_stop(void)    
{
    /* stop spi */
    mraa_spi_stop(spi);
	
	return SUCCEEDED;
}

int bhi_spi_init(void)
{
	mraa_result_t status = MRAA_SUCCESS;
	
	/* initialize mraa for the platform (not needed most of the times) */
    mraa_init();

	/* initialize SPI bus */
    spi2 = mraa_spi_init_raw(BHI_SPI_BUS,0);
    if (spi2 == NULL) 
    {
        fprintf(stderr, "Failed to initialize BHI SPI\n");
        mraa_deinit();
        return FAILED;
    }
    /* set SPI frequency */
    status = mraa_spi_frequency(spi2, BHI_SPI_FREQ);
    if (status != MRAA_SUCCESS)
	{
		fprintf(stdout, "Failed to set BHI SPI frequency to %d Hz\n", BHI_SPI_FREQ);
		goto err_exit;
	}
    /* set big endian mode */
    status = mraa_spi_lsbmode(spi2, 0);
    if (status != MRAA_SUCCESS) 
    {
		goto err_exit;
    }
    /* set SPI mode */
    status = mraa_spi_mode(spi2, MRAA_SPI_MODE0);
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

unsigned char bhi_spi_write(unsigned char data)
{
	mraa_spi_write(spi2,data);
}

unsigned char *bhi_spi_write_buf(unsigned char *data, int len)
{
	unsigned char *rx;
	rx = mraa_spi_write_buf(spi2, data, len);		
	return rx;
}


int bhi_spi_stop(void)    
{
    /* stop spi */
    mraa_spi_stop(spi2);
	
	return SUCCEEDED;
}
