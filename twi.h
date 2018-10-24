/*
 * TWI.h
 *
 * Created: 2015/10/30 14:48:59
 *  Author: Administrator
 */ 
/*******************************************************************
********************************************************************
**注意事项：在使用硬件查询方式和软件模拟方式进行IIC通信,每次读写数据之间***
**必须延时5ms,否者数据读操作会失败;硬件中断方式读写之间不需要延时时间;*****
********************************************************************
********************************************************************/

#include "global.h"
#ifndef TWI_H_
#define TWI_H_
//#include "config.h"


////硬件中断模式;
#define TWI_IIC_MODE 0 
////硬件查询模式;
//#define TWI_IIC_MODE 1
//软件模拟方式;
//#define TWI_IIC_MODE 2


#define TWI_START 0x08
#define TWI_REP_START 0x10
#define TWI_ARB_LOST 0x38


/*************************************************************
****************主机发送***********************************
**************************************************************/
#define TWI_MT_SLA_W_ACK 0x18
#define TWI_MT_SLA_W_NACK 0x20
#define TWI_MT_DATA_ACK 0x28
#define TWI_MT_DATA_NACK 0x30


/*************************************************************
****************主机接收***********************************
**************************************************************/
#define TWI_MR_SLA_R_ACK             0x40
#define TWI_MR_SLA_R_NACK           0x48
#define TWI_MR_DATA_ACK           0x50
#define TWI_MR_DATA_NACK             0x58


#define TWI_NO_INFO 0xF8
#define TWI_BUS_ERROR 0x00


#define TWI_TWCR_STATE (TWCR&0x0F)
#define TWI_NO_STATUS         0xF8


//数据缓存空间的大小;
#define TWI_BUFFER_SIZE 64


//数据返回;
#define TWI_OK 0x00
#define TWI_ERROR_NODEV                 0x01


//enable interrupt
#define Enable_Interrupt_TWI() TWCR|=(1<<TWIE)
//Disable interrupt
#define Disable_Interrupt_TWI() TWCR&=~(1<<TWIE)
//Send STO, clear TWINT, enable TWI;
#define TWI_Start() (TWCR=TWI_TWCR_STATE|(1<<TWINT)|(1<<TWSTA)|(1<<TWEN))
//send STO
#define TWI_Stop() (TWCR=TWI_TWCR_STATE|(1<<TWINT)|(1<<TWEA)|(1<<TWSTO))
//Wait for TWI interrupt;
#define TWI_Wait() {while (!(TWCR & (1<<TWINT)));}
//启动发送;
#define TWI_Enable_Send() TWCR=(TWI_TWCR_STATE|(1<<TWINT))
#define TWI_Enable_Rev() TWSR=(1<<TWINT)|(TWEA)
//接受应答回复;
#define TWI_ReceACK() TWCR=(TWI_TWCR_STATE|(1<<TWINT)|(1<<TWEA)|(1<<TWEN))
#define TWI_ReceNACK() TWCR=(TWI_TWCR_STATE|(1<<TWINT)|(1<<TWEN))


//总线状态;
#define TWI_STATUS (TWSR&TWI_NO_STATUS)


void TWI_Init(void);
UINT8 TWI_BitRate(UINT16 BitRateKHZ);
UINT8 TWI_GetReceByte(void);
void TWI_SendByte(UINT8 SendByte);
void TWI_RecByteAck(UINT8 AckFlag);
void TWI_WriteToDevice(UINT8 DeviceAddr,UINT8 length,UINT8 *pData);
void Fill_Data(UINT8 length,UINT8 *pData);
void TWI_StartTransmition(void);
void TWI_ReadFromDevice(UINT8 DeviceAddr,UINT8 length);
/******************************中断方式***********************/


/*******************************查询方式***********************/
UINT8 TWI_IIC_WriteToDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
UINT8 TWI_IIC_WriteToDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr ,UINT8 *pData,UINT8 DataLength);
UINT8 TWI_IIC_ReadFromDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
UINT8 TWI_IIC_ReadFromDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr,UINT8 *pData,UINT8 DataLength);
UINT8  TWI_ReciveDATA_NACK(UINT8 *pdata);
UINT8  TWI_ReciveDATA_ACK(UINT8 *pdata);
/********************************************************************************
*************通过I/O口模拟IIC时序************************************************
********************************************************************************/


//SDA--PC4   SCL--PC5


//IIC管脚定义;
#define SDA_Pin PC1
#define SCL_Pin PC0
#define TWI_PORT PORTC
#define TWI_PORT_DDR DDRC
#define TWI_PORT_IN PINC

#define SDA_OUT TWI_PORT_DDR|=(1<<SDA_Pin)
#define SDA_IN TWI_PORT_DDR&=~(1<<SDA_Pin)


#define SDA_LOW TWI_PORT&=~(1<<SDA_Pin)     //;delay_us(IIC_DELAY_TIME)
#define SDA_HIGH TWI_PORT|=(1<<SDA_Pin)     //;delay_us(IIC_DELAY_TIME)
#define SDA_Read TWI_PORT_IN&=(1<<SDA_Pin)


#define SCL_OUT TWI_PORT_DDR|=(1<<SCL_Pin)
#define SCL_IN TWI_PORT_DDR&=~(1<<SCL_Pin)


#define SCL_LOW TWI_PORT&=~(1<<SCL_Pin)     //;delay_us(IIC_DELAY_TIME)
#define SCL_HIGH TWI_PORT|=(1<<SCL_Pin)     //;delay_us(IIC_DELAY_TIME)
#define SCL_Read PINC&=(1<<SCL_Pin)


#define IIC_CLOCK_FREQUENT 100


#if ((IIC_CLOCK_FREQUENT>400)||(IIC_CLOCK_FREQUENT==0))
#error "IIC_CLOCK_FREQUENT overflow!!!"
#elif (IIC_CLOCK_FREQUENT>250)
#define IIC_DELAY_TIME 2
#else
#define IIC_DELAY_TIME (500/IIC_CLOCK_FREQUENT)
#endif


void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(UINT8 Wdata);
UINT8 IIC_ReadAck(void);
void IIC_Send_ACK_Or_NACK(UINT8 ACK_Or_NACK);
UINT8 IIC_WriteToDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
UINT8 IIC_WriteToDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr,UINT8 *pData,UINT8 DataLength);
UINT8 IIC_ReceiveByte(void);
UINT8 IIC_ReadFromDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
UINT8 IIC_ReadFromDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr, UINT8 *pData,UINT8 DataLength);


/******************************模拟方式***********************/
void IIC_TWI_Init(void);
void IIC_TWI_WriteToDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
void IIC_TWI_WriteToDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr ,UINT8 *pData,UINT8 DataLength);
void IIC_TWI_ReadFromDevice(UINT8 DeviceAddr,UINT8 *pData,UINT8 DataLength);
void IIC_TWI_ReadFromDeviceByAddr(UINT8 DeviceAddr,UINT8 DataAddr ,UINT8 *pData,UINT8 DataLength);


#endif /* TWI_H_ */





