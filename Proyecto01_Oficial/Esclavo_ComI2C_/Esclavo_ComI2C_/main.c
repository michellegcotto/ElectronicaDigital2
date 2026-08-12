/*
 * Esclavo_ComI2C_.c
 *
 * Created: 8/8/2026 7:26:53 PM
 * Author : ayala
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C_.h"

#define SlaveAdress 0x30

uint8_t buffer = 0;

volatile uint8_t valorADC = 0;

void initADC(void);

int main(void)
{
	cli();
    DDRB |= (1<<DDB5);
	PORTB &= ~(1<<PORTB5);
	initADC();
	
	I2C_Slave_Init(SlaveAdress);
	sei();
	
    while (1) 
    {
		if (buffer == 'R'){
			PINB |= (1<<PINB5);
			buffer = 0;
		}
		
		// iniciando secuencia adc
		ADCSRA |= (1<<ADSC);
		_delay_ms(100);
    }
}

void initADC(void){
	ADMUX = 0;
	
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	
	ADMUX |= (1<<ADLAR);
	
	ADCSRA = 0;
	ADCSRA |= (1<<ADEN);
	
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

	DIDR0 |= (1<<ADC0D)|(1<<ADC1D);
}
	
ISR(ADC_vect){
	valorADC = ADCH;
	ADCSRA |= (1<<ADIF);
}

	
ISR(TWI_vect){
	uint8_t estado = TWSR & 0xFC;
	switch(estado){
		// slave debe recibir dato
		
		case 0x60:	// SLA+W recibido
		case 0x70:	// General Call
			//TWCR |= (1<<TWINT)
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			break;
			
		case 0x80:	// Dato recibido, ACK enviado
		case 0x90:	// Dato recibido General Call, ACK enviado
			buffer = TWDR;
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			break;
			
		// Slave debe transmitir dato -----------------------------
		
		case 0xA8:	// SLA+R recibido
		case 0xB8:	// Dato transmitido, ACK recibido
			TWDR = valorADC;	// dato a enviar
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			break;
			
		case 0xC0:	// Dato transmitido, NACK recibido
		case 0xC8:	// Ultimo dato transmitido
			TWCR = 0;
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWIE)|(1<<TWEA);
			break;
			
		case 0xA0:	// STOP o repeated START recibido como slave
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			break;
			
		// Cualquier error ----------------------------------------
		default:
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			break;
	}
}

