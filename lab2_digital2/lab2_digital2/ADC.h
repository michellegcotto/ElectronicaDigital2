/*
 * ADC.h
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */ 

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void adc_init(void);
uint16_t adc_read(uint8_t channel);

#endif