/*
 * v1_.c
 *
 * Created: 8/11/2026 5:41:31 PM
 *  Author: ayala
 */ 
// MAESTRO

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>


// =====================================================
// CONFIGURACIÓN I2C
// =====================================================
#define S1_ADDR 0x08
#define S2_ADDR 0x09

// =====================================================
// VARIABLES DEL SISTEMA
// =====================================================

// Esclavo 1
uint16_t distancia = 0;
uint8_t stepper_activo = 0;

// Esclavo 2
uint8_t lluvia = 0;
uint8_t servo_posicion = 0;

// Lógica LM75
#define LM75_ADDR       0x48
#define LM75_TEMP_REG   0x00

#define TEMP_UMBRAL     30

#define TIEMPO_MOTOR_MS 4000

// Motor DC
#define MOTOR_IN1       PC1
#define MOTOR_IN2       PC2

// =====================================================
// UART
// =====================================================
void uart_init(unsigned int baud){
    unsigned int ubrr = F_CPU / 16 / baud - 1;

    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << TXEN0) |
             (1 << RXEN0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
}


// -----------------------------------------------------
// Enviar un carácter
// -----------------------------------------------------
void uart_putc(char c){
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}


// -----------------------------------------------------
// Enviar texto
// -----------------------------------------------------
void uart_print(const char *s)
{
    while (*s){
		uart_putc(*s++);
    }
}


// -----------------------------------------------------
// Enviar número entero sin signo
// -----------------------------------------------------
void uart_print_num(uint16_t num){
    char buffer[10];
    itoa(num, buffer, 10);
    uart_print(buffer);
}


// =====================================================
// TWI / I2C MASTER
// =====================================================
void twi_init(void){    
	TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}


// =====================================================
// START
// =====================================================
uint8_t twi_start(void){
    TWCR = (1 << TWINT) |
           (1 << TWSTA) |
           (1 << TWEN);

    /* Esperar a que el hardware TWI
     * termine la operación.
     *
     * esperamos la operación
     * física de I2C.	*/

    while (!(TWCR & (1 << TWINT)));
    /* Estados esperados:
     *
     * 0x08 = START transmitido
     * 0x10 = REPEATED START transmitido	*/
    return (TWSR & 0xF8);
}


// =====================================================
// STOP
// =====================================================

void twi_stop(void){
    TWCR = (1 << TWINT) |
           (1 << TWSTO) |
           (1 << TWEN);

    /* Pequeña espera para asegurar que
     * el STOP se complete.	*/
    _delay_us(100);
}

uint8_t twi_status(void)
{
	return TWSR & 0xF8;
}


// =====================================================
// WRITE
// =====================================================

uint8_t twi_write(uint8_t data){
    TWDR = data;
	
    //Iniciar transmisión.
    TWCR = (1 << TWINT) |
           (1 << TWEN);
		   
    // Esperar a que I2C termine.
    while (!(TWCR & (1 << TWINT)));

	// Devolver estado TWI
    return (TWSR & 0xF8);
}


// =====================================================
// READ + ACK
// =====================================================

uint8_t twi_read_ack(void){
	// TWEA = 1
	// post recibir datos, maestro responde ACK
	// -> Aún solicita más datos

    TWCR = (1 << TWINT) |
           (1 << TWEN) |
           (1 << TWEA);

    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}


// =====================================================
// READ + NACK
// =====================================================

uint8_t twi_read_nack(void){
	// TWEA no activo
	// post recibir dato, maestro responde NACK
	// -> Ya no solicita más datos

    TWCR = (1 << TWINT) |
           (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}


// =====================================================
// LEER ESCLAVO 1
// =====================================================

uint8_t leer_esclavo1(uint16_t *distancia, uint8_t *stepper){
    uint8_t estado;

    uint8_t distancia_low;
    uint8_t distancia_high;
    uint8_t estado_stepper;

    // =================================================
    // 1. START
    // =================================================

    estado = twi_start();

    if (estado != 0x08){
        return 0;}

    // =================================================
    // 2. SLA + READ
    // =================================================
	
	// Dirección Esclavo -> 0x08
	// Para Read -> 0x08<<1 = 0x10
	// 0x10|1 = 0x11

    estado = twi_write((S1_ADDR << 1) | 1);

	// 0x40 -> SLA+R transmitido
	// Esclavo responde ACK

    if (estado != 0x40){
        twi_stop();
        return 0;}


    // 3. LEER DISTANCIA LOW
    distancia_low = twi_read_ack();

    // 4. LEER DISTANCIA HIGH
    distancia_high = twi_read_ack();


    // =================================================
    // 5. LEER ESTADO DEL STEPPER
    // =================================================
	// Último byte, se utiliza NACK
    estado_stepper = twi_read_nack();


    // =================================================
    // 6. RECONSTRUIR DISTANCIA
    // =================================================
	// Esclavo envía -> BYTE0-LOW BYTE1=HIGH

    *distancia =
        ((uint16_t)distancia_high << 8) |
        distancia_low;

    // Guardar estado del stepper.
    *stepper = estado_stepper;


    // =================================================
    // 7. STOP
    // =================================================
    twi_stop();
    return 1;
}

// =====================================================
// LEER ESCLAVO 2
// =====================================================
//
// S2 = 0x09
//
// BYTE 0 -> lluvia
//           0 = NO
//           1 = SI
//
// BYTE 1 -> posición servo
//           0  = 0°
//           90 = 90°
//
// =====================================================

uint8_t leer_esclavo2(uint8_t *lluvia_estado,
                      uint8_t *servo_estado)
{
    uint8_t estado;

    uint8_t lluvia_recibida;
    uint8_t servo_recibido;


    // =================================================
    // 1. START
    // =================================================

    estado = twi_start();

    if (estado != 0x08)
    {
        return 0;
    }


    // =================================================
    // 2. SLA + READ
    // =================================================

    /*
     * Dirección del Esclavo 2:
     *
     * 0x09 << 1 = 0x12
     *
     * 0x12 | 1 = 0x13
     *
     * 0x13 = SLA + READ
     */

    estado = twi_write((S2_ADDR << 1) | 1);


    /*
     * 0x40:
     *
     * SLA+R transmitido
     * y esclavo respondió ACK.
     */

    if (estado != 0x40)
    {
        twi_stop();

        return 0;
    }


    // =================================================
    // 3. LEER LLUVIA
    // =================================================

    /*
     * Como todavía necesitamos otro byte,
     * respondemos ACK.
     */

    lluvia_recibida = twi_read_ack();


    // =================================================
    // 4. LEER POSICIÓN DEL SERVO
    // =================================================

    /*
     * Este es el ÚLTIMO byte.
     *
     * Por eso respondemos NACK.
     */

    servo_recibido = twi_read_nack();


    // =================================================
    // 5. GUARDAR INFORMACIÓN
    // =================================================

    *lluvia_estado = lluvia_recibida;

    *servo_estado = servo_recibido;


    // =================================================
    // 6. STOP
    // =================================================

    twi_stop();


    return 1;
}

// ============================================================
// LM75
// ============================================================

int16_t lm75_read_temperature_x2(void)
{
    uint8_t msb;
    uint8_t lsb;

    // --------------------------------------------------------
    // START
    // --------------------------------------------------------

    twi_start();

    if (twi_status() != 0x08 &&
        twi_status() != 0x10)
    {
        twi_stop();

        uart_print("ERROR LM75: START\r\n");

        return -999;
    }

    // --------------------------------------------------------
    // SLA + W
    // --------------------------------------------------------

    twi_write((LM75_ADDR << 1) | 0);

    if (twi_status() != 0x18)
    {
        twi_stop();

        uart_print("ERROR LM75: SLA+W\r\n");

        return -999;
    }

    // --------------------------------------------------------
    // Registro de temperatura
    // --------------------------------------------------------

    twi_write(LM75_TEMP_REG);

    if (twi_status() != 0x28)
    {
        twi_stop();

        uart_print("ERROR LM75: registro\r\n");

        return -999;
    }

    // --------------------------------------------------------
    // REPEATED START
    // --------------------------------------------------------

    twi_start();

    if (twi_status() != 0x10)
    {
        twi_stop();

        uart_print("ERROR LM75: RESTART\r\n");

        return -999;
    }

    // --------------------------------------------------------
    // SLA + R
    // --------------------------------------------------------

    twi_write((LM75_ADDR << 1) | 1);

    if (twi_status() != 0x40)
    {
        twi_stop();

        uart_print("ERROR LM75: SLA+R\r\n");

        return -999;
    }

    // --------------------------------------------------------
    // Leer los dos bytes
    // --------------------------------------------------------

    msb = twi_read_ack();

    lsb = twi_read_nack();

    twi_stop();

    /*
     * LM75 clasico:
     *
     * MSB = parte entera con signo
     * bit 7 de LSB = 0.5 °C
     *
     * Ejemplo:
     *
     * 25.0 °C -> MSB = 25, bit medio = 0
     * 25.5 °C -> MSB = 25, bit medio = 1
     *
     * Para negativos:
     * el MSB se interpreta como int8_t.
     */

    int8_t temperatura_entera = (int8_t)msb;

    int16_t temperatura_x2 =
        ((int16_t)temperatura_entera * 2);

    if (lsb & 0x80)
    {
        temperatura_x2++;
    }

    return temperatura_x2;
}

// ============================================================
// MOTOR DC
// ============================================================

void motor_stop(void)
{
	PORTC &= ~(1 << MOTOR_IN1);
	PORTC &= ~(1 << MOTOR_IN2);
}


void motor_avanzar(void)
{
	PORTC |=  (1 << MOTOR_IN1);
	PORTC &= ~(1 << MOTOR_IN2);
}


void motor_retroceder(void)
{
	PORTC &= ~(1 << MOTOR_IN1);
	PORTC |=  (1 << MOTOR_IN2);
}

void motor_hacer_avance(void)
{
	uart_print("MOTOR DC -> AVANZANDO 4 segundos\r\n");

	motor_avanzar();

	_delay_ms(TIEMPO_MOTOR_MS);

	motor_stop();

	uart_print("MOTOR DC -> DETENIDO\r\n");
}

void motor_hacer_retroceso(void)
{
	uart_print("MOTOR DC -> RETROCEDIENDO 4 segundos\r\n");

	motor_retroceder();

	_delay_ms(TIEMPO_MOTOR_MS);

	motor_stop();

	uart_print("MOTOR DC -> DETENIDO\r\n");
}


// =====================================================
// TEMPORIZACIÓN DE LA LECTURA I2C
// =====================================================
// Contador basado en un pequeño tick
uint16_t contador_i2c = 0;


// =====================================================
// MAIN
// =====================================================
int main(void)
{
    // Inicializar UART
    uart_init(9600);

    // Inicializar I2C
    twi_init();

	// Motor
	DDRC |= (1 << MOTOR_IN1) |
	(1 << MOTOR_IN2);

	motor_stop();
	
	uint8_t motor_posicion = 0;
	int16_t temperatura_x2;
	int16_t temperatura;
	
    // -------------------------------------------------
    // Mensaje inicial
    // -------------------------------------------------
    uart_print("\r\n");
    uart_print("================================\r\n");
    uart_print("       NANO MAESTRO\r\n");
    uart_print("================================\r\n");
    uart_print("I2C MASTER iniciado\r\n");
    uart_print("S1 = 0x08\r\n");
	uart_print("S2 = 0x09\r\n");
    uart_print("SCL = 100 kHz\r\n");
    uart_print("================================\r\n\r\n");

    uart_print("\r\n");
    uart_print("==============================\r\n");
    uart_print("MAESTRO LM75 + MOTOR DC\r\n");
    uart_print("==============================\r\n");
    uart_print("LM75 = 0x48\r\n");
    uart_print("SDA = A4\r\n");
    uart_print("SCL = A5\r\n");
    uart_print("Motor IN1 = PC1\r\n");
    uart_print("Motor IN2 = PC2\r\n");
    uart_print("Umbral = ");
    uart_print_num(TEMP_UMBRAL);
    uart_print(" C\r\n");
    uart_print("==============================\r\n");

    // =================================================
    // BUCLE PRINCIPAL
    // =================================================

    while (1)
    {
        
        // =================================================
        // LEER ESCLAVO 1
        // =================================================
        // Lectura cada 500ms

        if (contador_i2c >= 500)
        {
            contador_i2c = 0;

			// Preguntar a esclavo distancia, estado stepper
            if (leer_esclavo1(&distancia, &stepper_activo))
            {
                uart_print("S1 -> Distancia: ");
                uart_print_num(distancia);
                uart_print(" cm");

                uart_print(" | Stepper: ");
                if (stepper_activo){
                    uart_print("ON");
                } else{
                    uart_print("OFF");
                }

				uart_print("\r\n");
                uart_print("\r\n");
            } else{
                uart_print("ERROR: no se pudo leer S1\r\n");
            }
			
        // =================================================
        // LEER ESCLAVO 2
        // =================================================
        // Lectura cada 500ms
        
        if (leer_esclavo2(&lluvia, &servo_posicion))
        {
	        uart_print("S2 -> Lluvia: ");

	        if (lluvia)
	        {
		        uart_print("SI");
	        }
	        else
	        {
		        uart_print("NO");
	        }


	        uart_print(" | Servo: ");

	        uart_print_num(servo_posicion);

	        uart_print(" grados");

	        uart_print("\r\n");
        }
        else
        {
	        uart_print("ERROR: no se pudo leer S2\r\n");
        }
        
		// =================================================
		// LÓGICA SENSOR TEMPERATURA + MOTOR DC
		// =================================================
		// Lectura cada 500ms
        temperatura_x2 = lm75_read_temperature_x2();
        
		if (temperatura_x2 == -999)
        {
            uart_print("ERROR: no se pudo leer LM75\r\n");

            motor_stop();

            _delay_ms(1000);

            continue;
        }

        /*
         * temperatura_x2 esta multiplicada por 2.
         *
         * Ejemplo:
         *
         * 25.0 C -> 50
         * 25.5 C -> 51
         */

        temperatura = temperatura_x2 / 2;

        // ----------------------------------------------------
        // Mostrar temperatura
        // ----------------------------------------------------

        uart_print("Temperatura: ");

        uart_print_num(temperatura_x2 / 2);

        if (temperatura_x2 % 2)
        {
            uart_print(".5");
        }
        else
        {
            uart_print(".0");
        }

        uart_print(" C | ");

        // ----------------------------------------------------
        // LOGICA DEL MOTOR
        // ----------------------------------------------------

        if (temperatura > TEMP_UMBRAL)
        {
            uart_print("TEMP ALTA");

            /*
             * Solo avanzar si actualmente estamos
             * en posicion de reposo.
             */

            if (motor_posicion == 0)
            {
	            uart_print(" -> se requiere AVANCE\r\n");

	            motor_hacer_avance();

	            motor_posicion = 1;
            }
            else
            {
                uart_print(" -> motor ya esta avanzado\r\n");
            }
        }
        else if (temperatura < TEMP_UMBRAL)
        {
            uart_print("TEMP BAJA");

            /*
             * Solo retroceder si actualmente estamos
             * en posicion avanzada.
             */

            if (motor_posicion == 1)
            {
                uart_print(" -> se requiere RETROCESO\r\n");

                motor_hacer_retroceso();

                motor_posicion = 0;
            }
            else
            {
                uart_print(" -> motor ya esta en reposo\r\n");
            }
        }
        else
        {
            uart_print("TEMP IGUAL AL UMBRAL\r\n");
        }

        _delay_ms(500);
    
        
        
        
        }
		
		// =================================================
		// LÓGICA SENSOR TEMPERATURA + MOTOR DC
		// =================================================
		// Lectura cada 500ms
		




        // =================================================
        // PANTALLA LCD (Servos + Motores)
        // =================================================
		
		
		
		
		
		
		
		
		





		//uart_print("\r\n");
        _delay_ms(1);

        contador_i2c++;
    }
}