/*
 * UART.c
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */ 

#define F_CPU 16000000UL

#include "uart.h"
#include <avr/interrupt.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

volatile char uart_rx_char = 0;
volatile uint8_t uart_rx_flag = 0;

void uart_init(void)
{
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);

    UCSR0B = (1 << TXEN0) |
             (1 << RXEN0) |
             (1 << RXCIE0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
}

void uart_transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}

void uart_print(const char *str)
{
    while (*str)
    {
        uart_transmit(*str);
        str++;
    }
}

ISR(USART_RX_vect)
{
    uart_rx_char = UDR0;
    uart_rx_flag = 1;
}
