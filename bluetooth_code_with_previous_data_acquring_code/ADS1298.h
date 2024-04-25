typedef struct {
    int ecg1;
    int ecg2;
    int ecg3;
    int ecg4;
} ECGValues;


#ifndef ADS1298_H
	#define ADS1298_H
	
	extern int ADS1298_init_gpio(void);
	extern int ADS1298_init_device(void);
	
	extern void ADS1298_set_data_log_started(unsigned char state);
	
	extern unsigned char *ADS1298_read_data(void);
	extern int ADS1298_stop_spi(void);
	
	extern int ADS1298_data_read_task(void);
	
	extern ECGValues ADS1298_print_data_2_file(FILE *data_file, int buffer_index);
	
#endif
