#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>

// ============================================================
// CONFIGURACIÓN I2C
// ============================================================
#define S1_ADDR 0x08
#define S2_ADDR 0x09

// ============================================================
// VARIABLES DEL SISTEMA
// ============================================================

// ------------------------------------------------------------
// Esclavo 1
// ------------------------------------------------------------
uint16_t distancia = 0;
uint8_t stepper_activo = 0;

// ------------------------------------------------------------
// Esclavo 2
// ------------------------------------------------------------
uint8_t lluvia = 0;
uint8_t servo_posicion = 0;

// ------------------------------------------------------------
// Estados de comunicación
// ------------------------------------------------------------
uint8_t s1_ok = 0;
uint8_t s2_ok = 0;
uint8_t lm75_ok = 0;

// ============================================================
// LM75
// ============================================================
#define LM75_ADDR       0x48
#define LM75_TEMP_REG   0x00

#define TEMP_UMBRAL     30

#define TIEMPO_MOTOR_MS 4000

// ============================================================
// MOTOR DC
// ============================================================
#define MOTOR_IN1       PC1
#define MOTOR_IN2       PC2

// ============================================================
// LCD 16x2 - 8 BITS
// ============================================================
#define SET_BIT(reg, bit)       ((reg) |= (1 << (bit)))
#define CLR_BIT(reg, bit)       ((reg) &= ~(1 << (bit)))
#define WRITE_BIT(reg, bit, val) ((val) ? SET_BIT(reg, bit) : CLR_BIT(reg, bit))

// ------------------------------------------------------------
// RS
// ------------------------------------------------------------
#define RS_PORT PORTD
#define RS_DDR  DDRD
#define RS_BIT  PD2

// ------------------------------------------------------------
// Enable
// ------------------------------------------------------------
#define E_PORT PORTD
#define E_DDR  DDRD
#define E_BIT  PD3

// ------------------------------------------------------------
// D0
// ------------------------------------------------------------
#define D0_PORT PORTD
#define D0_DDR  DDRD
#define D0_BIT  PD4

// ------------------------------------------------------------
// D1
// ------------------------------------------------------------
#define D1_PORT PORTD
#define D1_DDR  DDRD
#define D1_BIT  PD5

// ------------------------------------------------------------
// D2
// ------------------------------------------------------------
#define D2_PORT PORTD
#define D2_DDR  DDRD
#define D2_BIT  PD6

// ------------------------------------------------------------
// D3
// ------------------------------------------------------------
#define D3_PORT PORTD
#define D3_DDR  DDRD
#define D3_BIT  PD7

// ------------------------------------------------------------
// D4
// ------------------------------------------------------------
#define D4_PORT PORTB
#define D4_DDR  DDRB
#define D4_BIT  PB0

// ------------------------------------------------------------
// D5
// ------------------------------------------------------------
#define D5_PORT PORTB
#define D5_DDR  DDRB
#define D5_BIT  PB1

// ------------------------------------------------------------
// D6
// ------------------------------------------------------------
#define D6_PORT PORTB
#define D6_DDR  DDRB
#define D6_BIT  PB2

// ------------------------------------------------------------
// D7
// ------------------------------------------------------------
#define D7_PORT PORTB
#define D7_DDR  DDRB
#define D7_BIT  PB3

// ============================================================
// LCD - PULSO ENABLE
// ============================================================
static void lcd_pulse_enable(void)
{
	SET_BIT(E_PORT, E_BIT);
	_delay_us(1);
	CLR_BIT(E_PORT, E_BIT);
	_delay_us(100);
}

// ============================================================
// LCD - ESCRIBIR BYTE
// ============================================================
static void lcd_write_byte(uint8_t byte, uint8_t rs)
{
	WRITE_BIT(RS_PORT, RS_BIT, rs);

	WRITE_BIT(D0_PORT, D0_BIT, (byte >> 0) & 0x01);
	WRITE_BIT(D1_PORT, D1_BIT, (byte >> 1) & 0x01);
	WRITE_BIT(D2_PORT, D2_BIT, (byte >> 2) & 0x01);
	WRITE_BIT(D3_PORT, D3_BIT, (byte >> 3) & 0x01);

	WRITE_BIT(D4_PORT, D4_BIT, (byte >> 4) & 0x01);
	WRITE_BIT(D5_PORT, D5_BIT, (byte >> 5) & 0x01);
	WRITE_BIT(D6_PORT, D6_BIT, (byte >> 6) & 0x01);
	WRITE_BIT(D7_PORT, D7_BIT, (byte >> 7) & 0x01);

	lcd_pulse_enable();
}

// ============================================================
// LCD - COMANDO
// ============================================================
void lcd_command(uint8_t cmd)
{
	lcd_write_byte(cmd, 0);
	if (cmd == 0x01 || cmd == 0x02)
	{
		_delay_ms(2);
	}
	else
	{
		_delay_us(50);
	}
}

// ============================================================
// LCD - DATO
// ============================================================
void lcd_data(uint8_t data)
{
	lcd_write_byte(data, 1);
	_delay_us(50);
}

// ============================================================
// LCD - CLEAR
// ============================================================
void lcd_clear(void)
{
	lcd_command(0x01);
}

// ============================================================
// LCD - POSICIÓN
// ============================================================
void lcd_gotoxy(uint8_t col, uint8_t row)
{
	uint8_t address;
	if (row == 0)
	{
		address = 0x00 + col;
	}
	else
	{
		address = 0x40 + col;
	}
	lcd_command(0x80 | address);
}

// ============================================================
// LCD - IMPRIMIR TEXTO
// ============================================================
void lcd_print(const char *str)
{
	while (*str)
	{
		lcd_data((uint8_t)(*str));
		str++;
	}
}

// ============================================================
// LCD - INICIALIZACIÓN
// ============================================================
void lcd_init(void)
{
	// RS
	SET_BIT(RS_DDR, RS_BIT);

	// E
	SET_BIT(E_DDR, E_BIT);

	// D0-D3
	SET_BIT(D0_DDR, D0_BIT);
	SET_BIT(D1_DDR, D1_BIT);
	SET_BIT(D2_DDR, D2_BIT);
	SET_BIT(D3_DDR, D3_BIT);

	// D4-D7
	SET_BIT(D4_DDR, D4_BIT);
	SET_BIT(D5_DDR, D5_BIT);
	SET_BIT(D6_DDR, D6_BIT);
	SET_BIT(D7_DDR, D7_BIT);

	CLR_BIT(E_PORT, E_BIT);
	_delay_ms(20);

	// Secuencia inicialización 8 bits
	lcd_write_byte(0x30, 0);
	_delay_ms(5);
	lcd_write_byte(0x30, 0);
	_delay_us(150);
	lcd_write_byte(0x30, 0);
	_delay_us(150);

	// 8 bits, 2 líneas, fuente 5x7
	lcd_command(0x38);
	// Display ON, cursor OFF, blink OFF
	lcd_command(0x0C);
	// Clear
	lcd_command(0x01);
	// Entry mode
	lcd_command(0x06);
}

// ============================================================
// LCD - IMPRIMIR ESPACIOS
// ============================================================
void lcd_espacios(uint8_t cantidad)
{
	uint8_t i;
	for (i = 0; i < cantidad; i++)
	{
		lcd_data(' ');
	}
}

// ============================================================
// UART
// ============================================================
void uart_init(unsigned int baud)
{
	unsigned int ubrr = F_CPU / 16 / baud - 1;

	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;

	UCSR0B = (1 << TXEN0) |
	(1 << RXEN0);

	UCSR0C = (1 << UCSZ01) |
	(1 << UCSZ00);
}

// ============================================================
// UART - ENVIAR CARÁCTER
// ============================================================
void uart_putc(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

// ============================================================
// UART - ENVIAR TEXTO
// ============================================================
void uart_print(const char *s)
{
	while (*s)
	{
		uart_putc(*s++);
	}
}

// ============================================================
// UART - ENVIAR NÚMERO
// ============================================================
void uart_print_num(uint16_t num)
{
	char buffer[10];
	itoa(num, buffer, 10);
	uart_print(buffer);
}

// ============================================================
// TWI / I2C MASTER
// ============================================================
void twi_init(void)
{
	TWSR = 0x00;
	TWBR = 72;
	TWCR = (1 << TWEN);
}

// ============================================================
// START
// ============================================================
uint8_t twi_start(void)
{
	TWCR = (1 << TWINT) |(1 << TWSTA) |(1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return (TWSR & 0xF8);
}

// ============================================================
// STOP
// ============================================================
void twi_stop(void)
{
	TWCR = (1 << TWINT) |(1 << TWSTO) |(1 << TWEN);
	_delay_us(100);
}

// ============================================================
// STATUS
// ============================================================
uint8_t twi_status(void)
{
	return TWSR & 0xF8;
}

// ============================================================
// WRITE
// ============================================================
uint8_t twi_write(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) |(1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return (TWSR & 0xF8);
}

// ============================================================
// READ + ACK
// ============================================================
uint8_t twi_read_ack(void)
{
	TWCR = (1 << TWINT) |(1 << TWEN) |(1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// ============================================================
// READ + NACK
// ============================================================
uint8_t twi_read_nack(void)
{
	TWCR = (1 << TWINT) |(1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// ============================================================
// LEER ESCLAVO 1
// ============================================================
uint8_t leer_esclavo1(uint16_t *distancia, uint8_t *stepper)
{
	uint8_t estado;
	uint8_t distancia_low;
	uint8_t distancia_high;
	uint8_t estado_stepper;

	// --------------------------------------------------------
	// START
	// --------------------------------------------------------
	estado = twi_start();
	if (estado != 0x08)
	{
		return 0;
	}

	// --------------------------------------------------------
	// SLA + READ
	// --------------------------------------------------------
	estado = twi_write((S1_ADDR << 1) | 1);
	if (estado != 0x40)
	{
		twi_stop();
		return 0;
	}

	// --------------------------------------------------------
	// DISTANCIA LOW
	// --------------------------------------------------------
	distancia_low = twi_read_ack();

	// --------------------------------------------------------
	// DISTANCIA HIGH
	// --------------------------------------------------------
	distancia_high = twi_read_ack();

	// --------------------------------------------------------
	// STEPPER
	// --------------------------------------------------------
	estado_stepper = twi_read_nack();

	// --------------------------------------------------------
	// RECONSTRUIR DISTANCIA
	// --------------------------------------------------------
	*distancia = ((uint16_t)distancia_high << 8) |distancia_low;
	*stepper = estado_stepper;

	// --------------------------------------------------------
	// STOP
	// --------------------------------------------------------
	twi_stop();
	return 1;
}

// ============================================================
// LEER ESCLAVO 2
// ============================================================
uint8_t leer_esclavo2(uint8_t *lluvia_estado,
uint8_t *servo_estado)
{
	uint8_t estado;

	uint8_t lluvia_recibida;
	uint8_t servo_recibido;

	// --------------------------------------------------------
	// START
	// --------------------------------------------------------
	estado = twi_start();
	if (estado != 0x08)
	{
		return 0;
	}

	// --------------------------------------------------------
	// SLA + READ
	// --------------------------------------------------------
	estado = twi_write((S2_ADDR << 1) | 1);
	if (estado != 0x40)
	{
		twi_stop();
		return 0;
	}

	// --------------------------------------------------------
	// LLUVIA
	// --------------------------------------------------------
	lluvia_recibida = twi_read_ack();

	// --------------------------------------------------------
	// SERVO
	// --------------------------------------------------------
	servo_recibido = twi_read_nack();

	// --------------------------------------------------------
	// GUARDAR
	// --------------------------------------------------------
	*lluvia_estado = lluvia_recibida;
	*servo_estado = servo_recibido;

	// --------------------------------------------------------
	// STOP
	// --------------------------------------------------------
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
	// REGISTRO TEMPERATURA
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
	// -------------------------------------------------------
	twi_write((LM75_ADDR << 1) | 1);
	if (twi_status() != 0x40)
	{
		twi_stop();
		uart_print("ERROR LM75: SLA+R\r\n");
		return -999;
	}

	// --------------------------------------------------------
	// LEER DOS BYTES
	// --------------------------------------------------------
	msb = twi_read_ack();
	lsb = twi_read_nack();
	twi_stop();

	// --------------------------------------------------------
	// CONVERTIR TEMPERATURA
	// --------------------------------------------------------
	int8_t temperatura_entera = (int8_t)msb;
	int16_t temperatura_x2 = ((int16_t)temperatura_entera * 2);
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

// ============================================================
// MOTOR AVANZAR
// ============================================================
void motor_avanzar(void)
{
	PORTC |=  (1 << MOTOR_IN1);
	PORTC &= ~(1 << MOTOR_IN2);
}

// ============================================================
// MOTOR RETROCEDER
// ============================================================
void motor_retroceder(void)
{
	PORTC &= ~(1 << MOTOR_IN1);
	PORTC |=  (1 << MOTOR_IN2);
}

// ============================================================
// MOTOR HACER AVANCE
// ============================================================
void motor_hacer_avance(void)
{
	uart_print("MOTOR DC -> AVANZANDO 4 segundos\r\n");
	motor_avanzar();
	_delay_ms(TIEMPO_MOTOR_MS);
	motor_stop();
	uart_print("MOTOR DC -> DETENIDO\r\n");
}

// ============================================================
// MOTOR HACER RETROCESO
// ============================================================
void motor_hacer_retroceso(void)
{
	uart_print("MOTOR DC -> RETROCEDIENDO 4 segundos\r\n");
	motor_retroceder();
	_delay_ms(TIEMPO_MOTOR_MS);
	motor_stop();
	uart_print("MOTOR DC -> DETENIDO\r\n");
}

// ============================================================
// LCD - MOSTRAR TEMPERATURA + MOTOR
// ============================================================
void lcd_pantalla_temperatura(int16_t temperatura_x2, uint8_t temperatura_ok, uint8_t motor_posicion)
{
	char buffer[17];

	// --------------------------------------------------------
	// FILA 0
	// --------------------------------------------------------
	lcd_gotoxy(0, 0);

	if (temperatura_ok)
	{
		// Temperatura positiva
		if (temperatura_x2 >= 0)
		{
			uint8_t entero = temperatura_x2 / 2;
			if (temperatura_x2 % 2)
			{
				sprintf(buffer, "T:%u.5C", entero);
			}
			else
			{
				sprintf(buffer, "T:%u.0C", entero);
			}
		}
		else
		{
			int16_t temp_abs = -temperatura_x2;
			uint8_t entero = temp_abs / 2;
			if (temp_abs % 2)
			{
				sprintf(buffer, "T:-%u.5C", entero);
			}
			else
			{
				sprintf(buffer, "T:-%u.0C", entero);
			}
		}
		lcd_print(buffer);
	}
	else
	{
		lcd_print("T:XX.XC");
	}

	// --------------------------------------------------------
	// LIMPIAR PARTE RESTANTE
	// --------------------------------------------------------
	lcd_gotoxy(8, 0);
	lcd_print("M:");
	if (motor_posicion == 0)
	{
		lcd_print("OFF");
	}
	else
	{
		lcd_print("AV ");
	}
	lcd_espacios(5);

	// --------------------------------------------------------
	// FILA 1
	// --------------------------------------------------------
	lcd_gotoxy(0, 1);

	if (temperatura_ok)
	{
		if (temperatura_x2 > (TEMP_UMBRAL * 2))
		{
			lcd_print("TEMP ALTA");
		}
		else if (temperatura_x2 < (TEMP_UMBRAL * 2))
		{
			lcd_print("TEMP BAJA");
		}
		else
		{
			lcd_print("TEMP OK");
		}
	}
	else
	{
		lcd_print("TEMP: X");
	}
	lcd_espacios(7);
}

// ============================================================
// LCD - MOSTRAR ESCLAVO 1
// ============================================================

void lcd_pantalla_esclavo1(void)
{
	char buffer[17];

	// --------------------------------------------------------
	// FILA 0
	// --------------------------------------------------------
	lcd_gotoxy(0, 0);
	if (s1_ok)
	{
		sprintf(buffer, "D:%ucm", distancia);

		lcd_print(buffer);
	}
	else
	{
		lcd_print("D:XXXXcm");
	}
	lcd_espacios(8);

	// --------------------------------------------------------
	// FILA 1
	// --------------------------------------------------------
	lcd_gotoxy(0, 1);
	if (s1_ok)
	{
		lcd_print("ST:");

		if (stepper_activo)
		{
			lcd_print("ON");
		}
		else
		{
			lcd_print("OFF");
		}
	}
	else
	{
		lcd_print("ST:X");
	}
	lcd_espacios(12);
}

// ============================================================
// LCD - MOSTRAR ESCLAVO 2
// ============================================================
void lcd_pantalla_esclavo2(void)
{
	char buffer[17];

	// --------------------------------------------------------
	// FILA 0
	// --------------------------------------------------------
	lcd_gotoxy(0, 0);

	if (s2_ok)
	{
		lcd_print("LL:");

		if (lluvia)
		{
			lcd_print("SI");
		}
		else
		{
			lcd_print("NO");
		}
	}
	else
	{
		lcd_print("LL:X");
	}
	lcd_espacios(12);

	// --------------------------------------------------------
	// FILA 1
	// --------------------------------------------------------
	lcd_gotoxy(0, 1);
	if (s2_ok)
	{
		sprintf(buffer, "SV:%03u", servo_posicion);

		lcd_print(buffer);

		lcd_print(" grados");
	}
	else
	{
		lcd_print("SV:X");
	}
	lcd_espacios(10);
}

// ============================================================
// LCD - ACTUALIZAR PANTALLA
// ============================================================
uint8_t lcd_pantalla_actual = 0;
void lcd_actualizar(int16_t temperatura_x2,
uint8_t temperatura_ok,
uint8_t motor_posicion)
{
	if (lcd_pantalla_actual == 0)
	{
		lcd_pantalla_temperatura(
		temperatura_x2,
		temperatura_ok,
		motor_posicion
		);
	}
	else if (lcd_pantalla_actual == 1)
	{
		lcd_pantalla_esclavo1();
	}
	else
	{
		lcd_pantalla_esclavo2();
	}

	// --------------------------------------------------------
	// Cambiar a la siguiente pantalla
	// --------------------------------------------------------
	lcd_pantalla_actual++;
	if (lcd_pantalla_actual >= 3)
	{
		lcd_pantalla_actual = 0;
	}
}

// ============================================================
// TEMPORIZACIÓN DE LA LECTURA I2C
// ============================================================
uint16_t contador_i2c = 0;

int main(void)
{
	// --------------------------------------------------------
	// VARIABLES
	// --------------------------------------------------------
	uint8_t motor_posicion = 0;

	int16_t temperatura_x2;
	int16_t temperatura;

	// --------------------------------------------------------
	// INICIALIZAR UART
	// --------------------------------------------------------
	uart_init(9600);

	// --------------------------------------------------------
	// INICIALIZAR I2C
	// --------------------------------------------------------
	twi_init();

	// --------------------------------------------------------
	// INICIALIZAR LCD
	// --------------------------------------------------------
	lcd_init();
	lcd_clear();

	// --------------------------------------------------------
	// MOTOR
	// --------------------------------------------------------
	DDRC |= (1 << MOTOR_IN1) |
	(1 << MOTOR_IN2);
	motor_stop();

	// --------------------------------------------------------
	// MENSAJE UART
	// --------------------------------------------------------

	uart_print("\r\n");
	uart_print("================================\r\n");
	uart_print("       NANO MAESTRO\r\n");
	uart_print("================================\r\n");

	uart_print("I2C MASTER iniciado\r\n");
	uart_print("S1 = 0x08\r\n");
	uart_print("S2 = 0x09\r\n");
	uart_print("SCL = 100 kHz\r\n");

	uart_print("================================\r\n\r\n");

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
	
	while (1)
	{
		// ====================================================
		// LECTURA CADA 500 ms
		// ====================================================
		if (contador_i2c >= 500)
		{
			contador_i2c = 0;

			// =================================================
			// LEER ESCLAVO 1
			// =================================================

			if (leer_esclavo1(
			&distancia,
			&stepper_activo))
			{
				s1_ok = 1;

				uart_print("S1 -> Distancia: ");

				uart_print_num(distancia);

				uart_print(" cm");

				uart_print(" | Stepper: ");

				if (stepper_activo)
				{
					uart_print("ON");
				}
				else
				{
					uart_print("OFF");
				}

				uart_print("\r\n");
			}
			else
			{
				s1_ok = 0;

				uart_print(
				"ERROR: no se pudo leer S1\r\n"
				);
			}

			// =================================================
			// LEER ESCLAVO 2
			// =================================================

			if (leer_esclavo2(
			&lluvia,
			&servo_posicion))
			{
				s2_ok = 1;

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

				uart_print(" grados\r\n");
			}
			else
			{
				s2_ok = 0;

				uart_print(
				"ERROR: no se pudo leer S2\r\n"
				);
			}

			// =================================================
			// LM75
			// =================================================

			temperatura_x2 =
			lm75_read_temperature_x2();

			if (temperatura_x2 == -999)
			{
				lm75_ok = 0;

				uart_print(
				"ERROR: no se pudo leer LM75\r\n"
				);

				motor_stop();

				lcd_pantalla_temperatura(
				temperatura_x2,
				0,
				motor_posicion
				);

				_delay_ms(1000);
				continue;
			}

			lm75_ok = 1;
			
			// =================================================
			// CONVERTIR TEMPERATURA
			// =================================================
			temperatura = temperatura_x2 / 2;

			// =================================================
			// UART - TEMPERATURA
			// =================================================
			uart_print("Temperatura: ");

			if (temperatura_x2 < 0)
			{
				int16_t temp_abs = -temperatura_x2;

				uart_putc('-');

				uart_print_num(temp_abs / 2);

				if (temp_abs % 2)
				{
					uart_print(".5");
				}
				else
				{
					uart_print(".0");
				}
			}
			else
			{
				uart_print_num(
				temperatura_x2 / 2
				);

				if (temperatura_x2 % 2)
				{
					uart_print(".5");
				}
				else
				{
					uart_print(".0");
				}
			}

			uart_print(" C | ");

			// =================================================
			// LÓGICA DEL MOTOR
			// =================================================
			if (temperatura > TEMP_UMBRAL)
			{
				uart_print("TEMP ALTA");

				if (motor_posicion == 0)
				{
					uart_print(
					" -> se requiere AVANCE\r\n"
					);

					// ------------------------------------------------
					// Mostrar inmediatamente que el motor avanzará
					// ------------------------------------------------
					motor_avanzar();
					motor_posicion = 1;

					// ------------------------------------------------
					// LCD muestra motor avanzado
					// ------------------------------------------------
					lcd_pantalla_temperatura(
					temperatura_x2,
					1,
					motor_posicion
					);

					_delay_ms(TIEMPO_MOTOR_MS);

					motor_stop();

					uart_print(
					"MOTOR DC -> DETENIDO\r\n"
					);
				}
				else
				{
					uart_print(
					" -> motor ya esta avanzado\r\n"
					);
				}
			}
			else if (temperatura < TEMP_UMBRAL)
			{
				uart_print("TEMP BAJA");

				if (motor_posicion == 1)
				{
					uart_print(
					" -> se requiere RETROCESO\r\n"
					);

					// ------------------------------------------------
					// Mostrar inmediatamente que el motor retrocede
					// ------------------------------------------------
					motor_retroceder();

					motor_posicion = 0;

					lcd_pantalla_temperatura(
					temperatura_x2,
					1,
					motor_posicion
					);

					_delay_ms(TIEMPO_MOTOR_MS);

					motor_stop();

					uart_print(
					"MOTOR DC -> DETENIDO\r\n"
					);
				}
				else
				{
					uart_print(
					" -> motor ya esta en reposo\r\n"
					);
				}
			}
			else
			{
				uart_print(
				"TEMP IGUAL AL UMBRAL\r\n"
				);
			}

			// =================================================
			// ACTUALIZAR LCD
			// =================================================
			lcd_actualizar(temperatura_x2,lm75_ok,motor_posicion);
			_delay_ms(500);
		}

		_delay_ms(1);

		contador_i2c++;
	}
}

