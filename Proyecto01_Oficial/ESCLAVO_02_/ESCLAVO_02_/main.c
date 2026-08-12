#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>


// =====================================================
// I2C
// =====================================================
#define I2C_SLAVE_ADDRESS 0x09


// =====================================================
// SENSOR DE LLUVIA
// =====================================================
#define SENSOR_LLUVIA_PIN PC0


// =====================================================
// SERVO
// =====================================================
#define SERVO_PIN PB1


// =====================================================
// VALORES DEL SERVO
// =====================================================
#define SERVO_0_OCR     2000
#define SERVO_90_OCR    3000
#define SERVO_180_OCR   4000


// =====================================================
// VARIABLES
// =====================================================
volatile uint8_t lluvia_estado = 0;
volatile uint8_t servo_posicion = 0;


// =====================================================
// INICIALIZAR SENSOR DE LLUVIA
// =====================================================
void sensor_lluvia_init(void)
{
    // PC0 como entrada
    DDRC &= ~(1 << SENSOR_LLUVIA_PIN);
    // Sin pull-up interno
    PORTC &= ~(1 << SENSOR_LLUVIA_PIN);
}


// =====================================================
// INICIALIZAR SERVO - TIMER1
// =====================================================
void servo_init(void)
{
    DDRB |= (1 << SERVO_PIN);
    TCCR1A =(1 << COM1A1) |(1 << WGM11);

    TCCR1B =(1 << WGM13) |(1 << WGM12) |(1 << CS11);
    ICR1 = 39999;
    OCR1A = SERVO_0_OCR;
    servo_posicion = 0;
}


// =====================================================
// MOVER SERVO A 0 GRADOS
// =====================================================
void servo_0_grados(void)
{
    OCR1A = SERVO_0_OCR;
    servo_posicion = 0;
}


// =====================================================
// MOVER SERVO A 90 GRADOS
// =====================================================
void servo_90_grados(void)
{
    OCR1A = SERVO_90_OCR;
    servo_posicion = 90;
}


// =====================================================
// ACTUALIZAR SENSOR Y SERVO
// =====================================================
void sensor_lluvia_actualizar(void)
{
    if (PINC & (1 << SENSOR_LLUVIA_PIN))
    {
        // Hay lluvia
        lluvia_estado = 1;
        servo_90_grados();
    }
    else
    {
        // No hay lluvia
        lluvia_estado = 0;
        servo_0_grados();
    }
}


// =====================================================
// INICIALIZAR I2C COMO ESCLAVO
// =====================================================
void i2c_slave_init(void)
{
    TWAR = (I2C_SLAVE_ADDRESS << 1);
    TWCR =(1 << TWEN) |(1 << TWEA) |(1 << TWIE);
}


// =====================================================
// INTERRUPCION I2C
// =====================================================
ISR(TWI_vect)
{
    uint8_t estado;
    estado = TWSR & 0xF8;

    switch (estado)
    {
        // =============================================
        // SLA+R recibido
        // =============================================
        case 0xA8:
            TWDR = lluvia_estado;
            TWCR =(1 << TWINT) |(1 << TWEN)  |(1 << TWEA)  |(1 << TWIE);
            break;

        // =============================================
        // Byte transmitido + ACK recibido
        // =============================================
        case 0xB8:
            TWDR = servo_posicion;
            TWCR = (1 << TWINT) |(1 << TWEN)  |(1 << TWEA)  |(1 << TWIE);
            break;

        // =============================================
        // Byte transmitido + NACK recibido
        // =============================================
        case 0xC0:
            TWCR =(1 << TWINT) |(1 << TWEN)  |(1 << TWEA)  | (1 << TWIE);
            break;

        // =============================================
        // Ultimo byte transmitido + ACK
        // =============================================
        case 0xC8:
            TWCR =(1 << TWINT) | (1 << TWEN)  |(1 << TWEA)  |(1 << TWIE);
            break;

        // =============================================
        // Cualquier otro estado
        // =============================================
        default:
            TWCR =(1 << TWINT) |(1 << TWEN)  |(1 << TWEA)  |(1 << TWIE);
            break;
    }
}


int main(void)
{
    // Sensor de lluvia
    sensor_lluvia_init();
    // Servo con Timer1 a 50 Hz
    servo_init();
    // I2C esclavo
    i2c_slave_init();
    sei();
	
    // Leer estado inicial
    sensor_lluvia_actualizar();

    while (1)
    {
        // Revisar continuamente el sensor
        sensor_lluvia_actualizar();
    }
    return 0;
}