/****************************************************************************
* File name: data_collection.c
* Description: data collection
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"

UINT8 ch_show = 'a';

#if 0
/*******************************************************************************
* Function:      timer_handler()
* Arguments:
* Return:
* Description:
*******************************************************************************/
UINT8 timer_handler()
{

	if 10s
	{
		get_data();	//or push get_data command to fifo
	}
		if 60s
		{
			1. remove max value
			2. remove min value
			3. average 4 remoains data
			4. save to eeprom data struct in(time[], data[]) to buffer
			5. start to EEPROM from buffer if buffer is not empty
		}

}
#endif

#if 1
/*******************************************************************************
* Function:      get_data()
* Arguments:   
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
UINT16 get_data(void)
{
	static UINT16 n = 0xA;
	return n++;
}

/*******************************************************************************
* Function:      get_time()
* Arguments:   currentTickCout and timestamp
* Return:		  newTime;
* Description:   caculate Date_t time according to currentTickCount and timestamp.
                       timestamp is updated through GPRS module, activated by server. (by url ???)
					   TODO
*******************************************************************************/
Date_t get_time(UINT32 currentTickCout)
{
	struct Date_t newTime;
	newTime = timestamp.time;
	newTime.min += 3;
	newTime.hour += 2;
	return newTime;
}
/*******************************************************************************
* Function:      timer_ticker()
* Arguments:
* Return:
* Description:
*******************************************************************************/
void timer_ticker(void)
{
	//1:0.2s, 50:10s, 300:1min, 3000: 10min
	static UINT8 tick_divider = 50;
	static UINT8 data_index = 0;
	static UINT16 data[6];
	UINT8 i;
	UINT16 maxdata = 0, mindata = 0, max_index = 0, min_index = 0;
	UINT32 currentTickCount = 0, TickCount = 0;
	UINT32 datasum = 0;
	static UINT32 lastTickCount = 0xFFFF;

	currentTickCount = SystemTickCount;	//buffer SystemTickCount, to avoid updating
	if (currentTickCount % tick_divider == 0)
	{
		if (lastTickCount != currentTickCount)
		{
			lastTickCount = currentTickCount;
			switch (data_index)
			{
			case 5:
				/*get 3rd data*/
				data[data_index] = get_data();
				data_index = 0;
				//1. remove max value
				//2. remove min value
				for (i = 0; i <= 5; i++)
				{
					if (data[i] > maxdata)
						max_index = i;
					else if (data[i] < mindata)
						min_index = i;
				}
				//data[max_index] = 0;
				//data[min_index] = 0;
				for (i = 0; i <= 5; i++)
				{
					if (i != max_index && i != min_index)
						//sum, remove max/min
						datasum += data[i];
				}
				//3. average 4 remoains data
				//4. save to eeprom data struct in (time[], data[]) to buffer
				dataIneeprom.data = (UINT16)(datasum >> 2);		//sum/4, and save it
				dataIneeprom.time = get_time(currentTickCount);			//save time, TODO

				// TODO: save max and min, with time
				//dataInRom_max, dataInRom_min

				//5. start to EEPROM from buffer if buffer is not empty
				(*CommandFifo.AddFifo)(&CommandFifo, 0x50);	//add to fifo, write eeprom commands
				printf("fifo cmd 0x50 is written\r\n");
				break;
			case 4:
				/*get 2nd data*/
				data[data_index] = get_data();
				data_index++;
				break;
			case 3:
				/*get 2nd data*/
				data[data_index] = get_data();
				data_index++;
				break;
			case 2:
				/*get 2nd data*/
				data[data_index] = get_data();
				data_index++;
				break;
			case 1:
				/*get 2nd data*/
				data[data_index] = get_data();
				data_index++;
				break;
			case 0:
				/*get 1st data*/
				data[data_index] = get_data();
				data_index++;
				printf("data[0] is collected\r\n");

				break;
			default:
				NOP();
				//printf("Illegal MeterAndSensorMode!!\r\n"); break;
			}
			//get_data();
			//ch_show

		}
		
	}
}



#endif

