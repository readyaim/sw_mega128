#include "global.h"
#include "fifo.h"

extern void find_key(UINT8 n);
extern void init_port_keyboard(void);
extern void test_timer0(void);
extern void init_SEG4(void);

/*******************************************************************************
* Function:  processCmd()
* Arguments:  data
* Return: 
* Description:  处理Fifo内的命令
*******************************************************************************/
void processCmd(UINT8 data)
{
    UINT8 i, command;
    UINT8 str[80];
    /*struct Result result;
    LED_Status = LED_QuickFlash;*/
    command = (data & 0xF0) >> 4;
    switch (command)
    {
        case GENERAL:
            switch (data & 0x1f)
            {
                case 0: NOP();      // printf("Helo, Shen\r\n");
                    //MCUCSR |= (1 << JTD);
                    //MCUCSR |= (1 << JTD);
                    ////			         PronunciationString("BBBBBBBBBBBBBBBBB");
                    break;
                case 1: (*CommandFifo.ClearFifo)(&CommandFifo); break;
                case 2: //DeselectAllChannel(); break;
                case 3: //Reset(); break;
                default://printf("Illegal command!!\r\n"); 
                    break;
            }
            break;
        case ShowNumOnLED:
            /*ChannelNameIndex = data & 0x1f;
            for (i = 0; i < 12; i++) ChannelName[i] = ChannelNames[ChannelNameIndex][i];
            printf("Channel Name : %s\r\n", ChannelName);*/
            find_key(data & 0x0F);
            break;
        case SELECTCHANNEL:
        case SELECTEXPENDCHANNEL:
            //SelectedChannelNo = data & 0x3f;
            //SelectChannel(SelectedChannelNo);
            //printf("Switch to Channel %d\r\n", SelectedChannelNo);
            ////		 sprintf(str,"C %u",SelectedChannelNo);
            ////		 PronunciationString(str);
            break;
        case SETMEASUREMETHOD:
            if (data & 0x10) 
            {
                switch (data & 0x0f) 
                {
                    case 0:
                        /*MeterEnable = 0;
                        EnableMeter(0);*/
                        break;
                    case 1:
                        /*MeterEnable = 1;
                        EnableMeter(1);*/
                        break;
                    default:
                        NOP();
                        //printf("Illegal MeterAndSensorMode!!\r\n"); break;
                }
            }
            else 
            {
                switch (data & 0x0f) 
                {
                    case 0:
                    case 1:
                    case 2:
                        /*SelectedSensor = data & 0x0f;
                        SelectSensor(SelectedSensor);*/
                        break;
                    case 15:
                        //SelectedSensor = data & 0x0f;
                        break;
                    default:
                        //printf("Illegal Sensor!!\r\n"); 
                        break;
                }
            }
            break;
        case MEASURECURRENT:
            /*MeasurePeriod = data & 0x1f;
            if (MeasurePeriod == 0x10) Analyse();
            printf(" _______________________________________________ \r\n");
            printf("|Channel %4u:%s, Test period %6.2fs...|\r\n", SelectedChannelNo, ChannelName, (float)(1 << (MeasurePeriod)) / 100);
            printf("|_______________________________________________|__________________________\r\n");
            printf("|Range|  Sensor |Res(mA)|CaliResult|Counts|absMax(mA)| Avg(mA)  |absMin(mA)|\r\n");
            result = Measure(1 << (MeasurePeriod));
            printf(" -------------------------------------------------------------------------- \r\n");*/
            //		 if (result.Max<15) result=Measure(1<<(MeasurePeriod),1);
            //		 sprintf(str,"B, C %u, M %.3fA,a %.3fA, m %.3fA",\
            //		 SelectedChannelNo, result.Max,result.Avg,result.Min);
            //		 PronunciationString(str);
            break;
        case SETChannelCV:
            /*SetCVPWM(data & 0x1f);
            printf("CVPWM %u\r\n", data & 0x1f);*/
            break;
        case SETCCPWM:
            //AdjustCCPWM(data & 0x1f);
            break;
        default:break;
    }
    //LED_Status = LED_SlowFlash;
    return;
}
/*******************************************************************************
* Function:  main_processCmd()
* Arguments:  
* Return: 
* Description:  处理命令的入口函数
*******************************************************************************/
void main_processCmd(void)
{
    int i = 0;
    BOOL empty;
    //init_port_keyboard();
    empty = IsEmpty(&CommandFifo);
    //for (i = 0; i < 10; i++)
    //{
    //    AddFifo(&CommandFifo, 0x10|i);
    //    //(*CommandFifo.AddFifo)(&CommandFifo, 0x11);
    //}
   
     //while (1) 
     //   {
     //       if (!(*CommandFifo.IsEmpty)(&CommandFifo))
     //           processCmd((*CommandFifo.FetchFifo)(&CommandFifo));
     //       /*if (KeyPressed) {
     //       HandleKey();
     //       }*/
     //       delay_ms(3000);
     //   }
    test_timer0();
    while (1)
    {
        empty = IsEmpty(&CommandFifo);
        if (!empty)
            processCmd(FetchFifo(&CommandFifo));
        /*if (KeyPressed) {
        HandleKey();
        }*/
        //delay_ms(5000);
        //NOP();
    }

}
