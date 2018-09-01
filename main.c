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

//extern void timer0_init(void);
//extern void timer0_ovf_isr(void);
//extern void port_init_timer0(void);
extern void test_timer0(void);
extern void main_adc1(void);
extern void main_watchdog(void);
extern void main_uart0(void);
extern void main_uart1(void);
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

#ifdef _DUMMY_CODE
extern void test_char2int(void);
extern void test_strcmp(void);
extern void test_usr(void);
#endif // _DUMMY_CODE

UINT32 SystemTickCount;
struct Fifo CommandFifo;

BOOL TimeIsUp(UINT32 StartTime, UINT32 Delay)
{
    if (SystemTickCount > StartTime)
        if (SystemTickCount - StartTime > Delay) return true;
    if (SystemTickCount < StartTime)
        if ((4294967295 - StartTime + SystemTickCount) > Delay) return true;
    return false;
}



void main(void)
{
    CLI();  //disable all interrupt until initialization is done
    init_beep();
    init_led();
    //main_uart0();
    //main_uart1();
    //main_twi();
    //main_key2SEG();
    //main_spi();
    //timer2_processCmd();
    //test_char2int();
    //test_usr();
    uart1_processCmd();


    
}

