/****************************************************************************
* File name: peripherals.c
* Description: includes functions for LED, SEG, BEEP
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"
#include "peripherals.h"


/****************************************************************************
Function: Beep子程序
Arguments: 
Return: 
****************************************************************************/
void beep(void)
{
    Set_Bit(PORTB, BEEP_BIT);   //开蜂鸣器
    delay_ms(100);
    Clr_Bit(PORTB, BEEP_BIT);   //关蜂鸣器
}
/****************************************************************************
Function: 初始化SEG端口1-4
Arguments: 
Return: 
****************************************************************************/
void init_SEG(void) {
    PORTD = 0XFF;   //数码管段选数据口, HIGH
    DDRD = 0XFF;    //OUT
    PORTG = 0x1B;//数码管位选数据口
    DDRG = 0X1B;
}

/****************************************************************************
Function: 初始化SEG4
Arguments: 
Return: 
****************************************************************************/
void init_SEG4(void) 
{

    DDRD = 0XFF;        //set SEG data as output
    PORTD = 0XFF;       //set SEG data as high
    DDRG = 0x1B;        //set SEG select as output
    PORTG = 0X1B;       //select SEG
    SEG4_ON;
}

/****************************************************************************
Function: 初始化Beep端口
Arguments: 
Return: 
****************************************************************************/
void init_beep(void) {
    Set_Bit(DDRB, BEEP_BIT);    //BIT0=OUT
    Clr_Bit(PORTB, BEEP_BIT);   //BIT0=LOW
}

/*******************************************************************************
* Function:  delay_us()
* Arguments:  microsecond : 输入延时微秒的时间
* Return: 
* Description:  微秒的延时
*******************************************************************************/
void delay_us(UINT16 microsecond)
{
    do
    {
        microsecond--;
    } while (microsecond > 1);
}


/*******************************************************************************
* Function:  delay_ms()
* Arguments:  millisecond : 输入延时毫秒的时间
* Return: 
* Description:  毫秒的延时
*******************************************************************************/
void delay_ms(UINT16 millisecond)
{
    while (millisecond--)
    {
        delay_us(999);
    }
}

/*******************************************************************************
* Function:  delay_nus()
* Arguments:  n : 输入延时n微妙的时间
* Return: 
* Description:  微秒的延时
*******************************************************************************/
void delay_nus(UINT16 n)//n微秒延时函数
{
    unsigned int i;
    for (i = 0; i<n; i++)
    {
        asm("nop");
    }
}
/*******************************************************************************
* Function:  init_watchdog()
* Arguments:  
* Return: 
* Description:  看门狗功能初始化
*******************************************************************************/
void init_watchdog(void)
{
    WDTCR |= (1 << WDCE) | (1 << WDE);   //enable watchdog
    WDR();       //看门狗计数清零
    //WDTCR = 0x0F;  //使能watchdog,并且，采用2048K分频，典型溢出时间5V时2.1S
                                         //To disable WDT, WDCE must be set to 1 before clear WDE
    //WDTCR |= (1 << WDCE) | (1 << WDE); Clr_Bit(WDTCR, WDE); //WDTCR = 0x00
    WDTCR |= (1 << WDP2) | (1 << WDP1) | (1 << WDP0);   //set time cycles: 2048k OSC cycles, 2s
    Set_Bit(WDTCR, WDE);    //Enable watchdog
}

/*******************************************************************************
* Function:  init_led()
* Arguments: 
* Return: 
* Description:  LED初始化(关闭LED)
*******************************************************************************/
void init_led(void)
{
    LED_PORT = 0xFF;    //HIGH,=Turn off LED
    LED_DDR = 0xFF;     //OUT
}

/*******************************************************************************
* Function:  led_on()
* Arguments:
* Return:
* Description:  LED2 ON
*******************************************************************************/
void led_on(void)
{
    Clr_Bit(LED_PORT, LED2);
}

/*******************************************************************************
* Function:  led_off()
* Arguments:
* Return:
* Description:  LED2 OFF
*******************************************************************************/
void led_off(void)
{
    Set_Bit(LED_PORT, LED2);
}


/*******************************************************************************
* Function:  main_watchdog()
* Arguments: 
* Return: 
* Description:  看门狗功能入口函数
*******************************************************************************/
void main_watchdog(void)
{
    init_beep();
    beep();
    init_led();
    init_watchdog();
    //delay_nus(5000);//50MS
    while (1)
    {
        //delay_nus(1);   //50MS
        WDR();            //看门狗计数清零  屏蔽此语句[没按时喂狗]系统将复位 
        PORTF = 0X00;   //Turn on LED
    }
}

void feed_watchdog(void)
{
    WDR();
}

