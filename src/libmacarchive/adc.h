#include <stdio.h>
#include <unistd.h>

#define ADC_PLAIN 0x01
#define ADC_2BYTE 0x02
#define ADC_3BYTE 0x03

#define bool short
#define true 1
#define false 0

int adc_decompress(int in_size, unsigned char *input, int avail_size, unsigned char *output, int *bytes_written);
int adc_chunk_type(char _byte);
int adc_chunk_size(char _byte);
int adc_chunk_offset(unsigned char *chunk_start);
