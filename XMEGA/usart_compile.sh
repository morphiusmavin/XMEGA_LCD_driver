rm *.hex *.o *.elf *.map


avr-gcc -mmcu=atxmega128a4u -DF_CPU=16000000UL -I. -gstabs   -Os -Wall -Wstrict-prototypes -std=gnu99  -c -o main.o main.c &> out.txt

avr-gcc -mmcu=atxmega128a4u -DF_CPU=16000000UL -I. -gstabs   -Os -Wall -Wstrict-prototypes -std=gnu99  -c -o usart_driver.o usart_driver.c &>> out.txt

avr-gcc -Wl,-Map,main.map -Wl,--gc-sections -mmcu=atxmega128a4u main.o usart_driver.o  -o main.elf &>> out.txt
#avr-gcc -Wl,-Map,main.map -Wl,--gc-sections -mmcu=atxmega128a4u main.o  -o main.elf

avr-objcopy -j .text -j .data -O ihex main.elf main.hex &>> out.txt

#avr-gcc -Os -g -std=gnu99 -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -DMAIN_C -DF_CPU=8200000UL	 -DBAUD=19200UL -I. -Ilib -mmcu=atxmega128a4u -c -o USART.o USART.c;

#avr-gcc -Os -g -std=gnu99 -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -DMAIN_C -DF_CPU=8200000UL	 -DBAUD=19200UL -I. -Ilib -mmcu=atmega328p -c -o t6963.o t6963.c;

#avr-gcc -Os -g -std=gnu99 -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -DMAIN_C -DF_CPU=8200000UL	 -DBAUD=19200UL -I. -Ilib -mmcu=atxmega128a4u -c -o main.o main.c;

#avr-gcc -Wl,-Map,main.map -Wl,--gc-sections -mmcu=atmega328p USART.o t6963.o main.o  -o main.elf
#avr-gcc -Wl,-Map,main.map -Wl,--gc-sections -mmcu=atxmega128a4u USART.o main.o  -o main.elf
#avr-objcopy -j .text -j .data -O ihex main.elf main.hex

#rm *.o *.map *.elf

if grep -q error out.txt
 find2 error out.txt
 then exit 1
fi

sudo dfu-programmer atxmega128a4u erase
sudo dfu-programmer atxmega128a4u flash --erase-first main.hex

