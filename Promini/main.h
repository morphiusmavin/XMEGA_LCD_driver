
#define PORT_C0	PORTC0
#define PORT_C1	PORTC1
#define PORT_C2	PORTC2
#define PORT_C3	PORTC3
#define PORT_C4	PORTC4
#define PORT_C5	PORTC5

#define SET_PC0()		_SB(PORTC,PORT_C0)
#define SET_PC1()		_SB(PORTC,PORT_C1)
#define SET_PC2()		_SB(PORTC,PORT_C2)
#define SET_PC3()		_SB(PORTC,PORT_C3)
#define SET_PC4()		_SB(PORTC,PORT_C4)
#define SET_PC5()		_SB(PORTC,PORT_C5)

#define CLR_PC0()		_CB(PORTC,PORT_C0)
#define CLR_PC1()		_CB(PORTC,PORT_C1)
#define CLR_PC2()		_CB(PORTC,PORT_C2)
#define CLR_PC3()		_CB(PORTC,PORT_C3)
#define CLR_PC4()		_CB(PORTC,PORT_C4)
#define CLR_PC5()		_CB(PORTC,PORT_C5)

#define PORTX2	PORTD2
#define PORTX3	PORTD3
#define PORTX4	PORTD4
#define PORTX5	PORTD5
#define PORTX6	PORTD6
#define PORTX7	PORTD7

#define PORTY0	PORTB0
#define PORTY1	PORTB1
#define PORTY2	PORTB2
#define PORTY3	PORTB3
#define PORTY4	PORTB4
#define PORTY5	PORTB5

#define SET_P2()		_SB(PORTD,PORTX2)
#define SET_P3()		_SB(PORTD,PORTX3)
#define SET_P4()		_SB(PORTD,PORTX4)
#define SET_P5()		_SB(PORTD,PORTX5)
#define SET_P6()		_SB(PORTD,PORTX6)
#define SET_P7()		_SB(PORTD,PORTX7)

#define CLR_P2()		_CB(PORTD,PORTX2)
#define CLR_P3()		_CB(PORTD,PORTX3)
#define CLR_P4()		_CB(PORTD,PORTX4)
#define CLR_P5()		_CB(PORTD,PORTX5)
#define CLR_P6()		_CB(PORTD,PORTX6)
#define CLR_P7()		_CB(PORTD,PORTX7)

#define SET_B0()		_SB(PORTB,PORTY0)
#define SET_B1()		_SB(PORTB,PORTY1)
#define SET_B2()		_SB(PORTB,PORTY2)
#define SET_B3()		_SB(PORTB,PORTY3)
#define SET_B4()		_SB(PORTB,PORTY4)
#define SET_B5()		_SB(PORTB,PORTY5)

#define CLR_B0()		_CB(PORTB,PORTY0)
#define CLR_B1()		_CB(PORTB,PORTY1)
#define CLR_B2()		_CB(PORTB,PORTY2)
#define CLR_B3()		_CB(PORTB,PORTY3)
#define CLR_B4()		_CB(PORTB,PORTY4)
#define CLR_B5()		_CB(PORTB,PORTY5)

#define B_SIG0 PB0
#define B_SIG1 PB1
#define B_SIG2 PB2
#define B_SIG3 PB3
#define B_SIG4 PB4
#define B_SIG5 PB5

#define D_SIG2 PD2
#define D_SIG3 PD3
#define D_SIG4 PD4
#define D_SIG5 PD5
#define D_SIG6 PD6
#define D_SIG7 PD7

#define C_SIG0 PC0
#define C_SIG1 PC1
#define C_SIG2 PC2
#define C_SIG3 PC3

/*
bottom view w/ reset button on right
TX		RX		RESET		gnd		PD2		PD3		PD4		PD5		PD6		PD7		PB0		PB1
RAW		gnd		RESET		VCC		PC3		PC2		PC1		PC0		PB5		PB4		PB3		PB2
*/

