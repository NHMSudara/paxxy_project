
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define FAILED -1
#define SUCCEEDED 0

#define TRUE 1
#define YES 1
#define NO 0

#define PROCESSING 2
#define VS_CONFIGURED 2

union int24_values
{
	int32_t i32;
	int32_t i24:24;
	uint8_t ui8[4];
};

union int16_values
{
	int16_t i16;
	uint16_t ui16;
	uint8_t ui8[2];
};


union float_values
{
	float f;
	uint8_t ui8[4];
};



#endif
