#ifndef I2C__H_
#define I2C__H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdint.h>

// Funciones originales
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler);
uint8_t I2C_Master_Start(void);
uint8_t I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
uint8_t I2C_Master_Write(uint8_t dato);
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);
void I2C_Slave_Init(uint8_t address);

// ----- NUEVAS: funciones con timeout -----
uint8_t I2C_Master_Start_Timeout(void);
uint8_t I2C_Master_RepeatedStart_Timeout(void);
uint8_t I2C_Master_Write_Timeout(uint8_t dato);
uint8_t I2C_Master_Read_Timeout(uint8_t *buffer, uint8_t ack);

#endif /* I2C__H_ */