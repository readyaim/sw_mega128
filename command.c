/****************************************************************************
* File name: command.c
* Description: polling commands in FIFO in main while. And run the cmd.
* MCU: ATmega128A AU 1036
* Crystal: External 8MHz
* Compile: ICCAVR 7.22
* Created: 20180907
* Author: s.z.
****************************************************************************/
#include "global.h"

extern void find_key(UINT8 n);
extern void init_SEG4(void);
extern void test_timer2(void);
extern void uart1_init(void);
extern void uart1_checkCMDPolling(void);
extern void ticker_timer1_handler(void);
extern void init_devices_timer1(void);
extern void write_dataSeries_to_eeprom(void);
extern void read_eepromCtrledByUART1(UINT8 addOffset);
extern void init_port_adc0(void);
extern void read_eeprom_to_UART1buffer(UINT16 addr);
extern void write_tickCountTime_to_eeprom(void);
extern void get_address_from_Time(UINT16* ptrAddr, Date_t *targetTime);
extern BOOL IsEmpty(struct Fifo *this);
extern UINT8 FetchFifo(struct Fifo *this);
extern volatile UINT16 addr_write_eeprom;
extern void get_current_time(Date_t* pTime, UINT32* currentTickCout);


/*******************************************************************************
* Function:     parseStr2Date()
* Arguments:  str: the date(Y,Y,M,D,H,M), the length of string is 6.
					 pdateTime: the pointer to Time
* Return:		 0: no error, 1 wrong input.
* Description:  transfer str[6](in hex) to Date_t
*******************************************************************************/
void parseStr2Date(UINT8 *str, Date_t *pdateTime)
{
	pdateTime->year1 = *str;
	pdateTime->year = *(str + 1);
	pdateTime->mon = *(str + 2);
	pdateTime->day = *(str + 3);
	pdateTime->hour = *(str + 4);
	pdateTime->min = *(str + 5);
	printf("new time is %d,%d:%d\r\n", pdateTime->day, pdateTime->hour, pdateTime->min);

}


/*******************************************************************************
* Function:     parseStr2Cmd()
* Arguments:  ch, the received char
* Return:
* Description:  parsing the received char, add related cmds to fifo, combine multi-chars to 1 command.(update time)
*******************************************************************************/
void parseStr2Cmd(UINT8 ch)
{
	static UINT8 state = 0;
	static UINT8 char_index = 0;
	static UINT8 str[UART1_MAX_RX_BUFFER];
	static UINT32 tickcoutStart;
	//Date_t newTime;
	//UINT16 addr_eeprom;
	UINT8 i = 0;

	switch (state)
	{
	case 0:
		//single char command
		if (ch > '0' && ch < '9')
		{
			AddFifo(&CommandFifo, ch);
		}
		else if (ch >= 'a' && ch <= 'o')
		{
			(*CommandFifo.AddFifo)(&CommandFifo, ch);	//add to fifo, read eeprom commands
			//printf("character %c is received\r\n", ch);
		}
		else if (ch >= 0x40 && ch <= 0x43)
		{
			//update transInterval_g
			(*CommandFifo.AddFifo)(&CommandFifo, ch);
		}
		else if (ch == '!')
		{
			//Time commands are coming.
			state = 1;				//to next state machine.
			char_index = 0;		//initiation
			tickcoutStart = SystemTickCount;		//timeout counter
		}
		else if (ch == '"')
		{
			//request upload data with attached time
			state = 2;
			char_index = 0;
		}
		break;

	case 1:
		//update timeStampShot_g, such as: !201809171750
		//multi-char commands
		if (ch > 60)
		{
			//non-number is received, quit update Time command mode.
			printf("non-valid input, quit state %d \r\n", state);
			state = 0;
		}
		else
		{
			//Update timeStampShot_g
			str[char_index++] = ch;
			if (char_index > 5)
			{
				//TODO:
				parseStr2Date(str, &timeStampShot_g.time);
				timeStampShot_g.tickeCounter = SystemTickCount;
				timeStampShot_g.currentAddrEEPROM = addr_write_eeprom;
				timeStampShot_g.flag = 1;		//new timeStamp, update get_current_time
				state = 0;

				printf("timeStampShot_g is updated\r\n");

			}
		}
		break;
	case 2:
		//upload eeprom data at requested time, such as: "201809201750
		//multi-char commands
		if (ch > 60)
		{
			//non-number is received, quit update Time command mode.
			printf("non-valid input, quit state %d \r\n", state);
			state = 0;
		}
		else
		{
			//Update timeStampShot_g
			str[char_index++] = ch;
			if (char_index > 5)
			{
				//TODO:
				parseStr2Date(str, &uploadTime_g);
				//AddFifo(&CommandFifo, 0x51);
				(*CommandFifo.AddFifo)(&CommandFifo, 0x51);		//add to fifo, read eeprom commands
				state = 0;
				printf("upload data is requested\r\n");
			}
		}
	default:
		break;
	}
}


/*******************************************************************************
* Function:  processCmd()
* Arguments:  data
* Return: 
* Description:  ����Fifo�ڵ�����
*******************************************************************************/
void processCmd(UINT8 data)
{
    UINT8 command, para;
    //UINT8 str[80];
	UINT16 addr_eeprom;
	Date_t uploadTime;
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
		case UPDATETIME:
			/* not used*/
			
			break;
        case UPDATE_transInterval_g:
            switch (para)
            {
                case 0: 
					transInterval_g = 5;
                    break;
                case 1:
					transInterval_g = 10;
                    break;
                case 2: 
					transInterval_g = 30; 
					break;
				case 3:
					transInterval_g = 60;
					break;
                default://printf("Illegal command!!\r\n"); 
                    break;
            }
			printf("transInterval_g = %d\n", transInterval_g);
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
				printf("write eeprom\r\n");
				write_dataSeries_to_eeprom();
				printf("write ends\r\n");
				//after writing max and min to eeprom, clear max/min value
				//TODO: use eman max_i, min_i, and array, void *p
				dataSample_max_g.temp.data = 0;
				dataSample_min_g.temp.data = 0xFFFF;
				dataSample_max_g.humidity.data = 0;
				dataSample_min_g.humidity.data = 0xFFFF;
				
				dataSample_max_g.airPressure.data = 0;
				dataSample_min_g.airPressure.data = 0xFFFF;
				dataSample_max_g.groundTemp.data = 0;
				dataSample_min_g.groundTemp.data = 0xFFFF;
				dataSample_max_g.radiation.data = 0;
				dataSample_min_g.radiation.data = 0xFFFF;
				
				dataSample_max_g.rain.data = 0;
				dataSample_min_g.rain.data = 0x0;
				dataSample_max_g.evaporation.data = 0;
				dataSample_min_g.evaporation.data = 0x0;
				dataSample_max_g.sunShineTime.data = 0;
				dataSample_min_g.sunShineTime.data = 0x0;
				
				//dataSample_max_g.windSpeed.data = 0;
				//dataSample_min_g.windSpeed.data = 0xFFFF;
				//dataSample_max_g.windDirection.data = 0;
				//dataSample_min_g.windDirection.data = 0xFFFF;
				break;
			case 1:
				//Upload data
				printf("run 0x51\r\n");
				get_address_from_Time(&addr_eeprom, &uploadTime_g);
				read_eeprom_to_UART1buffer(addr_eeprom);
				break;
			case 2:
				printf("run 0x52\r\n");
				write_tickCountTime_to_eeprom();
				break;
			case 3: 
				printf("0x53: routine upload\r\n");
				get_current_time(&uploadTime, &SystemTickCount);
				get_address_from_Time(&addr_eeprom, &uploadTime);
				read_eeprom_to_UART1buffer(addr_eeprom);
				
				//Reset(); break;
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
			/*
			switch (para)
			{
			case 0:
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
			*/
            //AdjustCCPWM(data & 0x1f);
            break;
        default:break;
    }
    //LED_Status = LED_SlowFlash;
    return;
}

#ifdef _TEST_CODE_INCLUDED
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
#endif
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
	//init_SEG4();    //enable SEG4, can be run in main()
	//test_timer2();  //enable timer2 ovf interrupt, can be run in main()
	//uart1_init();
	//init_port_adc0();
	init_devices_timer1();
	SEI();
	while (1)
	{
		empty = IsEmpty(&CommandFifo);
		if (!empty)
			processCmd(FetchFifo(&CommandFifo));

		ticker_timer1_handler();
		//uart1_checkCMDPolling();
	}
}
