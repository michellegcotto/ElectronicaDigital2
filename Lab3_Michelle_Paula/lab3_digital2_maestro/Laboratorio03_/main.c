/*
 * Laboratorio03.c
 * Maestro
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "MAESTRO/MAESTRO.h"
#include "UART/UART.h"

uint8_t adc0 = 0;
uint8_t adc1 = 0;

int main(void)
{
	
	UCSR0B	= 0x00;	// Apagar pines por UART
	
    cli();

	UCSR0B	= 0x00;	// Apagar pines por UART
	
    /* LEDs */
	DDRD = 0xFF;
    PORTD = 0;

    /* SS */

    DDRC |= (1<<DDC5);
    PORTC |= (1<<PORTC5);

    Init_UART();

    spiInit(
        SPI_MASTER_OSC_DIV4,
        SPI_DATA_ORDER_MSB,
        SPI_CLOCK_IDLE_LOW,
        SPI_CLOCK_FIRST_EDGE
    );

    sei();

    while(1)
    {
        /*============================
             PARTE 1
        ============================*/

        PORTC &= ~(1<<PORTC5);

        spiTransfer('a');
        adc0 = spiTransfer(0);

        PORTC |= (1<<PORTC5);

        write_str("ADC0: ");
        enviar_numero(adc0);
        write_str("\r\n");

        _delay_ms(100);

        PORTC &= ~(1<<PORTC5);

        spiTransfer('b');
        adc1 = spiTransfer(0);

        PORTC |= (1<<PORTC5);

        write_str("ADC1: ");
        enviar_numero(adc1);
        write_str("\r\n");

        _delay_ms(100);

        /*============================
             PARTE 2
        ============================*/

        UART_procesar_entrada();

        if(uart_nuevo)
        {
            PORTD = uart_valor;

            PORTC &= ~(1<<PORTC5);

            spiTransfer('L');

            spiTransfer(uart_valor);

            PORTC |= (1<<PORTC5);

            uart_nuevo = 0;
        }

        _delay_ms(200);
    }
}