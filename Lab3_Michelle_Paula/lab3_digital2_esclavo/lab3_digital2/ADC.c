/*
 * ADC.c
 *
 * Created: 23/07/2026
 * Author : Jaqueline Michelle González Cotto / Paula Melissa Ayala
 */ 

#include "adc.h"

void adc_init(void)
{
	DDRC &= ~((1<<PC0)|(1<<PC1));

	PORTC &= ~((1<<PC0)|(1<<PC1));

	ADMUX = (1<<REFS0);

	ADMUX |= (1<<ADLAR);

	ADCSRA = (1<<ADEN);

	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

	DIDR0 |= (1<<ADC0D)|(1<<ADC1D);
}

uint8_t adc_read(uint8_t channel)
{
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	ADCSRA |= (1<<ADSC);

	while(ADCSRA & (1<<ADSC));

	return ADCH;
}

