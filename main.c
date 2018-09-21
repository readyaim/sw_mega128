/****************************************************************************
* File name: main.c
* Description: main files for data collection system.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

/*********************************包含头文件********************************/
#include "global.h"
//#include "peripherals.h"
#include "fifo.h"


extern BOOL IsEmpty(struct Fifo *this);
extern BOOL AddFifo(struct Fifo *this, UINT8 data);
extern UINT8 FetchFifo(struct Fifo *this);
extern void ClearFifo(struct Fifo *this);
extern void ticker_processCmd(void);
extern void init_beep(void);
extern void init_port_adc0(void);
extern void resume_last_timeStampSlot(void);
//extern void timer0_init(void);
//extern void timer0_ovf_isr(void);
//extern void port_init_timer0(void);
extern void test_timer0(void);
extern void main_adc1(void);
extern void main_watchdog(void);
extern void main_uart0(void);
extern void main_uart1_loopback(void);
extern void timer2_processCmd(void);
extern void main_twi(void);
extern void main_spi_inquiry(void);
extern void main_spi_interrupt(void);
extern void test_spi_loop_Tx_inquiry(void);
extern void main_key2SEG(void);
extern void main_spi(void);
extern void init_SEG4(void);
extern void test_timer2(void);
extern void uart1_processCmd(void);
extern void init_uart0(void);
extern void uart1_init(void);
extern void test_timer1(void);
extern void test_EEPROM(void);
extern  void test_get_address(void);


#ifdef _DUMMY_CODE
extern void test_char2int(void);
extern void test_strcmp(void);
extern void test_usr(void);
extern void collectADC0(void);
extern void test_copystr2TimeStamp(void);

#endif // _DUMMY_CODE

UINT32 SystemTickCount;
struct dataInEEPROM_t dataInRom_g, dataInRom_max_g, dataInRom_min_g;
struct DataTimeSeries_t dataSample_g, dataSample_max_g, dataSample_min_g;
struct Fifo CommandFifo;
struct TimeStamp_t timeStampShot_g;
struct Date_t uploadTime_g;

//struct TimeAddr_t tempTimeAddr; //humidityTimeAddr, windSpeedTimeAddr...

BOOL TimeIsUp(UINT32 StartTime, UINT32 Delay)
{
    if (SystemTickCount > StartTime)
        if (SystemTickCount - StartTime > Delay) return true;
    if (SystemTickCount < StartTime)
        if ((4294967295 - StartTime + SystemTickCount) > Delay) return true;
    return false;
}


void init_vars(void)
{
	struct Date_t initTime = {20,18,9,14,23,11};


    SystemTickCount = 1;
	dataInRom_g.data = 0;

	//init max value
	dataInRom_max_g.data = 0;
	//init min value
	dataInRom_min_g.data = 0xFFFF;

/*
	tempTimeAddr.time = initTime;
	tempTimeAddr.addr = TEMP_START_ADDR_EEPROM;
	tempTimeAddr.size = 2;	//2 bytes, might be possibly removed later
	tempTimeAddr.grid = 60;	//60s sampling rate
*/
	dataSample_g.temp.time = initTime;
	dataSample_g.temp.data = 0;
	dataSample_g.humidity.time = initTime;
	dataSample_g.humidity.data = 0;
	dataSample_max_g.temp.data = 0;
	dataSample_min_g.temp.data = 0xFFFF;
	dataSample_max_g.humidity.data = 0;
	dataSample_min_g.humidity.data = 0xFFFF;
	dataSample_max_g.airPressure.data = 0;
	dataSample_min_g.airPressure.data = 0xFFFF;
	dataSample_max_g.groundTemp.data = 0;
	dataSample_min_g.groundTemp.data = 0xFFFF;
	dataSample_max_g.radiation.data = 0;
	dataSample_min_g.radiation.data = 0xFFFF;

	// Accumulated value, clear max/min to 0
	dataSample_max_g.rain.data = 0;
	dataSample_min_g.rain.data = 0;		//it's accumulated value, not minimum value
	dataSample_max_g.evaporation.data = 0;
	dataSample_min_g.evaporation.data = 0;	//it's accumulated value, not minimum value
	dataSample_max_g.sunShineTime.data = 0;
	dataSample_min_g.sunShineTime.data = 0;		//it's accumulated value, not minimum value

	timeStampShot_g.time = initTime;
	timeStampShot_g.tickeCounter = 1;
	timeStampShot_g.currentAddrEEPROM = START_ADDR_EEPROM;
	timeStampShot_g.pageSize = 256;		//bytes, 3*8(date6, data2)
	timeStampShot_g.flag = 1;		//update flag to 1 everytime time and tickout is changed

    CommandFifo.IsEmpty = IsEmpty;
    CommandFifo.AddFifo = AddFifo;
    CommandFifo.FetchFifo = FetchFifo;
    CommandFifo.ClearFifo = ClearFifo;
    (*CommandFifo.ClearFifo)(&CommandFifo);

	
}


void main(void)
{
    CLI();  //disable all interrupt until initialization is done
    init_vars();
	
    init_beep();
    init_led();
    init_uart0();   //enable printf
	init_port_adc0();
	uart1_init();
	printf("Start program! \r\n");
	resume_last_timeStampSlot();
	
    //main_uart0();
    //main_uart1_loopback();
    //main_twi();
    //main_key2SEG();
    //main_spi();
    //timer2_processCmd();
    //test_char2int();
    //test_usr();
    //uart1_processCmd();
    //main_adc1();
    //test_timer1();
    //collectADC0();
    //test_EEPROM();
	//(*CommandFifo.AddFifo)(&CommandFifo, 'B');	//add to fifo, read eeprom commands
	//test_copystr2TimeStamp();
	//test_get_address();
	
	
	ticker_processCmd();
	
	


    
}

