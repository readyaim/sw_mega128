/****************************************************************************
* File name: eeprom.c
* Description: includes functions for EEPROM
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"
#include <eeprom.h>

extern volatile UINT16 UART1_TxHead;
extern volatile UINT16 UART1_TxTail;
extern UINT8 UART1_TxBuf[UART1_TX_BUFFER_SIZE];   //定义发送缓冲区
extern Date_t get_current_time(UINT32 currentTickCout);

volatile UINT16 addr_write_eeprom = START_ADDR_EEPROM;
volatile UINT16 addr_read_eeprom = START_ADDR_EEPROM;


#if 1

#define EEPROM_read EEPROMread
#define EEPROM_write EEPROMwrite

#else
/*******************************************************************************
* Function:      EEPROM_read()
* Arguments:  uiAddress, read address
* Return:        data
* Description:  EEPROM read function, in Polling mode. 
					  TODO: realize inerrupt mode.
*******************************************************************************/
UINT8 EEPROM_read(UINT16 uiAddress)
{
    /* Wait for completion of previous write */
    while (EECR & (1 << EEWE))
        ;
    /* Set up address register */
    EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
    EECR |= (1 << EERE);
	/* Return data from data register */
    return EEDR;
}

/*******************************************************************************
* Function:     EEPROM_write()
* Arguments:  uiAddress, ucData
* Return:
* Description:  EEPROM read function
*******************************************************************************/
void EEPROM_write(UINT16 uiAddress, UINT8 ucData)
{
    /* Wait for completion of previous write */
    while (EECR & (1 << EEWE))
        ;
    /* Set up address and data registers */
    EEAR = uiAddress;
    EEDR = ucData;
    /* Write logical one to EEMWE */
    EECR |= (1 << EEMWE);
    /* Start eeprom write by setting EEWE */
    EECR |= (1 << EEWE);
}

#endif

/*******************************************************************************
* Function:     write_Time2eeprom()
* Arguments:  Date_t *time
					 UINT16 addr
* Return:
* Description: write time to eeprom
*******************************************************************************/
void write_Time2eeprom( UINT16 addr, Date_t *time)
{
	EEPROM_write(addr++, time->year1);
	EEPROM_write(addr++, time->year);
	EEPROM_write(addr++, time->mon);
	EEPROM_write(addr++, time->day);
	EEPROM_write(addr++, time->hour);
	EEPROM_write(addr, time->min);
}

/*******************************************************************************
* Function:     read_TimeIneeprom()
* Arguments:  Date_t *time
					 UINT16 addr
* Return:
* Description: read time from eeprom, save the result to *time
*******************************************************************************/
void read_TimeIneeprom(Date_t *time, UINT16 addr)
{
	time->year1 = EEPROM_read(addr++);
	time->year = EEPROM_read(addr++);
	time->mon = EEPROM_read(addr++);
	time->day = EEPROM_read(addr++);
	time->hour = EEPROM_read(addr++);
	time->min = EEPROM_read(addr);
}

/*******************************************************************************
* Function:     resume_last_timeStampSlot()
* Arguments:  true, if timeStampSlot is resumed; false, if no data is resumed.
* Return:
* Description: read time from eeprom, save the result to *time
*******************************************************************************/
BOOL resume_last_timeStampSlot(void)
{
	UINT16 addr = TIMESTAMP_ADDR_EEPROM;
	UINT16 eeprom_addr;
	UINT8 year1;
	year1 = EEPROM_read(addr++);
	if (year1 == 0xFF)
	{
		return false;
	}
	else
	{
		timeStampShot_g.time.year1 = year1;
		timeStampShot_g.time.year = EEPROM_read(addr++);
		timeStampShot_g.time.mon = EEPROM_read(addr++);
		timeStampShot_g.time.day = EEPROM_read(addr++);
		timeStampShot_g.time.hour = EEPROM_read(addr++);
		timeStampShot_g.time.min = EEPROM_read(addr++);
		eeprom_addr = EEPROM_read(addr++);
		eeprom_addr |= (EEPROM_read(addr)<<8);
		timeStampShot_g.currentAddrEEPROM = eeprom_addr;
		printf("AddrEEPROM is resumed: 0x%x\r\n", eeprom_addr);
		return true;
	}
}

/*******************************************************************************
* Function:     write_dataStruct_to_eeprom()
* Arguments:  Date_t *time
UINT16 addr
* Return:
* Description: write time to eeprom
*******************************************************************************/
void write_dataStruct_to_eeprom(UINT16 addr, dataInEEPROM_t *data2eeprom)
{
	EEPROM_write(addr++, data2eeprom->time.year1);
	EEPROM_write(addr++, data2eeprom->time.year);
	EEPROM_write(addr++, data2eeprom->time.mon);
	EEPROM_write(addr++, data2eeprom->time.day);
	EEPROM_write(addr++, data2eeprom->time.hour);
	EEPROM_write(addr++, data2eeprom->time.min);
	EEPROM_write(addr++, (UINT8)(data2eeprom->data));
	EEPROM_write(addr++, (UINT8)(data2eeprom->data >> 8));
}


#if 1
/*******************************************************************************
* Function:     write_dataSeries_to_eeprom()
* Arguments:  *data2eeprom
* Return:
* Description:  write data to eeprom, MSB in low address
*******************************************************************************/
void write_dataSeries_to_eeprom(void)
{
	//void EEPROMWriteBytes(int location, void *ptr, int size)
	void *ptr;
	int size=sizeof(dataSample_g);
	//int tmp;
	//tmp = sizeof(dataSample_g);
	//printf("sizeof dataSample_g is %d\r\n", tmp);
	ptr = &dataSample_g;
	EEPROMWriteBytes(addr_write_eeprom, ptr, size);
	addr_write_eeprom += size;
	
	size = sizeof(dataSample_max_g);
	ptr = &dataSample_max_g;
	EEPROMWriteBytes(addr_write_eeprom, ptr, size);
	addr_write_eeprom += size;
	
	ptr = &dataSample_min_g;
	size = 32;
	EEPROMWriteBytes(addr_write_eeprom, ptr, size);
	addr_write_eeprom += (size);

	//TODO: solve addr_write_eeprom overflow
	if (addr_write_eeprom > END_ADDR_EEPROM - timeStampShot_g.pageSize + 1)
	{
		addr_write_eeprom = START_ADDR_EEPROM;
	}
}


#else

/*******************************************************************************
* Function:     write_dataSeries_to_eeprom()
* Arguments:  *data2eeprom
* Return:
* Description:  write data to eeprom, MSB in low address
*******************************************************************************/
void write_dataSeries_to_eeprom(void)
{
	/* temperature */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.temp);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.temp);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.temp);
	addr_write_eeprom += 8;

	/* humidity */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.humidity);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.humidity);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.humidity);
	addr_write_eeprom += 8;


	/* airPressure */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.airPressure);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.airPressure);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.airPressure);
	addr_write_eeprom += 8;

	/* groundTemp */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.groundTemp);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.groundTemp);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.groundTemp);
	addr_write_eeprom += 8;

	/* radiation */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.radiation);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.radiation);
	addr_write_eeprom += 8;
	//write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.radiation);
	//addr_write_eeprom += 8;

	/* rain */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.rain);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.rain);
	addr_write_eeprom += 8;
	//write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.rain);
	//addr_write_eeprom += 8;

	/* sunShineTime */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.sunShineTime);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.sunShineTime);
	addr_write_eeprom += 8;
	//write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.sunShineTime);
	//addr_write_eeprom += 8;

	/* evaporation */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.evaporation);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.evaporation);
	addr_write_eeprom += 8;
	//write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_min_g.evaporation);
	//addr_write_eeprom += 8;

	/* windSpeed1m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windSpeed1m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windSpeed1m);
	addr_write_eeprom += 8;

	/* windSpeed2m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windSpeed2m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windSpeed2m);
	addr_write_eeprom += 8;

	/* windSpeed10m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windSpeed10m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windSpeed10m);
	addr_write_eeprom += 8;

	
	/* windDirection1m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windDirection1m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windDirection1m);
	addr_write_eeprom += 8;

	/* windDirection2m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windDirection2m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windDirection2m);
	addr_write_eeprom += 8;

	/* windDirection10m */
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_g.windDirection10m);
	addr_write_eeprom += 8;
	write_dataStruct_to_eeprom(addr_write_eeprom, &dataSample_max_g.windDirection10m);
	addr_write_eeprom += 8;


	//TODO: solve addr_write_eeprom overflow
	if (addr_write_eeprom > END_ADDR_EEPROM - timeStampShot_g.pageSize +1)
	{
		addr_write_eeprom = START_ADDR_EEPROM;
	}
}

#endif

#if 0
/*******************************************************************************
* Function:     read_dataIneeprom()
* Arguments:  uiAddress: address, *data2eeprom: the point of data to be saved
* Return:
* Description:  read from eeprom at uiAddress, and save the data to *data2eeprom
*******************************************************************************/
void read_dataIneeprom(UINT16 uiAddress, dataInEEPROM_t *data2eeprom)
{
	UINT16 tmp;
	UINT8 low8;
	data2eeprom->time.year1 = EEPROM_read(uiAddress++);
	data2eeprom->time.year = EEPROM_read(uiAddress++);
	data2eeprom->time.mon = EEPROM_read(uiAddress++);
	data2eeprom->time.day = EEPROM_read(uiAddress++);
	data2eeprom->time.hour = EEPROM_read(uiAddress++);
	data2eeprom->time.min = EEPROM_read(uiAddress++);
	low8 = EEPROM_read(uiAddress++);
	tmp = EEPROM_read(uiAddress++);
	data2eeprom->data = ((tmp << 8) | low8);
	printf("data read from eepprom is %x", data2eeprom->data);
	
}
#endif


/*******************************************************************************
* Function:     read_eepromCtrledByUART1()
* Arguments:  addOffset
* Return:
* Description:  read eeprom data at 0x0000+addOffset , and printf to UART0
*******************************************************************************/
void read_eepromCtrledByUART1(UINT8 addOffset)
{
	UINT8 data;
	//UINT16 addr;
	if (addOffset == 0x0F)
	{
		//reset addess
		addr_read_eeprom = START_ADDR_EEPROM;
		//printf("get %x, reset addr_read_eeprom to %d", addOffset, addr_read_eeprom);
	}
	else if (addOffset == 0x0E)
	{
		addr_read_eeprom = TIMESTAMP_ADDR_EEPROM;
	}
	else
	{
		addr_read_eeprom += addOffset;
	}
	//addr = (addr_read_eeprom) + addOffset-1;
	data = EEPROM_read(addr_read_eeprom);
	printf("data at 0x%x is %d(0x%x) \r\n", addr_read_eeprom, data, data);
}
#if 1
/*******************************************************************************
* Function:     read_eeprom_to_UART1buffer()
* Arguments:  addr, timeStampShot.pagesize
* Return:
* Description:  read eeprom from address, size= timeStampShot.pagesize
*******************************************************************************/
void read_eeprom_to_UART1buffer(UINT16 addr)
{
	//disable UART1 Rx Interrupt
	UINT8 data;
	UINT16 i=0, j=0;
	UINT16 tmphead;
	//TODO
	//1. make sure buffer is empty
	for (i = 0; i < timeStampShot_g.pageSize; i++)
	{
		//data = 0;
		data = EEPROM_read(addr++);
		//EEPROM_READ(addr++, data);
		//TODO: transfer dec to str
		/* calculate buffer index */
		tmphead = (UART1_TxHead + 1) & UART1_TX_BUFFER_MASK;
		
		/*
		if (UART1_TxHead >= UART1_TX_BUFFER_SIZE - 1)
		{
			tmphead = 0;
		}
		else
		{
			tmphead = UART1_TxHead + 1;
		}
		*/
		/* wait for free space in buffer */
		while (tmphead == UART1_TxTail)
			j++;
		
		UART1_TxBuf[tmphead] = data;	/* store data in buffer */
		UART1_TxHead = tmphead;	/* store new index */
		Set_Bit(UCSR1B, UDRIE1);          //enable UART1 Tx interrupt
	}
	printf("enable UART1 Rx INT, %d, %d\r\n", UART1_TxHead, UART1_TxTail);
	printf("j=%d\r\n", j++);
	//Set_Bit(UCSR1B, UDRIE1);          //enable UART1 Tx interrupt
}
#else
/*******************************************************************************
* Function:     read_eeprom_to_UART1buffer()
* Arguments:  addr, timeStampShot.pagesize
* Return:
* Description:  read eeprom from address, size= timeStampShot.pagesize
*******************************************************************************/
void read_eeprom_to_UART1buffer(UINT16 addr)
{
	//disable UART1 Rx Interrupt
	UINT16 data, i;
	//UINT16 tmpWR = 0;
	//TODO
	//1. make sure buffer is empty
	//TXC1_WR=0;
	//TXC1_RD = 0;
	//printf("before Tx, TXC1_WR= %d, TXC1_RD=%d\r\n", TXC1_WR, TXC1_RD);
	for (i = 0; i < timeStampShot_g.pageSize; i++)
	{
		data = EEPROM_read(addr++);
		//TODO: transfer dec to str
		UART1_TxBuf[TXC1_WR] = data;
		if (TXC1_WR < (UART1_TX_BUFFER_SIZE - 1))   //UART1_TX_BUFFER_SIZE  发送区数据大小
			TXC1_WR++;		// = 1;
		else
			TXC1_WR = 0;
	}
	UART1_TxBuf[TXC1_WR++] = 0x0D;
	UART1_TxBuf[TXC1_WR++] = 0x0A;


	//printf("After UART1 Tx, TXC1_WR= %d, TXC1_RD=%d\r\n", TXC1_WR, TXC1_RD);
	//UCSR1B |= (1 << UDRIE1);          //开启UDRE中断
	Set_Bit(UCSR1B, UDRIE1);          //开启UDRE中断

}
#endif

/*******************************************************************************
* Function:     write_tickCountTime_to_eeprom()
* Arguments:  Date_t *time
					 UINT16 addr
* Return:
* Description: write time to eeprom
*******************************************************************************/
void write_tickCountTime_to_eeprom(void)
{
	UINT16 addr = TIMESTAMP_ADDR_EEPROM;

	
	//write Time
	//time = get_current_time(timeStampShot_g.tickeCounter);
	EEPROM_write(addr++, timeStampShot_g.time.year1);
	EEPROM_write(addr++, timeStampShot_g.time.year);
	EEPROM_write(addr++, timeStampShot_g.time.mon);
	EEPROM_write(addr++, timeStampShot_g.time.day);
	EEPROM_write(addr++, timeStampShot_g.time.hour);
	EEPROM_write(addr++, timeStampShot_g.time.min);
	//write address
	//EEPROM_write(addr++, (UINT8)addr_write_eeprom);
	//EEPROM_write(addr++, (UINT8)(addr_write_eeprom>>8));
	EEPROM_write(addr++, (UINT8)timeStampShot_g.currentAddrEEPROM);
	EEPROM_write(addr++, (UINT8)(timeStampShot_g.currentAddrEEPROM>>8));
	//backup tickcounter
	/*
	EEPROM_write(addr++, (UINT8)(timeStampShot_g.tickeCounter));
	EEPROM_write(addr++, (UINT8)(timeStampShot_g.tickeCounter >> 8));
	EEPROM_write(addr++, (UINT8)(timeStampShot_g.tickeCounter >> 16));
	EEPROM_write(addr++, (UINT8)(timeStampShot_g.tickeCounter >> 24));
	*/
}


/*******************************************************************************
* Function:     test_EEPROM()
* Arguments:  
* Return:
* Description:  EEPROM write and read operation
*******************************************************************************/
void test_EEPROM(void)
{
    UINT16 i=0;
    UINT16 data;
	void *p;
	p = &data;
	for (i = 0; i < 10; i++)
	{
		EEPROM_READ(i, data);
		printf("data at %d is  %d \r\n", i, data);
	}
	
	for (i = 0; i < 10;)
    {
        EEPROM_WRITE(i, i);
		i += 2;
    }

}
