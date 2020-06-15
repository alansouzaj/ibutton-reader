/*
 * IButtonReader.c
 *
 * Created: 24/06/2016 17:20:11
 * Author : alan
 */
 
#define F_CPU 16000000UL	// Oscillator frequency.

#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>
#include "uart.h"
#include "ds1990a.h"



char numToHex(unsigned char num){

	if ((num >= 0) && (num < 10)){
		return num + '0';
	}
	if ((num >= 9) && (num < 16)){
		return num - 10 + 'A';
	}
	return '?';
}

int main(void)
{
	int i=0;
	unsigned char crc=0x00;
	unsigned char serial_request = 0x33;
	unsigned char serial_number[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	initUART();
	sendString("DS1990A - Test Program\r\n");
	sendString("Ready to read\r\n");
	
	sendString("Testing search function first. Attach the button!!!\r\n");
	sendString("You have 5 seconds...\r\n");
	_delay_ms(5000);
	
	search(serial_number);
	
	sendString("Search result: ");
	for(i=7;i>=0;i--){
		sendByte(numToHex(serial_number[i]/16));
		sendByte(numToHex(serial_number[i]%16));
		sendByte(' ');
	} /*end of for*/
	sendByte('\r');sendByte('\n');sendByte('\n');
	
	sendString("Now were inside a loop.\r\n");
	while(1){
		if(detectPresence()){
			sendString("Presence detected\r\n\n");
		
			writeByte(serial_request);//send command 0x33
			
			// loop to save data
			for (i=0;i<8;i++){
				serial_number[i]=readByte();
			} /*end of for*/
			_delay_ms(2000);
			
			sendString("Button Family Code: ");
			sendByte(numToHex(serial_number[0]/16));
			sendByte(numToHex(serial_number[0]%16));
			sendByte('\r');sendByte('\n');
			
			sendString("Button ID: ");
			for (i=6;i>=1;i--){
				sendByte(numToHex(serial_number[i]/16));
				sendByte(numToHex(serial_number[i]%16));
				sendByte(' ');
			} /*end of for*/
			sendByte('\r');sendByte('\n');
			
			sendString("CRC: ");
			sendByte(numToHex(serial_number[7]/16));
			sendByte(numToHex(serial_number[7]%16));
			sendByte('\r');sendByte('\n');
			
			crc=crc8(serial_number,7);//Compute the CRC
			if(crc==serial_number[7]){
				sendString("CRC CHECK: Successful\r\n\n");
			}else{
				sendString("CRC CHECK: Failed\r\n\n");
			}
			
			//erase the array
			for (i=0;i<8;i++){
				serial_number[i]=0x00;
			} /*end of for*/
		} /* end of if */
	} /* end of while */
	return 0;
}/*end of main */

