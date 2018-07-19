#include "global.h"
#include "peripherals.h"

static UINT8 key_state = 0; //static: not allow other file.c use key_state
extern void delay_us(UINT16 microsecond);
extern void init_beep(void);
const UINT8 tab[] = { 0X3F,0X06,0X5B,0X4F,0X66,0X6D,0X7D,0X07,0X7F,0X6F };// LED 7-Segments

void find_key(UINT8 n)
{
    switch (n)
    {
    case 1:SEG_PORT = tab[1];
        break;
    case 2:SEG_PORT = tab[2];
        break;
    case 3:SEG_PORT = tab[3];
        break;
    case 4:SEG_PORT = tab[4];
        break;
    case 5:SEG_PORT = tab[5];
        break;
    case 6:SEG_PORT = tab[6];
        break;
    case 7:SEG_PORT = tab[7];
        break;
    case 8:SEG_PORT = tab[8];
        break;
    case 9:SEG_PORT = tab[9];
        break;
    default:
        break;
    }
}
/*******************************************************************************
* Function:  void key_scan(void)
* Arguments: 
* Return: 
* Description:  键盘扫描
*******************************************************************************/
void key_scan(void)
{
    //key_scan_state = 1;
    ////第一行////////////////////////////////////////
    PORTE = 0XDF;
    delay_ms(10);
    if ((PINE & 0x1F) != 0x1F)
    {
        delay_ms(20);
        if ((PINE & 0x1F) != 0x1F)
        {
            if ((PINE & 0x10) == 0)//E5-E4	
            {
                key_state = 7;
                while ((PINE & 0x10) == 0);
            }
            else if ((PINE & 0x08) == 0)//E5-E3	
            {
                key_state = 4;
                while ((PINE & 0x08) == 0);
            }
            else if ((PINE & 0x04) == 0)//E5-E2	
            {
                key_state = 1;
                while ((PINE & 0x04) == 0);
            }
            beep();
            delay_ms(1);
        }
    }
    ////第二行////////////////////////////////////////
    PORTE = 0XBF;
    delay_ms(10);
    if ((PINE & 0x1F) != 0x1F)
    {
        delay_ms(20);
        if ((PINE & 0x1F) != 0x1F)
        {
            if ((PINE & 0x10) == 0)//E6-E4
            {
                key_state = 8;
                while ((PINE & 0x10) == 0);
            }
            else if ((PINE & 0x08) == 0)	//E6-E3		
            {
                key_state = 5;
                while ((PINE & 0x08) == 0);
            }
            else if ((PINE & 0x04) == 0)//E6-E2		
            {
                key_state = 2;
                while ((PINE & 0x04) == 0);
            }
            beep();
            delay_ms(1);
        }
    }
    ////第3行////////////////////////////////////////
    PORTE = 0X7F;
    delay_ms(10);
    if ((PINE & 0x1F) != 0x1F)
    {
        delay_ms(20);
        if ((PINE & 0x1F) != 0x1F)
        {
            if ((PINE & 0x10) == 0)//E7-E4
            {
                key_state = 9;
                while ((PINE & 0x10) == 0);
            }
            else if ((PINE & 0x08) == 0)//E7-E3
            {
                key_state = 6;
                while ((PINE & 0x08) == 0);
            }
            else if ((PINE & 0x04) == 0)//E7-E2
            {
                key_state = 3;
                while ((PINE & 0x04) == 0);
            }
            beep();
            delay_ms(1);
        }
    }
}
/****************************************************************************
Function: 延时子程序
Arguments: 
Return: 
****************************************************************************/
//void delay(void)
//{
//    INT16 i;
//    for (i = 0; i < 200; i++);
//}
void init_port_keyboard(void)
{
    CLI(); //disable all interrupts
    XDIV = 0x00; //xtal divider
    NOP();  //writing to XDIV cause unstable running of code. Run 8 NOP()s to wait clk stable
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    //SEI();
    XMCRA = 0x00; //external memory
    init_beep();
    DDRD = 0XFF;
    PORTD = 0XFF;
    DDRE = 0xE0 ;//高3位为写,低5位为读
    PORTE = 0x1F;
    DDRG = 0XFF;
    PORTG = 0X1A;
}
/****************************************************************************
Function: 主程序
Arguments: 
Return: 
****************************************************************************/
void main_key2SEG(void)
{
    
    INT16 keyvalue = 0;
    init_port_keyboard();
    beep();
    while (1)
    {
        key_scan();
        find_key(key_state);
    }
}
