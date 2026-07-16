/*
 * lab1_digital2.c
 * Created: 9/07/2026
 * Author : Jaqueline Michelle González Cotto
 * Carnet : 23020
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "display7.h"
 
// estado del juego
typedef enum {
    STATE_IDLE = 0,     // esperando boton de inicio
    STATE_COUNTDOWN,    // cuenta regresiva
    STATE_RACE,         // carrera
    STATE_FINISHED       // ganador
} game_state_t;
 
volatile game_state_t game_state = STATE_IDLE;
volatile uint8_t countdown_value = 0;
 
volatile uint8_t player1_count = 0;
volatile uint8_t player2_count = 0;
 
volatile uint8_t start_pressed_flag = 0;
volatile uint8_t j1_pressed_flag    = 0;
volatile uint8_t j2_pressed_flag    = 0;
 
#define DEBOUNCE_MAX 20
 
volatile uint8_t db_counter[3] = {0, 0, 0};
volatile uint8_t db_stable[3]  = {1, 1, 1};
 
static uint8_t count_to_bar(uint8_t count)
{
    uint8_t n = (count > 4) ? 4 : count; /* maximo 4 LEDs disponibles */
    uint8_t bar = 0;
    for (uint8_t i = 0; i < n; i++) {
        bar |= (1 << i);
    }
    return bar; // leds en contador de decadas
}
 
void set_leds_j1(uint8_t value)
{
    uint8_t bar = count_to_bar(value);
    PORTC = (PORTC & 0xF0) | bar;
}
 
void set_leds_j2(uint8_t value)
{
    uint8_t bar = count_to_bar(value);
 
    if (bar & 0x01) PORTC |= (1 << PC4); else PORTC &= ~(1 << PC4);
    if (bar & 0x02) PORTC |= (1 << PC5); else PORTC &= ~(1 << PC5);
    if (bar & 0x04) PORTB |= (1 << PB4); else PORTB &= ~(1 << PB4);
    if (bar & 0x08) PORTB |= (1 << PB5); else PORTB &= ~(1 << PB5);
}
 
// iniciación
void io_init(void)
{
    // Segmentos a-f en PORTD (salidas)
    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4) |
            (1 << PD5) | (1 << PD6) | (1 << PD7);
 
    // PORTB: seg g y bits de leds del jugador 2
    DDRB |= (1 << PB0) | (1 << PB4) | (1 << PB5);
    DDRB &= ~((1 << PB1) | (1 << PB2) | (1 << PB3));
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3);
 
    // PORTC: todos los leds del jugador 1
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) |
            (1 << PC3) | (1 << PC4) | (1 << PC5);
 
    display7_off();
    set_leds_j1(0);
    set_leds_j2(0);
}
 
void timer0_init(void)
{
    TCCR0A = (1 << WGM01);              // modo CTC 
    TCCR0B = (1 << CS01) | (1 << CS00); // prescaler 64
    OCR0A  = 249;                       // 16MHz/64/250 = 1000 Hz (1 ms)
    TIMSK0 = (1 << OCIE0A);
}
 
ISR(TIMER0_COMPA_vect)
{
    uint8_t raw[3];
    raw[0] = (PINB & (1 << PB1)) ? 1 : 0; // boton inicio
    raw[1] = (PINB & (1 << PB2)) ? 1 : 0; // boton J1
    raw[2] = (PINB & (1 << PB3)) ? 1 : 0; // boton J2
 
    for (uint8_t i = 0; i < 3; i++) {
        if (raw[i] == 0) {
            if (db_counter[i] < DEBOUNCE_MAX) db_counter[i]++;
        } else {
            if (db_counter[i] > 0) db_counter[i]--;
        }
 
        uint8_t new_stable = db_stable[i];
        if (db_counter[i] >= DEBOUNCE_MAX) new_stable = 0;
        else if (db_counter[i] == 0)        new_stable = 1;
 
        if (new_stable == 0 && db_stable[i] == 1) {
            if (i == 0) start_pressed_flag = 1;
            else if (i == 1) j1_pressed_flag = 1;
            else if (i == 2) j2_pressed_flag = 1;
        }
        db_stable[i] = new_stable;
    }
}
 
void timer1_init(void)
{
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC, prescaler 1024
    OCR1A  = 15624; //1 segundo                                    
    TIMSK1 = (1 << OCIE1A);
}
 
ISR(TIMER1_COMPA_vect)
{
    if (game_state == STATE_COUNTDOWN) {
        if (countdown_value > 0) {
            countdown_value--;
            display7_digit(countdown_value);
        } else {
            player1_count = 0;
            player2_count = 0;
            set_leds_j1(0);
            set_leds_j2(0);
            j1_pressed_flag = 0;
            j2_pressed_flag = 0;
            game_state = STATE_RACE;
        }
    }
}
 
//programa principal
int main(void)
{
    io_init();
    timer0_init();
    timer1_init();
    sei();
 
    while (1) {
 
// boton de carrera a iniciar
        if (start_pressed_flag) {
            start_pressed_flag = 0;
            if (game_state == STATE_IDLE) {
                countdown_value = 5;
                display7_digit(countdown_value);
                set_leds_j1(0);
                set_leds_j2(0);
                game_state = STATE_COUNTDOWN;
            }
        }
 
// deshabilitados hasta terminar cuenta
        if (game_state != STATE_RACE) {
            j1_pressed_flag = 0;
            j2_pressed_flag = 0;
        } else {
// jugador 1
            if (j1_pressed_flag) {
                j1_pressed_flag = 0;
                if (player1_count < 4) {
                    player1_count++;
                    set_leds_j1(player1_count);
                    if (player1_count == 4) {
                        game_state = STATE_FINISHED;
                        set_leds_j1(0x0F);
                        set_leds_j2(0x00);
                        display7_digit(1);
                    }
                }
            }
 
// jugaar 2
            if (j2_pressed_flag) {
                j2_pressed_flag = 0;
                if (player2_count < 4) {
                    player2_count++;
                    set_leds_j2(player2_count);
                    if (player2_count == 4) {
                        game_state = STATE_FINISHED;
                        set_leds_j2(0x0F);
                        set_leds_j1(0x00);
                        display7_digit(2);
                    }
                }
            }
        }
    }
}