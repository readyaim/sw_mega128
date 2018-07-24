/***************************************************************************/
/*����0���Գ���							                                   */
/*Ŀ��������ATmega 128   						                           */
/*����:RC 8MHZ								                               */
/*���뻷����ICCAVR 7.22 						                           */
/*�����������򿪴��ڲ����������t���س�����test ok 0123456789              */
/*ʱ�䣺2012��08��08��                                                     */
/*���ߣ����й�����
/***************************************************************************/

/*********************************����ͷ�ļ�********************************/
#include "global.h"


/***********************************�궨��**********************************/

#define baud 57600	 //������
#define RXC_BUFF_SIZE 128   //���ܻ������ֽ���
#define TXC_BUFF_SIZE 128   //���ͻ������ֽ���

// add static to forbiden other file to use
static UINT8 RXC_BUFF[RXC_BUFF_SIZE];   //������ܻ�����
static UINT8 TXC_BUFF[TXC_BUFF_SIZE];   //���巢�ͻ�����
static UINT8 RXC_RD;   //���ܻ�������ָ��
static UINT8 RXC_WR;   //���ܻ�����дָ��
static UINT8 TXC_RD;   //���ͻ�������ָ��
static UINT8 TXC_WR;   //���ͻ�����дָ��

/****************************************************************************
Function Name: uart0��ʼ������
Arguments: 
Returns: :
****************************************************************************/
void init_uart0(void)  //��ʼ��COM0
{
    UCSR0B = 0x00; //��ʼ��
    UCSR0A = 0x00; //��ʼ��, *U2X0=0:�Ǳ���ģʽ
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);//8bit
    Clr_Bit(UCSR0C, USBS0);        //USBS0=0: 1bit stop
    UBRR0L = (CPU_CLK / 16 / (baud + 1)) % 256;     //52
    UBRR0H = (CPU_CLK / 16 / (baud + 1)) / 256;     //0, baud rate = 9600, 0.2% error
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //����ʹ�ܣ�����ʹ�ܣ����ս����ж�ʹ��
}

/****************************************************************************
Function Name: uart0���͵��ֽ�����
Arguments: c
Returns: :
****************************************************************************/
int putchar(char ch)
{
    TXC_BUFF[TXC_WR] = ch;
    if (TXC_WR < (TXC_BUFF_SIZE - 1))   //TXC_BUFF_SIZE  ���������ݴ�С
        TXC_WR++;
    else
        TXC_WR = 0;
    //UCSR0B |= (1 << UDRIE0);          //����UDRE�ж�
    Set_Bit(UCSR0B, UDRIE0);          //����UDRE�ж�
    return 0;
}

/****************************************************************************
Function Name: uart0���յ��ֽ�����
Arguments: 
Returns: :
****************************************************************************/
UINT8 getchar_uart0(void)
{
    unsigned temp;
    while (RXC_RD == RXC_WR)   //�ȴ�������������������
        ;                   //��ѭ�����ȴ���������
    temp = RXC_BUFF[RXC_RD];
    if (RXC_RD < (RXC_BUFF_SIZE - 1))
        RXC_RD++;
    else
        RXC_RD = 0;
    return temp;
}

/****************************************************************************
Function Name: uart0�����ַ�������
Arguments: *s
Returns:
****************************************************************************/
void puts_uart0(char *s)     //�����ַ�������
{
    while (*s)
    {
        putchar(*s);
        s++;
    }
    putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
    putchar(0x0A);  //LF Line forward, for win+linux}
}
/****************************************************************************
Function Name: ��ʼ����Ƭ��
Arguments: 
Returns: :
****************************************************************************/
void init_devices_uart0(void)
{
    CLI(); //�ر������ж�
    XDIV = 0x00;
    XMCRA = 0x00;
    MCUCR = 0x00;
    EICRA = 0x00;
    EICRB = 0x00;
    EIMSK = 0x00;
    TIMSK = 0x00;
    ETIMSK = 0x00;
}

/****************************************************************************
Function Name: uart0_rx_isr
Arguments: 
Returns:
Descriptions: Interrupt Service Routines for TC0 RX complete. uart0���ս����ж�
****************************************************************************/
#pragma interrupt_handler uart0_rx_isr:iv_USART0_RXC   
void uart0_rx_isr(void)
{
    RXC_BUFF[RXC_WR] = UDR0;
    if (RXC_WR < (RXC_BUFF_SIZE - 1))
        RXC_WR++;
    else
        RXC_WR = 0;
}

/****************************************************************************
Function Name:  uart0_udre_isr
Arguments: 
Returns:
Descriptions: when UDR0 is empty, we receive this interrupt. It means one data is just sent out.
                    We shall increase the Tx buffer addr by 1. If the buffer is empty, we shall disable 
                    the UDR interrupt.
****************************************************************************/
#pragma interrupt_handler uart0_udre_isr:iv_USART0_UDRE   
void uart0_udre_isr(void)
{
    UDR0 = TXC_BUFF[TXC_RD];
    if (TXC_RD < (TXC_BUFF_SIZE - 1))
        TXC_RD++;
    else
        TXC_RD = 0;
    if (TXC_RD == TXC_WR)   //��ָ��==дָ��,ֹͣ�ж�
    {
        //UCSR0B &= ~(1 << UDRIE0);
        Clr_Bit(UCSR0B, UDRIE0);    //stop interrupt, no Tx data to send when TXC_RD == TXC_WR
    }
}
/****************************************************************************
Function Name: loopback_uart0
Arguments:
Returns:
Descriptions:
****************************************************************************/
void loopback_uart0(void)
{
    UINT8 flag = 0;
    Clr_Bit(UCSR0B, RXEN0); //Disable TC0 Rx. Otherwise, Tx data will lose due to Rx interrupt
    while (RXC_RD != RXC_WR)
    {
        flag = 1;
        putchar(RXC_BUFF[RXC_RD]);
        if (RXC_RD < (RXC_BUFF_SIZE - 1))
            RXC_RD++;
        else
            RXC_RD = 0;
    }
    if (flag ==1)
    {
        putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
        putchar(0x0A);  //LF Line forward, for win+linux
    }
    Set_Bit(UCSR0B, RXEN0); //Enable TC0 Rx
}
/****************************************************************************
Function Name: ������
Arguments: 
Returns:
Descriptions: 
****************************************************************************/
void main_uart0(void)
{
    unsigned int i;
    TXC_RD = 0;
    TXC_WR = 0;
    RXC_RD = 0;
    RXC_WR = 0;
    init_devices_uart0();
    init_uart0();
    SEI();  //�����ж�
    putchar('t');
    while (1)
    {
        delay_ms(1000);
        //if (getchar_uart0() == 't')//������t����ʼ����
        //{ 
        //    puts_uart0("test ok!");
        //    //for (i = 0; i < 10; i++) putchar(0x30 + i);
        //    puts_uart0("this is from printf function");
        //    printf("value is %d\r\n", i);
        //    //putchar(0x0D);
        //    //putchar(0x0A);
        //}
        loopback_uart0();
    }
}

/*******************************************************************************
* Function Name: : putcharA()
* Arguments: 
* Returns: :
* Descriptions:  Send out a data through uart0. For furture use.
*******************************************************************************/
int putcharA(char ch)
{
    //while (!(UCSR0A&(1 << UDRE0))); 
    while (!(Get_Bit(UCSR0A, UDRE0)));  //if UDRE0=1, Tx buffer is ready for next transmit.
    UDR0 = ch;
    return 0;
}
/*******************************************************************************
* Function Name: : getcharA()
* Arguments:
* Returns: :
* Descriptions:  receive a data through uart0. for future. TODO: will halt if no data come to uart0
*******************************************************************************/
UINT8 getcharA(void)
{
    while (!(Get_Bit(UCSR0A, RXC0)));
    return UDR0;
}
