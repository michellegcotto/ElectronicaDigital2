/*
 * Sensor_.c
 *
 * Implementaci?n de la medici?n de distancia con el sensor ultras?nico,
 * usando Timer1 como cron?metro de alta resoluci?n.
 */

#include "Sensor_.h"
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* =====================================================================
 *  Sensor_Init
 * ===================================================================== */
void Sensor_Init(void){
	TRIG_DDR |= (1 << TRIG_PIN);      // TRIG como salida
	ECHO_DDR &= ~(1 << ECHO_PIN);     // ECHO como entrada
	TRIG_PORT &= ~(1 << TRIG_PIN);    // TRIG inicia en bajo
}

/* =====================================================================
 *  trigger_pulso (funci?n privada del archivo, no va en el .h)
 *  Genera el pulso de disparo de 10us que necesita el sensor para
 *  empezar a medir.
 * ===================================================================== */
static void trigger_pulso(void) {
	TRIG_PORT |= (1 << TRIG_PIN);   // TRIG en alto...
	_delay_us(10);                  // ...durante 10 microsegundos...
	TRIG_PORT &= ~(1 << TRIG_PIN);  // ...y lo volvemos a bajar.
}

/* =====================================================================
 *  medir_tiempo_eco_us (funci?n privada del archivo)
 *  Mide, en microsegundos, cu?nto tiempo el pin ECHO permaneci? en alto.
 *  Ese tiempo es proporcional a la distancia (ida y vuelta del sonido).
 * ===================================================================== */
static uint16_t medir_tiempo_eco_us(void) {
	uint16_t timeout = 0;

	// 1) Disparamos el pulso de trigger
	trigger_pulso();

	// 2) Esperamos a que ECHO suba a nivel alto (inicio del eco).
	//    Si tarda demasiado (20ms), asumimos que no hay sensor/objeto
	//    y salimos con distancia 0 para no quedarnos trabados aqu?.
	while (!(ECHO_PINREG & (1 << ECHO_PIN))) {
		_delay_us(1);
		if (++timeout > 20000) return 0;
	}

	// 3) En cuanto ECHO sube, arrancamos Timer1 como cron?metro.
	//    Prescaler = 8  =>  cada "tick" del contador = 8/F_CPU segundos
	//    Con F_CPU=16MHz: 8/16000000 = 0.5us por tick.
	TCCR1B = (1 << CS11);    // Prescaler 8, arranca el conteo
	TCNT1 = 0;               // Reiniciamos el contador a 0

	// 4) Esperamos a que ECHO baje de nuevo (fin del eco = objeto
	//    detectado y sonido de regreso). Timeout de 40ms como margen
	//    de seguridad (equivale a unos 6-7 metros de distancia m?xima).
	timeout = 0;
	while (ECHO_PINREG & (1 << ECHO_PIN)) {
		_delay_us(1);
		if (++timeout > 40000) {
			TCCR1B = 0;   // Detenemos el timer si hubo timeout
			return 0;
		}
	}

	// 5) Detenemos el Timer1 (quitamos el reloj -> CS1[2:0] = 000)
	TCCR1B = 0;

	// 6) Leemos cu?ntos "ticks" pasaron mientras ECHO estuvo en alto
	uint16_t ticks = TCNT1;

	// 7) Convertimos ticks a microsegundos (cada tick = 0.5us)
	return ticks / 2;
}

/* =====================================================================
 *  Sensor_LeerDistanciaCM
 *  F?rmula f?sica: distancia = (velocidad_sonido * tiempo) / 2
 *  El /2 es porque el tiempo medido es IDA + VUELTA del sonido.
 *  Velocidad del sonido ? 340 m/s = 0.0343 cm/us, de ah? sale el
 *  factor (tiempo_us * 34) / 2000 que ya trae tu c?digo de ejemplo.
 * ===================================================================== */
uint8_t Sensor_LeerDistanciaCM(void){
	uint16_t tiempo_us = medir_tiempo_eco_us();

	uint32_t distancia_cm = (tiempo_us * 34UL) / 2000;

	// El bus I2C en este proyecto transmite la distancia en UN solo
	// byte (0-255), as? que saturamos el valor para no desbordarnos
	// si el objeto est? muy lejos (o no hay eco = valor gigante/timeout).
	if (distancia_cm > 255) {
		distancia_cm = 255;
	}

	return (uint8_t)distancia_cm;
}