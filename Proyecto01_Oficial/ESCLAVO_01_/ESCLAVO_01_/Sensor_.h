/*
 * Sensor_.h
 *
 * Librer?a para el sensor ultras?nico (tipo HC-SR04) que vive en el
 * ESCLAVO. Mide distancia usando el m?todo de "tiempo de vuelo" del eco:
 *
 *   1. Se manda un pulso corto por TRIG.
 *   2. El sensor responde poniendo ECHO en alto mientras el sonido va y
 *      regresa.
 *   3. Medimos cu?nto tiempo estuvo ECHO en alto (con Timer1).
 *   4. Esa duraci?n se convierte a distancia con la velocidad del sonido.
 *
 * BUG CORREGIDO respecto a tu c?digo de ejemplo (Prueba_S1_.c):
 *   Ten?as TRIG_DDR/ECHO_DDR apuntando a DDRC (con pines PC0/PC1), pero
 *   TRIG_PORT/ECHO_PINREG apuntaban a PORTB/PINB. Eso significa que
 *   configurabas la DIRECCI?N del pin en el puerto C, pero le?as/escrib?as
 *   el NIVEL en el puerto B -> el sensor nunca iba a funcionar bien,
 *   porque est?s tocando dos pines f?sicos distintos. Aqu? todo qued?
 *   consistente en el puerto C (PC0 = TRIG, PC1 = ECHO).
 */

#ifndef SENSOR__H_
#define SENSOR__H_

#include <avr/io.h>
#include <stdint.h>

// Pines del sensor ultras?nico (ambos en el mismo puerto: C)
#define TRIG_PIN    PC0
#define ECHO_PIN    PC1
#define TRIG_DDR    DDRC
#define ECHO_DDR    DDRC
#define TRIG_PORT   PORTC   // <-- corregido: antes dec?a PORTB
#define ECHO_PINREG PINC    // <-- corregido: antes dec?a PINB

// Configura TRIG como salida y ECHO como entrada.
void Sensor_Init(void);

// Devuelve la distancia medida en cent?metros (0 si hubo timeout, es
// decir, si no se detect? ning?n eco).
// El valor se limita (satura) a 255 porque lo vamos a transmitir por
// I2C en un solo byte.
uint8_t Sensor_LeerDistanciaCM(void);

#endif /* SENSOR__H_ */