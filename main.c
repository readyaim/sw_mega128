//ICC-AVR application builder : 2012/2/13 20:52:33
/*******************************************************************************
* from Home-Win7, work-HP
* 单片机:   ATMAGE128A AU 1036
* 晶振:     外部8MHz
* 编译器:   ICC 7.22
*
* 文件名:   main.c
* 版本:     1.0
* 完成日期:
* Description:  在8M晶振下,实现键盘指示数码管操作
*******************************************************************************/
/*********************************包含头文件********************************/
#include "global.h"
//#include "peripherals.h"
#include "fifo.h"


extern BOOL IsEmpty(struct Fifo *this);
extern BOOL AddFifo(struct Fifo *this, UINT8 data);
extern UINT8 FetchFifo(struct Fifo *this);
extern void ClearFifo(struct Fifo *this);
extern void ticker_processCmd(void);

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


#ifdef _DUMMY_CODE
extern void test_char2int(void);
extern void test_strcmp(void);
extern void test_usr(void);
extern void collectADC0(void);

#endif // _DUMMY_CODE

UINT32 SystemTickCount;
struct dataInEEPROM_t dataIneeprom, dataInRom_max, dataInRom_min;
struct Fifo CommandFifo;
struct TimeStamp_t timeStampShot;

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
	struct Date_t initTime = {20,18,9,12,17,0};

	initTime.year1 = 20;
	initTime.year = 18;
	initTime.mon = 9;
	initTime.day = 8;
	initTime.hour = 20;
	initTime.min = 58;


    SystemTickCount = 0;
	dataIneeprom.data = 0;
	timeStampShot.time = initTime;
	timeStampShot.tickeCounter = 0;
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
	printf("Start program! \r\n");
	uart1_init();
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
	ticker_processCmd();


    
}

