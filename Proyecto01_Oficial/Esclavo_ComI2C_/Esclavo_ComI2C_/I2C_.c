/*
 * I2C_.c
 *
 * Created: 8/9/2026 2:14:05 PM
 *  Author: ayala
 */ 

#include "I2C_.h"

// Inicializar I2C - MAESTRO
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler){
	
	// Pines SDA, SCL entradas (PC4, PC5)
	DDRC &= ~((1<<DDC4)|(1<<DDC5)); 
	
	// seleccion de valor de bits para prescaler de registro TWSR
	switch(Prescaler){
		case 1:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		break;
		case 4:
			TWSR &= ~(1<<TWPS1);
			TWSR |= (1<<TWPS0);
		break;
		case 16:
			TWSR &= ~(1<<TWPS0);
			TWSR |= (1<<TWPS1);
		break;
		case 64:
			TWSR |= (1<<TWPS1)|(1<<TWPS0);
		break;
		default:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
			Prescaler = 1;
		break;
	}
	TWBR = ((F_CPU/SCL_Clock)-16)/(2*Prescaler);	// Calculo de velocidad
	TWCR |= (1<<TWEN);	// Activar la interfase (TWI) I2C	
}

// FUNCION inicio comunicación I2C
uint8_t I2C_Master_Start(void){
	// master, reiniciar bandera int, condicion start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	
	while (!(TWSR & (1<<TWINT))); // espera para que encienda bandera
	
	return ((TWSR & 0xF8) == 0x08); // máscara para mantener bits de estado
	// 0x08 = A START condition has been transmitted
}

// FUNCION reinicio de comunicacion I2C
uint8_t I2C_Master_RepeatedStart(void){
	// master, reiniciar bandera int, condicion start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	
	while (!(TWSR & (1<<TWINT))); // espera para que encienda bandera
	
	return ((TWSR & 0xF8) == 0x10); // máscara para mantener bits de estado TWI
	// 0x10 = A repeated START condition has been transmitted
}

// FUNCION parar comunicación I2C
void I2C_Master_Stop(void){
	// Envio secuencia para STOP
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	
	while(TWCR & (1<<TWSTO)); // Esperamos a que el bit se limpie
}

// FUNCION transmision datos de maestro a esclavo
// devuelve 0 si el esclavo recibe el dato
uint8_t I2C_Master_Write(uint8_t dato){
	uint8_t estado;
	
	TWDR = dato;	// Cargar dato
	TWCR = (1<<TWEN)|(1<<TWINT);	// iniciar secuencia envio
	
	while(!(TWCR & (1<<TWINT)));	// espera al flag TWINT
	estado = TWSR & 0xF8;	// máscara para mantener bits de estado TWI
	
	// Verificamos si se transmitio una SLA+W con ACK (0x18), o un dato con ACK (0x28)
	if (estado == 0x18 || estado == 0x28){
		return 1;
	}else{
		return estado;
	}
}

// FUNCION recepcion de datos enviados por esclavo al maestro
// lectura de datos del esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack){
	uint8_t estado;
	
	if (ack){
		// ACK: quiero más datos
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); // Habilitamos interfase I2C con envio de ACK
	}else{
		// NACK: último byte
		TWCR = (1<<TWINT)|(1<<TWEN); // Habilitamos interfase I2C sin envio de ACK
	}
	
	while (!(TWCR & (1<<TWINT))); // Esperar flag TWINT
	
	estado = TWSR & 0xF8; // Máscara bits estado TWI
	
	// verificar recibir dato con ACK o sin ACK
	if (ack && estado != 0x50) return 0; // Data recibida, ACK
	if (!ack && estado != 0x58) return 0; // Data recibida, NACK
	
	*buffer = TWDR; // obtener resultado en registro de datos
	return 1;	
}

// FUNCION inicializar I2C Esclavo
void I2C_Slave_Init(uint8_t address){
	DDRC &= ~((1<<DDC4)|(1<<DDC5));	// pines de i2c como entradas
	
	TWAR = address << 1; // se asigna la dirección que tendrá
	
	// se habilita la interfaz, ACK automatico, se habilita la ISR
	TWCR = (1<<TWEA)|(1<<TWEN)|(1<<TWIE);
}

