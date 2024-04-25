typedef struct {
    int mic1;
    int mic2;
    int mic3;
    int mic4;
} AcousticVal;

#ifndef ADS131M08_H
#define ADS131M08_H

	extern int ADS131_init_gpio(void);
	extern int ADS131_init_device(void);
	
	extern void ADS131_set_data_log_started(unsigned char state);
	
	extern unsigned char *ADS131_read_data(void);
	extern int ADS131_stop_spi(void);
	
	extern int ADS131_data_read_task(void);

	extern AcousticVal ADS131_print_data_2_file(FILE *data_file, int buffer_index);

#endif
