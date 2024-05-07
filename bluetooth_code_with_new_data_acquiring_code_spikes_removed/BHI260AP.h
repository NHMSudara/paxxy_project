

#ifndef BHI260AP_H
	#define BHI260AP_H
	
	#include "BHI260AP_definitions.h"

	extern int BHI260AP_init_spi(void);
	extern int BHI260AP_init_sensor_gpio(struct BHI_sensor *sensor, uint8_t id, uint8_t cs_pin, uint8_t drdy_pin);
	extern int BHI260AP_init_sensor(struct BHI_sensor *sensor, uint8_t conf_sensors);
	
	extern int8_t BHI260AP_get_and_process_fifo(struct BHI_sensor *sensor);

	extern int BHI_get_drdy_status(struct BHI_sensor *sensor);
	extern int8_t BHI260AP_flush_fifo(struct BHI_sensor *sensor);

//	extern int BHI260AP_data_read_task(void);
//
//	extern void BHI260AP_print_data_2_file(FILE *data_file);
//
	
#endif
