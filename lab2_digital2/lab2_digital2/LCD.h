/*
 * LCD.h
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */ 

#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000UL

#include <avr/io.h>

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_clear(void);
void lcd_gotoxy(uint8_t col, uint8_t row);
void lcd_print(const char *str);

#endif
