/*
 * ds1990a.c
 *
 * Created: 26/06/2016 22:58:26
 *  Author: alan
 */ 

#include "ds1990a.h"
#include <util/delay.h>
#include <avr/io.h>
/************************************************************************/
/* Sets direction and value of the ATmega328p PB0 (PIN14)               */
/* Also checks the PIN state - HIGH or LOW								*/
/*																		*/
/* TODO -> Port parser to improve decoupling							*/
/************************************************************************/

#define GO_OUTPUT()	DDRB |= _BV(DDB0);
#define GO_INPUT()	DDRB &= ~_BV(DDB0);
#define GO_HIGH()	PORTB |= _BV(PORTB0);
#define GO_LOW()	PORTB &= ~_BV(PORTB0);
#define PIN_VALUE	(PINB & _BV(PINB0));

#define TRUE 1
#define FALSE 0

// global search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;

/*Send reset signal and check if the iButton is attached on the reader*/
unsigned char detectPresence(){
	char tmp;
	GO_OUTPUT();
	GO_LOW();
	_delay_us(TRST_LOW);
	GO_INPUT();
	_delay_us(TRST_SAMPLE);
	tmp = PIN_VALUE;
	_delay_us(TRST_HIGH);
	
	if(tmp) return NO_BUTTON;
	return BUTTON_READY;
}

/* Executes the write-one and write-zero time slots */
void writeBit(unsigned char bit)
{
	//write-one
	if(bit){
		GO_OUTPUT();
		GO_LOW();
		_delay_us(TW1_LOW);//low time 1-15us
		GO_HIGH();
		_delay_us(TW1_HIGH);
	}
	//write-zero
	else{
		GO_OUTPUT();
		GO_LOW();
		_delay_us(TW0_LOW);//low time 60-120us
		GO_HIGH();
		_delay_us(TW0_HIGH);
	}
}

/*Writes 8 bits in a row*/
void writeByte(unsigned char _byte){
	int i;
	for (i=0;i<8;i++){
		writeBit(_byte & 0x01); //send LSB first
		_byte >>=1;//shift the byte for the next bit
	}
}

/* Executes the read time slot*/
unsigned char readBit()
{
	unsigned char read_bit;
	GO_OUTPUT();
	GO_LOW();
	_delay_us(TREAD_LOW);
	GO_INPUT(); 
	_delay_us(TREAD_SAMPLE);
	read_bit = PIN_VALUE;
	_delay_us(TREAD_HIGH);
	
	if(read_bit) return 1;
	return 0;
}

/************************************************************************/
/* Reads 8 bits in a row												*/
/* Always set the MSB until all bits are back to their original position*/
/************************************************************************/
unsigned char readByte(){
	int i;
	unsigned char result=0;
	for (i=0;i<8;i++){
		result>>=1;//keeps shifting the byte
		if (readBit()){
			result|=0x80;//if 1 set the MSB
		}
	}
	return result;
}

/************************************************************************/
/* The 1-Wire CRC scheme is described in Maxim Application Note 27:		*/
/* "Understanding and Using Cyclic Redundancy Checks with Maxim iButton */
/* Products"															*/
/************************************************************************/
unsigned char crc8(const unsigned char *addr, unsigned char len)
{
	unsigned char crc = 0;
	
	while (len--) {
		unsigned char inbyte = *addr++;
		for (unsigned char i = 8; i; i--) {
			unsigned char mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}


/************************************************************************/
/* The search algorithm is available in the Maxim APPLICATION NOTE 187  */
/* 1-Wire Search Algorithm												*/
/************************************************************************/
unsigned char search(unsigned char *newAddr)
{
	unsigned char id_bit_number;
	unsigned char last_zero, rom_byte_number, search_result;
	unsigned char id_bit, cmp_id_bit;

	unsigned char rom_byte_mask, search_direction;

	// initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

	// if the last call was not the last one
	if (!LastDeviceFlag)
	{
		// 1-Wire reset
		if (!detectPresence())
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;
		}

		// issue the search command
		writeByte(0xF0);

		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = readBit();
			cmp_id_bit = readBit();

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
			break;
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
				search_direction = id_bit;  // bit write value for search
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					else
					// if equal to last pick 1, if not then pick 0
					search_direction = (id_bit_number == LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
						LastFamilyDiscrepancy = last_zero;
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
				ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
				ROM_NO[rom_byte_number] &= ~rom_byte_mask;

				// serial number search direction write bit
				writeBit(search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!(id_bit_number < 65))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
			LastDeviceFlag = TRUE;

			search_result = TRUE;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = FALSE;
		LastFamilyDiscrepancy = 0;
		search_result = FALSE;
	}
	for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
	return search_result;
}