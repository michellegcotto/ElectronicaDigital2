/*
 * display7.c
 * Created: 9/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carnet : 23020
 */

#include "display7.h"

static const uint8_t seg_table[10] =
{
	0b0111111,
	0b0000110,
	0b1011011,
	0b1001111,
	0b1100110,
	0b1101101,
	0b1111101,
	0b0000111,
	0b1111111,
	0b1101111
};

void display7_digit(uint8_t digit)
{
	if (digit > 9) return;

	uint8_t pattern = seg_table[digit];
	uint8_t portd_mask = 0;

	if (pattern & (1 << 0)) portd_mask |= (1 << PD2);
	if (pattern & (1 << 1)) portd_mask |= (1 << PD3);
	if (pattern & (1 << 2)) portd_mask |= (1 << PD4);
	if (pattern & (1 << 3)) portd_mask |= (1 << PD5);
	if (pattern & (1 << 4)) portd_mask |= (1 << PD6);
	if (pattern & (1 << 5)) portd_mask |= (1 << PD7);

	PORTD = (PORTD & 0x03) | portd_mask;

	if (pattern & (1 << 6))
	PORTB |= (1 << PB0);
	else
	PORTB &= ~(1 << PB0);
}

void display7_off(void)
{
	PORTD &= 0x03;
	PORTB &= ~(1 << PB0);
}
