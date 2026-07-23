/*
 * lab2_digital2.c
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd.h"
#include "adc.h"
#include "uart.h"

#define POT1_CHANNEL 0
#define POT2_CHANNEL 1

volatile int16_t contador = 0;

int main(void)
{
	uint16_t adc_pot1, adc_pot2;
	float voltaje_pot1;
	char buffer[17];
	char num_buffer[8];
	char uart_msg[40];

	lcd_init();
	adc_init();
	uart_init();

	sei();

	lcd_clear();

	while (1)
	{
		adc_pot1 = adc_read(POT1_CHANNEL);
		voltaje_pot1 = (adc_pot1 * 5.0) / 1023.0;

		adc_pot2 = adc_read(POT2_CHANNEL);

		if (uart_rx_flag)
		{
			uart_rx_flag = 0;

			if (uart_rx_char == '+')
			contador++;
			else if (uart_rx_char == '-')
			contador--;
		}

		dtostrf(voltaje_pot1,4,2,num_buffer);

		sprintf(uart_msg,
		"S1:%sV S2:%u S3:%d\r\n",
		num_buffer,
		adc_pot2,
		contador);

		uart_print(uart_msg);

		sprintf(buffer,"S1:%sV",num_buffer);
		lcd_gotoxy(0,0);
		lcd_print(buffer);

		sprintf(buffer,"S2:%-4u",adc_pot2);
		lcd_gotoxy(8,0);
		lcd_print(buffer);

		sprintf(buffer,"S3:%-6d",contador);
		lcd_gotoxy(0,1);
		lcd_print(buffer);

		_delay_ms(200);
	}
}