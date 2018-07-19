#include "global.h"
#include "peripherals.h"

extern void init_devices_timer0(void);
extern void init_beep(void);
extern void init_SEG(void);
extern const UINT8 tab[10];//数码管

//初始化 变量 声明//////////////////////////////////////////////////////////////
UINT8 cnn = 0;//定时器变量
UINT8 i = 0;//数码管位选用
UINT8 ad = 0;//AD采集间隔
UINT16 ad_data1 = 0, ad_data2 = 0;



//I、O口初始化
void init_port_adc1(void)
{
    Clr_Bit(PORTF, ADC0);   //PORTF = 0X00;//AD采集口 ADC0 设置为输入
    Clr_Bit(DDRF, ADC0);    //DDRF = 0X00;
    
}
//CPU总体初始化
void init_devices(void)
{
    
    init_beep();
    init_SEG();
    init_port_adc1();
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
UINT16 ad_change(UINT16 value)//电压转换函数
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
            ad_data2 = ad_change(ad_data1);
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
void main_adc1(void)
{
    CLI();
    init_devices();
    beep();
    SEI();
    while (1)
    {

    }
}