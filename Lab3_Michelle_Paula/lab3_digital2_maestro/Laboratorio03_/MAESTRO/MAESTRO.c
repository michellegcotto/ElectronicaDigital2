/*
 * MAESTRO.c
 *
 * Created: 7/23/2026
 * Author : Jaqueline Michelle González Cotto
 *          Paula Melissa Ayala Sandoval
 */

#include "MAESTRO.h"

void spiInit(Spi_Type sType, Spi_Data_Order sDataOrder, Spi_Clock_Polarity sClockPolarity, Spi_Clock_Phase sClockPhase)
{
    // Limpiar configuración previa
    SPCR = 0;
    SPSR = 0;

    // ¿Master o Slave?
    if(sType & 0b00010000)
    {
        // MASTER
        DDRB |= (1<<DDB3) | (1<<DDB5) | (1<<DDB2);   // MOSI, SCK, SS -> Salidas
        DDRB &= ~(1<<DDB4);                          // MISO -> Entrada

        SPCR |= (1<<MSTR);

        uint8_t temp = sType & 0x07;

        switch(temp)
        {
            case 0:     // Fosc/2
                SPCR &= ~((1<<SPR1) | (1<<SPR0));
                SPSR |= (1<<SPI2X);
                break;

            case 1:     // Fosc/4
                SPCR &= ~((1<<SPR1) | (1<<SPR0));
                SPSR &= ~(1<<SPI2X);
                break;

            case 2:     // Fosc/8
                SPCR &= ~(1<<SPR1);
                SPCR |= (1<<SPR0);
                SPSR |= (1<<SPI2X);
                break;

            case 3:     // Fosc/16
                SPCR &= ~(1<<SPR1);
                SPCR |= (1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;

            case 4:     // Fosc/32
                SPCR |= (1<<SPR1);
                SPCR &= ~(1<<SPR0);
                SPSR |= (1<<SPI2X);
                break;

            case 5:     // Fosc/64
                SPCR |= (1<<SPR1);
                SPCR &= ~(1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;

            case 6:     // Fosc/128
                SPCR |= (1<<SPR1) | (1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;
        }
    }
    else
    {
        // SLAVE
        DDRB |= (1<<DDB4);                           // MISO -> Salida
        DDRB &= ~((1<<DDB3) | (1<<DDB5) | (1<<DDB2));// MOSI,SCK,SS -> Entradas

        SPCR &= ~(1<<MSTR);
    }

    // Configuración general
    SPCR |= (1<<SPE) | sDataOrder | sClockPolarity | sClockPhase;
}

void spiWrite(uint8_t dat)
{
    SPDR = dat;

    while(!(SPSR & (1<<SPIF)));
}

uint8_t spiDataReady(void)
{
    return (SPSR & (1<<SPIF));
}

uint8_t spiRead(void)
{
    while(!(SPSR & (1<<SPIF)));

    return SPDR;
}

uint8_t spiTransfer(uint8_t data)
{
    SPDR = data;

    while(!(SPSR & (1<<SPIF)));

    return SPDR;
}