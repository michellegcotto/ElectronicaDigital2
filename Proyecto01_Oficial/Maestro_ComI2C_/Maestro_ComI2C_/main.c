/*
 * Maestro_ComI2C_.c
 *
 * Created: 8/8/2026 7:25:03 PM
 * Author : ayala
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include "I2C_.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (slave1 << 1) | 0x01			// lectura
#define slave1W (slave1 << 1) & 0b11111110		// escritura

uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;

void refreshPORT(uint8_t valor);

int main(void)
{
	// puerto de salida para leds
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	DDRB |= (1<<DDB0)|(1<<DDB1);
	// puerto led
	DDRB |= (1<<DDB5);
	// iniciar led en 0
	PORTB &= ~(1<<PORTB5);
	
	// limpiar puerto de leds
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1));
	PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7));
	
	// Inicializamos Master
	I2C_Master_Init(100000,1); // Fscl 100kHz, prescaler 1
    
    while (1) 
    {
		PORTB |= (1<<PORTB5);
		
		if (!I2C_Master_Start()) return 0;
		
		if (!I2C_Master_Write(slave1W)){
			I2C_Master_Stop();
			return 0;
		}
		
		I2C_Master_Write('R');
		
		if (!I2C_Master_RepeatedStart()){
			I2C_Master_Stop();
			return 0;
		}
		
		if (!I2C_Master_Write(slave1R)){
			I2C_Master_Stop();
			return 0; 
		}
		
		I2C_Master_Read(&bufferI2C, 0); // NACK
		I2C_Master_Stop();
		
		PORTB &= ~(1<<PORTB5);
		
		//refreshPORT(bufferI2C);
		PORTD = bufferI2C;
		_delay_ms(1000);
		
    }
}

