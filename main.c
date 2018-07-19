//ICC-AVR application builder : 2012/2/13 20:52:33
/*******************************************************************************
* from Home-Win7, work-HP
* ��Ƭ��:   ATMAGE128A AU 1036
* ����:     �ⲿ8MHz
* ������:   ICC 7.22
*
* �ļ���:   main.c
* �汾:     1.0
* �������:
* Description:  ��8M������,ʵ�ּ���ָʾ����ܲ���
*******************************************************************************/
/*********************************����ͷ�ļ�********************************/
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
extern void main_processCmd(void);

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
    main_uart0();
    
    main_processCmd();
    

}

