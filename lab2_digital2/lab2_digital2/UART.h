/*
 * UART.h
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */ 

#ifndef UART_H_
#define UART_H_

#define F_CPU 16000000UL

#include <avr/io.h>

extern volatile char uart_rx_char;
extern volatile uint8_t uart_rx_flag;

void uart_init(void);
void uart_transmit(char data);
void uart_print(const char *str);

#endif
