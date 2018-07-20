/*******************************
 *  TWI.c
 *  Created: 2015/10/30 14:48:50
 *  Author: Administrator
 * Function: TWI driver
 ******************************/
#include "global.h"
#include "TWI.h"

/*Transmit data buffer*/
UINT8 TWI_SendData[TWI_BUFFER_SIZE];
//Flag of writing done;
UINT8 TWI_Write_Finish = 0;


/*Receive data buffer*/
UINT8 TWI_ReceData[TWI_BUFFER_SIZE];
/*finish reading flag*/
UINT8 TWI_Read_Finish = 0;


/*Data length*/
UINT8 TWI_DataLength = 0;
UINT8 TWI_Index = 0;

/*Device Address*/
UINT8 IIC_DeviceAddr = 0;

/*feed watch dog*/
extern void feed_watchdog(void);

/*******************************************************************************
* Function:  TWI_Init()
* Arguments:
* Return:
* Description:  Initiate TWI devices
*******************************************************************************/
void TWI_Init(void)
{
    //Enable Pull-up resistor for portC(TWI port);
    Clr_Bit(SFIOR, PUD);    //enable all PUs
    TWI_PORT_DDR &= (~((1 << SDA_Pin) | (1 << SCL_Pin)));   //enable PU of SCL and SDA
    TWI_PORT |= (1 << SDA_Pin) | (1 << SCL_Pin);   //PU is enable by setting: "input" & PORTx=1
    //TWCR = (1 << TWEN);     
    Set_Bit(TWCR, TWEN);    //Set PD0/PD1 as TWI port, not GPIO;
    //Setup frequency of TWI;
    TWBR = TWI_BitRate(100);
    //Release SDA bus, ????
    TWDR = 0xff;        

}

/*******************************************************************************
* Function:  TWI_BitRate()
* Arguments: frequency(in KHz)
* Return:       TWBR value
* Description:  Caculate TWI frequency
*******************************************************************************/
UINT8 TWI_BitRate(UINT16 BitRateKHZ)
{
    if ((CPU_CLK / 1000) < (BitRateKHZ * 36))
    {
        return 10;      //TWBR must be larger than 10
    }
    else
    {
        return (UINT8)((((CPU_CLK / 1000) / BitRateKHZ) - 16) / 2);
    }
}

/*******************************************************************************
* Function:     TWI_GetReceByte()
* Arguments: 
* Return:       TWDR
* Description: Return the received data, to be compatible to "GPIO Mode" of TWI
*******************************************************************************/
UINT8 TWI_GetReceByte(void)
{
    return (TWDR);
}

/*******************************************************************************
* Function:     TWI_SendByte()
* Arguments:  the 1 byte Data to be sent   
* Return:       
* Description: Send out 1 byte
*******************************************************************************/
void TWI_SendByte(UINT8 SendByte)
{
    TWDR = SendByte;
    TWI_Enable_Send();
}
/*******************************************************************************
* Function:     TWI_RecByteAck()
* Arguments:  AckFlag
* Return:
* Description: 1: send ACK after data received, 0: No ACK after receive data
*******************************************************************************/
void TWI_RecByteAck(UINT8 AckFlag)
{
    if (AckFlag)
    {
        //Send ACK after data received
        TWI_ReceACK();
    }
    else
    {
        //No ACK after data received
        TWI_ReceNACK();
    }
}
/*******************************************************************************
* Function:     TWI_WriteToDevice()
* Arguments:  DeviceAddr, length of data,  DataBuffer
* Return:
* Description: TWI Master send out data in INTERRUPT MODE.
*******************************************************************************/
void TWI_WriteToDevice(UINT8 DeviceAddr, UINT8 length, UINT8 *pData)
{
    //diable interrupt;
    Disable_Interrupt_TWI();
    TWI_DataLength = length;
    //fill data to Tx buffer;
    Fill_Data(length, pData);
    //Device write address(High 7 bits);
    IIC_DeviceAddr = (DeviceAddr & 0xFE);   
    //Start Sending data
    TWI_StartTransmition();
    //waiting for write data to be finished
    while (!TWI_Write_Finish)   
    {
        //wdt_reset();
        feed_watchdog();
    }
    //reset write finished flag
    TWI_Write_Finish = 0;
    //send STO
    TWI_Stop();
    //disable TWI interrupt
    Disable_Interrupt_TWI();
}

/*******************************************************************************
* Function:     Fill_Data()
* Arguments:  length of data,  Data Buffer
* Return:
* Description: Move data to TWI_SendData, why????
*******************************************************************************/

void Fill_Data(UINT8 length, UINT8 *pData)
{
    UINT8 i;
    for (i = 0; i < length; i++)
    {
        TWI_SendData[i] = *(pData + i);
    }
}

/*******************************************************************************
* Function:     TWI_StartTransmition()
* Arguments:  
* Return:
* Description: Need to write TWINT = 1 to clear TWINT. Then Set TWIE=1 to start TWINT
*******************************************************************************/
void TWI_StartTransmition(void)
{
    //Start transmition;
    TWI_Start();    //Start IE, EN, "Start bit", //TODO: might be an issue. Should set TWINT after TWAR/TWSR/TWDR are set
    //enable TWI interrupt;
    Enable_Interrupt_TWI();
    

}
/*******************************************************************************
* Function:     TWI_ReadFromDevice()
* Arguments:  Device Address, Data Length  
* Return:
* Description: Read "Length" data from device. TWI Master device read data in INTERRUPT MODE.
*******************************************************************************/
void TWI_ReadFromDevice(UINT8 DeviceAddr, UINT8 length)
{
    Disable_Interrupt_TWI();
    //Data Length;
    TWI_DataLength = length;
    //Address;
    IIC_DeviceAddr = (DeviceAddr | 0x01);
    //Start;
    TWI_StartTransmition();     //Enable Interrupt
    //Wait for finish reading
    while (!TWI_Read_Finish);   //Wait for TWI ISR to move data. Might be good to use: while(TWI_Read_Finish>0);
    //reset flag, repeated
    TWI_Read_Finish = 0;
    //Send "STOP" signal;
    TWI_Stop();
    //Disable interrupt;
    Disable_Interrupt_TWI();
}


/*******************************************************************************
* Function:     TWI_isr()
* Arguments:  
* Return:
* Description: ISR (interrupt service routine) of TWI
*******************************************************************************/
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
        case TWI_START:     //STA is sent out
        case TWI_REP_START:     //Repeated STA is sent out
            TWI_SendByte(IIC_DeviceAddr);   //Send slave device address, ??? Need to check if TWIE ==1
            break;
        case TWI_ARB_LOST:      //Arbitration failed, send START when BUS is available
            TWI_Start();
            break;  //TODO: ERROR() 
         //Master send data, after STA->ADDR->W/R MODE->ACK
        case TWI_MT_SLA_W_ACK:  
        //Master send next data, after ACK
        case TWI_MT_DATA_ACK:
            if (TWI_Index < TWI_DataLength) //data finish?
            {
                //send 1 byte
                TWI_SendByte(TWI_SendData[TWI_Index]);
                TWI_Index++;
            }
            else
            {
                TWI_Index = 0;
                TWI_Write_Finish = 1;
            }
            break;  //break and wait for TWINT on 1 byte transmission finished
            
        /**********************MR MODE: Master RECEIVE MODE************************/

        //After SLA+R->ACK
        case TWI_MR_SLA_R_ACK: 
            if (TWI_Index < (TWI_DataLength - 1))
            {
                //ACK, more data to recieve
                TWI_RecByteAck(1);
            }
            else
            {
                //NACK, no more data
                TWI_RecByteAck(0);
            }
            break;
        //Savd the TWDR(data) to buffer and send ACK/NACK
        case TWI_MR_DATA_ACK:
            TWI_ReceData[TWI_Index] = TWI_GetReceByte();
            TWI_Index++;
            //add by sz, Send ACK if index==length, else NACK
            if (TWI_Index < (TWI_DataLength - 1))
            {
                //ACK, more data to recieve
                TWI_RecByteAck(1);
            }
            else
            {
                //NACK, no more data
                TWI_RecByteAck(0);
            }
            //end
            break;  //return NACK???
        //MR Mode, NACK is sent. Save last byte data
        case TWI_MR_DATA_NACK:
            TWI_ReceData[TWI_Index] = TWI_GetReceByte();
            //reset index
            TWI_Index = 0;
            //update flag
            TWI_Read_Finish = 1;
            //Send STO
            TWI_Stop();
            break;

        /*MT/MR Mode, Error status*/
        case TWI_MR_SLA_R_NACK:         //NACK to SLA+R in MR mode
        case TWI_MT_DATA_NACK:          //NACK to DATA in MT mode
        case TWI_MT_SLA_W_NACK:         //NACK to SLA+W in MT mode
            //STO->STA
            TWI_Stop();
            break;
        case TWI_BUS_ERROR:             //BUS ERROR
            TWI_Stop();
            break;
        default:
            //enable TWI BUS
            TWCR = (1 << TWEN) |
                (0 << TWIE) | (0 << TWINT) |
                (0 << TWEA) | (0 << TWSTA) | (0 << TWSTO) |
                (0 << TWWC); //
    }
}
/****************INTERRUPT END***********************/


/****************INQUIRY MODE START******************/


/*******************************************************************************
* Function:     TWI_IIC_WriteToDevice()
* Arguments:  DeviceAddr, databuffer, datalength  
* Return:       1: success, 0: fail
* Description: Inquiry Mode, Write data to device 
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
    //SLA+W
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //NACK to SLA+W
        TWI_Stop();
        return 0;
    }
    //Send data
    for (i = 0; i < DataLength; i++)
    {
        TWI_SendByte(*pData);
        TWI_Wait();
        if (TWI_STATUS != TWI_MT_DATA_ACK)
        {
            //receive NACK from SLAVE to MT DATA
            TWI_Stop();
            return 0;
        }
        pData++;
    }
    TWI_Stop();
    return 1;
}
/*******************************************************************************
* Function:     TWI_IIC_WriteToDeviceByAddr()
* Arguments:  DeviceAddr, dataAddr, databuffer, datalength
* Return:       1: success, 0: fail
* Description: Inquiry Mode, Write data to device by Addr
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
    //SLA+W
    TWI_SendByte(DeviceAddr & 0xFE);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //DataAddr
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        TWI_Stop();
        return 0;
    }
    //Send Data
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

/*******************************************************************************
* Function:     TWI_IIC_ReadFromDevice()
* Arguments:  DeviceAddr, databuffer, datalength
* Return:       1: success, 0: fail
* Description: Inquiry mode. Read data to device by Addr
*******************************************************************************/
UINT8 TWI_IIC_ReadFromDevice(UINT8 DeviceAddr, UINT8 *pData, UINT8 DataLength)
{
    UINT8 i = 0;
    TWI_Start();
    TWI_Wait();
    if (TWI_STATUS != TWI_START)
    {
        TWI_Stop();
        return 0;
    }
    //SLA+R, 0x01 means read, 0x00 means write
    TWI_SendByte(DeviceAddr | 0x01);
    TWI_Wait();
    if (TWI_STATUS != TWI_MR_SLA_R_ACK)
    {
        //NACK to SLA+R
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
    //Send NACK after the last byte data
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}

/*******************************************************************************
* Function:     TWI_IIC_ReadFromDeviceByAddr()
* Arguments:  DeviceAddr, Data Addr, databuffer, datalength
* Return:       1: success, 0: fail
* Description: Inquiry mode. Write data to device by Addr
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
    //Device Addr
    TWI_SendByte(DeviceAddr & 0xFE);    //TODO: 0xFE or 0x01????
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_SLA_W_ACK)
    {
        //Got NACK for SLA+R
        TWI_Stop();
        return 0;
    }
    //Send Data
    TWI_SendByte(DataAddr);
    TWI_Wait();
    if (TWI_STATUS != TWI_MT_DATA_ACK)
    {
        //NACK for DATA
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
        //NACK to SLA+R
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
    //In MR mdoe, send NACK for last byte
    if (TWI_ReciveDATA_NACK(pData + i) == 0)
    {
        return 0;
    }
    TWI_Stop();
    return 1;
}
/*******************************************************************************
* Function:     TWI_ReciveDATA_NACK()
* Arguments:  databuffer
* Return:       1: success, 0: fail
* Description: Inquiry mode. Receive data and feed back NACK. For last byte
*******************************************************************************/
UINT8  TWI_ReciveDATA_NACK(UINT8 *pdata)
{
    TWI_ReceNACK(); //Set NACK Mode and INT to start receiving data
    TWI_Wait(); //wait for data receiving finished
    if (TWI_STATUS != TWI_MR_DATA_NACK) //check status
    {
        return 0;
    }
    *pdata = TWI_GetReceByte(); //save data
    return 1;
}

/*******************************************************************************
* Function:     TWI_ReciveDATA_ACK()
* Arguments:  databuffer
* Return:       1: success, 0: fail
* Description: Inquiry mode. Receive data and feed back ACK. Not for the last byte
*******************************************************************************/
UINT8  TWI_ReciveDATA_ACK(UINT8 *pdata)
{
    TWI_ReceACK();  //Set NACK Mode and INT to start receiving data
    TWI_Wait();     //wait for data receiving finished
    if (TWI_STATUS != TWI_MR_DATA_ACK)
    {
        return 0;
    }
    *pdata = TWI_GetReceByte();
    return 1;
}



/**************GPIO Simulation Mode IIC SDA--PC4   SCL--PC5***************************
**************Attention��MUST delay 5ms between IIC read and write     *****************/

/*******************************************************************************
* Function:     IIC_Init()
* Arguments:  
* Return:       
* Description: GPIO Simulation IIC Mode. GPIO initiation for IIC
*******************************************************************************/
void IIC_Init(void)
{
    //Set Port to OUTPUT
    SDA_OUT;
    SCL_OUT;
    //output high
    SDA_HIGH;
    SCL_HIGH;
}


/*****************************************************************************
**********�� �ܣ� ģ��IIC����  ********************************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
*******************************************************************************/
/*******************************************************************************
* Function:     IIC_Start()
* Arguments:
* Return:
* Description: GPIO Simulation IIC Mode. IIC STA
*******************************************************************************/
void IIC_Start(void)
{
    SDA_OUT;
    SCL_OUT;
    //Set SCL Low to hold the bus
    SCL_LOW;
    //Init
    SDA_HIGH;
    SCL_HIGH;
    //Send STA
    SDA_LOW;
    //Set SCL Low to hold the bus
    SCL_LOW;

}
/*****************************************************************************
**********�� �ܣ� ģ��IICֹͣ  ********************************************
**********���������  ��  ********************************************
**********���� ֵ�� ��  ********************************************
*******************************************************************************/
/*******************************************************************************
* Function:     IIC_Start()
* Arguments:
* Return:
* Description: GPIO Simulation IIC Mode. IIC STA
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