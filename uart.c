/****************************************************************************
* File name: uart.c
* Description: driver for uart0 and uart1, both polling and interrupt.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

/*********************************包含头文件********************************/
#include "global.h"

/***********************************宏定义**********************************/

#define UART0_U2X 0
#define UART0_BAUD 38400	 //UART0_BAUD rate 
//#define UART0_UBRR (CPU_CLK/16/UART0_BAUD-1)
#define UART0_UBRR ((CPU_CLK + UART0_BAUD * (2 - UART0_U2X) * 4L) / (UART0_BAUD * (2 - UART0_U2X) * 8L) - 1)   //U2X=1

#define UART1_U2X 0
#define UART1_BAUD 38400	 //UART1_BAUD rate
#define UART1_UBRR ((CPU_CLK + UART1_BAUD * (2-UART1_U2X) * 4L) / (UART1_BAUD * (2-UART1_U2X) * 8L) - 1)   //U2X=1
//#define UART1_UBRR (CPU_CLK/8/UART1_BAUD-1)		//U2X=1	
//#define UART1_UBRR (CPU_CLK/16/UART1_BAUD-1)		//U2X=0

#define RXC0_BUFF_SIZE 128   //接受缓冲区字节数
#define TXC0_BUFF_SIZE 128   //发送缓冲区字节数

extern void parseStr2Date(UINT8 *str, Date_t *pdateTime);
extern void parseStr2Cmd(UINT8 ch);
extern BOOL AddFifo(struct Fifo *this, UINT8 data);


// add static to forbiden other file to use
static UINT8 RXC0_BUFF[RXC0_BUFF_SIZE];   //定义接受缓冲区
static UINT8 TXC0_BUFF[TXC0_BUFF_SIZE];   //定义发送缓冲区
static UINT8 RXC0_RD;   //接受缓冲区读指针
static UINT8 RXC0_WR;   //接受缓冲区写指针
static UINT8 TXC0_RD;   //发送缓冲区读指针
static UINT8 TXC0_WR;   //发送缓冲区写指针


// add static to forbiden other file to use
static UINT8 RXC1_BUFF[RXC1_BUFF_SIZE];   //定义接受缓冲区
UINT8 UART1_TxBuf[UART1_TX_BUFFER_SIZE];   //定义发送缓冲区
static UINT8 RXC1_RD;   //接受缓冲区读指针
static UINT8 RXC1_WR;   //接受缓冲区写指针
volatile UINT16 TXC1_RD;   //发送缓冲区读指针
volatile UINT16 TXC1_WR;   //发送缓冲区写指针

volatile UINT16 UART1_TxHead;
volatile UINT16 UART1_TxTail;



/****************************************************************************
Function Name: uart0初始化程序
Arguments:
Returns: :
****************************************************************************/
void uart0_init_register(void)  //初始化COM0
{
	UINT16 ubrr = UART0_UBRR;
	UCSR0B = 0x00; //初始化
	UCSR0A = 0x00 | (UART0_U2X << U2X0); //uart0 init, U2X0=0, 1x speed mode.
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);//8bit
	Clr_Bit(UCSR0C, USBS0);        //USBS0=0: 1bit stop
#ifdef _ATMEGA128A  
	//U2X = 0
	UBRR0L = (UINT8)(ubrr);
	UBRR0H = (UINT8)(ubrr >> 8);
	//UBRR0L = (CPU_CLK /  (UART0_BAUD * 16) - 1) % 256;     //
	//UBRR0H = (CPU_CLK / (UART0_BAUD * 16)  - 1) / 256;     //
#else   
	//atmega64, 128
	UBRR0L = (CPU_CLK / 16 / (UART0_BAUD + 1)) % 256;     //52
	UBRR0H = (CPU_CLK / 16 / (UART0_BAUD + 1)) / 256;     //0, BAUD rate = 9600, 0.2% error
#endif // _ATMEGA128A


	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //接收使能，发射使能，接收结束中断使能
}

/****************************************************************************
Function Name: uart0发送单字节数据
Arguments: c
Returns: :
****************************************************************************/
int uart0_putchar(char ch)
{
	TXC0_BUFF[TXC0_WR] = ch;
	if (TXC0_WR < (TXC0_BUFF_SIZE - 1))   //TXC0_BUFF_SIZE  发送区数据大小
		TXC0_WR++;
	else
		TXC0_WR = 0;
	//UCSR0B |= (1 << UDRIE0);          //开启UDRE中断
	Set_Bit(UCSR0B, UDRIE0);          //开启UDRE中断
	return 0;
}

/****************************************************************************
Function Name: uart0接收单字节数据
Arguments:
Returns: :
****************************************************************************/
UINT8 uart0_getchar(void)
{
	unsigned temp;
	while (RXC0_RD == RXC0_WR)   //等待缓冲区内有数据输入
		;                   //死循环，等待输入数据
	temp = RXC0_BUFF[RXC0_RD];
	if (RXC0_RD < (RXC0_BUFF_SIZE - 1))
		RXC0_RD++;
	else
		RXC0_RD = 0;
	return temp;
}

/****************************************************************************
Function Name: uart0发送字符串数据
Arguments: *s
Returns:
****************************************************************************/
void uart0_puts(char *s)     //发送字符串函数
{
	while (*s)
	{
		uart0_putchar(*s);
		s++;
	}
	uart0_putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
	uart0_putchar(0x0A);  //LF Line forward, for win+linux}
}
/****************************************************************************
Function Name: 初始化单片机
Arguments:
Returns: :
****************************************************************************/
void uart0_init_devices(void)
{
	CLI(); //关闭所有中断
	XDIV = 0x00;
	XMCRA = 0x00;
#ifndef _ATMEGA128A
	MCUCR = 0x00;
#endif // _ATMEGA128A
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
Descriptions: Interrupt Service Routines for TC0 RX complete. there is data in Rx buffer to be read.
****************************************************************************/
#pragma interrupt_handler uart0_rx_isr:iv_USART0_RXC   
void uart0_rx_isr(void)
{
	RXC0_BUFF[RXC0_WR] = UDR0;
	if (RXC0_WR < (RXC0_BUFF_SIZE - 1))
		RXC0_WR++;
	else
		RXC0_WR = 0;
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
	UDR0 = TXC0_BUFF[TXC0_RD];
	if (TXC0_RD < (TXC0_BUFF_SIZE - 1))
		TXC0_RD++;
	else
		TXC0_RD = 0;
	if (TXC0_RD == TXC0_WR)   //读指针==写指针,停止中断
	{
		//UCSR0B &= ~(1 << UDRIE0);
		Clr_Bit(UCSR0B, UDRIE0);    //DISABLE interrupt, no Tx data to send when TXC0_RD == TXC0_WR
	}
}
/****************************************************************************
Function Name: uart0_loopback
Arguments:
Returns:
Descriptions:
****************************************************************************/
void uart0_loopback(void)
{
	UINT8 flag = 0;
	//Clr_Bit(UCSR0B, RXEN0); //Disable TC0 Rx. Otherwise, Tx data will lose due to Rx interrupt
	while (RXC0_RD != RXC0_WR)
	{
		flag = 1;
		uart0_putchar(RXC0_BUFF[RXC0_RD]);
		if (RXC0_RD < (RXC0_BUFF_SIZE - 1))
			RXC0_RD++;
		else
			RXC0_RD = 0;
	}
	if (flag == 1)
	{
		uart0_putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
		uart0_putchar(0x0A);  //LF Line forward, for win+linux
	}
	//Set_Bit(UCSR0B, RXEN0); //Enable TC0 Rx
}
/****************************************************************************
Function Name: 主程序
Arguments:
Returns:
Descriptions:
****************************************************************************/
void main_uart0(void)
{
	//unsigned int i;
	TXC0_RD = 0;
	TXC0_WR = 0;
	RXC0_RD = 0;
	RXC0_WR = 0;
	uart0_init_devices();
	uart0_init_register();
	SEI();  //允许中断
	uart0_putchar('t');
	while (1)
	{
		delay_ms(1000);
		//if (uart0_getchar() == 't')//按键盘t键开始测试
		//{ 
		//    uart0_puts("test ok!");
		//    //for (i = 0; i < 10; i++) uart0_putchar(0x30 + i);
		//    uart0_puts("this is from printf function");
		//    printf("value is %d\r\n", i);
		//    //uart0_putchar(0x0D);
		//    //uart0_putchar(0x0A);
		//}
		uart0_loopback();
	}
}
/****************************************************************************
Function :      init_uart0
Arguments:
Returns:
Descriptions:
****************************************************************************/
void init_uart0(void)
{
	//unsigned int i;
	TXC0_RD = 0;
	TXC0_WR = 0;
	RXC0_RD = 0;
	RXC0_WR = 0;
	uart0_init_devices();
	uart0_init_register();
	SEI();  //允许中断

}
/*******************************************************************************
* Function Name: : uart0_putcharBackup()
* Arguments:
* Returns: :
* Descriptions:  Send out a data through uart0. For furture use.
*******************************************************************************/
int uart0_putcharBackup(char ch)
{
	//while (!(UCSR0A&(1 << UDRE0))); 
	while (!(Get_Bit(UCSR0A, UDRE0)));  //if UDRE0=1, Tx buffer is ready for next transmit.
	UDR0 = ch;
	return 0;
}

/*******************************************************************************
* Function Name: : putchar()
* Arguments:
* Returns: :
* Descriptions:  Send out a data through uart0. used for printf()
*******************************************************************************/
#if 0
int putchar(char ch)
{
	//while (!(Get_Bit(UCSR0A, UDRE0)));  //if UDRE0=1, Tx buffer is ready for next transmit.
	//UDR0 = ch;
	//return 0;
	TXC0_BUFF[TXC0_WR] = ch;
	if (TXC0_WR < (TXC0_BUFF_SIZE - 1))   //TXC0_BUFF_SIZE  发送区数据大小
		TXC0_WR++;
	else
		TXC0_WR = 0;
	//UCSR0B |= (1 << UDRIE0);          //开启UDRE中断
	Set_Bit(UCSR0B, UDRIE0);          //开启UDRE中断
	return 0;
}
#else
extern int _textmode;
int putchar(char c)
{
	if (_textmode && c == '\n')
		putchar('\r');
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
		;
	/* Putting data into buffer , sends the data */
	UDR0 = c;
	return c;
}
#endif

/*******************************************************************************
* Function Name: : getchar()
* Arguments:
* Returns: :
* Descriptions:  get a data through uart0. used for printf()
*******************************************************************************/
int getchar(void)
{
	while ((UCSR0A & 0x80) == 0)
		;
	return UDR0;
}


/*******************************************************************************
* Function Name: : uart0_getcharBackup()
* Arguments:
* Returns: :
* Descriptions:  receive a data through uart0. for future. TODO: will halt if no data come to uart0
*******************************************************************************/
UINT8 uart0_getcharBackup(void)
{
	while (!(Get_Bit(UCSR0A, RXC0)));
	return UDR0;
}





/****************************************************************************
Function Name: uart1发送单字节数据
Arguments: ch
Returns: :
****************************************************************************/
int uart1_putchar(char ch)
{
	UART1_TxBuf[TXC1_WR] = ch;
	if (TXC1_WR < (UART1_TX_BUFFER_SIZE - 1))   //UART1_TX_BUFFER_SIZE  发送区数据大小
		TXC1_WR++;
	else
		TXC1_WR = 0;
	//UCSR1B |= (1 << UDRIE1);          //开启UDRE中断
	Set_Bit(UCSR1B, UDRIE1);          //开启UDRE中断
	return 0;
}

/****************************************************************************
Function Name: uart1接收单字节数据
Arguments:
Returns: :
****************************************************************************/
UINT8 uart1_getchar(void)
{
	UINT8 temp;
	while (RXC1_RD == RXC1_WR)   //等待缓冲区内有数据输入
		;                   //死循环，等待输入数据
	temp = RXC1_BUFF[RXC1_RD];
	if (RXC1_RD < (RXC1_BUFF_SIZE - 1))
		RXC1_RD++;
	else
		RXC1_RD = 0;
	return temp;
}

/****************************************************************************
Function Name: uart1发送字符串数据
Arguments: *s
Returns:
****************************************************************************/
void uart1_puts(char *s)     //发送字符串函数
{
	while (*s)
	{
		uart1_putchar(*s);
		s++;
	}
	uart1_putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
	uart1_putchar(0x0A);  //LF Line forward, for win+linux}
}

/*******************************************************************************
* Function:     uart1_gets()
* Arguments:  *rcv
* Return:
* Description:  save UART1 received data to *rcv
*******************************************************************************/
void uart1_gets(UINT8 *rcv)
{
	while (RXC1_RD != RXC1_WR)
	{
		*rcv = RXC1_BUFF[RXC1_RD];
		if (RXC1_RD < (RXC1_BUFF_SIZE - 1))
			RXC1_RD++;
		else
			RXC1_RD = 0;
		rcv++;
	}
}

/*******************************************************************************
* Function:     uart1_checkCMDPolling()
* Arguments:
* Return:
* Description:  check if there is command received(polling)
*******************************************************************************/
void uart1_checkCMDPolling(void)
{
	UINT8 ch;
	while (RXC1_RD != RXC1_WR)
	{
		ch = RXC1_BUFF[RXC1_RD];
		uart1_putchar(ch);
		parseStr2Cmd(ch);
		if (RXC1_RD < (RXC1_BUFF_SIZE - 1))
			RXC1_RD++;
		else
			RXC1_RD = 0;
	}
}

/****************************************************************************
Function Name: 初始化单片机
Arguments:
Returns: :
****************************************************************************/
void uart1_init_devices(void)
{
	CLI(); //关闭所有中断
	XDIV = 0x00;
	XMCRA = 0x00;
#ifndef _ATMEGA128A
	MCUCR = 0x00;
#endif // _ATMEGA128A


	EICRA = 0x00;
	EICRB = 0x00;
	EIMSK = 0x00;
	TIMSK = 0x00;
	ETIMSK = 0x00;
}

/****************************************************************************
Function Name:  uart1_udre_isr
Arguments:
Returns:
Descriptions: when UDR1 is empty, we receive this interrupt. It means one data is just sent out.
We shall increase the Tx buffer addr by 1. If the buffer is empty, we shall disable
the UDR interrupt.
****************************************************************************/
#pragma interrupt_handler uart1_udre_isr:iv_USART1_UDRE   
#if 1
void uart1_udre_isr(void)
{
	UINT16 tmptail;
	/* check if all data is transmitted */
	if (UART1_TxHead != UART1_TxTail)
	{
		/* calculate buffer index */
		tmptail = (UART1_TxTail + 1) & UART1_TX_BUFFER_MASK;
		/*
		if (UART1_TxTail >= UART1_TX_BUFFER_SIZE - 1)
		{
			tmptail = 0;
		}
		else
		{
			tmptail = UART1_TxTail + 1;
		}
		*/
		UART1_TxTail = tmptail;				/* store new index */
		UDR1 = UART1_TxBuf[tmptail];	/* start transmition */
	}
	else
	{
		//UCR &= ~(1 << UDRIE);			/* disable UDRE interrupt */
		Clr_Bit(UCSR1B, UDRIE1);			//DISABLE interrupt, no Tx data to send when TXC1_RD == TXC1_WR
	}
}

#else
void uart1_udre_isr(void)
{
	UDR1 = UART1_TxBuf[TXC1_RD];
	if (TXC1_RD < (UART1_TX_BUFFER_SIZE - 1))
		TXC1_RD++;
	else
		TXC1_RD = 0;
	if (TXC1_RD == TXC1_WR)   //读指针==写指针,停止中断
	{
		//UCSR1B &= ~(1 << UDRIE1);
		Clr_Bit(UCSR1B, UDRIE1);    //DISABLE interrupt, no Tx data to send when TXC1_RD == TXC1_WR
	}
}

#endif
/****************************************************************************
Function Name: uart1_loopback
Arguments:
Returns:
Descriptions:
****************************************************************************/
void uart1_loopback(void)
{
	UINT8 flag = 0;
	//Clr_Bit(UCSR1B, RXEN1); //Disable TC0 Rx. Otherwise, Tx data will lose due to Rx interrupt
	while (RXC1_RD != RXC1_WR)
	{
		flag = 1;
		uart1_putchar(RXC1_BUFF[RXC1_RD]);
		if (RXC1_RD < (RXC1_BUFF_SIZE - 1))
			RXC1_RD++;
		else
			RXC1_RD = 0;
	}
	if (flag == 1)
	{
		uart1_putchar(0x0D);  //CR, cursors return. for windows, must be ahead of 0x0A
		uart1_putchar(0x0A);  //LF Line forward, for win+linux
	}
	//Set_Bit(UCSR1B, RXEN1); //Enable TC0 Rx
}


/*******************************************************************************
* Function Name: : uart1_putcharPolling()
* Arguments:
* Returns: :
* Descriptions:  Send out a data through uart1. For furture use.
*******************************************************************************/
int uart1_putcharPolling(char ch)
{
	//while (!(UCSR1A&(1 << UDRE1))); 
	while (!(Get_Bit(UCSR1A, UDRE1)));  //if UDRE1=1, Tx buffer is ready for next transmit.
	UDR1 = ch;
	return 0;
}


/****************************************************************************
Function Name: uart1初始化程序
Arguments:
Returns: :
****************************************************************************/
void uart1_init_register(void)  //初始化COM0
{
	UINT16 ubrr = UART1_UBRR;
	//printf("ubrr = %d\r\n", ubrr);
	UCSR1B = 0x00; //初始化
	UCSR1A = 0x00| (UART1_U2X<<U2X1); //uart1 initialization,  *U2X1=1, 2x speeds mode
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);//8bit
	Clr_Bit(UCSR1C, USBS1);        //USBS0=0: 1bit stop
#ifdef _ATMEGA128A  
	//U2X = 0
	UBRR1L = (UINT8)(ubrr);     //51 for UART1_BAUD rate 9600
	UBRR1H = (UINT8)(ubrr >> 8);     //0, UART1_BAUD rate = 9600, 0.2% error
#else   
								   //atmega64, 128
	UBRR1L = (CPU_CLK / 16 / (UART1_BAUD + 1)) % 256;     //52
	UBRR1H = (CPU_CLK / 16 / (UART1_BAUD + 1)) / 256;     //0, UART1_BAUD rate = 9600, 0.2% error
#endif // _ATMEGA128A
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1); //Rx enable, Tx enable, Rx interrupt enable
}

/*******************************************************************************
* Function:     uart1_DisableInterrupt()
* Arguments:
* Return:
* Description: disable UART1 interrupt
*******************************************************************************/
void uart1_DisableInterrupt(void)  //初始化COM0
{
	Clr_Bit(UCSR1B, RXCIE1);
	Clr_Bit(UCSR1B, UDRIE1);    //DISABLE interrupt, no Tx data to send when TXC1_RD == TXC1_WR
}

/*******************************************************************************
* Function Name: : uart1_getcharPolling()
* Arguments:
* Returns: :
* Descriptions:  receive a data through uart1. for future. TODO: will halt if no data come to uart1
*******************************************************************************/
UINT8 uart1_getcharPolling(void)
{
	while (!(Get_Bit(UCSR1A, RXC1)));
	return UDR1;
}
/****************************************************************************
Function Name: uart1_rx_isr
Arguments:
Returns:
Descriptions: Interrupt Service Routines for TC0 RX complete. there is data in Rx buffer to be read.
****************************************************************************/
#pragma interrupt_handler uart1_rx_isr:iv_USART1_RXC   
#if 1
void uart1_rx_isr(void)
{
	// TODO: check UPE/DOR/FE for errorflag before read UDR1: if error dummy=UDR1; else RXC1_BUFF[RXC1_WR]=UDR1
	/*UINT8 dummy;
	if (UCSR1A & (1 << FE1 | 1 << DOR1 | 1 << UPE1))
		dummy = UDR1;
	else*/
#if 1
	UINT8 ch;
	ch = UDR1;
	printf("%x\r\n", ch);
	//uart1_putchar(ch);
	parseStr2Cmd(ch);
#else
	RXC1_BUFF[RXC1_WR] = UDR1;
	if (RXC1_WR < (RXC1_BUFF_SIZE - 1))
		RXC1_WR++;
	else
		RXC1_WR = 0;
#endif
}
#else
void uart1_rx_isr(void)
{
	UINT8 data;
	UINT8 tmphead;
	data = UDR1; /* read the received data */
	/* calculate buffer index */
	tmphead = (UART_RxHead + 1) & UART_RX_BUFFER_MASK;
	UART_RxHead = tmphead; /* store new index */
	if (tmphead == UART_RxTail)
	{
		/* ERROR! Receive buffer overflow */
	}
	UART_RxBuf[tmphead] = data; /* store received data in buffer */
}
#endif
/****************************************************************************
Function Name: uart1_init_buffer
Arguments:
Returns:
Descriptions:
****************************************************************************/
void uart1_init_buffer(void)
{
	TXC1_RD = 0;
	TXC1_WR = 0;
	RXC1_RD = 0;
	RXC1_WR = 0;
	UART1_TxHead = 0;
	UART1_TxTail = 0;
	//printf("init TXC1_WR= %d, TXC1_RD=%d\n", TXC1_WR, TXC1_RD);
}

/****************************************************************************
Function Name: uart1_init
Arguments:
Returns:
Descriptions:
****************************************************************************/
void uart1_init(void)
{
	
	uart1_init_buffer();
	uart1_init_devices();
	uart1_init_register();
}
/****************************************************************************
Function Name: main_uart1_loopback
Arguments:
Returns:
Descriptions:
****************************************************************************/
void main_uart1_loopback(void)
{
	//unsigned int i;
	uart1_init();
	SEI();  //允许中断
	uart1_putchar('t');
	while (1)
	{
		delay_ms(1000);
		//if (uart1_getchar() == 't')//按键盘t键开始测试
		//{ 
		//    uart1_puts("test ok!");
		//    //for (i = 0; i < 10; i++) uart1_putchar(0x30 + i);
		//    uart1_puts("this is from printf function");
		//    printf("value is %d\r\n", i);
		//    //uart1_putchar(0x0D);
		//    //uart1_putchar(0x0A);
		//}
		uart1_loopback();
	}
}



#ifdef _TEST_CODE_INCLUDED
/****************************************************************************
Function:       char2int()
Arguments:    char
Returns:
Descriptions:  transfer char to int
****************************************************************************/
UINT8 char2int(UINT8 ch)
{
	UINT8 t;
	t = 'a';
	//return (UINT8)((UINT16)(ch));
	if ((ch >= '0') && (ch <= '9'))
		return ch - 48;
	else if ((ch >= 'A') && (ch <= 'Z'))
		return ch - 'A' + 65;
	else if ((ch >= 'a') && (ch <= 'z'))
		return ch - 'a' + 97;
	else return (-1);
}

/****************************************************************************
Function Name: test_char2int()
Arguments:
Returns:
Descriptions: transfer char to int
****************************************************************************/
void test_char2int(void)
{
	UINT8 *s = "123456789";
	UINT8 ch;
	UINT8 data;
	UINT8 command;
	TXC1_RD = 0;
	while (*s)
	{
		ch = *s;
		s++;
		command = (ch & 0xF0) >> 4;
		data = ch & 0x0F;
		switch (command)
		{
		case 0x00:
			break;
		case 0x3:
			if ((data >= 0) && (data <= 9))
				AddFifo(&CommandFifo, ch);
			break;
		default://printf("Illegal command!!\r\n"); 
			break;
		}
	}

	NOP();
	while (1);
}
/****************************************************************************
Function Name: test_copystr2TimeStamp()
Arguments:
Returns:
Descriptions: transfer char to int
****************************************************************************/
void test_copystr2TimeStamp(void)
{
	UINT8 str[8], *s;
	//!1412090C1100;
	s = str;
	s[0] = '"';
	s[1] = 0x14;
	s[2] = 0x12;
	s[3] = 0xA;
	s[4] = 0xB;
	s[5] = 0x12;
	s[6] = 0x2E;
	s[7] = 0;

	while (*s)
	{
		parseStr2Cmd(*s++);
	}
}
#endif // _TEST_CODE_INCLUDED

