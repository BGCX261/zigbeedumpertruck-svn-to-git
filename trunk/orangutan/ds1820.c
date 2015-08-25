
/********************************************************************
// ** DS1820 Funktiot by Jukka Pitkänen
// ** GetTemp-korjaus, Olli-Pekka Korpela
 ********************************************************************/

#include <avr/io.h>
#include <pololu/time.h>
#include "ds1820.h"

// OwReset() alustaa ja tarkastaa onko 'narun' päässä ketään ********
uint8_t OwReset(void)
{
	uint8_t temp=0;

	TEMP_REGISTER |= TEMP_BIT; // pinnin suunta ulos
	TEMP_PORT &= ~(TEMP_BIT);  // pinni maihin

	delay_us(500);
	// jos bittiä pidetään maassa n. 500us,
	// DS1820 ymmärtää sen reset-signaaliksi

	// muutetaan pinnin suunta takaisin sisäänpäin
	TEMP_REGISTER &= ~TEMP_BIT;
	TEMP_PORT |= TEMP_BIT;  // vedetään linja 'ylös'

	delay_us(60);
	// DS1820 vastaa reset-signaaliin n. 60us päästä

	temp = TEMP_PIN & TEMP_BIT; // luetaan DS1820:n vastaus
	// jos linja on maassa, on DS1820 ymmärtänyt signaalin
	// jos taas pinni on 1,  DS1820 ei ole vastannut signaaliin

	delay_us(450);
	// odotetaan vielä n. 450us, jotta resetointi on varmasti loppunut

	return temp;
}


/* OwReadByte() lukee tavun DS1820:n 9:n tavun muistista,
 * aloittaen vähiten merkitsevän tavun vähiten merkitsevästä bitistä.
 *
 * Ensimmäiset 2 tavua pitävät sisällään lämpötilan,
 * joista ensimmäinen tavu lämpötilan ja toinen tavu tiedon,
 * onko lampotila positiivinen vai negatiivinen.
 * 0 = positiivinen ja 1 = negatiivinen
 */
uint8_t OwReadByte(void)
{
	uint8_t data = 0;  // muuttuja, johon luettu arvo tallennetaan
	uint8_t w = 0;     // silmukkalaskuri

	// luetaan vähiten merkitsevät bitit lämpötila-rekisteristä
	for(w = 0; w <= 7; w++)
	{
		data >>= 1;
		TEMP_REGISTER |= TEMP_BIT;  // pinnin suunta ulos

		TEMP_PORT &= ~(TEMP_BIT); // linja maihin

		delay_us(2); // min. 1 us delay
		// 		_delay_loop_2(DELAY50); // 50 us viive
		// luku signaalissa pidetään linjaa maissa n. 50us

		// käännetään data-linjan suunta sisään
		TEMP_REGISTER &= ~TEMP_BIT;
		TEMP_PORT |= TEMP_BIT;    // ja vedetään se ylös

		delay_us(11); // bitti luettava 15 us:n sisällä linjan maadoituksesta
		// jos DS1820 lähettää 0:n, se ei päästä linjaa ylös
		// vaan pitää sitä vielä n.15us maissa

		if(TEMP_PIN & TEMP_BIT)   // testataan linjan tila
		{                // jos se on ylhäällä, on luettava bitti 1
			data |= 0x80;
		}

		else     // jos edellinen ehto ei toteutunut,
			// pitää DS1820 linjaa maissa ja bitti on silloin 0
			{
				data &= 0x7F;
			}

		delay_us(60);
		// pidetään pieni tauko ennen seuraavaa lukukertaa
	}
	return data;  // palautetaan luettu data
}

// OwWriteByte() funktiolla annetaan DS1820:lle komentoja, **********
// parametrina funktio saa suoritettavan käskyn
void OwWriteByte(uint8_t data)
{
	uint8_t e = 0;

	for(e = 0; e < 8; e++)
		// silmukassa annetaan DS1820:lle komennosta bitti kerrallaan
		// aloittaen vähiten merkitsevästä bitistä
	{
		if(data & 0x01)  // jos vähiten merkitsevä bitti on 1, ehto toteutuu
		{
			// kun kirjoitetaan 1, linja maihin n. 5us
			TEMP_REGISTER |= TEMP_BIT;
			TEMP_PORT &= ~(TEMP_BIT);
			delay_us(5);

			// ja odotetaan n. 60us
			TEMP_REGISTER &= ~(TEMP_BIT);
			TEMP_PORT |= TEMP_BIT;
			delay_us(60);
		}

		else     // jos kirjoitettava bitti on 0
		{
			// kun kirjoitetaan 0, linja maihin n. 60us
			TEMP_REGISTER |= TEMP_BIT;
			TEMP_PORT &= ~(TEMP_BIT);
			delay_us(60);

			// ja odotetaan n. 5us
			TEMP_REGISTER &= ~(TEMP_BIT);
			TEMP_PORT |= (TEMP_BIT);
			delay_us(5);
		}
		data >>= 1;  // siirrytään käskyssä seuraavaan bittiin
	}
}

// ConvertTemp hoitaa anturin alustuksen, ***************************
// käskyjen antamisen ja lämpötilan hakemisen
int16_t GetTemp(void)
{
	int16_t lsb = 0;  // lämpötilatieto
	int16_t msb = 0;  // onko pos vai neg, 0=+ ja 1=-

	OwReset(); // aina ennen käskyjen antamista DS1820 pitää resetoida
	OwWriteByte(0xCC);  // annetaan skip-komento,
	// jolloin ei tehdä laitteiden etsintää,
	// eli oletetaan, tai tiedetään, että väylällä on vain yksi DS1820

	OwWriteByte(0x44);
	// CONVERT T komento, jolloin DS1820 aloittaa lämpötilan laskemisen
	delay_us(45);
	// Odotetaan hetki, että linja on varmasti "normaalissa" tilassa

	// käännetään data-linjan suunta sisään
	TEMP_REGISTER &= ~TEMP_BIT;
	TEMP_PORT |= TEMP_BIT;    // ja vedetään se ylös
	// DS18B20 pitää linjaa maissa lämpötilamuunnoksen ajan
	while(!(TEMP_PIN & TEMP_BIT));

	OwReset(); // aina ennen käskyjen antamista DS1820 pitää resetoida
	OwWriteByte(0xCC); // jos antureita olisi enemmän kuin yksi,
	// pitäisi suorittaa laitteiden etsintä ja tunnistus ym.
	// nyt antureita on vain yksi, joten voidaan antaa skip-komento

	OwWriteByte(0xBE); // Read scratchpad-komento, jolla DS1820
	// valmistautuu lämpötilan lukemiseen ja alkaa lähettää bittejä
	lsb = OwReadByte(); // muistin lukeminen
	// täytyy tehdä heti 0xBE komennon jälkeen
	// muistin lukeminen aloitetaan vähiten merkitsevästä tavusta
	// siihen on talletettu lämpötila

	msb = OwReadByte(); // lukeminen jatkuu seuraavasta tavusta,
	// missä on tieto lämpötilan etumerkistä

	OwReset();
	// anna reset-signaali, jolla DS1820 lopettaa bittien lähettämisen

	return (msb << 8) | lsb; // palautetaan pääohjelmaan
}

