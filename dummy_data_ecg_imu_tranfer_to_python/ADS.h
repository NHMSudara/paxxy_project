
#ifndef ADS1298_H
	#define ADS1298_H

	#include "ADS_definitions.h"

	extern int ads_spi_init(void);

	extern int ADS1298_init_gpio(struct ADS_sensor *sensor, uint8_t id);
	extern int ADS1298_init_device(struct ADS_sensor *sensor);

	extern int ADS1298_get_and_process_data(struct ADS_sensor *sensor);
	
	extern int ADS131_init_gpio(struct ADS_sensor *sensor, uint8_t id);
	extern int ADS131_init_device(struct ADS_sensor *sensor);

	extern int ADS131_get_and_process_data(struct ADS_sensor *sensor);

#endif
