/*
 * ds1990a.h
 *
 * Created: 26/06/2016 22:15:25
 *  Author: alan
 */ 

#ifndef DS1990A_H_
#define DS1990A_H_

#define F_CPU 16000000UL	// Oscillator frequency.

/*Check button presence*/
#define NO_BUTTON	0
#define BUTTON_READY	1

/* Reset time slot */
#define TRST_LOW	480 //reset low time
#define TRST_HIGH	420 //reset high time
#define TRST_SAMPLE	70 //reset sample time

/* Write bit time slot */
#define TW1_LOW		10 //write-one low time
#define TW1_HIGH	55 //write-one high time
#define TW0_LOW		65 //write-zero low time
#define TW0_HIGH	5 //write-zero high time

/* Read bit time slot*/
#define TREAD_LOW		3 //read low time
#define TREAD_SAMPLE	10 //read sample time
#define TREAD_HIGH		53 //read high time

/*Prototypes*/
unsigned char detectPresence(void);
void writeBit(unsigned char bit);
void writeByte(unsigned char _byte);
unsigned char readBit(void);
unsigned char readByte(void);
unsigned char search(unsigned char *newAddr);
unsigned char crc8(const unsigned char *addr, unsigned char len);

#endif /* DS1990A_H_ */