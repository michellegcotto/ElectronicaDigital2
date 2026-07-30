/*
 * ADC.h
 *
 * Created: 23/07/2026
 * Author : Jaqueline Michelle González Cotto / Paula Melissa Ayala
 */  


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

void adc_init(void);

uint8_t adc_read(uint8_t channel);

#endif