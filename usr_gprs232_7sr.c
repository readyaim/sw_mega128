/*******************************
 *  usr_gprs232_7sr.c
 *  Created: 2018/08/06 17:04:50
 *  Author: Administrator
 *  Function: GPRS module driver for ATmega128
 ******************************/
#include "global.h"

extern void uart1_DisableInterrupt(void);
extern void uart1_init_devices(void);
extern void uart1_puts(char *s);     //发送字符串函数
extern UINT8 uart1_getchar(void);
extern void uart1_gets(UINT8 *rcv);

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
* Function:     enterCommandMode()
* Arguments:  
* Return:        0: fail; 1: success
* Description: Enter command Mode (from net/sms/httpd mode)
*******************************************************************************/
UINT8 enterCommandMode(void)
{
    UINT8 *s = "+";
    UINT8 rcv[5];       //define the length of received AT command, in 'char'
    UINT8 i=0;
    NOP();
    delay_ms(250);  //
    uart1_puts(s);
    uart1_puts(s);
    uart1_puts(s);

    //rcv = uart1_getcharPolling(); //查询方式?
    rcv[0] = uart1_getchar();  
    if (rcv[0] == 'a')
    {
        uart1_puts("a");
    }
    else
    {
        return 0;
    }
    //strcmp
    while (i < 3)
    {
        rcv[i] = uart1_getchar();
        i++;
    }
    rcv[i] = '\0';  //change rcv to a string
    strlowerCase(rcv);      //to lower case
    if (!strcmp(rcv, "+ok"))
    {
        //equal, success
        return 1;   
    }
    else
    {
        //not equal, failed
        return 0;
    }
}

/*******************************************************************************
* Function:     enterWorkMode()
* Arguments:
* Return:        0: fail; 1: success
* Description: Enter work(net/sms/httpd) mode (from command mode)
*******************************************************************************/
UINT8 enterWorkMode(void)
{
    UINT8 *s = "AT+ENTM";
    UINT8 rcv[3];
    UINT8 i = 0;
    NOP();
    uart1_puts(s);

    while (i < 2)
    {
        rcv[i] = uart1_getchar();
        i++;
    }
    rcv[i] = '\0';  //change rcv to a string
    strlowerCase(rcv);      //to lower case
    if (!strcmp(rcv, "+ok"))
    {
        //equal, success
        return 1;
    }
    else
    {
        //not equal, failed
        return 0;
    }
}
/*******************************************************************************
* Function:     at_cmd_send()
* Arguments:  *cmd: a string to be sent
* Return:        0: fail; 1: success
* Description:  send at command through UART1
*******************************************************************************/
UINT8 at_cmd_send(UINT8 *cmd)
{
    UINT8 at_cmd[60] = "at+";
    strcat(at_cmd, cmd);
    uart1_puts(at_cmd);
    return 1;
}

/*******************************************************************************
* Function:     at_read_until()
* Arguments:  *cmd: a string to be sent
* Return:        0: fail; 1: success
* Description:  send at command through UART1
*******************************************************************************/
UINT8 at_read_until(UINT8 *rcv)
{
    
    UINT16 counter = 0xFF;
    rcv[0] = '\0';
    while (counter > 1)
    {
        uart1_gets(rcv);
        counter--;
        if (rcv[0] == '\0')
            // no data received
            continue;
        else
            break;
    }
    if (counter <= 1)
        return 0;   //no data recived from UART1, timeout
    else
        return 1;   //data received, saved in rcv
    
}

/*******************************************************************************
* Function:     gprs_7SR_init()
* Arguments:
* Return:
* Description: 
*******************************************************************************/
void gprs_7SR_init(void)
{
    //uart1_DisableInterrupt();
    uart1_init_devices();
}

/*******************************************************************************
* Function:     strlowerCase()
* Arguments:  string  
* Return:
* Description: transfer A-Z to a-z if *s is in capital.
*******************************************************************************/
void strlowerCase(UINT8 *s)
{
    while (*s)
    {
        if ((*s >= 'A') && (*s <= 'Z'))
        {
            *s += 32;
        }
        s++;
    }
}



#ifdef _DUMMY_CODE
/*******************************************************************************
* Function:     test_strcmp
* Arguments:
* Return:
* Description: to verify strcmp() function
*******************************************************************************/
void test_strcmp(void)
{
    UINT8 *s = "+";
    UINT8 rcv[128];
    UINT8 i = 0;
    rcv[i] = '+';
    i++;
    rcv[i] = 'O';
    i++;
    rcv[i] = 'K';
    i++;
    rcv[i] = '\0';  //end of a string
    strlowerCase(rcv);
    if (!strcmp(rcv, "+ok"))
    {
        //equal
        NOP();
        while (1);
    }
    else
    {
        //not equal
        while (1);
    }
}

/*******************************************************************************
* Function:     test_usr()
* Arguments:
* Return:
* Description: test function
*******************************************************************************/
void test_usr()
{
    /*
    UINT8 command[20] = "at+";
    UINT8 *s = "VER";
    strcat(command, s);
    while (1);
    */

    UINT8 rcv[128] = { '\0' };
    UINT16 counter = 0x0003;
    while (counter > 1)
    {
        uart1_gets(rcv);
        counter--;
        if (rcv[0] == '\0')
            // no data received
            continue;
        else
            break;
    }

    while (1);
}


#endif // _DUMMY_CODE

