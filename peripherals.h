#include "global.h"

//BEEP
#define BEEP_BIT 0

//数码管
#define SEG_PORT PORTD     //数码管数据口

#define SEG_CTRL_PORT PORTG
#define SEG_1_ONOFF 4 
#define SEG_2_ONOFF 3 
#define SEG_3_ONOFF 1 
#define SEG_4_ONOFF 0 

#define SEG4_ON   Clr_Bit(SEG_CTRL_PORT, SEG_4_ONOFF);  // PORTG &= 0XFE;    //PG0->SEG-4
#define SEG4_OFF  Set_Bit(SEG_CTRL_PORT, SEG_4_ONOFF);  // PORTG |= 0X01; 

#define SEG3_ON  Clr_Bit(SEG_CTRL_PORT, SEG_3_ONOFF);   // PORTG &= 0XFD;    //PG1->SEG-3
#define SEG3_OFF Set_Bit(SEG_CTRL_PORT, SEG_3_ONOFF);  //PORTG |= 0X02;

#define SEG2_ON  Clr_Bit(SEG_CTRL_PORT, SEG_2_ONOFF);   //PORTG &= 0XF7;    //PG3->SEG-2
#define SEG2_OFF Set_Bit(SEG_CTRL_PORT, SEG_2_ONOFF);   // PORTG |= 0X08;

#define SEG1_ON   Clr_Bit(SEG_CTRL_PORT, SEG_1_ONOFF);  //PORTG &= 0XEF;    //PG4->SEG-1
#define SEG1_OFF  Set_Bit(SEG_CTRL_PORT, SEG_1_ONOFF);  //PORTG |= 0X10;

//LED: PORTF
#define LED2 2 
#define LED3 3 
#define LED_PORT PORTF
#define LED_DDR DDRF

//ADC
#define ADC1 1 //mismatch in SCH
#define ADC0 0 
#define ADC2 	2  
#define ADC3 	3  
#define ADC4 	4
#define ADC5	    5  
#define ADC6 	6  
#define ADC7 	7  
#define ADC0_ADC0_10x 	8  
#define ADC1_ADC0_10x 	9 	
#define ADC0_ADC0_200x 	10 
#define ADC1_ADC0_200x 	11 
#define ADC2_ADC2_10x 	12 
#define ADC3_ADC2_10x 	13
#define ADC2_ADC2_200x 	14
#define ADC3_ADC2_200x 	15 
#define ADC0_ADC1_1x 	16	
#define ADC1_ADC1_1x 	17 
#define VBG 	30 
#define GND 	31 