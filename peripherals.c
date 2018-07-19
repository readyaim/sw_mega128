#include "global.h"
#include "peripherals.h"

/****************************************************************************
Function: Beep�ӳ���
Arguments: 
Return: 
****************************************************************************/
void beep(void)
{
    Set_Bit(PORTB, BEEP_BIT);   //��������
    delay_ms(100);
    Clr_Bit(PORTB, BEEP_BIT);   //�ط�����
}
/****************************************************************************
Function: ��ʼ��SEG�˿�
Arguments: 
Return: 
****************************************************************************/
void init_SEG(void) {
    PORTD = 0XFF;   //����ܶ�ѡ���ݿ�, HIGH
    DDRD = 0XFF;    //OUT
    PORTG = 0x1B;//�����λѡ���ݿ�
    DDRG = 0X1B;
}

/****************************************************************************
Function: ��ʼ��Beep�˿�
Arguments: 
Return: 
****************************************************************************/
void init_beep(void) {
    Set_Bit(DDRB, BEEP_BIT);    //BIT0=OUT
    Clr_Bit(PORTB, BEEP_BIT);   //BIT0=LOW
}

/*******************************************************************************
* Function:  delay_us()
* Arguments:  microsecond : ������ʱ΢���ʱ��
* Return: 
* Description:  ΢�����ʱ
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
* Arguments:  millisecond : ������ʱ�����ʱ��
* Return: 
* Description:  �������ʱ
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
* Arguments:  n : ������ʱn΢���ʱ��
* Return: 
* Description:  ΢�����ʱ
*******************************************************************************/
void delay_nus(UINT16 n)//n΢����ʱ����
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
* Description:  ���Ź����ܳ�ʼ��
*******************************************************************************/
void init_watchdog(void)
{
    WDTCR |= (1 << WDCE) | (1 << WDE);   //enable watchdog
    WDR();       //���Ź���������
    //WDTCR = 0x0F;  //ʹ��watchdog,���ң�����2048K��Ƶ���������ʱ��5Vʱ2.1S
                                         //To disable WDT, WDCE must be set to 1 before clear WDE
    //WDTCR |= (1 << WDCE) | (1 << WDE); Clr_Bit(WDTCR, WDE); //WDTCR = 0x00
    WDTCR |= (1 << WDP2) | (1 << WDP1) | (1 << WDP0);   //set time cycles: 2048k OSC cycles, 2s
    Set_Bit(WDTCR, WDE);    //Enable watchdog
}

/*******************************************************************************
* Function:  init_led()
* Arguments: 
* Return: 
* Description:  LED��ʼ��(�ر�LED)
*******************************************************************************/
void init_led(void)
{
    PORTF = 0xFF;   //HIGH,=Turn off LED
    DDRF = 0xFF;    //OUT
}
/*******************************************************************************
* Function:  main_watchdog()
* Arguments: 
* Return: 
* Description:  ���Ź�������ں���
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
        WDR();            //���Ź���������  ���δ����[û��ʱι��]ϵͳ����λ 
        PORTF = 0X00;   //Turn on LED
    }
}

void feed_watchdog(void)
{
    WDR();
}