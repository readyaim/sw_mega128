/*******************************
 *  usr_gprs232_7sr.c
 *  Created: 2018/08/06 17:04:50
 *  Author: Administrator
 * Function: GPRS module driver for ATmega128
 ******************************/
#include "global.h"

extern void uart1_DisableInterrupt(void);
extern void uart1_init_devices(void);
extern void uart1_puts(char *s);     //·¢ËÍ×Ö·û´®º¯Êý
extern UINT8 uart1_getchar(void);

 /*******************************************************************************
 * Function:     test_spi_loop_Tx_inquiry()
 * Arguments:
 * Return:
 * Description: output 0xA5 and 0x86
 *******************************************************************************/
void run(void)
{
    NOP();
}

/*******************************************************************************
* Function:     commandMode
* Arguments: 
* Return:
* Description: Enter command Mode (from net/sms/httpd mode)
*******************************************************************************/
void commandMode(void)
{
    UINT8 *s = "+";
    UINT8 rcv;
    NOP();
    delay_ms(250);  //
    uart1_puts(s);
    uart1_puts(s);
    uart1_puts(s);
    rcv = uart1_getchar();
    //if (rcv == "a")
    //{
    //    uart1_puts(s);
    //}
    rcv = uart1_getchar();
    
}

/*******************************************************************************
* Function:     commandMode
* Arguments:
* Return:
* Description: Enter command Mode (from net/sms/httpd mode)
*******************************************************************************/
void gprs_7SR_init(void)
{
    //uart1_DisableInterrupt();
    uart1_init_devices();
}

