<h1>XMEGA atxmega128a4u used to drive (2) LED modules to show rpm & mph on dash</h1>
The XMEGA uses USART1 (pin D2) as a serial input 

commands to LED modules (see: https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands)

LED_CLRDISP		0x76
LED_DECIMAL		0x77
LED_CURSOR		0x79
LED_BRIGHT		0x7A
LED_BAUDRATE	0x7F
LED_RESET		0x81

// which display to control
#define RPM_DISPLAY		0
#define MPH_DISPLAY		1

// commands received on comm port
RPM_CMD 0xFF		- display int value
MPH_CMD 0xFE
RPM_BR_CMD 0xFD		- adjust brightness
MPH_BR_CMD 0xFC
RPM_CL_CMD 0xFB		- clear display
MPH_CL_CMD 0xFA
SPECIAL_CMD 0xF9	- display pattern on both displays

the parameters for the XXX_BR_CMD's are 0-100, 0 is the dimmest

The TX's of USART's 1 & 2 go to the LED module (rpm/mph).
2) one is for mph and the other is rpm.
The RX's are interrupt driven. 
Only USART1 RX is used to receive data/cmds from the XMEGA.
The data stream is:
1) RPM_CMD or MPH_CMD - header (nothing else in data stream can be 0xFF)
2) low byte
3) high byte

pins on XMEGA 
D2 - extra serial port
D3 - left LED RX 
C2 - ?
C3 - right LED RX 
vcc pin is 5v while data pins are 3v3  

I am using an AVR Promini to test the XMEGA 
