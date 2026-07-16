/*
 * display7.h
 * Created: 9/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carnet : 23020
 */



#ifndef DISPLAY7_H_
#define DISPLAY7_H_

#include <avr/io.h>
#include <stdint.h>

void display7_digit(uint8_t digit);
void display7_off(void);

#endif