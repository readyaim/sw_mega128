/****************************************************************************
* File name: command.c
* Description: polling commands in FIFO in main while. And run the cmd.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/
#include "global.h"

extern volatile UINT16 UART1_TxHead;
extern volatile UINT16 UART1_TxTail;
extern UINT8 UART1_TxBuf[UART1_TX_BUFFER_SIZE];   //定义发送缓冲区


/*******************************************************************************
* Function:     init_modem()
* Arguments:
* Return:
* Description:  Initiae modem device
*******************************************************************************/
void init_modem()
{
    //TODO: check if modem is ready
    Clr_Bit(DDRC, PORTC7);		//1:OUT, 0:IN
    Set_Bit(PORTC, PORTC7);

}

/*******************************************************************************
* Function:     modem_ready()
* Arguments:
* Return:        true if modem is ready, false if modem is not ready
* Description:  GPIO for modem ready shall be initiated.(INPUT, PU)
*******************************************************************************/
BOOL modem_ready(void)
{
    //TODO: check if modem is ready
    if Get_Bit(PORTC, PORTC7)
        return true;
    else
        return true;
}


/*******************************************************************************
* Function:     requestTimeFromServer()
* Arguments:
* Return:
* Description: request Time from server by sending "r+Time" when modem module is ready.
                     GPIO for modem ready shall be initiated.(INPUT, PU)
*******************************************************************************/
void requestTimeFromServer(void)
{
    UINT8 data;
    UINT16 i = 0;
    UINT16 tmphead;
    UINT8 cmd[8] = { 'r','+','T','i','m','e',0x0D, 0x0A };		//LOW8, HIGH8, LOW8, HIGH8
    if (modem_ready())
    {
        for (i = 0; i < 7; i++)
        {

            tmphead = (UART1_TxHead + 1) & UART1_TX_BUFFER_MASK;
            /* wait for free space in buffer */
            while (tmphead == UART1_TxTail)
                ;
            UART1_TxBuf[tmphead] = cmd[i];	/* store data in buffer */
            UART1_TxHead = tmphead;	/* store new index */
            Set_Bit(UCSR1B, UDRIE1);          //enable UART1 Tx interrupt
        }
    }
}
