/*
 * SPI.c
 *
 * Created: 23/07/2026
 * Author : Jaqueline Michelle González Cotto / Paula Melissa Ayala Sandoval
 */

#include "SPI.h"

void spiInit(Spi_Type sType,
             Spi_Data_Order sDataOrder,
             Spi_Clock_Polarity sClockPolarity,
             Spi_Clock_Phase sClockPhase)
{
    SPCR = 0;
    SPSR = 0;

    if(sType & 0b00010000)
    {
        // MASTER
        DDRB |= (1<<DDB3) | (1<<DDB5) | (1<<DDB2);
        DDRB &= ~(1<<DDB4);

        SPCR |= (1<<MSTR);

        uint8_t temp = sType & 0x07;

        switch(temp)
        {
            case 0:
                SPCR &= ~((1<<SPR1)|(1<<SPR0));
                SPSR |= (1<<SPI2X);
                break;

            case 1:
                SPCR &= ~((1<<SPR1)|(1<<SPR0));
                SPSR &= ~(1<<SPI2X);
                break;

            case 2:
                SPCR &= ~(1<<SPR1);
                SPCR |= (1<<SPR0);
                SPSR |= (1<<SPI2X);
                break;

            case 3:
                SPCR &= ~(1<<SPR1);
                SPCR |= (1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;

            case 4:
                SPCR |= (1<<SPR1);
                SPCR &= ~(1<<SPR0);
                SPSR |= (1<<SPI2X);
                break;

            case 5:
                SPCR |= (1<<SPR1);
                SPCR &= ~(1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;

            case 6:
                SPCR |= (1<<SPR1) | (1<<SPR0);
                SPSR &= ~(1<<SPI2X);
                break;
        }
    }
    else
    {
        // SLAVE

        DDRB |= (1<<DDB4);
        DDRB &= ~((1<<DDB3) | (1<<DDB5) | (1<<DDB2));

        SPCR &= ~(1<<MSTR);
    }

    SPCR |= (1<<SPE) | sDataOrder | sClockPolarity | sClockPhase;
}

void spiWrite(uint8_t dat)
{
    SPDR = dat;
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