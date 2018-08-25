#include "global.h"
#include "peripherals.h"
#include "fifo.h"

static UINT16 ext_counter = 0;	//static means only allow timer.c use global ext_counter
static UINT16 num_of_timer0_int = 0;
extern UINT32 SystemTickCount;

/*******************************************************************************
* Function:  timer0_init()
* Arguments:
* Return:
* Description:  timer0 register initialization
*******************************************************************************/
void timer0_init(void)
{
    TCCR0 |= (1 << CS01) | (1 << CS00);//TC0  xxxx xx11=32分频, 4*2^(CS02CS01CS00)
    TCNT0 = 0X83;//only valid at begining,counter = (0XFF-0X83)
    Set_Bit(TIMSK, TOIE0);   //timer interrupt sources, TOIE0=1: TC0 overflow interrupt enable. OCIE0=1: TC0 output match B interrupt is enable
}
/*******************************************************************************
* Function:  timer1_init()
* Arguments:
* Return:
* Description:  timer1 register initialization
*******************************************************************************/
void timer1_init(void)
{
    
    TCCR0 |= (1 << CS01) | (1 << CS00);//TC0  xxxx xx11=32分频, 4*2^(CS02CS01CS00)
    TCNT0 = 0X83;//only valid at begining,counter = (0XFF-0X83)
    Set_Bit(TIMSK, TOIE0);   //timer interrupt sources, TOIE0=1: TC0 overflow interrupt enable. OCIE0=1: TC0 output match B interrupt is enable
}
#ifndef TIMER0_ADC
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
    num_of_timer0_int += 1;
}
#endif // TIMER0_ADC

/*******************************************************************************
* Function:  timer0_comp_isr()
* Arguments: 
* Return: 
* Description:  timer0比较器中断服务子程序
*******************************************************************************/
#pragma interrupt_handler timer0_comp_isr:iv_TIMER0_COMPA
void timer0_comp_isr(void)
{
    SystemTickCount++;
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
* Function:  init_timer1()
* Arguments: 
* Return: 
* Description:  timer1初始化函数
*******************************************************************************/
void init_timer1(void)
{
    TCNT1H = 0XCF;//初值
    TCNT1L = 0X2C;
    TCCR1B = 0X03;//CS02 CS01 CS0 0000 0011======64分频
}
/*******************************************************************************
* Function:  timer1_ovf_isr()
* Arguments: 
* Return: 
* Description:  timer1中断服务函数
*******************************************************************************/
#pragma interrupt_handler timer1_ovf_isr:iv_TIM1_OVF
void timer1_ovf_isr(void)
{
    TCNT1H = 0XCF;
    TCNT1L = 0X2C;
    beep();
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


void test_timer0(void)
{
    CLI();
    init_devices_timer0();
    beep();
    //Set_Bit(MCUCR, SE);	//enable sleep enable = 1
    SEI();
    //SLEEP();
    //while (1)
    //{
    //}
}
void test_timer1(void)
{
    CLI();
    init_devices_timer0();
    beep();
    Set_Bit(MCUCR, SE);	//enable sleep enable = 1
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
* Description:  timer2 interrupt
*******************************************************************************/
void test_timer2(void)
{
    CLI();
    init_devices_timer0();
    beep();
    Set_Bit(MCUCR, SE);	//enable sleep enable = 1
    SEI();
    SLEEP();
    //while (1)
    //{
    //}
}