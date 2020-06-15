/*
 * uart.h
 *
 * Created: 26/06/2016 21:15:25
 *  Author: alan
 */ 

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdint.h>

#define	F_CPU			16000000UL

#define BAUD			9600					// Baud rate

/* Enable/Disable TX and RX */
#define TX_START()		UCSR0B |= _BV(TXEN0)	// Enable TX
#define TX_STOP()		UCSR0B &= ~_BV(TXEN0)	// Disable TX
#define RX_START()		UCSR0B |= _BV(RXEN0)	// Enable RX
#define RX_STOP()		UCSR0B &= ~_BV(RXEN0)	// Disable RX

/* Prototypes */
void initUART(void);
void sendByte(unsigned char data);
void sendString(char *str);

#endif /* UART_H_ */