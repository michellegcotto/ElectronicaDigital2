/*
 * I2C_.h
 *
 * Created: 8/9/2026 2:14:14 PM
 *  Author: ayala
 */ 


#ifndef I2C__H_
#define I2C__H_

#ifndef	F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <stdint.h>

// Inicializar I2C - MAESTRO
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler);

// Inicio comunicación I2C
uint8_t I2C_Master_Start(void);
uint8_t I2C_Master_RepeatedStart(void);

// Detener comunicación I2C
void I2C_Master_Stop(void);

// Transmision de datos del maestro al esclavo
// devuelve 0 si el esclavo recibe el dato
uint8_t I2C_Master_Write(uint8_t dato);

// Recepcion de datos enviados por el esclavo al maestro
// lectura de datos del esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);

// Inicializar I2C - ESCLAVO
void I2C_Slave_Init(uint8_t address);

#endif /* I2C__H_ */