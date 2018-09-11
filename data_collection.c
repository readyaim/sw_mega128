/****************************************************************************
* File name: data_collection.c
* Description: data collection
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"


//#define _DATA_COLLECT_DEBUG
//enum TransIntervalMode_t transInterval = 5;
UINT8 transInterval = 3;
extern UINT16 get_data_adc(UINT8 channel);

/*******************************************************************************
* Function:      get_data()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
UINT16 get_data(UINT8 channel)
{
#ifdef _DATA_COLLECT_DEBUG
	// for debug only
	static UINT16 n = 0xA;
	return n++;
#else
	//for ADC0 sampling
	UINT16 data;
	data = get_data_adc(channel);
	return data;
#endif // _DATA_COLLECT_DEBUG
}



/*******************************************************************************
* Function:      get_current_time()
* Arguments:   currentTickCout and timeStampShot
* Return:		  newTime;
* Description:   caculate Date_t time according to currentTickCount and timeStampShot.
					   timeStampShot is updated through GPRS module, activated by server. (by url ???)
					   TODO
*******************************************************************************/
Date_t get_current_time(UINT32 currentTickCout)
{
	/*
#ifdef _DATA_COLLECT_DEBUG
	struct Date_t newTime;
	newTime = timeStampShot.time;
	newTime.min += 3;
	newTime.hour += 2;
	return newTime;

#else
*/
	Date_t  newTime;
	UINT32 tickCounterDiff = 0;
	UINT32 tmp_day, tmp_hour, tmp_min;
	if (currentTickCout >= timeStampShot.tickeCounter)
	{
		tickCounterDiff = currentTickCout - timeStampShot.tickeCounter;
	}
	else
	{
		//TODO, exception when tickcouter overflow.
		tickCounterDiff = 4294967295 - timeStampShot.tickeCounter + currentTickCout;
		//NOP();
	}
	newTime = timeStampShot.time;	//copy time to new time
	//TODO caculate difference.

	//1s: 5tick
	//1min: 5*60 tick, 300
	//1hour: 5*60*60 tick, 18000
	//1day: 5*60*60*12, 216000
	//1mon: 30*1day, 6,480,000
	//1year: 77,760,000

	if (tickCounterDiff < HOURTICKERTIME)
	{
		// tickerDiff is small, save the caculation
		tmp_min = (tickCounterDiff) / MINTICKERTIME;
		newTime.min += tmp_min;
	}
	else
	{	//full calculation
		tmp_day = tickCounterDiff / DAYTICKERTIME;
		newTime.day += tmp_day;
		tmp_hour = (tickCounterDiff % DAYTICKERTIME) / HOURTICKERTIME;
		newTime.hour += tmp_hour;
		tmp_min = (tickCounterDiff%HOURTICKERTIME) / MINTICKERTIME;
		newTime.min += tmp_min;
	}

	if (newTime.min > 59)
	{
		newTime.min -= 60;
		newTime.hour += 1;
	}
	else if (newTime.hour > 23)
	{
		newTime.hour -= 24;
		newTime.day += 1;
	}
	else if (newTime.day > 30)
	{
		//TODO: day 30 or 31 a month???
		newTime.day -= 30;
		newTime.mon += 1;
	}
	else if (newTime.mon > 12)
	{
		newTime.mon -= 12;
		newTime.year += 1;
	}
	else if (newTime.year > 99)
	{
		newTime.year -= 99;
		newTime.year1 += 1;
	}

	//Update timeStampShot
	timeStampShot.time = newTime;
	timeStampShot.tickeCounter = currentTickCout;
	return newTime;
	//#endif
}

/*******************************************************************************
* Function:      get_addr()
* Arguments:
* Return:
* Description:  calc address offset with Time
*******************************************************************************/
UINT16 get_addr(Date_t *time, TimeStamp_t *timestamp)
{
	NOP();
}
/*******************************************************************************
* Function:      get_series_data()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
void get_series_data(DataSeries_t *dataseries)
{
#ifdef _DATA_COLLECT_DEBUG
	// for debug only
	static UINT16 n = 0xA;
	static UINT16 m = 1301;
	dataseries->temp = n++;		//ADC0, temperature
	dataseries->humidity = m++;		//ADC1, humidity
	//return n++;
#else
	//for ADC0 sampling
	UINT16 data;
	dataseries->temp = get_data_adc(0x00);		//ADC0, temperature
	dataseries->humidity = get_data_adc(0x01);		//ADC1, humidity
	//TODO: others data sampling

#endif // _DATA_COLLECT_DEBUG
}

/*******************************************************************************
* Function:      process_series_data()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
void process_series_data(DataSeries_t *dataseries, UINT32 currentTickCount)
{
	UINT8 i, max_index, min_index;
	UINT16 maxdata = 0, mindata = 0xFFFF;
	UINT32 datasum = 0, tickCountDiff;
	/* temperature */
	//1. remove max value
	//2. remove min value
	for (i = 0; i <= 5; i++)
	{
		if (dataseries[i].temp > maxdata)
		{
			max_index = i;
			maxdata = dataseries[i].temp;
		}
		//TODO: should be else if
		if (dataseries[i].temp < mindata)
		{
			min_index = i;
			mindata = dataseries[i].temp;
		}
	}
	dataseries[min_index].temp = 0;
	dataseries[max_index].temp = 0;
	for (i = 0; i <= 5; i++)
	{
		datasum += dataseries[i].temp;
	}
	//3. average 4 remoains data
	//4. save to eeprom data struct in (time, data) to buffer
	dataSample_g.temp.data = (UINT16)(datasum >> 2);	//divide 4
	printf("temperature sum is %d \r\n", datasum);
	dataSample_g.temp.time = get_current_time(currentTickCount);
	// 5. save max and min, with time
	if (dataSample_g.temp.data > dataSample_max_g.temp.data)
	{
		dataSample_max_g.temp = dataSample_g.temp;
	}
	//TODO: should be else if
	if (dataSample_g.temp.data < dataSample_min_g.temp.data)
	{
		dataSample_min_g.temp = dataSample_g.temp;
	}
	/* init values*/
	maxdata = 0;
	mindata = 0xFFFF;
	datasum = 0;

	/* humidity */
	for (i = 0; i <= 5; i++)
	{
		if (dataseries[i].humidity > maxdata)
		{
			max_index = i;
			maxdata = dataseries[i].humidity;
		}
		//TODO: should be else if
		if (dataseries[i].humidity < mindata)
		{
			min_index = i;
			mindata = dataseries[i].humidity;
		}
	}
	//printf("humidity: max_index is %d, min_index is %d\r\n", max_index, min_index);
	dataseries[min_index].humidity = 0;
	dataseries[max_index].humidity = 0;
	for (i = 0; i <= 5; i++)
	{
		datasum += dataseries[i].humidity;
	}
	dataSample_g.humidity.data = (UINT16)(datasum >> 2);	//divide 4
	printf("humidity sum is %d \r\n", datasum);
	dataSample_g.humidity.time = get_current_time(currentTickCount);

	if (dataSample_g.humidity.data > dataSample_max_g.humidity.data)
	{
		dataSample_max_g.humidity = dataSample_g.humidity;
	}
	//TODO: should be else if
	if (dataSample_g.humidity.data < dataSample_min_g.humidity.data)
	{
		dataSample_min_g.humidity = dataSample_g.humidity;
	}
	

}
/*******************************************************************************
* Function:      ticker_timer1_handler()
* Arguments:
* Return:
* Description:  data collection handler
*******************************************************************************/
void ticker_timer1_handler(void)
{
	//1:0.2s, 50:10s, 300:1min, 3000: 10min
	static UINT8 tick_divider = 50;
	static UINT8 data_index = 0;
	static UINT16 data[6];
	static struct DataSeries_t dataseries[6];
	UINT8 i;
	UINT16 maxdata = 0, mindata = 0, max_index = 0, min_index = 0;
	UINT32 currentTickCount = 0,tickCountDiff = 0;
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
				get_series_data(&dataseries[data_index]); 
				data_index = 0;
				process_series_data(dataseries, currentTickCount );
				/*
				//Caculate howmany tick does it need
				tickCountDiff = SystemTickCount - currentTickCount;
				printf("TickCountDiff for case5 is %x\r\n", tickCountDiff);
				*/
				break;
			case 4:
				/*get 2nd data*/
				get_series_data(&dataseries[data_index]);
				//data[data_index] = get_data(0x0);
				data_index++;
				break;
			case 3:
				/*get 2nd data*/
				get_series_data(&dataseries[data_index]); 
				//data[data_index] = get_data(0x0);
				data_index++;
				break;
			case 2:
				/*get 2nd data*/
				//data[data_index] = get_data(0x0);
				get_series_data(&dataseries[data_index]);
				data_index++;
				break;
			case 1:
				/*get 2nd data*/
				get_series_data(&dataseries[data_index]);
				//data[data_index] = get_data(0x0);
				data_index++;
				break;
			case 0:
				/*get 1st data*/
				//printf("data[0].temp is collected as %d\r\n", dataseries[data_index].temp);
				printf(".....1st data......\r\n");
				get_series_data(&dataseries[data_index]);
				data_index++;
				break;
			default:
				NOP();
				//printf("Illegal MeterAndSensorMode!!\r\n"); break;
			}
			//get_data();
			//ch_show
			if (currentTickCount % (transInterval * 300) == 0)
			{
				// TODO: Possible to miss 1 tick. need to modify to be robust
				(*CommandFifo.AddFifo)(&CommandFifo, 0x50);	//add to fifo, write eeprom commands, extreme value(with date)
				//printf("fifo cmd 0x50: write eeprom commands\r\n");

			}
		}
	}
}




