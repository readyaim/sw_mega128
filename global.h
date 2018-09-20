/****************************************************************************
* File name: global.h
* Description: global head file
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

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


/* command.h*/
#define GENERAL 0
#define UPDATETIME 2
#define ShowNumOnLED 3
#define UPDATE_transInterval_g 4
#define SAVE2EEPROM 5
#define READEEPROM 6

#define SETMEASUREMETHOD 1

#define SELECTEXPENDCHANNEL 8


#define SETCCPWM 7

#define FifoLength 150
#define ChannelCount 60
#define BoardCount 6

/* uart.h start*/
#define RXC1_BUFF_SIZE 128   //接受缓冲区字节数
#define UART1_TX_BUFFER_SIZE 320   /*发送缓冲区字节数*/
/*
#define UART1_TX_BUFFER_MASK ( UART1_TX_BUFFER_SIZE - 1 )
#if ( UART1_TX_BUFFER_SIZE & UART1_TX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
*/





extern UINT16 TXC1_RD;   //发送缓冲区读指针
extern UINT16 TXC1_WR;   //发送缓冲区写指针
/* uart.h end*/
extern void beep(void);
extern void delay_us(UINT16 microsecond);
extern void delay_ms(UINT16 millisecond);

/* timer.c Timer1 compA interrupt interval, in ms*/
#define TICKERTIME 200
#define CLKDIV 256
#define TCNTVALUE (CPU_CLK*TICKERTIME/256/1000) 

/* data_collection.h start*/
#define DAYTICKERTIME (12*60*60*1000/TICKERTIME)
#define HOURTICKERTIME (60*60*1000/TICKERTIME)
#define MINTICKERTIME (60*1000/TICKERTIME)
extern UINT8 transInterval_g;



//#define SECTICKERTIME (1000/TICKERTIME)
/* data_collection.h end*/


/* eeprom.h start*/
#define EEPROM_DATA_SIZE 256
#define START_ADDR_EEPROM EEPROM_DATA_SIZE
#define END_ADDR_EEPROM 0xFFF
#define MAX_DATA_ADDR_EEPPROM 0
#define MIN_DATA_ADDR_EEPPROM EEPROM_DATA_SIZE

//#define TEMP_START_ADDR_EEPROM 0x0000
//#define TEMP_END_ADDR_EEPROM 0x1FFF

/* eeprom.h end*/

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
extern Date_t uploadTime_g;
typedef struct dataInEEPROM_t {
	Date_t time;
	UINT16 data;

}dataInEEPROM_t;
extern struct dataInEEPROM_t dataInRom_g, dataInRom_max_g, dataInRom_min_g;




typedef struct DataTimeSeries_t {
	dataInEEPROM_t temp;
	dataInEEPROM_t humidity;
	dataInEEPROM_t airPressure;
	dataInEEPROM_t groundTemp;
	dataInEEPROM_t radiation;
	dataInEEPROM_t rain;
	dataInEEPROM_t sunShineTime;
	dataInEEPROM_t evaporation;
	//dataInEEPROM_t windSpeed;
	//dataInEEPROM_t windDirection;
	dataInEEPROM_t windSpeed1m;
	dataInEEPROM_t windSpeed2m;
	dataInEEPROM_t windSpeed10m;
	dataInEEPROM_t windDirection1m;
	dataInEEPROM_t windDirection2m;
	dataInEEPROM_t windDirection10m;

}DataTimeSeries_t;
extern struct DataTimeSeries_t dataSample_g, dataSample_max_g, dataSample_min_g;

/*
TODO:
//change the data type as below:
UINT16 dataArraySample_g[14]
#define IDX_TEMP 0
#define IDX_HUMIDITY 1
...


*/




typedef struct DataSeries_t {
	UINT16 temp;
	UINT16 humidity;
	UINT16 airPressure;
	UINT16 groundTemp;
	UINT16 radiation;
	UINT16 windSpeed;
	UINT16 windDirection;
	UINT16 rain;
	UINT16 sunShineTime;
	UINT16 evaporation;
}DataSeries_t;

typedef struct TimeStamp_t {
	Date_t time;
	UINT32 tickeCounter;
	UINT16 currentAddrEEPROM;
	UINT16 pageSize;
	UINT8 flag;		//1, new Time is updated, for get_current_time()
}TimeStamp_t;
extern struct TimeStamp_t timeStampShot_g;
/*
enum TransIntervalMode_t {
	min5 = 3,
	min10 = 10,
	min30 = 30,
	min60 = 60
};
*/
/*typedef struct TimeAddr_t {
	Date_t time;
	UINT16 addr;
	UINT8 size;		//2 bytes, might be possibly removed later
	UINT8 grid;
}TimeAddr_t;
*/


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










