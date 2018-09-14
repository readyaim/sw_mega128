/****************************************************************************
* File name: timer.c
* Description: driver for TC0, TC1, TC2, TC3. TC1 used as system ticker generator every 200ms.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/
#include "global.h"
#include "peripherals.h"
#include "fifo.h"

//#define _DEBUG_TIMER1
//#define _DEBUG_TIMER3


static UINT16 ext_counter = 0;	//static means only allow timer.c use global ext_counter
extern UINT32 SystemTickCount;

/*******************************************************************************
* Function:  timer0_init()
* Arguments:
* Return:
* Description:  timer0 register initialization
*******************************************************************************/
void timer0_init(void)
{
#ifndef _TIMER0_COMP
    //normal mode
    ASSR = 0x00; //set async mode
    TCCR0 = (1 << CS01) | (1 << CS00);//normal mode TC0  xxxx xx11=32分频, 4*2^(CS02CS01CS00)
    TCNT0 = 0X83;//only valid at begining,counter = (0XFF-0X83)
    Set_Bit(TIMSK, TOIE0);   //timer interrupt sources, TOIE0=1: TC0 overflow interrupt enable. OCIE0=1: TC0 output match B interrupt is enable
#else
    ASSR = 0x00; //set async mode
    TCCR0 = 0x00; //stop
    TCNT0 = 0x00; //set count
    OCR0 = 0x4D;
    TCCR0 = 0x0F; //start timer, CTC mode ,autoreload, 1024 prescale
#endif
}
/*******************************************************************************
* Function:  timer1_init()
* Arguments:
* Return:
* Description:  timer1 register initialization
*******************************************************************************/
void timer1_init(void)
{
    TCCR1A = 0x00;  //disconnect comparator OCnA/B/C to IO, PWM
    TCCR1C = 0x00;  // force output compare for channel A/B/C
#ifdef _DEBUG_TIMER1
    TCCR1B = (1 << CS10 | 1<<WGM12);  //clock select b'001', CPU_CLK/1
    //TCCR1B = (1 << CS10 );  //clock select b'001', CPU_CLK/1
    TCNT1H = 0x00;  //
    TCNT1L = 0x00;  //
    OCR1AH = 0x18; //设置 TC1 的 输出比较寄存器A 高8位值
    OCR1AL = 0x6A; //设置 TC1 的 输出比较寄存器A 低8位值
    //OCR1BH = 0x00; //设置 TC1 的 输出比较寄存器B 高8位值
    //OCR1BL = 0x20; //设置 TC1 的 输出比较寄存器B 低8位值
    //OCR1CH = 0x00; //设置 TC1 的 输出比较寄存器B 高8位值
    //OCR1CL = 0x20; //设置 TC1 的 输出比较寄存器B 低8位值
    //Set_Bit(TIMSK, TOIE1);  //enable timer1 overflow interrupt
    Set_Bit(TIMSK, OCIE1A);  //enable timer1 overflow interrupt
    //Set_Bit(TIMSK, OCIE1B);  //enable timer1 overflow interrupt
    //Set_Bit(ETIMSK, OCIE1C);  //enable timer1 overflow interrupt
#else
    TCCR1B = (1 << CS12 | 1<<WGM12);  //clock select b'100', CPU_CLK/256
    TCNT1H = 0x00;  //Set TC1 counter TCNT1 = 0xE796, div=64,  counter=0xFFFF-0xe796=6250，fosc=8MHz, 0.2s
    TCNT1L = 0x00;  //Set TC1 counter TCNT1 = 0xCF2C, div=64,  counter=0xFFFF-0xCF2C=12500，fosc=8MHz, 0.1s
 //   OCR1AH = 0x18; //设置 TC1 的 输出比较寄存器A 高8位值
  //  OCR1AL = 0x6A; //设置 TC1 的 输出比较寄存器A 低8位值
 	OCR1AH = (UINT8)(TCNTVALUE >> 8);
	OCR1AL = (UINT8)(TCNTVALUE);

	//OCR1BH = 0x1C; //设置 TC1 的 输出比较寄存器B 高8位值
    //OCR1BL = 0x20; //设置 TC1 的 输出比较寄存器B 低8位值
    //Set_Bit(TIMSK, TOIE1);  //enable timer1 overflow interrupt
	Set_Bit(TIMSK, OCIE1A);  //enable timer1 overflow interrupt
#endif // !_DEBUG_TIMER1    
    
    //OCR1BH = 0x1C; //设置 TC1 的 输出比较寄存器B 高8位值
    //OCR1BL = 0x20; //设置 TC1 的 输出比较寄存器B 低8位值
    //ICR1H  = 0x1C; //设置 TC1 的 输入捕获寄存器 高8位值
    //ICR1L  = 0x20; //设置 TC1 的 输入捕获寄存器 低8位值
#ifndef _ATMEGA128A
    MCUCR = 0x00;  //设置 MCU 的 控制寄存器
#endif // _ATMEGA128A

    //TGICR = 0x00;  //设置 中断控制寄存器
    
                   //SEI();  //enable interrupts
/*    
    TCCR1 |= (1 << CS01) | (1 << CS00);//TC0  xxxx xx11=32分频, 4*2^(CS02CS01CS00)
    TCNT1 = 0X83;//only valid at begining,counter = (0XFF-0X83)
    Set_Bit(TIMSK, TOIE1);   //timer interrupt sources, TOIE0=1: TC0 overflow interrupt enable. OCIE0=1: TC0 output match B interrupt is enable
*/
}

/*******************************************************************************
* Function:  timer2_init()
* Arguments:
* Return:
* Description:  timer0 register initialization
*******************************************************************************/
void timer2_init(void)
{
    TCCR2 |= (1 << CS21) | (1 << CS20);//TC2  xxxx x011=64分频, (CS22CS21CS20), T = (TCCR2(64)*(0XFF-TCNT2)*counter)/CLK(8M)
    TCNT2 = 0X83;//only valid at begining,counter = (0XFF-0X83)
    Set_Bit(TIMSK, TOIE2);   //timer interrupt sources, TOIE2=1: TC2 overflow interrupt enable. OCIE2=1: TC2 output match B interrupt is enable
}



/*******************************************************************************
* Function:  timer3_init()
* Arguments:
* Return:
* Description:  timer3 register initialization
*******************************************************************************/
void timer3_init(void)
{
    TCCR3A = 0x00;  //disable comparator, PWM
    TCCR3C = 0x00;  // force output compare for channel A/B/C
#ifdef _DEBUG_TIMER3
    TCCR1B = (1 << CS10);  //clock select b'001', CPU_CLK/1
    TCNT1H = 0xFF;  //设置 TC1 的 计数寄存器 高8位值，基于7.3728M晶振
    TCNT1L = 0xF1;  //设置 TC1 的 计数寄存器 低8位值，基于7.3728M晶振
#else
    TCCR3B = (1 << CS32);  //clock select b'100', CPU_CLK/256
    TCNT3H = 0xE7;  //Set TC1 counter TCNT1 = 0xE796, div=64,  counter=0xFFFF-0xe796=6250，fosc=8MHz, 0.2s
    TCNT3L = 0x96;  //Set TC1 counter TCNT1 = 0xCF2C, div=64,  counter=0xFFFF-0xCF2C=12500，fosc=8MHz, 0.1s
#endif // !_DEBUG_TIMER1    
                    //OCR1AH = 0x1C; //设置 TC1 的 输出比较寄存器A 高8位值
                    //OCR1AL = 0x20; //设置 TC1 的 输出比较寄存器A 低8位值
                    //OCR1BH = 0x1C; //设置 TC1 的 输出比较寄存器B 高8位值
                    //OCR1BL = 0x20; //设置 TC1 的 输出比较寄存器B 低8位值
                    //ICR1H  = 0x1C; //设置 TC1 的 输入捕获寄存器 高8位值
                    //ICR1L  = 0x20; //设置 TC1 的 输入捕获寄存器 低8位值
#ifndef _ATMEGA128A
    MCUCR = 0x00;  //设置 MCU 的 控制寄存器
#endif // _ATMEGA128A

                   //TGICR = 0x00;  //设置 中断控制寄存器
    Set_Bit(ETIMSK, TOIE3);  //enable timer1 overflow interrupt
                            //SEI();  //enable interrupts
                            /*
                            TCCR1 |= (1 << CS01) | (1 << CS00);//TC0  xxxx xx11=32分频, 4*2^(CS02CS01CS00)
                            TCNT1 = 0X83;//only valid at begining,counter = (0XFF-0X83)
                            Set_Bit(TIMSK, TOIE1);   //timer interrupt sources, TOIE0=1: TC0 overflow interrupt enable. OCIE0=1: TC0 output match B interrupt is enable
                            */
}

#ifndef TIMER0_ADC
/*******************************************************************************
* Function:  timer0_ovf_isr()
* Arguments:
* Return:
* Description:  interrupt service rouine of overflow for timer0/count0
*******************************************************************************/
#pragma interrupt_handler timer0_ovf_isr:iv_TIMER0_OVF
void timer0_ovf_isr(void)
{
    static int i = 0;
    ext_counter++;
    TCNT0 = 0X00;   //reload the counter, 8-bit
    if (ext_counter < 2000)
    {
        //PORTF = 0X00;//定时时间2000*2倍, T = CLK(8M)/(TCCR0(32)*(0XFF-TCNT0)*2000)
        Clr_Bit(PORTF, LED2);//定时时间2000*2倍, T = CLK(8M)/(TCCR0(32)*(0XFF-TCNT0)*2000)
    }
    else
    {
        //PORTF = 0XFF;
        Set_Bit(PORTF, LED2);
    }
    if (ext_counter == 4000)
    {
        ext_counter = 0;
        //beep();

        AddFifo(&CommandFifo, 0x10 | i );
        i++;
        if (i == 10)
            i = 0;
    }
    //printf("ext_counter = %d\r\n",ext_counter);
    
}
#endif // TIMER0_ADC


/*******************************************************************************
* Function:  timer1_ovf_isr()
* Arguments:
* Return:
* Description:  timer1中断服务函数
*******************************************************************************/
#pragma interrupt_handler timer1_ovf_isr:iv_TIM1_OVF
void timer1_ovf_isr(void)
{
#ifdef _DEBUG_TIMER1
    TCNT1H = 0xFF;  //设置 TC1 的 计数寄存器 高8位值，基于7.3728M晶振
    TCNT1L = 0xE0;  //设置 TC1 的 计数寄存器 低8位值，基于7.3728M晶振
#else
    TCNT1H = 0xE7;  //Set TC1 counter TCNT1 = 0xE796, div=64,  counter=0xFFFF-0xe796=6250，fosc=8MHz, 0.2s
    TCNT1L = 0x96;  //Set TC1 counter TCNT1 = 0xCF2C, div=64,  counter=0xFFFF-0xCF2C=12500，fosc=8MHz, 0.1s
    beep();
#endif // !_DEBUG_TIMER1    
    
}

/*******************************************************************************
* Function:  timer2_ovf_isr()
* Arguments:
* Return:
* Description:  interrupt service rouine of overflow for timer2/count2
*******************************************************************************/
#pragma interrupt_handler timer2_ovf_isr:iv_TIMER2_OVF
void timer2_ovf_isr(void)
{
    static int i = 0;
    ext_counter++;
    TCNT2 = 0X80;   //reload the counter, 8-bit
    if (ext_counter < 2000)
    {
        //PORTF = 0X00;//定时时间2000*2倍, T = CLK(8M)/(TCCR2(32)*(0XFF-TCNT2)*2000)
        Clr_Bit(PORTF, LED2);//定时时间2000*2倍, T = CLK(8M)/(TCCR2(32)*(0XFF-TCNT2)*2000)

    }
    else
    {
        //PORTF = 0XFF;
        Set_Bit(PORTF, LED2);

    }
    if (ext_counter == 4000)
    {
        ext_counter = 0;
        //beep();

        AddFifo(&CommandFifo, 0x30 | i);
        i++;
        if (i == 10)
            i = 0;
    }
}

/*******************************************************************************
* Function:  timer3_ovf_isr()
* Arguments:
* Return:
* Description:  timer1中断服务函数
*******************************************************************************/
#pragma interrupt_handler timer3_ovf_isr:iv_TIM3_OVF
void timer3_ovf_isr(void)
{
#ifdef _DEBUG_TIMER3
    TCNT1H = 0xFF;  //设置 TC1 的 计数寄存器 高8位值，基于7.3728M晶振
    TCNT1L = 0xF1;  //设置 TC1 的 计数寄存器 低8位值，基于7.3728M晶振
#else
    TCNT3H = 0xE7;  //Set TC1 counter TCNT1 = 0xE796, div=64,  counter=0xFFFF-0xe796=6250，fosc=8MHz, 0.2s
    TCNT3L = 0x96;  //Set TC1 counter TCNT1 = 0xCF2C, div=64,  counter=0xFFFF-0xCF2C=12500，fosc=8MHz, 0.1s
#endif // !_DEBUG_TIMER1    
    beep();
}





/*******************************************************************************
* Function:  timer0_comp_isr()
* Arguments: 
* Return: 
* Description:  timer0比较器中断服务子程序
*******************************************************************************/
#pragma interrupt_handler timer0_comp_isr:iv_TIMER0_COMPA
void timer0_comp_isr(void)
{
	NOP();	// SystemTickCount++;
    // LEDPWM=0;
    // LEDPWM=LED_Lumin[(SystemTickCount>>(LED_Status))&0x1f];
}
/*******************************************************************************
* Function:  timer1_compA_isr()
* Arguments:
* Return:
* Description:  timer1比较器A中断服务子程序
*******************************************************************************/
#pragma interrupt_handler timer1_compA_isr:iv_TIMER1_COMPA
void timer1_compA_isr(void)
{
	SystemTickCount++;
    // LEDPWM=0;
    // LEDPWM=LED_Lumin[(SystemTickCount>>(LED_Status))&0x1f];
}

/*******************************************************************************
* Function:  timer1_compB_isr()
* Arguments:
* Return:
* Description:  timer1比较器B中断服务子程序
*******************************************************************************/
#pragma interrupt_handler timer1_compB_isr:iv_TIMER1_COMPB
void timer1_compB_isr(void)
{
    NOP();
    NOP();
    // LEDPWM=0;
    // LEDPWM=LED_Lumin[(SystemTickCount>>(LED_Status))&0x1f];
}

/*******************************************************************************
* Function:  timer1_compC_isr()
* Arguments:
* Return:
* Description:  timer1比较器C中断服务子程序
*******************************************************************************/
#pragma interrupt_handler timer1_compC_isr:iv_TIMER1_COMPC
void timer1_compC_isr(void)
{
    NOP();
    NOP();
    // LEDPWM=0;
    // LEDPWM=LED_Lumin[(SystemTickCount>>(LED_Status))&0x1f];
}

/******TIMER 0 Interrupt**********/
// call this routine to initialize all peripherals
/*******************************************************************************
* Function:  init_LED_port()
* Arguments:
* Return:
* Description:  initiate LED for timer0 test
*******************************************************************************/
void init_LED_port(void)
{
    Set_Bit(PORTF, LED2);
    Set_Bit(DDRF, LED2);
}
/*******************************************************************************
* Function:  init_devices_timer0()
* Arguments:
* Return:
* Description:  timer0 hardware device initiation
*******************************************************************************/
void init_devices_timer0(void)
{
    //stop errant interrupts until set up
    CLI(); //disable all interrupts
    Clr_Bit(XDIV, XDIVEN);		//disable xtal divider, run at 8MHz, if XDIV(7)==1, clk=src_clk/(129-div), write div only when xdiv(7) is 1
    NOP();  //writing to XDIV cause unstable running of code. Run 8 NOP()s to wait clk stable
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    init_LED_port();
    timer0_init();
}

/*******************************************************************************
* Function:  init_devices_timer1()
* Arguments:
* Return:
* Description:  timer1 hardware device initiation
*******************************************************************************/
void init_devices_timer1(void)
{
    //stop errant interrupts until set up
    CLI(); //disable all interrupts
    Clr_Bit(XDIV, XDIVEN);		//disable xtal divider, run at 8MHz, if XDIV(7)==1, clk=src_clk/(129-div), write div only when xdiv(7) is 1
    NOP();  //writing to XDIV cause unstable running of code. Run 8 NOP()s to wait clk stable
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    timer1_init();
}
/*******************************************************************************
* Function:  init_devices_timer2()
* Arguments:
* Return:
* Description:  timer2 hardware device initiation
*******************************************************************************/
void init_devices_timer2(void)
{
    //stop errant interrupts until set up
    CLI(); //disable all interrupts
    Clr_Bit(XDIV, XDIVEN);		//disable xtal divider, run at 8MHz, if XDIV(7)==1, clk=src_clk/(129-div), write div only when xdiv(7) is 1
    NOP();  //writing to XDIV cause unstable running of code. Run 8 NOP()s to wait clk stable
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    init_LED_port();
    timer2_init();
}


/*******************************************************************************
* Function:  init_devices_timer3()
* Arguments:
* Return:
* Description:  timer3 hardware device initiation
*******************************************************************************/
void init_devices_timer3(void)
{
    //stop errant interrupts until set up
    CLI(); //disable all interrupts
    Clr_Bit(XDIV, XDIVEN);		//disable xtal divider, run at 8MHz, if XDIV(7)==1, clk=src_clk/(129-div), write div only when xdiv(7) is 1
    NOP();  //writing to XDIV cause unstable running of code. Run 8 NOP()s to wait clk stable
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    timer3_init();
}


/*******************************************************************************
* Function:  timer1_ovEnable()
* Arguments: 
* Return: 
* Description:  timer1溢出中断使能
*******************************************************************************/
void timer1_ovEnable(void)
{
    Set_Bit(TIMSK, TOIE1);  //Timer1 Overflow interrupt enable
}

/*******************************************************************************
* Function:  timer2_ovEnable()
* Arguments:
* Return:
* Description:  timer2溢出中断使能
*******************************************************************************/
void timer2_ovEnable(void)
{
    Set_Bit(TIMSK, TOIE2);  //Timer2 Overflow interrupt enable
}
/*******************************************************************************
* Function:  timer3_ovEnable()
* Arguments:
* Return:
* Description:  timer3溢出中断使能
*******************************************************************************/
void timer3_ovEnable(void)
{
    Set_Bit(ETIMSK, TOIE3);  //Timer1 Overflow interrupt enable
}

void test_timer0(void)
{
    CLI();
    init_devices_timer0();
    beep();
#ifndef _ATMEGA128A
    Set_Bit(MCUCR, SE);	//enable sleep enable = 1
#endif // _ATMEGA128A
    SEI();
    //SLEEP();
    //while (1)
    //{
    //}
}
void test_timer1(void)
{
    CLI();
    init_devices_timer1();
    beep();
#ifndef _ATMEGA128A
    Set_Bit(MCUCR, SE);	//enable sleep enable = 1
#endif // _ATMEGA128A
    SEI();
    SLEEP();
    //while (1)
    //{
    //}
}

/*******************************************************************************
* Function:  test_timer2()
* Arguments:
* Return:
* Description: test code for timer2 interrupt
*******************************************************************************/
void test_timer2(void)
{
    CLI();
    init_devices_timer2();
    beep();
    //Set_Bit(MCUCR, SE);	//enable sleep enable = 1
    SEI();
    
}
