/****************************************************************************
* File name: eeprom.c
* Description: includes functions for EEPROM
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"

struct dataInEEPROM_t dataInRom;

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

UINT16 addr_eeprom = 0;
UINT16 addr_read_eeprom = 0;

/*******************************************************************************
* Function:     write_data2eeprom()
* Arguments:  *data2eeprom
* Return:
* Description:  write data to eeprom, MSB in low address
*******************************************************************************/
void write_data2eeprom(dataInEEPROM_t *data2eeprom)
{
	UINT16 tmp;
	tmp = data2eeprom->data;
	EEPROM_write(addr_eeprom++, (UINT8)tmp);
	EEPROM_write(addr_eeprom++, (UINT8)(tmp>>8));
}

/*******************************************************************************
* Function:     read_dataFromeeprom()
* Arguments:  addOffset
* Return:
* Description:  read eeprom data at 0x0000+addOffset , and printf to UART0
*******************************************************************************/
void read_dataFromeeprom(UINT8 addOffset)
{
	UINT8 data;
	UINT16 addr;
	addr = (addr_read_eeprom) + addOffset-1;
	data = EEPROM_read(addr);
	printf("the data at %x is %d \r\n", addr, data);
}

/*******************************************************************************
* Function:     test_EEPROM()
* Arguments:  
* Return:
* Description:  EEPROM write and read operation
*******************************************************************************/
void test_EEPROM(void)
{
    UINT8 i=0;
    UINT8 data;
	for (i = 0; i < 10; i++)
	{
		data = EEPROM_read((UINT16)i);
		printf("the data at %d is  %d \r\n", i, data);
	}
	
	for (i = 0; i < 10; i++)
    {
        EEPROM_write((UINT16)i, i+1);
    }

}
