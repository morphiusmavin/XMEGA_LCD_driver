/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA USART interrupt driven driver example source.
 *
 *      This file contains an example application that demonstrates the
 *      interrupt driven USART driver. The code example sends three bytes, waits
 *      for three bytes to be received and tests if the received data equals the
 *      sent data.
 *
 * \par Application note:
 *      AVR1307: Using the XMEGA USART
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1694 $
 * $Date: 2008-07-29 14:21:58 +0200 (ti, 29 jul 2008) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "usart_driver.h"
#include "TC_driver.h"
#include "avr_compiler.h"
#include <string.h>

uint32_t dec2bcd_r(uint16_t dec);
uint32_t dec2bcd(uint16_t dec);
#define USART1 USARTC0
#define USART2 USARTD0
//#define USART3 USARTE0
void process_digits(uint16_t val, int uart);
void sendChar(UCHAR ch, int which);
void clear_display(int uart);
void special_cmd(int type, int ttimes);
void decimal_cmd(int dec_place);
// commands to LED modules (see: https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands)
#define LED_CLRDISP		0x76
#define LED_DECIMAL		0X77
#define LED_CURSOR		0x79
#define LED_BRIGHT		0x7A
#define LED_BAUDRATE	0x7F
#define LED_RESET		0x81
#define LED_ZEPPLIN		0xFF

// which display to control
#define RPM_DISPLAY		0
#define MPH_DISPLAY		1

// commands received on comm port
#define RPM_CMD 0xFF		// display int value
#define MPH_CMD 0xFE
#define RPM_BR_CMD 0xFD		// adjust brightness
#define MPH_BR_CMD 0xFC
#define RPM_CL_CMD 0xFB		// clear display
#define MPH_CL_CMD 0xFA
#define SPECIAL_CMD 0xF9	// display pattern on both displays
#define MPH_DEC_CMD 0xF7

/* baudrate params:
0 	2400
1 	4800
2 	9600
3 	14400
4 	19200
5 	38400
6 	57600
7 	76800
8 	115200
9 	250000
10 	500000
11 	1000000
*/

USART_data_t USART_data1;
USART_data_t USART_data2;

static uint16_t rpm, mph;
static int state;
static 	uint16_t temp;
static int ttimes;
enum
{
	IDLE,
	RPM_LOW_BYTE,
	RPM_HIGH_BYTE,
	MPH_LOW_BYTE,
	MPH_HIGH_BYTE,
	RPM_BRIGHT,
	MPH_BRIGHT,
	RPM_CLR,
	MPH_CLR,
	SPEC_CMD,
	SPEC_CMD2,
	DEC_CMD,
	DEC_CMD2,
	DONE_RPM,
	DONE_MPH
}STATES;

/*********************************************************************************************/
int main(void)
{
	UCHAR bright;
	int j;

	PORTC.DIRSET   = PIN3_bm;
	PORTC.DIRCLR   = PIN2_bm;
	PORTD.DIRSET   = PIN3_bm;
	PORTD.DIRCLR   = PIN2_bm;

	PORTE.DIRSET = 2;

	USART_InterruptDriver_Initialize(&USART_data1, &USART1, USART_DREINTLVL_LO_gc);
	USART_InterruptDriver_Initialize(&USART_data2, &USART2, USART_DREINTLVL_LO_gc);

	USART_Format_Set(USART_data1.usart, USART_CHSIZE_8BIT_gc,USART_PMODE_DISABLED_gc, false);
	USART_Format_Set(USART_data2.usart, USART_CHSIZE_8BIT_gc,USART_PMODE_DISABLED_gc, false);

	USART_RxdInterruptLevel_Set(USART_data1.usart, USART_RXCINTLVL_LO_gc);
	USART_RxdInterruptLevel_Set(USART_data2.usart, USART_RXCINTLVL_LO_gc);

/* Set Baudrate to 19200 bps:
 * Use the default I/O clock frequency that is 2 MHz.
 * Do not use the baudrate scale factor
 *
 * Baudrate select = (1/(16*(((I/O clock frequency)/Baudrate)-1)
 * USART_Baudrate_Set(&USART1, 12 , 0);	(for 9600)
 */
	USART_Baudrate_Set(&USART1, 705 , -7);	// 19200 bps
	USART_Baudrate_Set(&USART2, 705 , -7);
//	USART_Baudrate_Set(&USART1, 12 , 0);

/* Enable both RX a TX. */
//	USART_Rx_Enable(USART_data1.usart);
	USART_Tx_Enable(USART_data1.usart);

//	USART_Rx_Enable(USART_data2.usart);
	USART_Tx_Enable(USART_data2.usart);

	/* Set period/TOP value. */
//	TC_SetPeriod( &TCC0, 61 );		// 1/32 second
//	TC_SetPeriod( &TCC0, 122 );		// 1/16 second
//	TC_SetPeriod( &TCC0, 244 );		// 1/8 second
	TC_SetPeriod( &TCC0, 488 );		// 1/4 second
//	TC_SetPeriod( &TCC0, 976 );		// 1/2 second
//	TC_SetPeriod( &TCC0, 1953 );	// one second
//	TC_SetPeriod( &TCC0, 3906 );	// two seconds

	/* Select clock source. */
	TC0_ConfigClockSource( &TCC0, TC_CLKSEL_DIV1024_gc );
	TC0_ConfigWGM(&TCC0, TC_WGMODE_NORMAL_gc);
	TC0_SetOverflowIntLevel(&TCC0, TC_OVFINTLVL_HI_gc);

/* Enable PMIC interrupt level low. */
	PMIC.CTRL |= PMIC_LOLVLEX_bm;
	PMIC.CTRL |= PMIC_HILVLEN_bm;

/* Enable global interrupts. */
	sei();


/* sometimes the LCD baudrate gets reset to factory 9600
	so if that happens, set the baudrate above to 9600 and
	uncomment the following:
*/
/*	
	sendChar(LED_BAUDRATE,MPH_DISPLAY);
	sendChar(4,MPH_DISPLAY);

	while(1)
	{
		_delay_ms(10);
	}	
*/

	sendChar(LED_CURSOR,MPH_DISPLAY);
	sendChar(0,MPH_DISPLAY);
	sendChar(0x10,MPH_DISPLAY);
	sendChar(LED_CURSOR,MPH_DISPLAY);
	sendChar(1,MPH_DISPLAY);
	sendChar(0x10,MPH_DISPLAY);
	sendChar(LED_CURSOR,MPH_DISPLAY);
	sendChar(2,MPH_DISPLAY);
	sendChar(0x10,MPH_DISPLAY);
	sendChar(LED_CURSOR,MPH_DISPLAY);
	sendChar(3,MPH_DISPLAY);
	sendChar(0x10,MPH_DISPLAY);
	_delay_ms(100);
	bright = 100;
	sendChar(LED_BRIGHT, MPH_DISPLAY);
	_delay_ms(1);
	sendChar(bright, MPH_DISPLAY);
	_delay_ms(1);
	sendChar(LED_BRIGHT, RPM_DISPLAY);
	_delay_ms(1);
	sendChar(bright, RPM_DISPLAY);
	_delay_ms(1);
	special_cmd(1,1);
	_delay_ms(1);
	special_cmd(2,1);
	_delay_ms(1);
	special_cmd(0,1);
	_delay_ms(1);
	decimal_cmd(1);
	_delay_ms(300);
	decimal_cmd(2);
	_delay_ms(300);
	decimal_cmd(3);
	_delay_ms(300);
	decimal_cmd(4);
	_delay_ms(300);
	decimal_cmd(3);
	_delay_ms(300);
	decimal_cmd(2);
	_delay_ms(300);
	decimal_cmd(1);
	sendChar(LED_CLRDISP,MPH_DISPLAY);
	sendChar(LED_CLRDISP,RPM_DISPLAY);
	_delay_ms(1);

	state = IDLE;

	USART_Rx_Enable(USART_data1.usart);
	mph = 2000;
	rpm = 0;
	USART_Rx_Enable(USART_data2.usart);

	for(j = 0;j < 10;j++)
	{
		PORTE_OUTTGL = 2;
		_delay_ms(10);
	}

	// to test, comment out the USART_Rx_Enable's above and
	// uncomment the code below

	while(1)
	{
/*
		if(mph > 20)
		{
			_delay_ms(3);
			mph -= 19;
		}else
		{
			_delay_ms(100);
			mph--;
			if(mph < 1)
			{
				special_cmd(1,1);
				special_cmd(2,1);
				special_cmd(0,1);
				clear_display(MPH_DISPLAY);
				_delay_ms(500);
				mph = 2000;
			}
		}
//		process_digits(rpm,RPM_DISPLAY);
//		_delay_ms(10);
		process_digits(mph,MPH_DISPLAY);
*/
//		PORTE_OUTTGL = 2;
		_delay_ms(1000);
	}
}

/*********************************************************************************************/
ISR(USARTC0_RXC_vect)	// USART1 is the data/cmd channel for the rpm LED
{
	UCHAR ch;
	USART_RXComplete(&USART_data1);
	ch = USART_RXBuffer_GetByte(&USART_data1);

	switch(state)
	{
		case IDLE:
			switch(ch)
			{
				case RPM_CMD:
					state = RPM_LOW_BYTE;
					rpm = 0;
					break;
				case MPH_CMD:
					state = MPH_LOW_BYTE;
					mph = 0;
					break;
				case RPM_BR_CMD:
					state = RPM_BRIGHT;
					break;
				case MPH_BR_CMD:
					state = MPH_BRIGHT;
					break;
				case MPH_CL_CMD:
					state = MPH_CLR;
					break;	
				case RPM_CL_CMD:
					state = RPM_CLR;
					break;
				case MPH_DEC_CMD:
					state = DEC_CMD;
					break;
				case SPECIAL_CMD:
					state = SPEC_CMD;
					break;
				default:	
					break;
			}
			break;
		case RPM_LOW_BYTE:
			rpm = (uint16_t)ch;
			state = RPM_HIGH_BYTE;
			break;
		case RPM_HIGH_BYTE:
			temp = (uint16_t)ch;
			temp <<= 8;
			rpm |= temp;
			state = DONE_RPM;
			break;
		case MPH_LOW_BYTE:
			mph = (uint16_t)ch;
			state = MPH_HIGH_BYTE;
			break;
		case MPH_HIGH_BYTE:
			temp = (uint16_t)ch;
			temp <<= 8;
			mph |= temp;
			state = DONE_MPH;
			break;
		case RPM_BRIGHT:
			sendChar(LED_BRIGHT, RPM_DISPLAY);
			_delay_ms(1);
			sendChar(ch, RPM_DISPLAY);
			_delay_ms(1);
			state = IDLE;
			break;
		case MPH_BRIGHT:
			sendChar(LED_BRIGHT, MPH_DISPLAY);
			_delay_ms(1);
			sendChar(ch, MPH_DISPLAY);
			_delay_ms(1);
			state = IDLE;
			break;
		case RPM_CLR:
			clear_display(RPM_DISPLAY);
			state = IDLE;
			break;
		case MPH_CLR:
			clear_display(MPH_DISPLAY);
			state = IDLE;
			break;
		case SPEC_CMD:
			state = SPEC_CMD2;
			break;
		case SPEC_CMD2:
			special_cmd((int)ch,1);
//			PORTE_OUTTGL = 2;
			state = IDLE;
			break;
		case DEC_CMD:
			state = DEC_CMD2;
			break;
		case DEC_CMD2:
			decimal_cmd((int)ch);
//			PORTE_OUTTGL = 2;
			state = IDLE;
			break;	
		case DONE_RPM:
			process_digits(rpm, RPM_DISPLAY);
			state = IDLE;
			break;
		case DONE_MPH:
			process_digits(mph, MPH_DISPLAY);
			state = IDLE;
			break;
		default:
			break;
	}
}
/*********************************************************************************************/
ISR(USARTD0_RXC_vect)	// USART2
{
	UCHAR ch;
	USART_RXComplete(&USART_data2);
	ch = USART_RXBuffer_GetByte(&USART_data2);
//	USART_TXBuffer_PutByte(&USART_data2, ch);
}
/*********************************************************************************************/
ISR(USARTC0_DRE_vect)
{
	USART_DataRegEmpty(&USART_data1);
}
/*********************************************************************************************/
ISR(USARTD0_DRE_vect)
{
	USART_DataRegEmpty(&USART_data2);
}
/*********************************************************************************************/
ISR(TCC0_OVF_vect)
{
}
/*********************************************************************************************/
void sendChar(UCHAR ch, int which)
{
	if(which == MPH_DISPLAY)
	{
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,ch);

	}
/*
	if(which == RPM_DISPLAY)
	{
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,ch);
	}
*/
	_delay_ms(3);
}
/*********************************************************************************************/
void process_digits(uint16_t val, int uart)
{
	UCHAR buff[10];
	int i;
	uint16_t utemp;
	UCHAR temp;
	int digit_ptr;
	uint32_t res;
	int digit_len;

	digit_ptr = 0;
	res = dec2bcd(val);
	utemp = (uint16_t)res;

	temp = (UCHAR)(utemp & 0x000f);
	buff[digit_ptr++] = temp;

	utemp >>= 4;
	temp = (UCHAR)(utemp & 0x000f);
	buff[digit_ptr++] = temp;

	utemp >>= 4;
	temp = (UCHAR)(utemp & 0x000f);
	buff[digit_ptr++] = temp;

	utemp >>= 4;
	temp = (UCHAR)(utemp & 0x000f);
	buff[digit_ptr] = temp;

//	sendChar(LED_CLRDISP,uart);

	if(val < 1000 && val > 99)
	{
		sendChar(LED_CURSOR,uart);
		sendChar(0,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(1,uart);
		digit_len = 2;
	}
	else if(val < 100 && val > 9)
	{
		sendChar(LED_CURSOR,uart);
		sendChar(0,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(1,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(2,uart);
		digit_len = 1;
	}
	else if(val < 10)
	{
		sendChar(LED_CURSOR,uart);
		sendChar(0,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(1,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(2,uart);
		sendChar(0x10,uart);
		sendChar(LED_CURSOR,uart);
		sendChar(3,uart);
		digit_len = 0;
	}
	else
	{
		sendChar(LED_CURSOR,uart);
		sendChar(0,uart);
		digit_len = 3;
	}

	for(i = digit_len;i >= 0;i--)
	{
		sendChar(buff[i],uart);
	}
	sendChar(LED_CURSOR,uart);
	sendChar(0,uart);
}
/*********************************************************************************************/
uint32_t dec2bcd(uint16_t dec)
{
    uint32_t result = 0;
    int shift = 0;

    while (dec)
    {
        result +=  (dec % 10) << shift;
        dec = dec / 10;
        shift += 4;
    }
    return result;
}

// does the same as above
/*********************************************************************************************/
uint32_t dec2bcd_r(uint16_t dec)
{
    return (dec) ? ((dec2bcd_r( dec / 10 ) << 4) + (dec % 10)) : 0;
}

void clear_display(int uart)
{
	sendChar(LED_CURSOR,uart);
	sendChar(0,uart);
	sendChar(0x10,uart);
	sendChar(0x10,uart);
	sendChar(0x10,uart);
	sendChar(0x10,uart);
	sendChar(LED_CURSOR,uart);
	sendChar(0,uart);
}
/*********************************************************************************************/
void special_cmd(int type, int ttimes)
{
	UCHAR mask_array[8];
	UCHAR temp_array[8];
	int i,j,k;
	int no_times = 7;
	int tno_times = no_times*ttimes;

	memset(mask_array,0,sizeof(mask_array));

/*	segments:
				1
			6		2
				7
			5		3
				4	

bit:	7	6	5	4	3	2	1	0
seg:	x	7	6	5	4	3	2	1

	to do figure '8':   1,2,7,5,4,3,7,6
						0,1,6,4,3,2,6,5
	to do circle:       1,2,3,4,5,6
						0,1,2,3,4,5
*/

	mask_array[0] = 1;
	mask_array[1] = 2;
	mask_array[2] = 0x40;
	mask_array[3] = 0x10;
	mask_array[4] = 8;
	mask_array[5] = 4;
	mask_array[6] = 0x40;
	mask_array[7] = 0x20;

	switch(type)
	{
		case 0:
			mask_array[0] = 1;
			mask_array[1] = 2;
			mask_array[2] = 0x40;
			mask_array[3] = 0x10;
			mask_array[4] = 8;
			mask_array[5] = 4;
			mask_array[6] = 0x40;
			mask_array[7] = 0x20;
			break;
		case 1:
			mask_array[0] = 1;
			mask_array[1] = 2;
			mask_array[2] = 4;
			mask_array[3] = 8;
			mask_array[4] = 0x10;
			mask_array[5] = 0x20;
			mask_array[6] = 0x20;
			mask_array[7] = 0;
			no_times = 5;
			break;
		case 2:	
			mask_array[7] = 1;
			mask_array[6] = 1;
			mask_array[5] = 1;
			mask_array[4] = 2;
			mask_array[3] = 4;
			mask_array[2] = 8;
			mask_array[1] = 0x10;
			mask_array[0] = 0x20;
			no_times = 5;
		default:
			break;
	}
	memcpy(temp_array,mask_array,8);
	j = 0;
	tno_times = no_times*10;
	for(i = 0;i < tno_times;i++)
	{
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,0x7B);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,mask_array[j]);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,0x7C);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,mask_array[j]);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,0x7D);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,mask_array[j]);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,0x7E);
		_delay_ms(1);
		if(USART_IsTXDataRegisterEmpty(&USART1))
			USART_PutChar(&USART1,mask_array[j]);
/*
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,0x7B);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,mask_array[j]);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,0x7C);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,mask_array[j]);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,0x7D);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,mask_array[j]);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,0x7E);
		_delay_ms(10);
		if(USART_IsTXDataRegisterEmpty(&USART2))
			USART_PutChar(&USART2,mask_array[j]);
*/
		if(++j > no_times)
			j = 0;

		if(type == 0)
			_delay_ms(30);
		else
			_delay_ms(20);

		if(type == 0 && i == tno_times/2)		// reverse direction
		{
			for(k = 0;k < 8;k++)
			{
				mask_array[k] = temp_array[8-k];
			}
		}
	}
}
/*********************************************************************************************/
// 0 - off; 1 - dec pnt after digit 1; 2 - after digit 2; 3 - digit 3; 4 - 4
void decimal_cmd(int dec_place)
{
	UCHAR param = 0;

	if(USART_IsTXDataRegisterEmpty(&USART1))
		USART_PutChar(&USART1,LED_DECIMAL);
	_delay_ms(1);

	switch(dec_place)
	{
		case 0:
			param = 0;
			break;
		case 1:
			param = 1;
			break;
		case 2:
			param = 2;
			break;
		case 3:
			param = 4;
			break;
		case 4:
			param = 8;
			break;
		default:
			PORTE_OUTTGL = 2;
			param = 0;
			break;
	}
	_delay_ms(1);
	if(USART_IsTXDataRegisterEmpty(&USART1))
		USART_PutChar(&USART1,param);
	_delay_ms(1);
}	

