#include <stdio.h>
//#include <iom64v.h>
#include <iom128v.h>
#include <AVRdef.h>
#include <string.h>

#ifndef GLOBAL_H
#define GLOBAL_H

#define UINT8 unsigned char
#define INT8 signed char
#define UINT16 unsigned int
#define INT16 signed int
#define UINT32 unsigned long
#define INT32  signed long
#define BOOL unsigned char
#define true 1
#define false 0

#define CPU_CLK 8000000 //CLOCK, 8MHz    

#define LED_SlowFlash 2
#define LED_QuickFlash 0

//#define ConstantVoltagePWM OCR3C
#define ConstantCurrentPWM OCR3A
#define LEDPWM OCR3B
#define BEEPERPWM OCR1B
//#define VREFPWM OCR1CL

#define GENERAL 0
#define ShowNumOnLED 3
#define CollectData 4
#define SAVE2EEPROM 5
#define READEEPROM 6

#define SETMEASUREMETHOD 1
#define SELECTCHANNEL 2
#define SELECTEXPENDCHANNEL 8


#define SETCCPWM 7

#define FifoLength 150
#define ChannelCount 60
#define BoardCount 6




extern void beep(void);
extern void delay_us(UINT16 microsecond);
extern void delay_ms(UINT16 millisecond);


#define Set_Bit(val, bitn)    (val |=(1<<(bitn))) 
#define Clr_Bit(val, bitn)    (val&=~(1<<(bitn))) 
#define Get_Bit(val, bitn)    (val &(1<<(bitn))) 

//#define TIMER0_ADC 
#define _ATMEGA128A
#define _DUMMY_CODE

extern BOOL TimeIsUp(UINT32 StartTime, UINT32 Delay);
extern void Delay(UINT32);
extern struct Fifo CommandFifo;
extern UINT32 SystemTickCount;


typedef struct Fifo {
	UINT8 fifodata[FifoLength];
	UINT8 start;
	UINT8 end;
	BOOL(*IsEmpty)(struct Fifo*);
	BOOL(*AddFifo)(struct Fifo *, UINT8);
	UINT8(*FetchFifo)(struct Fifo *);
	void(*ClearFifo)(struct Fifo *);
}FIFO;


typedef struct Date_t{
	UINT8 year1;
	UINT8 year;
	UINT8 mon;
	UINT8 day;
	UINT8 hour;
	UINT8 min;
}Date_t;

typedef struct dataInEEPROM_t {
	Date_t time;
	UINT16 data;

}dataInEEPROM_t;

typedef struct dataSeries_t {

	UINT8 data;
	UINT8 index;
}dataSeries_t;
extern struct dataInEEPROM_t dataIneeprom, dataInRom_max, dataInRom_min;

typedef struct TimeStamp_t {
	Date_t time;
	UINT32 tickeCounter;
}TimeStamp_t;
extern struct TimeStamp_t timestamp;
/* for debug only*/
//#define _SPI_TX
//#define _SPI_MASTER

//typedef struct adscale {
//    UINT8 Sensor;//sensor No.
//    float Resistance;//sensor resistor
//    UINT32 CCPWM;//CCPWM for calibrate
//    float CaliCurrent;//calibrate current
//    float NextScaleCurrent;//switch to lower scale
//    UINT8 ZeroOffsetMux;
//    UINT8 ChannelMux;
//    float MaxCurrent;
//    float ADFactor;
//    UINT8 OverSamplingCount;
//}ADSCALE;
//
//typedef struct Result {
//    float Max;
//    float Avg;
//    float Min;
//}RESULT;


#endif // !GLOBAL_H










