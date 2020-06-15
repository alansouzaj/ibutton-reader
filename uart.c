/*
 * uart.c
 *
 * Created: 26/06/2016 22:00:26
 *  Author: alan
 */
 
#include "uart.h"
#include <util/delay.h>
#include <util/setbaud.h>

void initUART(void)
{
	// Set baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	TX_START();
	RX_START();

	//Set frame format 8-bit data
	 UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); 

}

void sendByte(unsigned char data)
{
	// Stay here until data buffer is empty
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = (unsigned char) data;

}


void sendString(char *str)
{
	while (*str != '\0')
	{
		sendByte(*str);
		++str;
	}
}

