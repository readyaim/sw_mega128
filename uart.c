/***************************************************************************/
/*串口0测试程序							                                   */
/*目标器件：ATmega 128   						                           */
/*晶振:RC 8MHZ								                               */
/*编译环境：ICCAVR 7.22 						                           */
/*运行寿命：打开串口测试软件输入t，回车看到test ok 0123456789              */
/*时间：2012年08月08日                                                     */
/*作者：涛仔工作室
/***************************************************************************/

/*********************************包含头文件********************************/
#include "global.h"


/***********************************宏定义**********************************/

#define baud 57600	 //波特率
#define RXC_BUFF_SIZE 128   //接受缓冲区字节数
#define TXC_BUFF_SIZE 128   //发送缓冲区字节数

// add static to forbiden other file to use
static UINT8 RXC_BUFF[RXC_BUFF_SIZE];   //定义接受缓冲区
static UINT8 TXC_BUFF[TXC_BUFF_SIZE];   //定义发送缓冲区
static UINT8 RXC_RD;   //接受缓冲区读指针
static UINT8 RXC_WR;   //接受缓冲区写指针
static UINT8 TXC_RD;   //发送缓冲区读指针
static UINT8 TXC_WR;   //发送缓冲区写指针

/****************************************************************************
Function Name: uart0初始化程序
Arguments: 
Returns: :
****************************************************************************/
void init_uart0(void)  //初始化COM0
{
    UCSR0B = 0x00; //初始化
    UCSR0A = 0x00; //初始化, *U2X0=0:非倍速模式
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);//8bit
    Clr_Bit(UCSR0C, USBS0);        //USBS0=0: 1bit stop
    UBRR0L = (CPU_CLK / 16 / (baud + 1)) % 256;     //52
    UBRR0H = (CPU_CLK / 16 / (baud + 1)) / 256;     //0, baud rate = 9600, 0.2% error
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //接收使能，发射使能，接收结束中断使能
}

/****************************************************************************
Function Name: uart0发送单字节数据
Arguments: c
Returns: :
****************************************************************************/
int putchar(char ch)
{
    TXC_BUFF[TXC_WR] = ch;
    if (TXC_WR < (TXC_BUFF_SIZE - 1))   //TXC_BUFF_SIZE  发送区数据大小
        TXC_WR++;
    else
        TXC_WR = 0;
    //UCSR0B |= (1 << UDRIE0);          //开启UDRE中断
    Set_Bit(UCSR0B, UDRIE0);          //开启UDRE中断
    return 0;
}

/****************************************************************************
Function Name: uart0接收单字节数据
Arguments: 
Returns: :
****************************************************************************/
UINT8 getchar_uart0(void)
{
    unsigned temp;
    while (RXC_RD == RXC_WR)   //等待缓冲区内有数据输入
        ;                   //死循环，等待输入数据
    temp = RXC_BUFF[RXC_RD];
    if (RXC_RD < (RXC_BUFF_SIZE - 1))
        RXC_RD++;
    else
        RXC_RD = 0;
    return temp;
}

/****************************************************************************
Function Name: uart0发送字符串数据
Arguments: *s
Returns:
****************************************************************************/
void puts_uart0(char *s)     //发送字符串函数
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
Function Name: 初始化单片机
Arguments: 
Returns: :
****************************************************************************/
void init_devices_uart0(void)
{
    CLI(); //关闭所有中断
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
Descriptions: Interrupt Service Routines for TC0 RX complete. uart0接收结束中断
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
    if (TXC_RD == TXC_WR)   //读指针==写指针,停止中断
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
Function Name: 主程序
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
    SEI();  //允许中断
    putchar('t');
    while (1)
    {
        delay_ms(1000);
        //if (getchar_uart0() == 't')//按键盘t键开始测试
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
