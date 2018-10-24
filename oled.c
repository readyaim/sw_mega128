/****************************************************************************
* File name: oled.c
* Description: driver for uart0 and uart1, both polling and interrupt.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

/****************************** include header ********************************/
#include "global.h"
#include "twi.h"
#include "oledfont.h" 
/**************************** Marco Definition **********************************/
/* Pin Definition */
#define     OLED_SCLK_Set()			SCL_HIGH	//PD0（SCL）输出高
#define     OLED_SCLK_Clr()			SCL_LOW	//PD0（SCL）输出低
#define	    OLED_SDIN_Set()			SDA_HIGH	//PD1（SDA）输出高
#define	    OLED_SDIN_Clr()			SDA_LOW	//PD1（SDA）输出高
//#define 	    OLED_READ_SDIN()		SDA_IN;delay_us(IIC_DELAY_TIME);SDA_Read;delay_us(IIC_DELAY_TIME);SDA_OUT		//读取PB4（SDA）电平

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

#define SIZE 		16		//显示字符的大小
#define Max_Column	128		//最大列数
#define Max_Row		64		//最大行数
#define X_WIDTH 	128		//X轴的宽度
#define Y_WIDTH 	64	    //Y轴的宽度					  				   
#define	IIC_ACK		0		//应答
#define	IIC_NO_ACK	1		//不应答


extern void IIC_Init(void);
extern void delay_us(UINT16 microsecond);
extern void delay_ms(UINT16 millisecond);


/********************************************************
* @file        	oled.c
* @author      	LGG
* @version     	V1.0.0
* @data        	2017年06月6号
* @brief       	OELD文件
*
*******************************************************
*	@attention
*
*	PG0-->LED1		PG1-->LED2
* Px_ODR-输出数据寄存器
* Px_IDR-输入数据寄存器
* Px_DDR-数据方向寄存器
* Px_CR1-控制寄存器1
* Px_CR2-控制寄存器2
*
********************************************************/

/*
@brief			延迟1us
static void delay(UINT8 num)
{
    while (num--);
}



static void delay_ms(UINT16 ms)//延迟函数，MS级别
{
    UINT16 x, y;
    for (x = ms; x > 0; x--)
    {
        for (y = 1300; y > 0; y--);
    }
}


/*
@brief			模拟IIC起始信号
@param			无
@retval			无
*/
/*******************************************************************************
* Function     : OLED_IIC_Start()
* Arguments:
* Returns: :
* Descriptions: 
*******************************************************************************/
void OLED_IIC_Start(void)
{

    OLED_SCLK_Set();	//时钟线置高
    OLED_SDIN_Set();	//信号线置高
    delay_us(1);	//延迟1us
    OLED_SDIN_Clr();	//信号线置低
    delay_us(1);	//延迟1us
    OLED_SCLK_Clr();	//时钟线置低
    delay_us(1);	//延迟1us
}


/*
@brief			模拟IIC停止信号
@param			无
@retval			无
*/
static void OLED_IIC_Stop(void)
{
    OLED_SDIN_Clr();	//信号线置低
    delay_us(1);	//延迟1us
    OLED_SCLK_Set();	//时钟线置高
    delay_us(1);	//延迟1us
    OLED_SDIN_Set();	//信号线置高
    delay_us(1);	//延迟1us
}

UINT8 OLED_READ_SDIN(void)
{
    UINT8 ack;
    SDA_IN; 
    //delay_us(IIC_DELAY_TIME); 
    delay_us(1);
    ack = TWI_PORT_IN & (1 << SDA_Pin); 
    //delay_us(IIC_DELAY_TIME); 
    delay_us(1);
    SDA_OUT;		//读取（SDA）电平
    //printf("ack is %d", ack);
    //return ack;
    //TODO: why ACK is not received?
    return ack=0;   //Mandatory ack back
}


/*
@brief			模拟IIC读取从机应答信号
@param			无
@retval			无
*/
static UINT8 IIC_Wait_Ack_bk(void)
{
    UINT8 ack;

    OLED_SCLK_Clr();	//时钟线置低
    delay_us(1);	//延迟1us
    OLED_SDIN_Set();	//信号线置高
    delay_us(1);	//延迟1us
    OLED_SCLK_Set();	//时钟线置高
    delay_us(1);	//延迟1us

    if (OLED_READ_SDIN())	//读取SDA的电平
        ack = IIC_NO_ACK;	//如果为1，则从机没有应答
    else
        ack = IIC_ACK;		//如果为0，则从机应答

    OLED_SCLK_Clr();//时钟线置低
    delay_us(1);	//延迟1us

    return ack;	//返回读取到的应答信息
}
static UINT8 IIC_Wait_Ack(void)
{
    UINT8 ack;
    
    OLED_SCLK_Clr();	//时钟线置低
    //delay_us(1);	//延迟1us
    OLED_SCLK_Set();	//时钟线置高
    OLED_SCLK_Clr();	//时钟线置低
                        //delay_us(1);	//延迟1us
    return ack=0;	//返回读取到的应答信息
}


/*
@brief			模拟IIC主机发送应答信号
@param			无
@retval			无
*/
// static void IIC_Send_Ack(UINT8 ack)
// {
// 	OLED_SCLK_Clr();	//时钟线置低
// 	delay_us(1);	//延迟1us

// 	if(ack == IIC_ACK)	//根据情况发送应答/不应答
// 		OLED_SDIN_Clr();	//应答
// 	else
// 		OLED_SDIN_Set();	//不应答
// 	delay_us(1);	//延迟1us

// 	OLED_SCLK_Set();	//时钟线置高，发送应答/不应答信号
// 	delay_us(1);	//延迟1us
// 	OLED_SCLK_Clr();	//时钟线置低
// 	delay_us(1);	//延迟1us
// }


/*
@brief			IIC写入一个字节
@param			IIC_Byte：写入的字节
@retval			无
*/
static void Write_IIC_Byte(UINT8 IIC_Byte)
{
    UINT8 i;  //定义变量
    for (i = 0; i < 8; i++) //for循环8次
    {
        OLED_SCLK_Clr();	//时钟线置低，为传输数据做准备
        delay_us(1);	//延迟1us

        if (IIC_Byte & 0x80)	//读取最高位
        {
            OLED_SDIN_Set();	//最高位为1
        }
        else
        {
            OLED_SDIN_Clr();	//最高位为0
        }
        IIC_Byte <<= 1;  //数据左移1位
        delay_us(1);	//延迟1us
        OLED_SCLK_Set(); //时钟线置高，产生上升沿，把数据发送出去
        delay_us(1);	//延迟1us
    }
    OLED_SCLK_Clr();	//时钟线置低
    delay_us(1);	//延迟1us
    //printf("wait for ack");
    while (IIC_Wait_Ack());	//从机应答
}



/*
@brief			IIC写读取一个字节
@param			ack：应答/不应答
@retval			无
*/
// static UINT8 Read_IIC_Byte(UINT8 ack)
// {
// 	UINT8 data = 0;  //定义变量
// 	UINT8 i;  //定义变量

// 	OLED_SCLK_Clr();	//时钟线置低，为传输数据做准备
// 	delay_us(1);	//延迟1us

// 	OLED_SDIN_Set();

// 	for(i=0;i<8;i++) //for循环8次
// 	{
// 		OLED_SCLK_Set();	//时钟线置高，读取数据
// 		delay_us(1);	//延迟1us

// 		data <<= 1;	//左移1位
// 		if(OLED_READ_SDIN())	//读取最高位
// 		  	data |= 0x01;	//读到高位位1
// 		else 
// 			data &= 0xfe;	//读到高位为0

// 		OLED_SCLK_Clr();	//时钟线置低，准备读取下一个数据
// 		delay_us(1);	//延迟1us
// 	}

// 	IIC_Send_Ack(ack);	//发送应答/不应答

// 	return data;	//返回读取到的数据
// }


/*
@brief			IIC写入命令
@param			IIC_Command：写入的命令
@retval			无
*/
static void Write_IIC_Command(UINT8 IIC_Command)
{
    OLED_IIC_Start();
    Write_IIC_Byte(0x78);//写入从机地址，SD0 = 0
    Write_IIC_Byte(0x00);//写入命令
    Write_IIC_Byte(IIC_Command);//数据
    OLED_IIC_Stop();  //发送停止信号
}


/*
@brief			IIC写入数据
@param			IIC_Data：数据
@retval			无
*/
static void Write_IIC_Data(UINT8 IIC_Data)
{
    OLED_IIC_Start();
    Write_IIC_Byte(0x78);	//写入从机地址，SD0 = 0
    Write_IIC_Byte(0x40);	//写入数据
    Write_IIC_Byte(IIC_Data);//数据
    OLED_IIC_Stop();		//发送停止信号
}


/*
@brief			对OLED写入一个字节
@param			dat：数据
cmd：1，写诶数据；0，写入命令
@retval			无
*/
void OLED_WR_Byte(UINT8 dat, UINT8 cmd)
{
    if (cmd)
    {
        Write_IIC_Data(dat); //写入数据
    }
    else {
        Write_IIC_Command(dat); //写入命令
    }
}


/*
@brief			设置数据写入的起始行、列
@param			x： 列的起始低地址与起始高地址；0x00~0x0f：设置起始列低地址（在页寻址模式）；
0x10~0x1f：设置起始列高地址（在页寻址模式）
y：起始页地址 0~7
@retval			无
*/
void OLED_Set_Pos(UINT8 x, UINT8 y)
{
    OLED_WR_Byte(0xb0 + y, OLED_CMD);//写入页地址
    OLED_WR_Byte((x & 0x0f), OLED_CMD);  //写入列的地址  低半字节
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);//写入列的地址 高半字节
}


/*
@brief			开显示
@param			无
@retval			无
*/
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //设置OLED电荷泵
    OLED_WR_Byte(0X14, OLED_CMD);  //使能，开
    OLED_WR_Byte(0XAF, OLED_CMD);  //开显示
}


/*
@brief			关显示
@param			无
@retval			无
*/
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0XAE, OLED_CMD);  //关显示
    OLED_WR_Byte(0X8D, OLED_CMD);  //设置OLED电荷泵
    OLED_WR_Byte(0X10, OLED_CMD);  //失能，关
}


/*
@brief			清屏
@param			无
@retval			无
*/
void OLED_Clear(void)
{
    UINT8 i, n;		    //定义变量
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);    //从0~7页依次写入
        OLED_WR_Byte(0x00, OLED_CMD);      //列低地址
        OLED_WR_Byte(0x10, OLED_CMD);      //列高地址  
        for (n = 0; n < 128; n++)OLED_WR_Byte(0, OLED_DATA); //写入 0 清屏
    }
}


/*
@brief			显示一个字符
@param			x：起始列
y：起始页，SIZE = 16占两页；SIZE = 12占1页
chr：字符
@retval			无
*/
void OLED_ShowChar(UINT8 x, UINT8 y, UINT8 chr)
{
    UINT8 c = 0, i = 0;
    c = chr - ' '; //获取字符的偏移量	
    if (x > Max_Column - 1) { x = 0; y = y + 2; } //如果列数超出了范围，就从下2页的第0列开始

    if (SIZE == 16) //字符大小如果为 16 = 8*16
    {
        OLED_Set_Pos(x, y);	//从x y 开始画点
        for (i = 0; i < 8; i++)  //循环8次 占8列
            OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA); //找出字符 c 的数组位置，先在第一页把列画完
        OLED_Set_Pos(x, y + 1); //页数加1
        for (i = 0; i < 8; i++)  //循环8次
            OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA); //把第二页的列数画完
    }
    else 	//字符大小为 6 = 6*8
    {
        OLED_Set_Pos(x, y + 1); //一页就可以画完
        for (i = 0; i < 6; i++) //循环6次 ，占6列
            OLED_WR_Byte(F6x8[c][i], OLED_DATA); //把字符画完
    }
}


/*
@brief			计算m^n
@param			m：输入的一个数
n：输入数的次方
@retval			result：一个数的n次方
*/
UINT16 oled_pow(UINT8 m, UINT8 n)
{
    UINT16 result = 1;
    while (n--)result *= m;
    return result;
}


/*
@brief			在指定的位置，显示一个指定长度大小的数
@param			x：起始列
y：起始页
num：数字
len：数字的长度
size：显示数字的大小
@retval			无
*/
void OLED_ShowNum(UINT8 x, UINT8 y, UINT16 num, UINT8 len, UINT8 size)
{
    UINT8 t, temp;  //定义变量
    UINT8 enshow = 0;		//定义变量

    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;//取出输入数的每个位，由高到低
        if (enshow == 0 && t < (len - 1)) //enshow：是否为第一个数；t<(len-1)：判断是否为最后一个数
        {
            if (temp == 0) //如果该数为0 
            {
                OLED_ShowChar(x + (size / 2)*t, y, ' ');//显示 0 ；x+(size2/2)*t根据字体大小偏移的列数（8）
                continue; //跳过剩下语句，继续重复循环（避免重复显示）
            }
            else enshow = 1;
        }
        OLED_ShowChar(x + (size / 2)*t, y, temp + '0'); //显示一个位；x+(size2/2)*t根据字体大小偏移的列数（8）
    }
}


/*
@brief			显示字符串
@param			x：起始列
y：起始页
*chr：第一个字符首地址
@retval			无
*/
void OLED_ShowString(UINT8 x, UINT8 y, UINT8 *chr)
{
    UINT8 j = 0; //定义变量

    while (chr[j] != '\0') //如果不是最后一个字符
    {
        OLED_ShowChar(x, y, chr[j]); //显示字符
        x += 8; //列数加8 ，一个字符的列数占8
        if (x >= 128) { x = 0; y += 2; } //如果x大于等于128，切换页，从该页的第一列显示
        j++; //下一个字符
    }
}


/*
@brief			显示中文
@param			x：起始列；一个字体占16列
y：起始页；一个字体占两页
no：字体的序号
@retval			无
*/
void OLED_ShowCHinese(UINT8 x, UINT8 y, UINT8 no)
{
    UINT8 t, adder = 0; //定义变量

    OLED_Set_Pos(x, y);	//从 x y 开始画点，先画第一页
    for (t = 0; t < 16; t++) //循环16次，画第一页的16列
    {
        OLED_WR_Byte(Hzk[2 * no][t], OLED_DATA);//画no在数组位置的第一页16列的点
    }
    OLED_Set_Pos(x, y + 1); //画第二页
    for (t = 0; t < 16; t++)//循环16次，画第二页的16列
    {
        OLED_WR_Byte(Hzk[2 * no + 1][t], OLED_DATA);//画no在数组位置的第二页16列的点
    }
}


/*
@brief			显示图片
@param			x0：起始列地址
y0：起始页地址
x1：终止列地址
y1：终止页地址
BMP[]：存放图片代码的数组
@retval			无
*/
void OLED_DrawBMP(UINT8 x0, UINT8 y0, UINT8 x1, UINT8 y1, UINT8 BMP[])
{
    UINT16 j = 0; //定义变量
    UINT8 x, y; //定义变量

    if (y1 % 8 == 0) y = y1 / 8;   //判断终止页是否为8的整数倍
    else y = y1 / 8 + 1;

    for (y = y0; y < y1; y++) //从起始页开始，画到终止页
    {
        OLED_Set_Pos(x0, y); //在页的起始列开始画
        for (x = x0; x < x1; x++) //画x1 - x0 列
        {
            OLED_WR_Byte(BMP[j++], OLED_DATA);	//画图片的点    	
        }
    }
}


/*
@brief			OLED初始化函数
@param			无
@retval			无
*/
void OLED_Init(void)
{
    IIC_Init();	//GPIO口初始化

    delay_ms(200);	//延迟，由于单片机上电初始化比OLED快，所以必须加上延迟，等待OLED上复位完成
    printf("1 byte\n");
    OLED_WR_Byte(0xAE, OLED_CMD);	//关闭显示
    printf("2 byte\n");
    OLED_WR_Byte(0x00, OLED_CMD);	//设置低列地址
    OLED_WR_Byte(0x10, OLED_CMD);	//设置高列地址
    OLED_WR_Byte(0x40, OLED_CMD);	//设置起始行地址
    OLED_WR_Byte(0xB0, OLED_CMD);	//设置页地址

    OLED_WR_Byte(0x81, OLED_CMD); 	// 对比度设置，可设置亮度
    OLED_WR_Byte(0xFF, OLED_CMD);	//  265  

    OLED_WR_Byte(0xA1, OLED_CMD);	//设置段（SEG）的起始映射地址；column的127地址是SEG0的地址
    OLED_WR_Byte(0xA6, OLED_CMD);	//正常显示；0xa7逆显示

    OLED_WR_Byte(0xA8, OLED_CMD);	//设置驱动路数（16~64）
    OLED_WR_Byte(0x3F, OLED_CMD);	//64duty

    OLED_WR_Byte(0xC8, OLED_CMD);	//重映射模式，COM[N-1]~COM0扫描

    OLED_WR_Byte(0xD3, OLED_CMD);	//设置显示偏移
    OLED_WR_Byte(0x00, OLED_CMD);	//无偏移

    OLED_WR_Byte(0xD5, OLED_CMD);	//设置震荡器分频
    OLED_WR_Byte(0x80, OLED_CMD);	//使用默认值

    OLED_WR_Byte(0xD9, OLED_CMD);	//设置 Pre-Charge Period
    OLED_WR_Byte(0xF1, OLED_CMD);	//使用官方推荐值

    OLED_WR_Byte(0xDA, OLED_CMD);	//设置 com pin configuartion
    OLED_WR_Byte(0x12, OLED_CMD);	//使用默认值

    OLED_WR_Byte(0xDB, OLED_CMD);	//设置 Vcomh，可调节亮度（默认）
    OLED_WR_Byte(0x40, OLED_CMD);	////使用官方推荐值

    OLED_WR_Byte(0x8D, OLED_CMD);	//设置OLED电荷泵
    OLED_WR_Byte(0x14, OLED_CMD);	//开显示

    OLED_WR_Byte(0xAF, OLED_CMD);	//开启OLED面板显示
    OLED_Clear();        //清屏
    OLED_Set_Pos(0, 0); 	 //设置数据写入的起始行、列
}


/*
@brief			OLED滚屏函数，范围0~1页，水平向左
@param			无
@retval			无
*/
void OLED_Scroll(void)
{
    OLED_WR_Byte(0x2E, OLED_CMD);	//关闭滚动
    OLED_WR_Byte(0x27, OLED_CMD);	//水平向左滚动
    OLED_WR_Byte(0x00, OLED_CMD);	//虚拟字节
    OLED_WR_Byte(0x00, OLED_CMD);	//起始页 0
    OLED_WR_Byte(0x00, OLED_CMD);	//滚动时间间隔
    OLED_WR_Byte(0x01, OLED_CMD);	//终止页 1
    OLED_WR_Byte(0x00, OLED_CMD);	//虚拟字节
    OLED_WR_Byte(0xFF, OLED_CMD);	//虚拟字节
    OLED_WR_Byte(0x2F, OLED_CMD);	//开启滚动
}


/*******************************************************************************
* Function Name: : test_OLED()
* Arguments:
* Returns: :
* Descriptions:  receive a data through uart1. for future. TODO: will halt if no data come to uart1
*******************************************************************************/
void test_OLED(void)
{

    OLED_Init();  //OLED初始化
  
    //OLED_ShowString(30, 2, "Kevin LET's GO!");//OLED显示  OLED TEST
    OLED_ShowString(0, 2, "Kevin I LOVE U");//OLED显示  OLED TEST
    OLED_ShowString(0, 4, "Lisa I LOVE U");//OLED显示  OLED TEST
    OLED_ShowCHinese(16, 0, 0);//OLED显示  技
    OLED_ShowCHinese(32, 0, 1);//OLED显示  新
    OLED_ShowCHinese(48, 0, 2);//OLED显示  电
    OLED_ShowCHinese(64, 0, 3);//OLED显示  子
    OLED_ShowCHinese(80, 0, 4);//OLED显示  科
    OLED_ShowCHinese(96, 0, 5);//OLED显示  技
    delay_ms(2000); //延迟2000ms
    //OLED_Clear(); //OELD清屏
    //OLED_DrawBMP(0, 0, 128, 8, BMP1); //显示图片

    while (1)
    {
        delay_ms(300); //延迟300ms
    }

}
