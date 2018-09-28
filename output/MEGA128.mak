CC = iccavr
LIB = ilibw
CFLAGS =  -e -D__ICC_VERSION=722 -DATMega128  -l -A -A -g -MLongJump -MHasMul -MEnhanced -Wf-use_elpm -Wf-const_is_flash -DCONST="" 
ASFLAGS = $(CFLAGS) 
LFLAGS =  -g -e:0x20000 -ucrtatmega.o -bfunc_lit:0x8c.0x20000 -dram_end:0x10ff -bdata:0x100.0x10ff -dhwstk_size:60 -beeprom:0.4096 -fihx_coff -S2
FILES = adc.o command.o fifo.o keys.o main.o peripherals.o timer.o twi.o uart.o spi.o usr_gprs232_7sr.o data_collection.o eeprom.o iostudio.o modem.o 

MEGA128:	$(FILES)
	$(CC) -o MEGA128 $(LFLAGS) @MEGA128.lk   -llpatm128 -lcatm128
adc.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\peripherals.h
adc.o:	..\adc.c
	$(CC) -c $(CFLAGS) ..\adc.c
command.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
command.o:	..\command.c
	$(CC) -c $(CFLAGS) ..\command.c
fifo.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
fifo.o:	..\fifo.c
	$(CC) -c $(CFLAGS) ..\fifo.c
keys.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\peripherals.h
keys.o:	..\keys.c
	$(CC) -c $(CFLAGS) ..\keys.c
main.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
main.o:	..\main.c
	$(CC) -c $(CFLAGS) ..\main.c
peripherals.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\peripherals.h
peripherals.o:	..\peripherals.c
	$(CC) -c $(CFLAGS) ..\peripherals.c
timer.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\peripherals.h
timer.o:	..\timer.c
	$(CC) -c $(CFLAGS) ..\timer.c
twi.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\TWI.h
twi.o:	..\twi.c
	$(CC) -c $(CFLAGS) ..\twi.c
uart.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
uart.o:	..\uart.c
	$(CC) -c $(CFLAGS) ..\uart.c
spi.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\peripherals.h
spi.o:	..\spi.c
	$(CC) -c $(CFLAGS) ..\spi.c
usr_gprs232_7sr.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
usr_gprs232_7sr.o:	..\usr_gprs232_7sr.c
	$(CC) -c $(CFLAGS) ..\usr_gprs232_7sr.c
data_collection.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h D:\PROGRA~1\iccv7avr\include\stdlib.h D:\PROGRA~1\iccv7avr\include\limits.h D:\PROGRA~1\iccv7avr\include\math.h
data_collection.o:	..\data_collection.c
	$(CC) -c $(CFLAGS) ..\data_collection.c
eeprom.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h .\..\..\..\..\..\PROGRA~1\iccv7avr\include\eeprom.h
eeprom.o:	..\eeprom.c
	$(CC) -c $(CFLAGS) ..\eeprom.c
iostudio.o:	..\iostudio.s
	$(CC) -c $(ASFLAGS) ..\iostudio.s
modem.o: .\..\global.h D:\PROGRA~1\iccv7avr\include\stdio.h D:\PROGRA~1\iccv7avr\include\stdarg.h D:\PROGRA~1\iccv7avr\include\_const.h .\..\AVRdef_l.h .\..\iom128v_l.h D:\PROGRA~1\iccv7avr\include\string.h
modem.o:	..\modem.c
	$(CC) -c $(CFLAGS) ..\modem.c
