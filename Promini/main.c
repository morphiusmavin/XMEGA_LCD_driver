// test PORTC as inputs - to use round rocker switch: gold pin is gnd, middle is 5v
// lower pin goes to input with a 4k7 pull-down
#include <avr/io.h>
#include<avr/interrupt.h>
//#include "../avr8-gnu-toolchain-linux_x86/avr/include/util/delay.h"
#include "../../Atmel_other/avr8-gnu-toolchain-linux_x86/avr/include/util/delay.h"
#include "sfr_helper.h"
#include <avr/eeprom.h>
#include <stdlib.h>
#include "macros.h"
#include <string.h>
#include "main.h"
#include "USART.h"
#include <stdio.h>
#include <stdlib.h>

volatile int dc2;
volatile UCHAR xbyte = 0x21;
volatile int test = 0;
volatile int input_changed = 0;
volatile UCHAR status = 0;
volatile UCHAR bit_changed = 0;
volatile UCHAR prev_status = 0;
volatile int onoff = 0;

#define false 0
#define true 1
#define LED PB5

#define RPM_CMD 0xFF
#define MPH_CMD 0xFE
#define RPM_BR_CMD 0xFD
#define MPH_BR_CMD 0xFC

// timer int's every 50ms
ISR(TIMER1_OVF_vect)
{
	TCNT1 = 0x37FF;

	dc2++;

	if(dc2 % 5 == 0)
	{
		if(test == 0)
		{
			PORTB |= (1 << LED);
			test = 1;
		}
		else
		{
			PORTB &= ~(1 << LED);
			test = 0;
		}
	}
}

int main(void)
{
	int i,j,k;
	int rpm, mph;
	UCHAR out_array[10];
	UCHAR bright = 100;
	UINT temp;

//	TCCR1B = (1<<CS10) | (1<<CS12);;  // Timer mode with 1024 prescler
//	TCCR1B = (1<<CS11) | (1<<CS10);
//	TCCR1B = (1<<CS12);
	TCCR1B = (1<<CS11);
	TIMSK1 = (1 << TOIE1) ;   // Enable timer1 overflow interrupt(TOIE1)

	sei(); // Enable global interrupts by setting global interrupt enable bit in SREG

	initUSART();

	i = 0;
	j = 0;
	k = 0;
	rpm = 0;
	mph = 1;
	memset(out_array,0,sizeof(out_array));
	_delay_ms(200);

	out_array[0] = MPH_BR_CMD;
	out_array[1] = bright;
	out_array[2] = 0;

	for(i = 0;i < 2;i++)
	{
		transmitByte(out_array[i]);
		_delay_us(100);
	}

	while(true)
	{
#if 0
		if(1)
//		if((k % 2) == 0)
		{
			out_array[0] = RPM_CMD;
			temp = (UINT)rpm;
			out_array[1] = (UCHAR)temp;
			temp >>= 8;
			out_array[2] = (UCHAR)temp;
			out_array[3] = 0;
			for(i = 0;i < 5;i++)
			{
				transmitByte(out_array[i]);
				_delay_us(1000);
			}
			if(++rpm > 5000)
				rpm = 0;
		}
#endif
//		if((k % 3) == 0)
		_delay_ms(20);
		if(1)
		{
			if(--mph < 0)
			{
				mph = 1150;
			}
			out_array[0] = MPH_CMD;
			temp = (UINT)mph;
			out_array[1] = (UCHAR)temp;
			temp >>= 8;
			out_array[2] = (UCHAR)temp;
			out_array[3] = 0;
			for(i = 0;i < 5;i++)
			{
				transmitByte(out_array[i]);
				_delay_us(1000);
			}
		}

		if(++j > 1)
		{
			out_array[0] = MPH_BR_CMD;
			out_array[1] = bright;
			out_array[2] = 0;

			for(i = 0;i < 2;i++)
			{
				transmitByte(out_array[i]);
				_delay_us(100);
			}
			if(--bright < 1)
			{
				bright = 100;
				_delay_ms(100);
			}
			j = 0;
		}
		if(mph < 13)
			_delay_ms(1000);
		_delay_ms(20);

		if(mph == 10)
		{
			out_array[0] = MPH_BR_CMD;
			out_array[1] = 100;
			out_array[2] = 0;

			for(i = 0;i < 2;i++)
			{
				transmitByte(out_array[i]);
				_delay_us(100);
			}
		}
		k++;
	}
	return 0;
}
