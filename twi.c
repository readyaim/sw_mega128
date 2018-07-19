/*******************************
 *  TWI.c
 *  Created: 2015/10/30 14:48:50
 *  Author: Administrator
 ******************************/
#include "global.h"
#include "TWI.h"

 /**************************************
 **********发送缓存区*******************
 ***************************************/
UINT8 TWI_SendData[TWI_BUFFER_SIZE];
//发送完成标志位;
UINT8 TWI_Write_Finish = 0;


/**************************************
**********发送缓存区*******************
**************************************/
UINT8 TWI_ReceData[TWI_BUFFER_SIZE];
//读取完成标志位;
UINT8 TWI_Read_Finish = 0;


//定义传输数据长度;
UINT8 TWI_DataLength = 0;
UINT8 TWI_Index = 0;
//定义IIC设备的地址;
UINT8 IIC_DeviceAddr = 0;
extern void feed_watchdog(void);

/***************************************
**********功 能： TWI初始化***********
**********输入参数： 无 ***********
**********返回 值： 无 ***********
****************************************/
void TWI_Init(void)
{
    //Enable Pull-up resistor for portC(TWI port);
    Clr_Bit(SFIOR, PUD);    //enable all PUs
    DDRD &= (~((1 << DDD0) | (1 << DDD1)));   //enable PU of SCL and SDA
    PORTD |= (1 << DDD0) | (1 << DDD1);   //PU is enable by setting: "input" & PORTx=1
    TWCR = (1 << TWEN);     //Set PD0/PD1 as TWI port, not GPIO;
    //Setup frequency of TWI;
    TWBR = TWI_BitRate(100);
    //Release SDA bus
    TWDR = 0xff;

}
/***************************************
**********功 能： TWI时钟设置 ********
**********输入参数： 设置速率 ********
**********返回 值： TWBR的值   ********
****************************************/
UINT8 TWI_BitRate(UINT16 BitRateKHZ)
{
    if ((CPU_CLK / 1000) < (BitRateKHZ * 36))
    {
        return 10;
    }
    else
    {
        return (UINT8)((((CPU_CLK / 1000) / BitRateKHZ) - 16) / 2);
    }
}
/***************************************
**********功 能： 接受的数据  ******
**********输入参数： 无  ******
**********返回 值： TWDR中的数据  ******
****************************************/
UINT8 TWI_GetReceByte(void)
{
    return (TWDR);
}


/****************************************
**********功 能： 单字节发送数据 ******
**********输入参数： 发送的数据   ******
**********返回 值： 无   ******
****************************************/
void TWI_SendByte(UINT8 SendByte)
{
    TWDR = SendByte;
    TWI_Enable_Send();
}
/****************************************
**********功 能： 接收后是否应答  ******
**********输入参数： 1 应答；0 不应答 ****
**********返回 值： 无   ******
****************************************/
void TWI_RecByteAck(UINT8 AckFlag)
{
    if (AckFlag)
    {
        //Ack=true,接受数据完成应答回复;
        TWI_ReceACK();
    }
    else
    {
        //接收完成后不应答;
        TWI_ReceNACK();
    }
}
/*****************************************************************************
**********功 能： 主机中断模式发送数据 **************************************
**********输入参数： DeviceAddr 器件地址；length 数据长度 ；*pData 发送数据 ****
**********返回 值： 无 **************************************
*******************************************************************************/
void TWI_WriteToDevice(UINT8 DeviceAddr, UINT8 length, UINT8 *pData)
{
    //diable interrupt;
    Disable_Interrupt_TWI();
    TWI_DataLength = length;
    //fill data to Tx buffer;
    Fill_Data(length, pData);
    //Device write address;
    IIC_DeviceAddr = (DeviceAddr & 0xFE);
    //启动传输;
    TWI_StartTransmition();
    //等待写操作完成;
    while (!TWI_Write_Finish)
    {
        //wdt_reset();
        feed_watchdog();
    }
    //清楚写操作标志;
    TWI_Write_Finish = 0;
    //发送停止条件;
    TWI_Stop();
    //关闭中断;
    Disable_Interrupt_TWI();
}
/****************************************************
**********功 能： 将发送的数据填充缓存空间 *********
**********输入参数： length 数据长度；*pData 数据 ****
**********返回 值： 无   ******************
******************************************************/
void Fill_Data(UINT8 length, UINT8 *pData)
{
    UINT8 i;
    for (i = 0; i < length; i++)
    {
        TWI_SendData[i] = *(pData + i);
    }
}
/****************************************************
**********功 能： 启动数据传输   ******************
**********输入参数： 无   ******************
**********返回 值： 无   ******************
******************************************************/
void TWI_StartTransmition(void)
{
    //Start transmition;
    TWI_Start();
    //enable TWI interrupt;
    Enable_Interrupt_TWI();
}
/*****************************************************************************
**********功 能： 主机中断模式读取数据 **************************************
**********输入参数： DeviceAddr 器件地址；length 数据长度 *********************
**********返回 值： 无 **************************************
*******************************************************************************/
void TWI_ReadFromDevice(UINT8 DeviceAddr, UINT8 length)
{
    Disable_Interrupt_TWI();
    //要读取的数据的长度;
    TWI_DataLength = length;
    //读数据地址;
    IIC_DeviceAddr = (DeviceAddr | 0x01);
    //启动传输;
    TWI_StartTransmition();
    //等待读取完成;
    while (!TWI_Read_Finish);
    //清除IIC读标志位;
    TWI_Read_Finish = 0;
    //发送停止条件;
    TWI_Stop();
    //关闭TWI中断;
    Disable_Interrupt_TWI();
}


/***********************************************************************
**************中断函数**************************************************
************************************************************************/
#ifdef __GNUC__
SIGNAL(TWI_vect)
#else
//#pragma vector=TWI_vect
//__interrupt void TWI_ISR(void)
#pragma interrupt_handler TWI_isr:iv_TWI
void TWI_isr(void)
#endif
{
    switch (TWI_STATUS)
    {
        // 主机START信号传输完成;
    case TWI_START:
        // 主机重新开始信号传输完成;
    case TWI_REP_START:
        //发送IIC器件地址;
        TWI_SendByte(IIC_DeviceAddr);
        break;
        // 使能TWI总线, 使能TWI中断, 发送RESTART信号;
    case TWI_ARB_LOST:
        //启动TWI;
        TWI_Start();
        break;
        /************************************************
        *****主机发送模式,发送数据*************************
        *************************************************/
        // 主机发送-->从机地址及读写标志传输完成并收到ACK信号;
    case TWI_MT_SLA_W_ACK:
        // 主机发送-->数据传输完成并收到ACK信号;
    case TWI_MT_DATA_ACK:
        if (TWI_Index < TWI_DataLength)
        {
            //发送数据;
            TWI_SendByte(TWI_SendData[TWI_Index]);
            //加载下一个数据;
            TWI_Index++;
        }
        else
        {
            //清楚数组指向,为接受准备;
            TWI_Index = 0;
            TWI_Write_Finish = 1;
        }
        break;
        /************************************************
        *****主机接收模式,接收数据*************************
        *************************************************/
        // 主机接收模式下-->发送完成SLA+R 并收到ACK信号;
    case TWI_MR_SLA_R_ACK:
        if (TWI_Index < (TWI_DataLength - 1))
        {
            //接收到数据后应答;
            TWI_RecByteAck(1);
        }
        else
        {
            //接收到数据后不应答;
            TWI_RecByteAck(0);
        }
        break;
        // 主机接收模式下-->收到一个字节数据并发送完ACK信号;
    case TWI_MR_DATA_ACK:
        TWI_ReceData[TWI_Index] = TWI_GetReceByte();
        TWI_Index++;
        break;
        // 主机接收模式下收到数据，发送了NACK信号;
    case TWI_MR_DATA_NACK:
        //存储最后一组数据;
        TWI_ReceData[TWI_Index] = TWI_GetReceByte();
        //清楚数组指向,为下一组数据准备;
        TWI_Index = 0;
        //接收完成标志;
        TWI_Read_Finish = 1;
        //停止通信;
        TWI_Stop();
        break;
        /************************************************
          *****主机接收和发送模式,其他错误状态*************************
          *************************************************/
          // 主机发送模式发送从机地址后无应答;
    case TWI_MR_SLA_R_NACK:
        // 主机发送模式下数据传输完成无应答;
    case TWI_MT_DATA_NACK:
        // 主机接收模式发送从机地址后无应答;
    case TWI_MT_SLA_W_NACK:
        //释放总线;
        TWI_Stop();
        break;

        // 总线错误;
    case TWI_BUS_ERROR:
        TWI_Stop();
        break;
    default:
        TWCR = (1 << TWEN) |                          // 使能TWI总线，释放TWI端口;
            (0 << TWIE) | (0 << TWINT) |                      // 禁止中断;
            (0 << TWEA) | (0 << TWSTA) | (0 << TWSTO) |           //
            (0 << TWWC); //


    }
}


/******************************中断方式***********************/


/********************************************************************************
***********************查询方式*************************************************
*********************************************************************************/


/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 0 写入失败，1 写入成功  **********************************
*******************************************************************************/
UINT8 TWI_IIC_WriteToDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    TWI_Start();
    TWI_Wait();
    if (TWI_STATUS != TWI_START)
    {
        TWI_Stop();
        return 0;
    }
    //发送器件地址;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //没有应答结束通信;
        TWI_Stop();
        return 0;
    }
    //发送数据;
    for (i = 0; i < DataLength; i++)
    {
        TWI_SendByte(*pData);
        TWI_Wait();
        if (TWI_STATUS != TWI_MT_DATA_ACK)
        {
            //没有应答结束通信;
            TWI_Stop();
            return 0;
        }
        pData++;
    }
    TWI_Stop();
    return 1;
}
/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据存储的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 0 写入失败，1 写入成功  **********************************
*******************************************************************************/
UINT8 TWI_IIC_WriteToDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    TWI_Start();
    TWI_Wait();
    if (TWI_STATUS != TWI_START)
    {
        TWI_Stop();
        return 0;
    }
    //发送器件地址;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //发送数据地址;
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //发送数据;
    for (i = 0; i < DataLength; i++)
    {
        TWI_SendByte(*pData);
        TWI_Wait();
        if (TWI_STATUS != TWI_MT_DATA_ACK)
        {
            TWI_Stop();
            return 0;
        }
        pData++;
    }
    TWI_Stop();
    return 1;
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 0 读出失败，1 读出成功  **********************************
*******************************************************************************/
UINT8 TWI_IIC_ReadFromDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    //UINT8 ACK_NACK=0;
    TWI_Start();
    TWI_Wait();
    if (TWI_STATUS != TWI_START)
    {
        TWI_Stop();
        return 0;
    }
    //发送器件地址;
    TWI_SendByte(DeviceAddr | 0x01);
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_SLA_R_ACK)
    {
        //没有应答结束通信;
        TWI_Stop();
        return 0;
    }
    for (i = 0; i < DataLength - 1; i++)
    {
        if (TWI_ReciveDATA_ACK(pData + i) == 0)
        {
            return 0;
        }
    }
    //读取数据时，最后1字节的数据读取完成以后发送NAK而不是ACK
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据读出的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 0 读出失败，1 读出成功  **********************************
*******************************************************************************/
UINT8 TWI_IIC_ReadFromDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    //UINT8 ACK_NACK=0;
    TWI_Start();
    TWI_Wait();
    if (TWI_STATUS != TWI_START)
    {
        TWI_Stop();
        return 0;
    }
    //发送器件地址;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //没有应答结束通信;
        TWI_Stop();
        return 0;
    }
    //发送数据地址;
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        //没有应答结束通信;
        TWI_Stop();
        return 0;
    }
    TWI_Start();
    TWI_Wait();
    if ((TWI_STATUS != TWI_START) && (TWI_STATUS != TWI_REP_START))
    {
        TWI_Stop();
        return 0;
    }
    TWI_SendByte(DeviceAddr | 0x01);
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_SLA_R_ACK)
    {
        //没有应答结束通信;
        TWI_Stop();
        return 0;
    }

    for (i = 0; i < DataLength - 1; i++)
    {
        if (TWI_ReciveDATA_ACK(pData + i) == 0)
        {
            return 0;
        }
    }
    //读取数据时，最后1字节的数据读取完成以后发送NAK而不是ACK
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}
/*********************************************************************************
*****功能：接收数据，不发应答信号***************************************************
*****输入参数：存储接收到的数据buffer***********************************************
****输出参数：0：接收失败；1接收成功************************************************
*********************************************************************************/
UINT8  TWI_ReciveDATA_NACK(UINT8 *pdata)
{
    TWI_ReceNACK();
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_DATA_NACK)
    {
        return 0;
    }
    *pdata = TWI_GetReceByte();
    return 1;
}
/******************************************************************************
*****功能：接收数据，发应答信号***************************************************
*****输入参数：存储接收到的数据buffer*********************************************
****输出参数：0：接收失败；1接收成功***********************************************
*******************************************************************************/
UINT8  TWI_ReciveDATA_ACK(UINT8 *pdata)
{
    TWI_ReceACK();
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_DATA_ACK)
    {
        return 0;
    }
    *pdata = TWI_GetReceByte();
    return 1;
}
/********************************************************************************
***********************模拟IIC SDA--PC4   SCL--PC5********************************
*********************************************************************************/
/************注意：模拟IIC读和写数据之间必须有5ms的延时时****************************/




/*****************************************************************************
**********功 能： 模拟IIC初始化 ********************************************
**********输入参数：  无  ********************************************
**********返回 值： 无  ********************************************
*******************************************************************************/
void IIC_Init(void)
{
    //设置为输出模式;
    SDA_OUT;
    SCL_OUT;
    //设置输出为高;
    SDA_HIGH;
    SCL_HIGH;
}


/*****************************************************************************
**********功 能： 模拟IIC启动  ********************************************
**********输入参数：  无  ********************************************
**********返回 值： 无  ********************************************
*******************************************************************************/
void IIC_Start(void)
{
    //设置为输出模式;
    SDA_OUT;
    SCL_OUT;
    //钳住I2C总线，避免Start和Stop信号;
    SCL_LOW;
    //发送起始条件的数据信号;
    SDA_HIGH;
    SCL_HIGH;
    //发送起始信号;
    SDA_LOW;
    //钳住I2C总线，准备发送或接收数据
    SCL_LOW;

}
/*****************************************************************************
**********功 能： 模拟IIC停止  ********************************************
**********输入参数：  无  ********************************************
**********返回 值： 无  ********************************************
*******************************************************************************/
void IIC_Stop(void)
{
    //设置为输出模式;
    SDA_OUT;
    SCL_OUT;
    //钳住I2C总线，避免Start和Stop信号;
    SCL_LOW;
    //发送结束条件的数据信号;
    SDA_LOW;
    SCL_HIGH;
    SDA_HIGH;
}
/*****************************************************************************
**********功 能： 读取应答信号  ********************************************
**********输入参数：  无         ****************************************
**********返回 值： SDA_Read 1不应答，SDA_Read 0应答 **********************************
*******************************************************************************/
UINT8 IIC_ReadAck(void)
{
    UINT8 ACK_NACK = 0;
    UINT8 i = 0;
    SCL_OUT;
    SCL_LOW;
    //设置SDA为输入;
    SDA_IN;
    SCL_HIGH;
    for (i = 255; i > 0; i--)
    {
        if (SDA_Read)
        {
            ACK_NACK = 1;
            SCL_LOW;
            return ACK_NACK;
        }
    }
    SCL_LOW;
    return ACK_NACK = 0;
}


/*****************************************************************************
**********功 能： 发送数据  ********************************************
**********输入参数： 要发送的数据  ****************************************
**********返回 值： 无  **********************************
*******************************************************************************/
void IIC_SendByte(UINT8 Wdata)
{
    UINT8 Bitcnt = 0;
    SDA_OUT;
    SCL_OUT;
    for (Bitcnt = 0; Bitcnt < 8; Bitcnt++)
    {
        SCL_LOW;
        if (Wdata & 0x80)
        {
            SDA_HIGH;
        }
        else
        {
            SDA_LOW;
        }
        Wdata <<= 1;
        SCL_HIGH;
    }
}
/*****************************************************************************
**********功 能： 发送应答    *******************************************
**********输入参数： 1不应答，0应答 ********************************************
**********返回 值： 无 ******************************************
*******************************************************************************/
void IIC_Send_ACK_Or_NACK(UINT8 ACK_Or_NACK)
{
    SDA_OUT;
    SCL_OUT;
    SCL_LOW;
    if (ACK_Or_NACK)
    {
        //无应答信号;
        SDA_HIGH;
    }
    else
    {
        //应答信号;
        SDA_LOW;
    }
    SCL_HIGH;
    SCL_LOW;
}


/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 0 写入失败，1 写入成功  **********************************
*******************************************************************************/
UINT8 IIC_WriteToDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    IIC_Start();
    IIC_SendByte(DeviceAddr & 0xFE);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //发送数据;
        IIC_SendByte(*pData);
        if (IIC_ReadAck())
        {
            //没有应答，结束通信;
            IIC_Stop();
            return 0;
        }
        pData++;
    }
    IIC_Stop();
    return 1;
}


/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据存储的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 0 写入失败，1 写入成功  **********************************
*******************************************************************************/
UINT8 IIC_WriteToDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    //启动总线;
    IIC_Start();
    //发送器件地址;
    IIC_SendByte(DeviceAddr);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }
    //发送数据地址;
    IIC_SendByte(DataAddr);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //发送数据;
        IIC_SendByte(*pData);
        if (IIC_ReadAck())
        {
            //没有应答，结束通信;
            IIC_Stop();
            return 0;
        }
        pData++;
    }
    IIC_Stop();
    return 1;
}
/*****************************************************************************
**********功 能： 接收数据函数 ******************************************
**********输入参数： 无 ******************************************
**********返回 值： 接收到的数据 ******************************************
*******************************************************************************/
UINT8 IIC_ReceiveByte(void)
{
    UINT8 Bitcnt = 0;
    UINT8 RevData = 0;
    SCL_OUT;
    //将SDA设置为输入;
    SDA_IN;
    //SDA_LOW;
    for (Bitcnt = 0; Bitcnt < 8; Bitcnt++)
    {
        //置时钟线为低，准备接收数据位;
        SCL_LOW;
        //上拉使;
        SDA_HIGH;
        //置时钟线为高使数据线上数据有效;
        SCL_HIGH;
        RevData = RevData << 1;
        if (SDA_Read)
        {
            RevData += 1;
        }
        else
        {
            RevData += 0;
        }
    }
    return RevData;
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 0 读出失败，1 读出成功  **********************************
*******************************************************************************/
UINT8 IIC_ReadFromDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    UINT8 ACK_NACK = 1;
    IIC_Start();
    IIC_SendByte(DeviceAddr | 0x01);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //接收的数据;
        *pData = IIC_ReceiveByte();
        if (i == (DataLength - 1))
        {
            ACK_NACK = 1;
        }
        else
        {
            pData++;
            ACK_NACK = 0;
        }
        //发送应答或不应答信号;
        IIC_Send_ACK_Or_NACK(ACK_NACK);
    }
    IIC_Stop();
    return 1;
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据读出的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 0 读出失败，1 读出成功  **********************************
*******************************************************************************/
UINT8 IIC_ReadFromDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    UINT8 ACK_NACK = 1;
    IIC_Start();
    IIC_SendByte(DeviceAddr & 0xFE);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }

    IIC_SendByte(DataAddr);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_SendByte(DeviceAddr | 0x01);
    if (IIC_ReadAck())
    {
        //没有应答，结束通信;
        IIC_Stop();
        return 0;
    }

    for (i = 0; i < DataLength; i++)
    {
        //接收的数据;
        *pData = IIC_ReceiveByte();
        if (i == (DataLength - 1))
        {
            ACK_NACK = 1;
        }
        else
        {
            pData++;
            ACK_NACK = 0;
        }
        //发送应答h或不应答信号;
        IIC_Send_ACK_Or_NACK(ACK_NACK);
    }
    IIC_Stop();
    return 1;
}


/******************************模拟方式***********************/




/*****************************************************************************
**********功 能： 硬件还是软件模拟的IIC通信的初始化 ***************************
**********输入参数：  无  ********************************************
**********返回 值： 无  ********************************************
*******************************************************************************/
void IIC_TWI_Init(void)
{
    if (TWI_IIC_MODE == 0)
    {
        Enable_interrupt();
        TWI_Init();
    }
    else if (TWI_IIC_MODE == 1)
    {
        TWI_Init();
    }
    else
    {
        IIC_Init();
    }
}
/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 无                     **********************************
*******************************************************************************/
void IIC_TWI_WriteToDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    if (TWI_IIC_MODE == 0)
    {
        TWI_WriteToDevice(DeviceAddr, DataLength, pData);
    }
    else if (TWI_IIC_MODE == 1)
    {
        TWI_IIC_WriteToDevice(DeviceAddr, pData, DataLength);
    }
    else
    {
        IIC_WriteToDevice(DeviceAddr, pData, DataLength);
    }
}
/*****************************************************************************
**********功 能： 向设备写入数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据存储的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 无                     **********************************
*******************************************************************************/
void IIC_TWI_WriteToDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    if (TWI_IIC_MODE == 0)
    {
        //TWI_WriteToDevice(DeviceAddr,DataLength,*pData);
    }
    else if (TWI_IIC_MODE == 1)
    {
        TWI_IIC_ReadFromDeviceByAddr(DeviceAddr, DataAddr, pData, DataLength);
    }
    else
    {
        IIC_WriteToDeviceByAddr(DeviceAddr, DataAddr, pData, DataLength);
    }
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，pdata 数据，DataLength 数据长度 ******
**********返回 值： 无                     **********************************
*******************************************************************************/
void IIC_TWI_ReadFromDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    if (TWI_IIC_MODE == 0)
    {
        TWI_ReadFromDevice(DeviceAddr, DataLength);
    }
    else if (TWI_IIC_MODE == 1)
    {
        TWI_IIC_ReadFromDevice(DeviceAddr, pData, DataLength);
    }
    else
    {
        IIC_ReadFromDevice(DeviceAddr, pData, DataLength);
    }
}
/*****************************************************************************
**********功 能： 从设备读出数据  ****************************************
**********输入参数： DeviceAddr 设备地址，DataAddr 数据读出的地址 **************
******************   pdata 数据，DataLength 数据长度  *************************
**********返回 值： 无                     **********************************
*******************************************************************************/
void IIC_TWI_ReadFromDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    if (TWI_IIC_MODE == 0)
    {
        //TWI_ReadFromDevice(DeviceAddr,DataLength);
    }
    else if (TWI_IIC_MODE == 1)
    {
        TWI_IIC_ReadFromDeviceByAddr(DeviceAddr, DataAddr, pData, DataLength);
    }
    else
    {
        IIC_ReadFromDeviceByAddr(DeviceAddr, DataAddr, pData, DataLength);
    }
}