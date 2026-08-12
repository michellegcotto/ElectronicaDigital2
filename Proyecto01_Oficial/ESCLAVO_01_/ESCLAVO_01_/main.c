#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>


// =====================================================
// CONFIGURACIÓN I2C
// =====================================================
#define SLAVE_ADDR 0x08


// =====================================================
// SENSOR PROXIMIDAD
// =====================================================
#define TRIG_PIN       PC0
#define ECHO_PIN       PC1

#define TRIG_DDR       DDRC
#define ECHO_DDR       DDRC

#define TRIG_PORT      PORTC
#define ECHO_PORT      PORTC
#define ECHO_PINREG    PINC


// =====================================================
// STEPPER
// =====================================================
#define STEP_IN1       PD5
#define STEP_IN2       PD4
#define STEP_IN3       PD3
#define STEP_IN4       PD2

#define STEP_DDR       DDRD
#define STEP_PORT      PORTD


// =====================================================
// PARÁMETROS 
// =====================================================
#define DISTANCIA_UMBRAL_CM    10

// tiempo funcionamiento stepper
#define STEPPER_TIEMPO_MS      30000UL
#define STEP_DELAY_MS          10UL
#define ULTRASONIC_INTERVAL_MS 60UL
#define ULTRASONIC_TIMEOUT_MS  30UL


// =====================================================
// UART
// =====================================================
void uart_init(unsigned int baud)
{
    unsigned int ubrr;

    ubrr = F_CPU / 16 / baud - 1;

    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << TXEN0) |
             (1 << RXEN0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
}


// -----------------------------------------------------
// Enviar carácter
// -----------------------------------------------------
void uart_putc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}


// -----------------------------------------------------
// Enviar texto
// -----------------------------------------------------
void uart_print(const char *s)
{
    while (*s)
    {
        uart_putc(*s++);
    }
}


// -----------------------------------------------------
// Enviar número
// -----------------------------------------------------
void uart_print_num(uint16_t num)
{
    char buffer[10];
    itoa(num, buffer, 10);
    uart_print(buffer);
}


// =====================================================
// TIMER0
// =====================================================
volatile uint32_t tiempo_ms = 0;


void timer0_init(void)
{
	// ctc
    TCCR0A = (1 << WGM01);
	// Prescaler 64
    TCCR0B = (1 << CS01) |
             (1 << CS00);
	// 250 vueltas = 1ms
    OCR0A = 249;
	// interrupcion
    TIMSK0 = (1 << OCIE0A);
}


// -----------------------------------------------------
// Interrupción Timer0
// -----------------------------------------------------
ISR(TIMER0_COMPA_vect)
{
    tiempo_ms++;
}


// =====================================================
// TIMER1
// MEDICIÓN DEL ECHO
// =====================================================
void timer1_init(void)
{
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCCR1B |= (1 << CS11);
}


// =====================================================
// ESTADOS DEL SENSOR
// =====================================================
#define US_IDLE       0
#define US_WAIT_HIGH  1
#define US_MEASURE    2

uint8_t ultrasonic_estado = US_IDLE;
uint32_t ultrasonic_inicio_ms = 0;
// ultima medicion
uint32_t ultrasonic_ultima_medicion_ms = 0;
uint16_t echo_inicio_timer = 0;
volatile uint16_t distancia_cm = 0;


// =====================================================
// INICIALIZAR ULTRASONIDO
// =====================================================
void ultrasonic_init(void)
{
	// TRIG salida
    TRIG_DDR |= (1 << TRIG_PIN);

	// ECHO entrada
    ECHO_DDR &= ~(1 << ECHO_PIN);

    // TRIG low inicialmente
    TRIG_PORT &= ~(1 << TRIG_PIN);
}


// =====================================================
// INICIAR UNA MEDICIÓN
// =====================================================
void ultrasonic_start(void)
{
    TRIG_PORT &= ~(1 << TRIG_PIN);
    _delay_us(2);
    TRIG_PORT |= (1 << TRIG_PIN);
    _delay_us(10);
    TRIG_PORT &= ~(1 << TRIG_PIN);

	// esoerar ECHO high
    ultrasonic_estado = US_WAIT_HIGH;
    ultrasonic_inicio_ms = tiempo_ms;
}


// =====================================================
// ACTUALIZAR ULTRASONIDO
// =====================================================
void ultrasonic_update(void)
{
    uint16_t echo_fin;
    uint16_t duracion;

    switch (ultrasonic_estado)
    {

        // =================================================
        // ESTADO IDLE
        // =================================================
        case US_IDLE:

            if ((tiempo_ms - ultrasonic_ultima_medicion_ms)
                >= ULTRASONIC_INTERVAL_MS)
            {
                ultrasonic_ultima_medicion_ms =
                    tiempo_ms;

                ultrasonic_start();
            }
            break;


        // =================================================
        // ESPERANDO ECHO HIGH
        // =================================================
        case US_WAIT_HIGH:

			// ECHO = high ?
            if (ECHO_PINREG & (1 << ECHO_PIN))
            {

                echo_inicio_timer = TCNT1;

				// esperar ECHO low
                ultrasonic_estado = US_MEASURE;
            }else if ((tiempo_ms - ultrasonic_inicio_ms)
                     >= ULTRASONIC_TIMEOUT_MS)
            {
                distancia_cm = 0;

                ultrasonic_estado = US_IDLE;
            }
            break;


        // =================================================
        // MIDIENDO ECHO
        // =================================================

        case US_MEASURE:
            if (!(ECHO_PINREG & (1 << ECHO_PIN)))
            {
                echo_fin = TCNT1;
                duracion = echo_fin - echo_inicio_timer;
                distancia_cm = duracion / 116;

                ultrasonic_estado = US_IDLE;
            }else if ((tiempo_ms - ultrasonic_inicio_ms)
                     >= ULTRASONIC_TIMEOUT_MS)
            {
                distancia_cm = 0;
                ultrasonic_estado = US_IDLE;
            }
            break;
    }
}


// =====================================================
// STEPPER
// =====================================================
#define STEPPER_IDLE       0
#define STEPPER_MOVIENDO   1

volatile uint8_t stepper_estado = STEPPER_IDLE;
uint8_t stepper_paso = 0;
uint32_t stepper_inicio_ms = 0;
uint32_t stepper_ultimo_paso_ms = 0;


// =====================================================
// INICIALIZAR STEPPER
// =====================================================
void stepper_init(void)
{
    STEP_DDR |= (1 << STEP_IN1) | (1 << STEP_IN2) | (1 << STEP_IN3) | (1 << STEP_IN4);
    STEP_PORT &= ~( (1 << STEP_IN1) | (1 << STEP_IN2) | (1 << STEP_IN3) | (1 << STEP_IN4)
    );
}


// =====================================================
// APAGAR STEPPER
// =====================================================
void stepper_off(void)
{
    STEP_PORT &= ~( (1 << STEP_IN1) | (1 << STEP_IN2) | (1 << STEP_IN3) | (1 << STEP_IN4)
    );
}


// =====================================================
// HACER UN PASO
// =====================================================

void stepper_step(void)
{
    STEP_PORT &= ~( (1 << STEP_IN1) | (1 << STEP_IN2) | (1 << STEP_IN3) | (1 << STEP_IN4));

    switch (stepper_paso)
    {
        case 0:
            STEP_PORT |= (1 << STEP_IN4);
            break;
        case 1:
            STEP_PORT |= (1 << STEP_IN3);
            break;
        case 2:
            STEP_PORT |= (1 << STEP_IN2);
            break;
        case 3:
            STEP_PORT |= (1 << STEP_IN1);
            break;
    }

    stepper_paso++;
    if (stepper_paso >= 4)
    {
        stepper_paso = 0;
    }
}


// =====================================================
// INICIAR STEPPER
// =====================================================
void stepper_start(void)
{
    stepper_estado = STEPPER_MOVIENDO;
    stepper_inicio_ms = tiempo_ms;
    stepper_ultimo_paso_ms = tiempo_ms;
    stepper_paso = 0;
    uart_print("STEPPER -> INICIO\r\n");
}


// =====================================================
// ACTUALIZAR STEPPER
// =====================================================
void stepper_update(void)
{
    if (stepper_estado == STEPPER_IDLE)
    {
        return;
    }

    if ((tiempo_ms - stepper_inicio_ms)
        >= STEPPER_TIEMPO_MS)
    { 
        stepper_off();
        stepper_estado = STEPPER_IDLE;
        uart_print("STEPPER -> FIN 6s\r\n");
        return;
    }

    if ((tiempo_ms - stepper_ultimo_paso_ms)
        >= STEP_DELAY_MS)
    {
        stepper_ultimo_paso_ms = tiempo_ms;
        stepper_step();
    }
}


// =====================================================
// CONTROL DEL STEPPER SEGÚN DISTANCIA
// =====================================================
void controlar_stepper(void)
{
    if ((distancia_cm > 0) &&
        (distancia_cm <= DISTANCIA_UMBRAL_CM))
    {
        if (stepper_estado == STEPPER_IDLE)
        {
            uart_print("UMBRAL <= 10cm\r\n");
            uart_print("Distancia = ");
            uart_print_num(distancia_cm);
            uart_print(" cm\r\n");
            stepper_start();
        }
    }
}


// =====================================================
// TWI / I2C ESCLAVO
// =====================================================
void twi_slave_init(uint8_t direccion)
{
    TWAR = (direccion << 1);
    TWCR = (1 << TWEN) | (1 << TWEA) |(1 << TWINT);
}


// =====================================================
// VARIABLES PARA TRANSMISIÓN I2C
// =====================================================
uint8_t byte_tx = 0;


// =====================================================
// PROCESAR I2C SIN BLOQUEAR
// =====================================================
void twi_update(void)
{
    if (!(TWCR & (1 << TWINT)))
    {
        return;
    }
    uint8_t estado = TWSR & 0xF8;
	
    switch (estado)
    {

        // =================================================
        // SLA+W RECIBIDO
        // =================================================
        case 0x60:
            TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
            break;

        // =================================================
        // DATO RECIBIDO
        // =================================================
        case 0x80:
            TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
            break;

        // =================================================
        // SLA+R RECIBIDO
        // =================================================
        case 0xA8:
            byte_tx = 0;
            TWDR =
                (uint8_t)(distancia_cm & 0xFF);
            TWCR = (1 << TWINT) | (1 << TWEN) |(1 << TWEA);
            break;

        // =================================================
        // BYTE TRANSMITIDO + MASTER ACK
        // =================================================
        case 0xB8:
            byte_tx++;
            if (byte_tx == 1)
            {
                TWDR =
                    (uint8_t)(distancia_cm >> 8);
            }


            else if (byte_tx == 2)
            {
                TWDR = stepper_estado;
            }

            TWCR = (1 << TWINT) |
                   (1 << TWEN) |
                   (1 << TWEA);
            break;

        // =================================================
        // BYTE TRANSMITIDO + NACK
        // =================================================
        case 0xC0:
            TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
            break;

        // =================================================
        // STOP / REPEATED START
        // =================================================
        case 0xA0:
            TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
            break;

        // =================================================
        // ERROR / ESTADO NO ESPERADO
        // =================================================
        default:
            uart_print("I2C estado = 0x");
            char buffer[5];
            itoa(estado, buffer, 16);
            uart_print(buffer);
            uart_print("\r\n");

            TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
            break;
    }
}


// =====================================================
// MAIN
// =====================================================

int main(void)
{
    // =================================================
    // INICIALIZACIONES
    // =================================================
    uart_init(9600);
    ultrasonic_init();
    stepper_init();
    timer0_init();
    timer1_init();
    twi_slave_init(SLAVE_ADDR);
    sei();


    // =================================================
    // MENSAJE INICIAL
    // =================================================
    uart_print("\r\n");
    uart_print("================================\r\n");
    uart_print("          ESCLAVO 1\r\n");
    uart_print("================================\r\n");
    uart_print("I2C = 0x08\r\n");
    uart_print("Ultrasonido = A0/A1\r\n");
    uart_print("Stepper = D2/D3/D4/D5\r\n");
    uart_print("Umbral = 10 cm\r\n");
    uart_print("Tiempo stepper = 6 segundos\r\n");
    uart_print("Velocidad = 10 ms/fase\r\n");
    uart_print("Sentido = IN4 -> IN3 -> IN2 -> IN1\r\n");
    uart_print("Sistema NO BLOQUEANTE\r\n");
    uart_print("================================\r\n\r\n");

    while (1)
    {
		// actualizar sensor
        ultrasonic_update();
		// control stepper
        controlar_stepper();
		// actualizar stepper
        stepper_update();
        twi_update();
    }
}