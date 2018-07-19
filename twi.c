/*******************************
 *  TWI.c
 *  Created: 2015/10/30 14:48:50
 *  Author: Administrator
 ******************************/
#include "global.h"
#include "TWI.h"

 /**************************************
 **********���ͻ�����*******************
 ***************************************/
UINT8 TWI_SendData[TWI_BUFFER_SIZE];
//������ɱ�־λ;
UINT8 TWI_Write_Finish = 0;


/**************************************
**********���ͻ�����*******************
**************************************/
UINT8 TWI_ReceData[TWI_BUFFER_SIZE];
//��ȡ��ɱ�־λ;
UINT8 TWI_Read_Finish = 0;


//���崫�����ݳ���;
UINT8 TWI_DataLength = 0;
UINT8 TWI_Index = 0;
//����IIC�豸�ĵ�ַ;
UINT8 IIC_DeviceAddr = 0;
extern void feed_watchdog(void);

/***************************************
**********�� �ܣ� TWI��ʼ��***********
**********��������� �� ***********
**********���� ֵ�� �� ***********
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
**********�� �ܣ� TWIʱ������ ********
**********��������� �������� ********
**********���� ֵ�� TWBR��ֵ   ********
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
**********�� �ܣ� ���ܵ�����  ******
**********��������� ��  ******
**********���� ֵ�� TWDR�е�����  ******
****************************************/
UINT8 TWI_GetReceByte(void)
{
    return (TWDR);
}


/****************************************
**********�� �ܣ� ���ֽڷ������� ******
**********��������� ���͵�����   ******
**********���� ֵ�� ��   ******
****************************************/
void TWI_SendByte(UINT8 SendByte)
{
    TWDR = SendByte;
    TWI_Enable_Send();
}
/****************************************
**********�� �ܣ� ���պ��Ƿ�Ӧ��  ******
**********��������� 1 Ӧ��0 ��Ӧ�� ****
**********���� ֵ�� ��   ******
****************************************/
void TWI_RecByteAck(UINT8 AckFlag)
{
    if (AckFlag)
    {
        //Ack=true,�����������Ӧ��ظ�;
        TWI_ReceACK();
    }
    else
    {
        //������ɺ�Ӧ��;
        TWI_ReceNACK();
    }
}
/*****************************************************************************
**********�� �ܣ� �����ж�ģʽ�������� **************************************
**********��������� DeviceAddr ������ַ��length ���ݳ��� ��*pData �������� ****
**********���� ֵ�� �� **************************************
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
    //��������;
    TWI_StartTransmition();
    //�ȴ�д�������;
    while (!TWI_Write_Finish)
    {
        //wdt_reset();
        feed_watchdog();
    }
    //���д������־;
    TWI_Write_Finish = 0;
    //����ֹͣ����;
    TWI_Stop();
    //�ر��ж�;
    Disable_Interrupt_TWI();
}
/****************************************************
**********�� �ܣ� �����͵�������仺��ռ� *********
**********��������� length ���ݳ��ȣ�*pData ���� ****
**********���� ֵ�� ��   ******************
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
**********�� �ܣ� �������ݴ���   ******************
**********��������� ��   ******************
**********���� ֵ�� ��   ******************
******************************************************/
void TWI_StartTransmition(void)
{
    //Start transmition;
    TWI_Start();
    //enable TWI interrupt;
    Enable_Interrupt_TWI();
}
/*****************************************************************************
**********�� �ܣ� �����ж�ģʽ��ȡ���� **************************************
**********��������� DeviceAddr ������ַ��length ���ݳ��� *********************
**********���� ֵ�� �� **************************************
*******************************************************************************/
void TWI_ReadFromDevice(UINT8 DeviceAddr, UINT8 length)
{
    Disable_Interrupt_TWI();
    //Ҫ��ȡ�����ݵĳ���;
    TWI_DataLength = length;
    //�����ݵ�ַ;
    IIC_DeviceAddr = (DeviceAddr | 0x01);
    //��������;
    TWI_StartTransmition();
    //�ȴ���ȡ���;
    while (!TWI_Read_Finish);
    //���IIC����־λ;
    TWI_Read_Finish = 0;
    //����ֹͣ����;
    TWI_Stop();
    //�ر�TWI�ж�;
    Disable_Interrupt_TWI();
}


/***********************************************************************
**************�жϺ���**************************************************
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
        // ����START�źŴ������;
    case TWI_START:
        // �������¿�ʼ�źŴ������;
    case TWI_REP_START:
        //����IIC������ַ;
        TWI_SendByte(IIC_DeviceAddr);
        break;
        // ʹ��TWI����, ʹ��TWI�ж�, ����RESTART�ź�;
    case TWI_ARB_LOST:
        //����TWI;
        TWI_Start();
        break;
        /************************************************
        *****��������ģʽ,��������*************************
        *************************************************/
        // ��������-->�ӻ���ַ����д��־������ɲ��յ�ACK�ź�;
    case TWI_MT_SLA_W_ACK:
        // ��������-->���ݴ�����ɲ��յ�ACK�ź�;
    case TWI_MT_DATA_ACK:
        if (TWI_Index < TWI_DataLength)
        {
            //��������;
            TWI_SendByte(TWI_SendData[TWI_Index]);
            //������һ������;
            TWI_Index++;
        }
        else
        {
            //�������ָ��,Ϊ����׼��;
            TWI_Index = 0;
            TWI_Write_Finish = 1;
        }
        break;
        /************************************************
        *****��������ģʽ,��������*************************
        *************************************************/
        // ��������ģʽ��-->�������SLA+R ���յ�ACK�ź�;
    case TWI_MR_SLA_R_ACK:
        if (TWI_Index < (TWI_DataLength - 1))
        {
            //���յ����ݺ�Ӧ��;
            TWI_RecByteAck(1);
        }
        else
        {
            //���յ����ݺ�Ӧ��;
            TWI_RecByteAck(0);
        }
        break;
        // ��������ģʽ��-->�յ�һ���ֽ����ݲ�������ACK�ź�;
    case TWI_MR_DATA_ACK:
        TWI_ReceData[TWI_Index] = TWI_GetReceByte();
        TWI_Index++;
        break;
        // ��������ģʽ���յ����ݣ�������NACK�ź�;
    case TWI_MR_DATA_NACK:
        //�洢���һ������;
        TWI_ReceData[TWI_Index] = TWI_GetReceByte();
        //�������ָ��,Ϊ��һ������׼��;
        TWI_Index = 0;
        //������ɱ�־;
        TWI_Read_Finish = 1;
        //ֹͣͨ��;
        TWI_Stop();
        break;
        /************************************************
          *****�������պͷ���ģʽ,��������״̬*************************
          *************************************************/
          // ��������ģʽ���ʹӻ���ַ����Ӧ��;
    case TWI_MR_SLA_R_NACK:
        // ��������ģʽ�����ݴ��������Ӧ��;
    case TWI_MT_DATA_NACK:
        // ��������ģʽ���ʹӻ���ַ����Ӧ��;
    case TWI_MT_SLA_W_NACK:
        //�ͷ�����;
        TWI_Stop();
        break;

        // ���ߴ���;
    case TWI_BUS_ERROR:
        TWI_Stop();
        break;
    default:
        TWCR = (1 << TWEN) |                          // ʹ��TWI���ߣ��ͷ�TWI�˿�;
            (0 << TWIE) | (0 << TWINT) |                      // ��ֹ�ж�;
            (0 << TWEA) | (0 << TWSTA) | (0 << TWSTO) |           //
            (0 << TWWC); //


    }
}


/******************************�жϷ�ʽ***********************/


/********************************************************************************
***********************��ѯ��ʽ*************************************************
*********************************************************************************/


/*****************************************************************************
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� 0 д��ʧ�ܣ�1 д��ɹ�  **********************************
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
    //����������ַ;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //û��Ӧ�����ͨ��;
        TWI_Stop();
        return 0;
    }
    //��������;
    for (i = 0; i < DataLength; i++)
    {
        TWI_SendByte(*pData);
        TWI_Wait();
        if (TWI_STATUS != TWI_MT_DATA_ACK)
        {
            //û��Ӧ�����ͨ��;
            TWI_Stop();
            return 0;
        }
        pData++;
    }
    TWI_Stop();
    return 1;
}
/*****************************************************************************
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݴ洢�ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� 0 д��ʧ�ܣ�1 д��ɹ�  **********************************
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
    //����������ַ;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //�������ݵ�ַ;
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //��������;
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
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� 0 ����ʧ�ܣ�1 �����ɹ�  **********************************
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
    //����������ַ;
    TWI_SendByte(DeviceAddr | 0x01);
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_SLA_R_ACK)
    {
        //û��Ӧ�����ͨ��;
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
    //��ȡ����ʱ�����1�ֽڵ����ݶ�ȡ����Ժ���NAK������ACK
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}
/*****************************************************************************
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݶ����ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� 0 ����ʧ�ܣ�1 �����ɹ�  **********************************
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
    //����������ַ;
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //û��Ӧ�����ͨ��;
        TWI_Stop();
        return 0;
    }
    //�������ݵ�ַ;
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        //û��Ӧ�����ͨ��;
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
        //û��Ӧ�����ͨ��;
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
    //��ȡ����ʱ�����1�ֽڵ����ݶ�ȡ����Ժ���NAK������ACK
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}
/*********************************************************************************
*****���ܣ��������ݣ�����Ӧ���ź�***************************************************
*****����������洢���յ�������buffer***********************************************
****���������0������ʧ�ܣ�1���ճɹ�************************************************
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
*****���ܣ��������ݣ���Ӧ���ź�***************************************************
*****����������洢���յ�������buffer*********************************************
****���������0������ʧ�ܣ�1���ճɹ�***********************************************
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
***********************ģ��IIC SDA--PC4   SCL--PC5********************************
*********************************************************************************/
/************ע�⣺ģ��IIC����д����֮�������5ms����ʱʱ****************************/




/*****************************************************************************
**********�� �ܣ� ģ��IIC��ʼ�� ********************************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
*******************************************************************************/
void IIC_Init(void)
{
    //����Ϊ���ģʽ;
    SDA_OUT;
    SCL_OUT;
    //�������Ϊ��;
    SDA_HIGH;
    SCL_HIGH;
}


/*****************************************************************************
**********�� �ܣ� ģ��IIC����  ********************************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
*******************************************************************************/
void IIC_Start(void)
{
    //����Ϊ���ģʽ;
    SDA_OUT;
    SCL_OUT;
    //ǯסI2C���ߣ�����Start��Stop�ź�;
    SCL_LOW;
    //������ʼ�����������ź�;
    SDA_HIGH;
    SCL_HIGH;
    //������ʼ�ź�;
    SDA_LOW;
    //ǯסI2C���ߣ�׼�����ͻ��������
    SCL_LOW;

}
/*****************************************************************************
**********�� �ܣ� ģ��IICֹͣ  ********************************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
*******************************************************************************/
void IIC_Stop(void)
{
    //����Ϊ���ģʽ;
    SDA_OUT;
    SCL_OUT;
    //ǯסI2C���ߣ�����Start��Stop�ź�;
    SCL_LOW;
    //���ͽ��������������ź�;
    SDA_LOW;
    SCL_HIGH;
    SDA_HIGH;
}
/*****************************************************************************
**********�� �ܣ� ��ȡӦ���ź�  ********************************************
**********���������  ��         ****************************************
**********���� ֵ�� SDA_Read 1��Ӧ��SDA_Read 0Ӧ�� **********************************
*******************************************************************************/
UINT8 IIC_ReadAck(void)
{
    UINT8 ACK_NACK = 0;
    UINT8 i = 0;
    SCL_OUT;
    SCL_LOW;
    //����SDAΪ����;
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
**********�� �ܣ� ��������  ********************************************
**********��������� Ҫ���͵�����  ****************************************
**********���� ֵ�� ��  **********************************
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
**********�� �ܣ� ����Ӧ��    *******************************************
**********��������� 1��Ӧ��0Ӧ�� ********************************************
**********���� ֵ�� �� ******************************************
*******************************************************************************/
void IIC_Send_ACK_Or_NACK(UINT8 ACK_Or_NACK)
{
    SDA_OUT;
    SCL_OUT;
    SCL_LOW;
    if (ACK_Or_NACK)
    {
        //��Ӧ���ź�;
        SDA_HIGH;
    }
    else
    {
        //Ӧ���ź�;
        SDA_LOW;
    }
    SCL_HIGH;
    SCL_LOW;
}


/*****************************************************************************
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� 0 д��ʧ�ܣ�1 д��ɹ�  **********************************
*******************************************************************************/
UINT8 IIC_WriteToDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    IIC_Start();
    IIC_SendByte(DeviceAddr & 0xFE);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //��������;
        IIC_SendByte(*pData);
        if (IIC_ReadAck())
        {
            //û��Ӧ�𣬽���ͨ��;
            IIC_Stop();
            return 0;
        }
        pData++;
    }
    IIC_Stop();
    return 1;
}


/*****************************************************************************
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݴ洢�ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� 0 д��ʧ�ܣ�1 д��ɹ�  **********************************
*******************************************************************************/
UINT8 IIC_WriteToDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    //��������;
    IIC_Start();
    //����������ַ;
    IIC_SendByte(DeviceAddr);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }
    //�������ݵ�ַ;
    IIC_SendByte(DataAddr);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //��������;
        IIC_SendByte(*pData);
        if (IIC_ReadAck())
        {
            //û��Ӧ�𣬽���ͨ��;
            IIC_Stop();
            return 0;
        }
        pData++;
    }
    IIC_Stop();
    return 1;
}
/*****************************************************************************
**********�� �ܣ� �������ݺ��� ******************************************
**********��������� �� ******************************************
**********���� ֵ�� ���յ������� ******************************************
*******************************************************************************/
UINT8 IIC_ReceiveByte(void)
{
    UINT8 Bitcnt = 0;
    UINT8 RevData = 0;
    SCL_OUT;
    //��SDA����Ϊ����;
    SDA_IN;
    //SDA_LOW;
    for (Bitcnt = 0; Bitcnt < 8; Bitcnt++)
    {
        //��ʱ����Ϊ�ͣ�׼����������λ;
        SCL_LOW;
        //����ʹ;
        SDA_HIGH;
        //��ʱ����Ϊ��ʹ��������������Ч;
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
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� 0 ����ʧ�ܣ�1 �����ɹ�  **********************************
*******************************************************************************/
UINT8 IIC_ReadFromDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    UINT8 ACK_NACK = 1;
    IIC_Start();
    IIC_SendByte(DeviceAddr | 0x01);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }
    for (i = 0; i < DataLength; i++)
    {
        //���յ�����;
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
        //����Ӧ���Ӧ���ź�;
        IIC_Send_ACK_Or_NACK(ACK_NACK);
    }
    IIC_Stop();
    return 1;
}
/*****************************************************************************
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݶ����ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� 0 ����ʧ�ܣ�1 �����ɹ�  **********************************
*******************************************************************************/
UINT8 IIC_ReadFromDeviceByAddr(UINT8 DeviceAddr, UINT8 DataAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    UINT8 ACK_NACK = 1;
    IIC_Start();
    IIC_SendByte(DeviceAddr & 0xFE);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }

    IIC_SendByte(DataAddr);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_SendByte(DeviceAddr | 0x01);
    if (IIC_ReadAck())
    {
        //û��Ӧ�𣬽���ͨ��;
        IIC_Stop();
        return 0;
    }

    for (i = 0; i < DataLength; i++)
    {
        //���յ�����;
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
        //����Ӧ��h��Ӧ���ź�;
        IIC_Send_ACK_Or_NACK(ACK_NACK);
    }
    IIC_Stop();
    return 1;
}


/******************************ģ�ⷽʽ***********************/




/*****************************************************************************
**********�� �ܣ� Ӳ���������ģ���IICͨ�ŵĳ�ʼ�� ***************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
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
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� ��                     **********************************
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
**********�� �ܣ� ���豸д������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݴ洢�ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� ��                     **********************************
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
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��pdata ���ݣ�DataLength ���ݳ��� ******
**********���� ֵ�� ��                     **********************************
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
**********�� �ܣ� ���豸��������  ****************************************
**********��������� DeviceAddr �豸��ַ��DataAddr ���ݶ����ĵ�ַ **************
******************   pdata ���ݣ�DataLength ���ݳ���  *************************
**********���� ֵ�� ��                     **********************************
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