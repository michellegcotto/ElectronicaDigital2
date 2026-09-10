/*
 * posLAB5_digital2.c
 *
 * Created: 10/09/2026
 * Author : michelle gonzález cotto
 */ 


#define F_CPU 16000000UL
#define UART_BAUD 9600UL
#define UART_UBRR ((F_CPU / (16UL * UART_BAUD)) - 1UL)

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define BTN_ARRIBA       PD2
#define BTN_ABAJO        PD3
#define BTN_DERECHA      PD4
#define BTN_IZQUIERDA    PD5
#define BTN_A            PD6
#define BTN_B            PD7

#define MASCARA_BOTONES ((1U << BTN_ARRIBA)    | \
(1U << BTN_ABAJO)     | \
(1U << BTN_DERECHA)   | \
(1U << BTN_IZQUIERDA) | \
(1U << BTN_A)         | \
(1U << BTN_B))

static void UART_Init(void)
{
	UBRR0H = (uint8_t)(UART_UBRR >> 8);
	UBRR0L = (uint8_t)UART_UBRR;

	/* Velocidad normal, transmisor habilitado, formato 8-N-1. */
	UCSR0A = 0U;
	UCSR0B = (1U << TXEN0);
	UCSR0C = (1U << UCSZ01) | (1U << UCSZ00);
}

static void UART_Transmitir(uint8_t dato)
{
	while ((UCSR0A & (1U << UDRE0)) == 0U)
	{
	}

	UDR0 = dato;
}

static void Botones_Init(void)
{
	/* PD2-PD7 como entradas. */
	DDRD &= (uint8_t)(~MASCARA_BOTONES);

	/* Pull-up interno: cada pulsador se conecta entre el pin y GND. */
	PORTD |= MASCARA_BOTONES;
}

static void Enviar_Botones(uint8_t flancos)
{
	if ((flancos & (1U << BTN_ARRIBA)) != 0U)
	{
		UART_Transmitir('U');
	}

	if ((flancos & (1U << BTN_ABAJO)) != 0U)
	{
		UART_Transmitir('D');
	}

	if ((flancos & (1U << BTN_DERECHA)) != 0U)
	{
		UART_Transmitir('R');
	}

	if ((flancos & (1U << BTN_IZQUIERDA)) != 0U)
	{
		UART_Transmitir('L');
	}

	if ((flancos & (1U << BTN_A)) != 0U)
	{
		UART_Transmitir('A');
	}

	if ((flancos & (1U << BTN_B)) != 0U)
	{
		UART_Transmitir('B');
	}
}

int main(void)
{
	uint8_t estado_anterior;
	uint8_t estado_actual;
	uint8_t flancos_presionados;

	Botones_Init();
	UART_Init();

	estado_anterior = PIND & MASCARA_BOTONES;

	while (1)
	{
		estado_actual = PIND & MASCARA_BOTONES;

		/* Detectar exclusivamente la transición de botón libre a presionado. */
		flancos_presionados =
		estado_anterior & (uint8_t)(~estado_actual) & MASCARA_BOTONES;

		if (flancos_presionados != 0U)
		{
			_delay_ms(20);
			estado_actual = PIND & MASCARA_BOTONES;

			/* Confirmar que sigue presionado después del antirrebote. */
			flancos_presionados =
			estado_anterior & (uint8_t)(~estado_actual) & MASCARA_BOTONES;

			Enviar_Botones(flancos_presionados);
		}

		estado_anterior = estado_actual;
		_delay_ms(5);
	}
}

