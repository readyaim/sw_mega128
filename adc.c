/****************************************************************************
* File name: adc.c
* Description: driver for adc on chip.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/

#include "global.h"
#include "peripherals.h"

#define _ADC_updated
extern void init_devices_timer0(void);
extern void init_beep(void);
extern void init_SEG(void);
extern const UINT8 tab[10];//数码管


//初始化 变量 声明//////////////////////////////////////////////////////////////
UINT8 cnn = 0;//定时器变量
UINT8 i = 0;//数码管位选用
UINT8 ad = 0;//AD采集间隔
UINT16 ad_data1 = 0, ad_data2 = 0;


/*******************************************************************************
* Function:     choose_ADC_channel()
* Arguments:
* Return:
* Description:  Choose adc channel. choose adc0 when channel==0x00, adc01 when 0x01.
Only 1 channel is allowed in one time. Need to add excepion TODO
*******************************************************************************/
void choose_ADC_channel(UINT8 channel)
{
	ADMUX = (ADMUX & 0xE0) | channel;
}

//I、O口初始化
void init_port_adc0(void)
#ifndef _ADC_updated
{
    //because of hardware fault, adc0 connects to RP2
    Clr_Bit(PORTF, ADC0);   //PORTF = 0X00;//AD采集口 ADC0 设置为输入
    Clr_Bit(DDRF, ADC0);    //DDRF = 0X00;
    
}
#else
{
    //because of hardware fault in demo board, adc0 connects to RP2
    Clr_Bit(PORTF, ADC0);   //PORTF = 0X00;//AD采集口 ADC0 设置为输入
    Clr_Bit(DDRF, ADC0);    //DDRF = 0X00;

	Clr_Bit(PORTF, ADC1);   //PORTF = 0X00;//AD采集口 ADC1 设置为输入
	Clr_Bit(DDRF, ADC1);    //DDRF = 0X00;

    //ADMUX = 0x00;   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    //ADMUX |= (1 << REFS1) | (1 << REFS0);   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    ADMUX = (1 << REFS1) | (1 << REFS0);   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    //ADMUX &= 0xE0;      //BIT[4:0]: choose ADC0
    choose_ADC_channel(0x00);   //BIT[4:0]: choose ADC0 as default. Shall be set before adc_polling.
    Clr_Bit(ADMUX, ADLAR);  // BIT(ADLAR)=0:right adjusted result

    ACSR = 0x80;//此处加不加都可以, 比较器设置
    Set_Bit(ADCSRA, ADEN);  //enable ADC
    //Set_Bit(ADCSRA, ADSC);  //启动转换, ADSC will be 0 when conversion is finished(generally after 13 ADCclk)
    Clr_Bit(ADCSRA, ADFR);  //disable free running mode. ADCSRA &= ~BIT(ADFR); In free-running(set to 1) mode, the ADC samples and updates the Data Registers continuously
    Clr_Bit(ADCSRA, ADIE);  //ADC interrupt disable: 0. Use polling.
    //Set_Bit(ADCSRA, ADSC);  //启动转换, ADSC will be 0 when conversion is finished(generally after 13 ADCclk)
}
#endif
//CPU总体初始化
void init_ADC_devices(void)
{
    
    init_port_adc0();
    init_devices_timer0();
    
}

UINT16 ad_cat(void)//电压采集函数
{
    UINT16 t1, t2;

    //ADMUX = 0x00;   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    ADMUX |= (1 << REFS1) | (1 << REFS0);   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    ADMUX &= 0xE0;      //BIT[4:0]: choose ADC0
    Clr_Bit(ADMUX, ADLAR);  // BIT(ADLAR)=0:right adjusted result

    ACSR = 0x80;//此处加不加都可以, 比较器设置
    Set_Bit(ADCSRA, ADEN);  //使能ADC
    Set_Bit(ADCSRA, ADSC);  //启动转换
    Clr_Bit(ADCSRA, ADFR);  //ADCSRA &= ~BIT(ADFR);
    Clr_Bit(ADCSRA, ADIE);  //ADC interrupt disable: 0


    while (!(ADCSRA&(BIT(ADIF))));//等待转换结束, 查询方式
    Set_Bit(ADCSRA, ADIF);  //ADIF在ADC转换结束后置位, write ADIF to '1' to clear ADIF.
    t1 = (UINT16)ADCL;
    t2 = (UINT16)ADCH;
    t2 = ((t2 << 8)&(0XFF00)) + (t1&(0X00FF));
    return t2;
}

/*******************************************************************************
* Function:     ad_start_conversion()
* Arguments:
* Return:        
* Description:  start adc conversion
*******************************************************************************/
void ad_start_conversion(void)
{
    Set_Bit(ADCSRA, ADSC);  //启动转换
}


/*******************************************************************************
* Function:     ad_conversion_polling()
* Arguments:
* Return:        return the average ADC conversion data
* Description:  read ADC0, and average in 10 samples(skip the obvious bad result).
*******************************************************************************/
UINT16 ad_conversion_polling(void)//电压采集函数
{
    UINT16 t1, t2;
    UINT8 exp_index = 4;     //need to be 2^n, to avoid using division. 
    UINT16 dataADC_sum = 0;  //ADCL=8bit, ADCH is 2bit width. dataADC_sum can be 64*ADCH before overflow.
    //UINT32 dataADC_sum=0;  //ADCL=8bit, ADCH is 2bit width. dataADC_sum can be 64*ADCH before overflow. for exp_index >7
    UINT16 dataADC = 0;
    UINT8 i = 0;
#ifndef _ADC_updated
    //ADMUX = 0x00;   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    ADMUX |= (1 << REFS1) | (1 << REFS0);   //1100 0000     11:内部基准电压 0:左对齐 00000:ADC0通道
    ADMUX &= 0xE0;      //BIT[4:0]: choose ADC0
    Clr_Bit(ADMUX, ADLAR);  // BIT(ADLAR)=0:right adjusted result

    ACSR = 0x80;//此处加不加都可以, 比较器设置
    Set_Bit(ADCSRA, ADEN);  //使能ADC
    Set_Bit(ADCSRA, ADSC);  //启动转换
    Clr_Bit(ADCSRA, ADFR);  //ADCSRA &= ~BIT(ADFR);
    Clr_Bit(ADCSRA, ADIE);  //ADC interrupt disable: 0
#endif
    for (i=0;i<(1 << exp_index); i++)
    {
        ad_start_conversion();
        while (!(ADCSRA&(BIT(ADIF))));//等待转换结束, 查询方式
        Set_Bit(ADCSRA, ADIF);  //ADIF在ADC转换结束后置位, write ADIF to '1' to clear ADIF.
        t1 = (UINT16)ADCL;
        t2 = (UINT16)ADCH;
        t2 = ((t2 << 8)&(0XFF00)) + (t1&(0X00FF));
        //printf("t2 = %d", t2);
        //return t2;  //need to be removed
        //dataADC_sum += (UINT32)t2;
        dataADC_sum += t2;
    }
    dataADC = (UINT16)(dataADC_sum >> exp_index);
    return dataADC;
}

UINT16 dataADC_HEX2mv(UINT16 value)//电压转换函数
{
    INT32 x;
    UINT16 y;
    x = (2560 * (INT32)value) / 1024;   
    y = (UINT16)x;
    return y;   //unit: mv
}
#ifdef TIMER0_ADC
//定时器0///////////////////////////////////////////////////////////////////////
#pragma interrupt_handler timer0_ovf_isr_adc0:iv_TIMER0_OVF
void timer0_ovf_isr_adc0(void)
{
    TCNT0 = 0X05;//内部晶振，几分频算出时间初值
    cnn++;
    if (4 == cnn)//4*1ms=4ms
    {
        cnn = 0;
        ad++;
        if (25 == ad)//AD采集
        {
            ad = 0;
            ad_data1 = ad_cat();
            printf("ad_data1 = %d", ad_data1);
            ad_data2 = dataADC_HEX2mv(ad_data1);
        }
        switch (i++)
        {
        case 0:SEG4_OFF; SEG_PORT = (tab[ad_data2 / 1000]) | (0X80); SEG1_ON; break;    //(0x80 is the dot)
        case 1:SEG1_OFF; SEG_PORT = tab[(ad_data2 / 100) % 10]; SEG2_ON; break;
        case 2:SEG2_OFF; SEG_PORT = tab[(ad_data2 / 10) % 10]; SEG3_ON; break;
        case 3:SEG3_OFF; SEG_PORT = tab[ad_data2 % 10]; SEG4_ON; i = 0; break;
        default:i = 0; break;
        }//end of switch(i++)
    }//end of if(10==++cnn)
}//end of void timer0_ovf_isr_adc0(void)
////////////////////////////////////////////////////////////////////////////////
#endif

void collectADC0(void)
{
    UINT16 dataADC;
    UINT16 dataADC_mv;
    init_port_adc0();
    init_beep();
    beep();
    SEI();
    dataADC=ad_conversion_polling();
    printf("ADC0 HEX result is %d\r\n", dataADC);
    dataADC_mv = dataADC_HEX2mv(dataADC);
    printf("ADC0 mv result is %d mv\r\n", dataADC_mv);


}
/*******************************************************************************
* Function:     get_data_adc()
* Arguments:  channel: 0=adc0, 1=adc1,...,7=adc7
* Return:        return the average ADC conversion data
* Description:  read ADC0, and average in 10 samples(skip the obvious bad result).
*******************************************************************************/

UINT16 get_data_adc(UINT8 channel)
{
	UINT16 dataADC;
	UINT16 dataADC_mv;
	choose_ADC_channel(channel);
	dataADC = ad_conversion_polling();
	//printf("ADC0 HEX result is %d\r\n", dataADC);
	dataADC_mv = dataADC_HEX2mv(dataADC);
	//printf("ADC%d is %d mv\r\n", channel, dataADC_mv);
	return dataADC_mv;
}

void collectADC1(void)
{
    NOP();
}
void main_adc1(void)
{
    CLI();
    init_beep();
    init_SEG();
    init_ADC_devices();
    beep();
    SEI();
    while (1)
    {

    }
}