/*
 * LCD.c
 * Created: 16/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carné  : 23020
 */

#define F_CPU 16000000UL

#include "lcd.h"
#include <util/delay.h>

#define SET_BIT(reg, bit)         ((reg) |= (1 << (bit)))
#define CLR_BIT(reg, bit)         ((reg) &= ~(1 << (bit)))
#define WRITE_BIT(reg, bit, val)  ((val) ? SET_BIT(reg, bit) : CLR_BIT(reg, bit))

#define RS_PORT PORTD
#define RS_DDR  DDRD
#define RS_BIT  PD2

#define E_PORT  PORTD
#define E_DDR   DDRD
#define E_BIT   PD3

#define D0_PORT PORTD
#define D0_DDR  DDRD
#define D0_BIT  PD4

#define D1_PORT PORTD
#define D1_DDR  DDRD
#define D1_BIT  PD5

#define D2_PORT PORTD
#define D2_DDR  DDRD
#define D2_BIT  PD6

#define D3_PORT PORTD
#define D3_DDR  DDRD
#define D3_BIT  PD7

#define D4_PORT PORTB
#define D4_DDR  DDRB
#define D4_BIT  PB0

#define D5_PORT PORTB
#define D5_DDR  DDRB
#define D5_BIT  PB1

#define D6_PORT PORTB
#define D6_DDR  DDRB
#define D6_BIT  PB2

#define D7_PORT PORTB
#define D7_DDR  DDRB
#define D7_BIT  PB3

static void lcd_pulse_enable(void)
{
    SET_BIT(E_PORT, E_BIT);
    _delay_us(1);
    CLR_BIT(E_PORT, E_BIT);
    _delay_us(100);
}

static void lcd_write_byte(uint8_t byte, uint8_t rs)
{
    WRITE_BIT(RS_PORT, RS_BIT, rs);

    WRITE_BIT(D0_PORT, D0_BIT, (byte >> 0) & 1);
    WRITE_BIT(D1_PORT, D1_BIT, (byte >> 1) & 1);
    WRITE_BIT(D2_PORT, D2_BIT, (byte >> 2) & 1);
    WRITE_BIT(D3_PORT, D3_BIT, (byte >> 3) & 1);
    WRITE_BIT(D4_PORT, D4_BIT, (byte >> 4) & 1);
    WRITE_BIT(D5_PORT, D5_BIT, (byte >> 5) & 1);
    WRITE_BIT(D6_PORT, D6_BIT, (byte >> 6) & 1);
    WRITE_BIT(D7_PORT, D7_BIT, (byte >> 7) & 1);

    lcd_pulse_enable();
}


void lcd_command(uint8_t cmd)
{
    lcd_write_byte(cmd, 0);

    if (cmd == 0x01 || cmd == 0x02)
        _delay_ms(2);
    else
        _delay_us(50);
}

void lcd_data(uint8_t data)
{
    lcd_write_byte(data, 1);
    _delay_us(50);
}

void lcd_clear(void)
{
    lcd_command(0x01);
}

void lcd_gotoxy(uint8_t col, uint8_t row)
{
    uint8_t address;

    if (row == 0)
        address = col;
    else
        address = 0x40 + col;

    lcd_command(0x80 | address);
}

void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data(*str);
        str++;
    }
}

void lcd_init(void)
{

    SET_BIT(RS_DDR, RS_BIT);
    SET_BIT(E_DDR, E_BIT);

    SET_BIT(D0_DDR, D0_BIT);
    SET_BIT(D1_DDR, D1_BIT);
    SET_BIT(D2_DDR, D2_BIT);
    SET_BIT(D3_DDR, D3_BIT);
    SET_BIT(D4_DDR, D4_BIT);
    SET_BIT(D5_DDR, D5_BIT);
    SET_BIT(D6_DDR, D6_BIT);
    SET_BIT(D7_DDR, D7_BIT);

    CLR_BIT(E_PORT, E_BIT);

    _delay_ms(20);

    lcd_write_byte(0x30, 0);
    _delay_ms(5);

    lcd_write_byte(0x30, 0);
    _delay_us(150);

    lcd_write_byte(0x30, 0);
    _delay_us(150);

    lcd_command(0x38);
    lcd_command(0x0C);
    lcd_command(0x01);
    lcd_command(0x06);
}
