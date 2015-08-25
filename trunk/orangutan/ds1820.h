#ifndef DS1820_H
#define DS1820_H

/*
ds1820.h
DS1820_lampomittaus_m32.c aputiedosto
keskeneräinen, mutta toimii ainakin 4 MHz kiteellä, 
Ilari Nummila, 07042006
tarvitaan myös ds1820.c

pieniä muokkauksia Petri Hyötylä 26022008
*/

#include "inttypes.h"

uint8_t OwReset(void);
uint8_t OwReadByte(void);
int16_t GetTemp(void);
void OwWriteByte(uint8_t data);

// Lämpötila-anturin portti ja pinni
#define TEMP_PORT     PORTC
#define TEMP_PIN      PINC
#define TEMP_REGISTER DDRC
#define TEMP_BIT (1<<0) // DS1820 0-bitissä

#endif // DS1820_H
