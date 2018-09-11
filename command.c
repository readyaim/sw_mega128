#include "global.h"
#include "fifo.h"

extern void find_key(UINT8 n);
extern void init_SEG4(void);
extern void test_timer2(void);
extern void uart1_init(void);
extern void uart1_checkCMDPolling(void);
extern void ticker_timer1_handler(void);
extern void init_devices_timer1(void);
extern void write_dataSeries2eeprom(void);
extern void read_eepromCtrledByUART1(UINT8 addOffset);
extern void init_port_adc0(void);

/*******************************************************************************
* Function:  processCmd()
* Arguments:  data
* Return: 
* Description:  处理Fifo内的命令
*******************************************************************************/
void processCmd(UINT8 data)
{
    UINT8 i, command, para;
    UINT8 str[80];
    /*struct Result result;
    LED_Status = LED_QuickFlash;*/
    command = (data & 0xF0) >> 4;
	para = (data & 0x0F);
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
        case CollectData:
            switch (data & 0x0f)
            {
                case 1: 
                    //'A'
                    NOP(); 
                    // read temperature
                    break;
                case 2:
                    //'B'
                    NOP();
                    // read humidity
                    break;
                case 3: //Reset(); break;
                default://printf("Illegal command!!\r\n"); 
                    break;
            }
            break;
        case SAVE2EEPROM:
            //SelectedChannelNo = data & 0x3f;
            //SelectChannel(SelectedChannelNo);
            //printf("Switch to Channel %d\r\n", SelectedChannelNo);
            ////		 sprintf(str,"C %u",SelectedChannelNo);
            ////		 PronunciationString(str);
			switch (para)
			{
			case 0:
				// write data to eeprom
				write_dataSeries2eeprom();
				//after writing max and min to eeprom, clear max/min value
				dataSample_max_g.temp.data = 0;
				dataSample_min_g.temp.data = 0xFFFF;
				dataSample_max_g.humidity.data = 0;
				dataSample_min_g.humidity.data = 0xFFFF;

				break;
			case 1:
				//write max value
				write_extremeData2eeprom(&dataInRom_max_g, para);
				break;
			case 2:
				//write min value
				write_extremeData2eeprom(&dataInRom_min_g, para);
				break;
			case 3: //Reset(); break;
			default://printf("Illegal command!!\r\n"); 
				break;
			}
			
            break;
		case READEEPROM:
			read_eepromCtrledByUART1(data&0xF);
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
        

        case SETCCPWM:
            //AdjustCCPWM(data & 0x1f);
            break;
        default:break;
    }
    //LED_Status = LED_SlowFlash;
    return;
}
/*******************************************************************************
* Function:  timer2_processCmd()
* Arguments:  
* Return: 
* Description:  timer2 writes CommandFifo every 4s.
                      processCmd executes (show data in SEG4) fifo command when CommandFifo is not empty
*******************************************************************************/
void timer2_processCmd(void)
{
    int i = 0;
    BOOL empty;
    init_SEG4();    //enable SEG4, can be run in main()
    test_timer2();  //enable timer2 ovf interrupt, can be run in main()
    while (1)
    {
        empty = IsEmpty(&CommandFifo);
        if (!empty)
            processCmd(FetchFifo(&CommandFifo));
        /*if (KeyPressed) {
        HandleKey();
        }*/
    }
}
/*******************************************************************************
* Function:  uart1_processCmd()
* Arguments:
* Return:
* Description:  if the ch from UART1 is between '0' to '9', then AddFifo cmd to display num in SEG4.
                processCmd executes (show data in SEG4) fifo command when CommandFifo is not empty
*******************************************************************************/
void uart1_processCmd(void)
{
    int i = 0;
    BOOL empty;
    CLI();
    init_SEG4();    //enable SEG4, can be run in main()
    //test_timer2();  //enable timer2 ovf interrupt, can be run in main()
    uart1_init();
    SEI();
    while (1)
    {
        empty = IsEmpty(&CommandFifo);
        if (!empty)
            processCmd(FetchFifo(&CommandFifo));
        /*if (KeyPressed) {
        HandleKey();
        }*/
        uart1_checkCMDPolling();
    }
}
/*******************************************************************************
* Function:     ticker_processCmd()
* Arguments:
* Return:
* Description: test ticker_time() 
*******************************************************************************/
void ticker_processCmd(void)
{
	int i = 0;
	BOOL empty;
	CLI();
	init_SEG4();    //enable SEG4, can be run in main()
	//test_timer2();  //enable timer2 ovf interrupt, can be run in main()
	uart1_init();
	init_port_adc0();
	init_devices_timer1();
	SEI();
	while (1)
	{
		empty = IsEmpty(&CommandFifo);
		if (!empty)
			processCmd(FetchFifo(&CommandFifo));

		ticker_timer1_handler();
		uart1_checkCMDPolling();
	}
}
