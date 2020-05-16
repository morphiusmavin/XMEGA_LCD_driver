<h1>XMEGA atxmega128a4u used to drive (2) LED modules to show rpm & mph on dash</h1>
The XMEGA uses USART1 (pin D2) as a serial input <br />
<br />
commands to LED modules (see: https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Special-Commands)<br />
<br />
LED_CLRDISP		0x76<br />
LED_DECIMAL		0x77<br />
LED_CURSOR		0x79<br />
LED_BRIGHT		0x7A<br />
LED_BAUDRATE	0x7F<br />
LED_RESET		0x81<br />
<br />
// which display to control<br />
#define RPM_DISPLAY		0<br />
#define MPH_DISPLAY		1<br />
<br />
// commands received on comm port<br />
RPM_CMD 0xFF		- display int value<br />
MPH_CMD 0xFE<br />
RPM_BR_CMD 0xFD		- adjust brightness<br />
MPH_BR_CMD 0xFC<br />
RPM_CL_CMD 0xFB		- clear display<br />
MPH_CL_CMD 0xFA<br />
SPECIAL_CMD 0xF9	- display pattern on both displays<br />
<br />
the parameters for the XXX_BR_CMD's are 0-100, 0 is the dimmest<br />
<br />
The TX's of USART's 1 & 2 go to the LED module (rpm/mph).<br />
2) one is for mph and the other is rpm.<br />
The RX's are interrupt driven. <br />
Only USART1 RX is used to receive data/cmds from the XMEGA.<br />
The data stream is:<br />
1) RPM_CMD or MPH_CMD - header (nothing else in data stream can be 0xFF)<br />
2) low byte<br />
3) high byte<br />
<br />
pins on XMEGA <br />
D2 - extra serial port<br />
D3 - left LED RX <br />
C2 - input<br />
C3 - right LED RX <br />
vcc pin is 5v while data pins are 3v3  <br />
<br />
I am using an AVR Promini to test the XMEGA<br />
note: the main.c in the Promini directory needs to point to the AVR toolchain directory<br />
