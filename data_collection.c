/****************************************************************************
* File name: data_collection.c
* Description: data collection
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"


#define _DATA_COLLECT_DEBUG
//#define _PRINT_ADC
//enum TransIntervalMode_t transInterval_g = 5;
#define index_3s 0
#define index_1m 1
#define index_2m 2
#define index_10m 3

#define index_windDirection 0
#define index_windSpeed 1

#define _ACCUMULATION_ENABLE
UINT8 transInterval_g = 3;
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
* Arguments:   currentTickCout and timeStampShot_g
* Return:		  newTime;
* Description:   caculate Date_t time according to currentTickCount and timeStampShot_g.
					   timeStampShot_g is updated through GPRS module, activated by server. (by url ???)
					   TODO
*******************************************************************************/
Date_t get_current_time(UINT32 currentTickCout)
{
	/*
#ifdef _DATA_COLLECT_DEBUG
	struct Date_t newTime;
	newTime = timeStampShot_g.time;
	newTime.min += 3;
	newTime.hour += 2;
	return newTime;

#else
*/
	Date_t  newTime;
	UINT32 tickCounterDiff = 0;
	UINT32 tmp_day, tmp_hour, tmp_min;
	static TimeStamp_t timeStamp_s;
	static flag = 0;
	if (timeStampShot_g.flag == 1)
	{
		timeStampShot_g.flag = 0;
		timeStamp_s = timeStampShot_g;
	}

	if (currentTickCout == timeStamp_s.tickeCounter)
	{
		return timeStamp_s.time;
	}
	else if (currentTickCout > timeStamp_s.tickeCounter)
	{
		tickCounterDiff = currentTickCout - timeStamp_s.tickeCounter;
	}
	else
	{
		//TODO, exception when tickcouter overflow.
		tickCounterDiff = 4294967295 - timeStamp_s.tickeCounter + currentTickCout;
		//NOP();
	}
	newTime = timeStamp_s.time;	//copy time to new time
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

	//Update timeStamp_s
	timeStamp_s.time = newTime;
	timeStamp_s.tickeCounter = currentTickCout;
	return newTime;
	//#endif
}

/*******************************************************************************
* Function:      get_address()
* Arguments:   (timeStampShot_g)
* Return:		  the start address of data for target date(Date_t)
* Description:  calc address offset with Time
*******************************************************************************/
UINT16 get_address(Date_t *targetTime)
{
	UINT16 addr_estimate;
	INT32 interval;
	//TODO: make sure targetTime is older than timeStampShot

	interval = (timeStampShot_g.time.day - targetTime->day) * 24 * 60 / transInterval_g +
		(timeStampShot_g.time.hour - targetTime->hour) * 60 / transInterval_g +
		(timeStampShot_g.time.min - targetTime->min) / transInterval_g;
	addr_estimate = timeStampShot_g.currentAddrEEPROM;
	addr_estimate -= interval * timeStampShot_g.pageSize;

	printf("interval is %d\r\n", interval);
	printf("addr_estimate is %d\r\n", addr_estimate);

	return addr_estimate;

}
/*******************************************************************************
* Function:      get_series_data_sec()
* Arguments:
* Return:		  data;
* Description:   collect windDirection, windSpeed
					   TODO
*******************************************************************************/
void get_series_data_sec(UINT16 (*data_wind_series)[2])
{
	//const UINT8 index_windDirection = 0, index_windSpeed = 1;
	//const UINT8 index_3s = 0, index_1m = 1, index_2m = 2, index_10m = 3;
	static UINT16 data_wind_3sec_array[3][2];
	static UINT8 index_sample = 0;
	static UINT8 flag = 0;		/* to initiate all moving average values */

#ifdef _DATA_COLLECT_DEBUG
	// for debug only
	static UINT16 n = 7000;
	static UINT16 m = 8000;

	data_wind_3sec_array[index_sample][index_windDirection] = n;		//winDirection collection
	data_wind_3sec_array[index_sample][index_windSpeed] = m;		// windSpeed collection
	n += 50;
	m += 90;
#else
	data_wind_3sec_array[index_sample][index_windDirection] = n;		//winDirection collection
	data_wind_3sec_array[index_sample][index_windSpeed] = m;		// windSpeed collection
	NOP();
#endif
	if (flag == 0)
	{
		data_wind_3sec_array[1][index_windDirection]= data_wind_3sec_array[index_sample][index_windDirection];
		data_wind_3sec_array[2][index_windDirection] = data_wind_3sec_array[index_sample][index_windDirection];
		data_wind_series[index_3s][index_windDirection] = data_wind_3sec_array[index_sample][index_windDirection];
		data_wind_series[index_1m][index_windDirection] = data_wind_3sec_array[index_sample][index_windDirection];
		data_wind_series[index_2m][index_windDirection] = data_wind_3sec_array[index_sample][index_windDirection];
		data_wind_series[index_10m][index_windDirection] = data_wind_3sec_array[index_sample][index_windDirection];

		data_wind_3sec_array[1][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		data_wind_3sec_array[2][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		data_wind_series[index_3s][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		data_wind_series[index_1m][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		data_wind_series[index_2m][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		data_wind_series[index_10m][index_windSpeed] = data_wind_3sec_array[index_sample][index_windSpeed];
		flag = 1;
	}

	data_wind_series[index_3s][index_windDirection] = (data_wind_3sec_array[0][index_windDirection] +
																				data_wind_3sec_array[1][index_windDirection]+
																				data_wind_3sec_array[2][index_windDirection]) / 3;
	data_wind_series[index_3s][index_windSpeed] = (data_wind_3sec_array[0][index_windSpeed] +
																					data_wind_3sec_array[1][index_windSpeed] +
																					data_wind_3sec_array[2][index_windSpeed]) / 3;
	if (index_sample == 2)
	{
		index_sample = 0;
	}
	else
	{
		index_sample += 1;
	}
	//index_sample = (index_sample + 1) & 3;	//0,1,2,3

	/* 60s moving average method*/
	data_wind_series[index_1m][index_windDirection] = data_wind_series[index_1m][index_windDirection] +
		(data_wind_series[index_3s][index_windDirection] - data_wind_series[index_1m][index_windDirection]) / 60;
	data_wind_series[index_1m][index_windSpeed] = data_wind_series[index_1m][index_windSpeed] +
		(data_wind_series[index_3s][index_windSpeed] - data_wind_series[index_1m][index_windSpeed]) / 60;

	/* 120s moving average method*/
	data_wind_series[index_2m][index_windDirection] = data_wind_series[index_2m][index_windDirection] +
		(data_wind_series[index_3s][index_windDirection] - data_wind_series[index_2m][index_windDirection]) / 120;
	data_wind_series[index_2m][index_windSpeed] = data_wind_series[index_2m][index_windSpeed] +
		(data_wind_series[index_3s][index_windSpeed] - data_wind_series[index_2m][index_windSpeed]) / 120;
}
/*******************************************************************************
* Function:      get_series_data_10sec()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
void get_series_data_10sec(DataSeries_t *dataseries)
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
	static UINT16 o = 5000;
	static UINT16 p = 6000;
	static UINT16 q = 7000;
	dataseries->temp = get_data_adc(0x00);		//ADC0, temperature
	dataseries->humidity = get_data_adc(0x01);		//ADC1, humidity
	dataseries->airPressure = o++;		//ADC0, temperature
	dataseries->groundTemp = p++;		//ADC1, humidity
	dataseries->radiation = q++;		//ADC1, humidity

	o += 30;
	p += 40;
	q += 50;

#ifdef _PRINT_ADC
	printf("temp is %d mv\r\n", dataseries->temp);
	printf("humidity is %d mv\r\n", dataseries->humidity);
#endif
	//TODO: others data sampling

#endif // _DATA_COLLECT_DEBUG
}

/*******************************************************************************
* Function:      get_series_data_1min()
* Arguments:
* Return:		  data;
* Description:   collect rain, evaporation, sunShine, data through ADC or I2C
					   TODO
*******************************************************************************/
void get_series_data_1min(void)
{
#if 1
	// for debug only
	static UINT16 n = 200;
	static UINT16 m = 50;
	static UINT16 o = 3000;

	//sample data
	dataSample_g.rain.data = n;
	dataSample_g.evaporation.data = m;
	dataSample_g.sunShineTime.data = o;
	n += 8;
	m += 2;
	o += 20;



#ifdef _PRINT_ADC
	printf("rain is %d mv\r\n", n);
	printf("evaporation is %d mv\r\n", m);
	printf("sunShineTime is %d mv\r\n", o);
#endif
#else
	//for ADC0 sampling
	UINT16 data;
	dataSample_g.rain.data = get_value_TODO();
	dataSample_g.evaporation.data = get_value_TODO();
	dataSample_g.sunShineTime.data = get_value_TODO();

#endif // _DATA_COLLECT_DEBUG
}

/*******************************************************************************
* Function:      process_series_data_wind_1m()
* Arguments:
* Return:		  data;
* Description:   process windDirection, windSpeed;
*******************************************************************************/
void process_series_data_wind_1m(UINT32 currentTickCount, UINT16 (*data_wind_series)[2])
{
	/* TODO: change these index to #define*/
	//const UINT8 index_windDirection = 0, index_windSpeed = 1;
	//const UINT8 index_3s = 0, index_1m = 1, index_2m = 2, index_10m = 3;
	/* TODO: change these index to #define, end*/
	
	/* windDirection */
	dataSample_g.windDirection1m.data = data_wind_series[index_1m][index_windDirection];
	dataSample_g.windDirection1m.time = get_current_time(currentTickCount);
	if (dataSample_g.windDirection1m.data > dataSample_max_g.windDirection1m.data)
	{
		dataSample_max_g.windDirection1m = dataSample_g.windDirection1m;
	}

	/* windSpeed */
	dataSample_g.windSpeed1m.data = data_wind_series[index_1m][index_windSpeed];
	dataSample_g.windSpeed1m.time = get_current_time(currentTickCount);
	if (dataSample_g.windSpeed1m.data > dataSample_max_g.windSpeed1m.data)
	{
		dataSample_max_g.windSpeed1m = dataSample_g.windSpeed1m;
	}
}

/*******************************************************************************
* Function:      process_series_data_wind_2m()
* Arguments:
* Return:		  data;
* Description:   process windDirection, windSpeed;
*******************************************************************************/
void process_series_data_wind_2m(UINT32 currentTickCount, UINT16 (*data_wind_series)[2])
{
	/* TODO: change these index to #define*/
	//const UINT8 index_windDirection = 0, index_windSpeed = 1;
	//const UINT8 index_3s = 0, index_1m = 1, index_2m = 2, index_10m = 3;
	/* TODO: change these index to #define, end*/

	/* windDirection */
	dataSample_g.windDirection2m.data = data_wind_series[index_2m][index_windDirection];
	dataSample_g.windDirection2m.time = get_current_time(currentTickCount);
	if (dataSample_g.windDirection2m.data > dataSample_max_g.windDirection2m.data)
	{
		dataSample_max_g.windDirection2m = dataSample_g.windDirection2m;
	}

	/* windSpeed */
	dataSample_g.windSpeed2m.data = data_wind_series[index_2m][index_windSpeed];
	dataSample_g.windSpeed2m.time = get_current_time(currentTickCount);
	if (dataSample_g.windSpeed2m.data > dataSample_max_g.windSpeed2m.data)
	{
		dataSample_max_g.windSpeed2m = dataSample_g.windSpeed2m;
	}
}

/*******************************************************************************
* Function:      process_series_data_wind_10m()
* Arguments:
* Return:		  data;
* Description:   process windDirection, windSpeed;
*******************************************************************************/
void process_series_data_wind_10m(UINT32 currentTickCount, UINT16 (*data_wind_series)[2])
{
	/* TODO: change these index to #define*/
	//const UINT8 index_windDirection = 0, index_windSpeed = 1;
	//const UINT8 index_3s = 0, index_1m = 1, index_2m = 2, index_10m = 3;
	/* TODO: change these index to #define, end*/

	/* windDirection */
	dataSample_g.windDirection10m.data = data_wind_series[index_10m][index_windDirection];
	dataSample_g.windDirection10m.time = get_current_time(currentTickCount);
	if (dataSample_g.windDirection10m.data > dataSample_max_g.windDirection10m.data)
	{
		dataSample_max_g.windDirection10m = dataSample_g.windDirection10m;
	}

	/* windSpeed */
	dataSample_g.windSpeed10m.data = data_wind_series[index_10m][index_windSpeed];
	dataSample_g.windSpeed10m.time = get_current_time(currentTickCount);
	if (dataSample_g.windSpeed10m.data > dataSample_max_g.windSpeed10m.data)
	{
		dataSample_max_g.windSpeed10m = dataSample_g.windSpeed10m;
	}
}

/*******************************************************************************
* Function:      process_series_data_1min()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
void process_series_data_1min(UINT32 currentTickCount)
{
	//UINT8 max_index, min_index;
	//UINT16 maxdata = 0, mindata = 0xFFFF;
	//UINT32 datasum = 0, tickCountDiff;
	/* evaporation */
	dataSample_g.evaporation.time = get_current_time(currentTickCount);
	// save max and min, with time
	if (dataSample_g.evaporation.data > dataSample_max_g.evaporation.data)
	{
		dataSample_max_g.evaporation = dataSample_g.evaporation;
	}
#ifdef _ACCUMULATION_ENABLE
	dataSample_min_g.evaporation.data += dataSample_g.evaporation.data;
	dataSample_g.evaporation.data = dataSample_min_g.evaporation.data;
#else
	//TODO: should be else if
	if (dataSample_g.evaporation.data < dataSample_min_g.evaporation.data)
	{
		dataSample_min_g.evaporation = dataSample_g.evaporation;
	}
#endif
	/* sunShineTime */
	dataSample_g.sunShineTime.time = get_current_time(currentTickCount);
	// save max and min, with time
	if (dataSample_g.sunShineTime.data > dataSample_max_g.sunShineTime.data)
	{
		dataSample_max_g.sunShineTime = dataSample_g.sunShineTime;
	}
#ifdef _ACCUMULATION_ENABLE
	dataSample_min_g.sunShineTime.data += dataSample_g.sunShineTime.data;
	// can be move run every transInterval*300
	dataSample_g.sunShineTime.data = dataSample_min_g.sunShineTime.data;
#else
	//TODO: should be else if
	if (dataSample_g.sunShineTime.data < dataSample_min_g.sunShineTime.data)
	{
		dataSample_min_g.sunShineTime = dataSample_g.sunShineTime;
	}
#endif


	/* rain */
	dataSample_g.rain.time = get_current_time(currentTickCount);
	// save max and min, with time
	if (dataSample_g.rain.data > dataSample_max_g.rain.data)
	{
		dataSample_max_g.rain = dataSample_g.rain;
	}
	//TODO: should be else if
#ifdef _ACCUMULATION_ENABLE
	// not min value, it's accumulated value
	//calculate accumulated data, which is saved in min_g. (not minimum any more)
	dataSample_min_g.rain.data += dataSample_g.rain.data;
	//data is not useful any more, replaced by accumulated value.
	dataSample_g.rain.data = dataSample_min_g.rain.data;
	//dataSample_min_g.rain.data needs to be clear to 0 when the data is saved(every 5min)
	//init dataSample_min_g.rain.data to zero
#else

	if (dataSample_g.rain.data < dataSample_min_g.rain.data)
	{
		dataSample_min_g.rain = dataSample_g.rain;
	}
#endif

}
/*******************************************************************************
* Function:      process_series_data_10sec()
* Arguments:
* Return:		  data;
* Description:   collect temp, humidity....data through ADC or I2C
					   TODO
*******************************************************************************/
void process_series_data_10sec(DataSeries_t *dataseries, UINT32 currentTickCount)
{
	UINT8 i, max_index, min_index;
	UINT16 maxdata = 0, mindata = 0xFFFF;
	UINT32 datasum = 0;
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
#ifdef _PRINT_ADC
	printf("temperature sum is %d \r\n", datasum);
#endif
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
#ifdef _PRINT_ADC
	printf("humidity sum is %d \r\n", datasum);
#endif
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


	/* init values*/
	maxdata = 0;
	mindata = 0xFFFF;
	datasum = 0;

	/* airPressure */
	for (i = 0; i <= 5; i++)
	{
		if (dataseries[i].airPressure > maxdata)
		{
			max_index = i;
			maxdata = dataseries[i].airPressure;
		}
		//TODO: should be else if
		if (dataseries[i].airPressure < mindata)
		{
			min_index = i;
			mindata = dataseries[i].airPressure;
		}
	}
	//printf("airPressure: max_index is %d, min_index is %d\r\n", max_index, min_index);
	dataseries[min_index].airPressure = 0;
	dataseries[max_index].airPressure = 0;
	for (i = 0; i <= 5; i++)
	{
		datasum += dataseries[i].airPressure;
	}
	dataSample_g.airPressure.data = (UINT16)(datasum >> 2);	//divide 4
#ifdef _PRINT_ADC
	printf("airPressure sum is %d \r\n", datasum);
#endif
	dataSample_g.airPressure.time = get_current_time(currentTickCount);


	if (dataSample_g.airPressure.data > dataSample_max_g.airPressure.data)
	{
		dataSample_max_g.airPressure = dataSample_g.airPressure;
	}
	//TODO: should be else if
	if (dataSample_g.airPressure.data < dataSample_min_g.airPressure.data)
	{
		dataSample_min_g.airPressure = dataSample_g.airPressure;
	}

	/* init values*/
	maxdata = 0;
	mindata = 0xFFFF;
	datasum = 0;

	/* groundTemp */
	for (i = 0; i <= 5; i++)
	{
		if (dataseries[i].groundTemp > maxdata)
		{
			max_index = i;
			maxdata = dataseries[i].groundTemp;
		}
		//TODO: should be else if
		if (dataseries[i].groundTemp < mindata)
		{
			min_index = i;
			mindata = dataseries[i].groundTemp;
		}
	}
	//printf("groundTemp: max_index is %d, min_index is %d\r\n", max_index, min_index);
	dataseries[min_index].groundTemp = 0;
	dataseries[max_index].groundTemp = 0;
	for (i = 0; i <= 5; i++)
	{
		datasum += dataseries[i].groundTemp;
	}
	dataSample_g.groundTemp.data = (UINT16)(datasum >> 2);	//divide 4
#ifdef _PRINT_ADC
	printf("groundTemp sum is %d \r\n", datasum);
#endif
	dataSample_g.groundTemp.time = get_current_time(currentTickCount);


	if (dataSample_g.groundTemp.data > dataSample_max_g.groundTemp.data)
	{
		dataSample_max_g.groundTemp = dataSample_g.groundTemp;
	}
	//TODO: should be else if
	if (dataSample_g.groundTemp.data < dataSample_min_g.groundTemp.data)
	{
		dataSample_min_g.groundTemp = dataSample_g.groundTemp;
	}

	/* init values*/
	maxdata = 0;
	mindata = 0xFFFF;
	datasum = 0;

	/* radiation */
	for (i = 0; i <= 5; i++)
	{
		if (dataseries[i].radiation > maxdata)
		{
			max_index = i;
			maxdata = dataseries[i].radiation;
		}
		//TODO: should be else if
		if (dataseries[i].radiation < mindata)
		{
			min_index = i;
			mindata = dataseries[i].radiation;
		}
	}
	//printf("radiation: max_index is %d, min_index is %d\r\n", max_index, min_index);
	dataseries[min_index].radiation = 0;
	dataseries[max_index].radiation = 0;
	for (i = 0; i <= 5; i++)
	{
		datasum += dataseries[i].radiation;
	}
	dataSample_g.radiation.data = (UINT16)(datasum >> 2);	//divide 4
#ifdef _PRINT_ADC
	printf("radiation sum is %d \r\n", datasum);
#endif
	dataSample_g.radiation.time = get_current_time(currentTickCount);


	if (dataSample_g.radiation.data > dataSample_max_g.radiation.data)
	{
		dataSample_max_g.radiation = dataSample_g.radiation;
	}
	//TODO: should be else if
	if (dataSample_g.radiation.data < dataSample_min_g.radiation.data)
	{
		dataSample_min_g.radiation = dataSample_g.radiation;
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
	//UINT8 i;
	UINT16 maxdata = 0, mindata = 0, max_index = 0, min_index = 0;
	UINT32 currentTickCount = 0, tickCountDiff = 0;
	UINT32 datasum = 0;
	static UINT32 lastTickCount = 0xFFFFFFFF;

	UINT8 n_3s = 3, n_2m = 120, n_10m = 600;
	static UINT8 flag = 0;
	UINT8 windDirection = 0, windSpeed = 1;
	//UINT16 data_wind[2];		//saved sampling result.
	static UINT8 wind_index = 0;
	static  UINT16 data_wind_array[2][3], data_3s[2], data_1m[2], data_2m[2], data_10m[2];
	static UINT16 data_wind_series[4][2];	//4: 3s, 1m, 2m, 10m; 2: speed, direction
	//const UINT8 index_windDirection = 0, index_windSpeed = 1;
	//const UINT8 index_3s = 0, index_1m = 1, index_2m = 2, index_10m = 3;

	//data_wind_array saves 3sec data
	currentTickCount = SystemTickCount;	//buffer SystemTickCount, to avoid updating
	if (currentTickCount % 5 == 0)
	{
		if (lastTickCount != currentTickCount)
		{
			lastTickCount = currentTickCount;
			//TODO: wind direction and wind speed

			//get_series_data_sec(data_wind);
			get_series_data_sec(data_wind_series);
#if 0
			if (flag == 0)
			{
				// init data
				data_3s[windDirection] = data_wind[windDirection];
				data_wind_array[windDirection][1] = data_wind[windDirection];
				data_wind_array[windDirection][2] = data_wind[windDirection];
				data_1m[windDirection] = data_wind[windDirection];
				data_2m[windDirection] = data_wind[windDirection];
				data_10m[windDirection] = data_wind[windDirection];


				data_3s[windSpeed] = data_wind[windSpeed];
				data_wind_array[windSpeed][1] = data_wind[windSpeed];
				data_wind_array[windSpeed][2] = data_wind[windSpeed];
				data_1m[windSpeed] = data_wind[windSpeed];
				data_2m[windSpeed] = data_wind[windSpeed];
				data_10m[windSpeed] = data_wind[windSpeed];
				flag = 1;
			}
			/* 3 seconds, moving average method*/
			data_wind_array[windDirection][wind_index] = data_wind[windDirection];
			data_wind_array[windSpeed][wind_index] = data_wind[windSpeed];
			data_3s[windDirection] = (data_wind_array[windDirection][0] +
				data_wind_array[windDirection][1] +
				data_wind_array[windDirection][2]) / 3;
			data_3s[windSpeed] = (data_wind_array[windSpeed][0] +
				data_wind_array[windSpeed][1] +
				data_wind_array[windSpeed][2]) / 3;
			//wind_index = (wind_index + 1) & 3;

			/* 60s moving average method*/
			data_1m[windDirection] = data_1m[windDirection] + (data_wind[windDirection] - data_1m[windDirection]) / 60;
			data_1m[windSpeed] = data_1m[windSpeed] + (data_wind[windSpeed] - data_1m[windSpeed]) / 60;

			/* 120s moving average method*/
			data_2m[windDirection] = data_2m[windDirection] + (data_wind[windDirection] - data_2m[windDirection]) / 120;
			data_2m[windSpeed] = data_2m[windSpeed] + (data_wind[windSpeed] - data_2m[windSpeed]) / 120;
			//data_10m[windDirection] = data_10m[windDirection] + (data_wind[windDirection] - data_10m[windDirection]) / 600;
			//data_10m[windSpeed] = data_10m[windSpeed] + (data_wind[windSpeed] - data_10m[windSpeed]) / 600;
#endif
			if (currentTickCount % 300 == 0)
			{
				/* TODO: combine with 1min data processing*/
				// 1min, wind
				data_wind_series[index_10m][index_windDirection] = data_wind_series[index_10m][index_windDirection] +
					(data_wind_series[index_1m][index_windDirection] - data_wind_series[index_10m][index_windDirection]) / 10;
				data_wind_series[index_10m][index_windSpeed] = data_wind_series[index_10m][index_windSpeed] +
					(data_wind_series[index_1m][index_windSpeed] - data_wind_series[index_10m][index_windSpeed]) / 10;
				
				process_series_data_wind_1m(currentTickCount, data_wind_series);
				
				/*
				data_10m[windDirection] = data_1m[windDirection];
				data_10m[windSpeed] = data_1m[windSpeed];
				data_10m[windDirection] = data_10m[windDirection] + (data_wind[windDirection] - data_10m[windDirection]) / (600 / 60);
				data_10m[windSpeed] = data_10m[windSpeed] + (data_wind[windSpeed] - data_10m[windSpeed]) / (600 / 60);
				*/
			}

			if (currentTickCount % 600 == 0)
			{
				// 2min, wind
				process_series_data_wind_2m(currentTickCount, data_wind_series);

			}

			if (currentTickCount % 3000 == 0)
			{
				// 10min, wind
				process_series_data_wind_10m(currentTickCount, data_wind_series);

			}


			if (currentTickCount % tick_divider == 0)
			{
				//
				switch (data_index)
				{
					//temp, humidity, airPressure, groundTemp, radiation
				case 5:
					/*get 3rd data*/
					get_series_data_10sec(&dataseries[data_index]);
					data_index = 0;
					process_series_data_10sec(dataseries, currentTickCount);
					/*
					//Caculate howmany tick does it need
					tickCountDiff = SystemTickCount - currentTickCount;
					printf("TickCountDiff for case5 is %x\r\n", tickCountDiff);
					*/
					break;
				case 4:
					/*get 2nd data*/
					get_series_data_10sec(&dataseries[data_index]);
					//data[data_index] = get_data(0x0);
					data_index++;
					break;
				case 3:
					/*get 2nd data*/
					get_series_data_10sec(&dataseries[data_index]);
					//data[data_index] = get_data(0x0);
					data_index++;
					break;
				case 2:
					/*get 2nd data*/
					//data[data_index] = get_data(0x0);
					get_series_data_10sec(&dataseries[data_index]);
					data_index++;
					break;
				case 1:
					/*get 2nd data*/
					get_series_data_10sec(&dataseries[data_index]);
					//data[data_index] = get_data(0x0);
					data_index++;
					break;
				case 0:
					/*get 1st data*/
					//printf("data[0].temp is collected as %d\r\n", dataseries[data_index].temp);
					printf(".....1st data......\r\n");
					get_series_data_10sec(&dataseries[data_index]);
					data_index++;
					break;
				default:
					NOP();
					//printf("Illegal MeterAndSensorMode!!\r\n"); break;
				}
			}
			//get_data();
			//ch_show
			if (currentTickCount % 300 == 0)
			{// 1min
				//evaporation, sunShineTime, rain
				get_series_data_1min();
				process_series_data_1min(currentTickCount);
			}
			/* TODO: Shift 1 tick, possible to miss 1 tick. need to modify to be robust */
			// if ((currentTickCount+1) % (transInterval_g * 300) == 0)
			if (currentTickCount % (transInterval_g * 300) == 0)
			{
				/* process wind data(speed and direction), start */
				
				/* process wind data(speed and direction), end */
				
				/* add to fifo, write eeprom commands, extreme value(with date)*/
				(*CommandFifo.AddFifo)(&CommandFifo, 0x50);	
				//printf("fifo cmd 0x50: write eeprom commands\r\n");
#ifdef _ACCUMULATION_ENABLE
				dataSample_min_g.rain.data = 0;		//clear accumulated value every 5min, not minmum value
#endif
			}
		}
	}
}

/*******************************************************************************
* Function:      test_get_address()
* Arguments:
* Return:
* Description:
*******************************************************************************/
void test_get_address(void)
{
	Date_t targetTime = { 20,18,9,12,17,5 };
	UINT16 addr;
	addr = get_address(&targetTime);
	NOP();
}


