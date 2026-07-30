/*
 * lab3_digital2.c
 *
 * MCU Esclavo
 * Laboratorio 3 - SPI
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "SPI.h"
#include "ADC.h"

volatile uint8_t adc0 = 0;
volatile uint8_t adc1 = 0;

volatile uint8_t esperandoLED = 0;

int main(void)
{
	
	UCSR0B	= 0x00;	// Apagar pines por UART
	
	/* LEDs */
	DDRD = 0xFF;
	PORTD = 0x00;

	/* ADC */
	adc_init();

	/* SPI Esclavo */
	spiInit(
		SPI_SLAVE_SS,
		SPI_DATA_ORDER_MSB,
		SPI_CLOCK_IDLE_LOW,
		SPI_CLOCK_FIRST_EDGE
	);

	/* Interrupción SPI */
	SPCR |= (1<<SPIE);

	sei();

	while (1)
	{
		/* Leer continuamente ambos potenciómetros */

		adc0 = (uint8_t)adc_read(0);

		adc1 = (uint8_t)adc_read(1);
		
		//PORTD = adc0;

		_delay_ms(1);
	}
}

/*=============================
      INTERRUPCIÓN SPI
==============================*/

ISR(SPI_STC_vect)
{
    uint8_t dato = SPDR;

    switch(dato)
    {
        case 'a':
            SPDR = adc0;
        break;

        case 'b':
            SPDR = adc1;
        break;

        case 'L':
            esperandoLED = 1;
            SPDR = 0;
        break;

        default:

            if(esperandoLED)
            {
                PORTD = dato;
                esperandoLED = 0;
            }

            SPDR = 0;
        break;
    }
}