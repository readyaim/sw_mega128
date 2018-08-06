#include "global.h"
#include "peripherals.h"
#include<string.h>



#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_SS_BIT 0
#define SPI_INT_EN() Set_Bit(SPCR, SPIE)
#define SPI_INT_DISABLE() Clr_Bit(SPCR, SPIE)

#define SS_0 Clr_Bit(SPI_PORT , SPI_SS_BIT)
#define SS_1 Set_Bit(SPI_PORT, SPI_SS_BIT)

#define xtal 8
UINT8 TXbuffer[8] = { 0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f };
UINT8 RXbuffer[8] = { 0,0,0,0,0,0,0,0 };
UINT8 cnt = 0;
UINT8 M_flag = 0;

extern void delay_ms(UINT16 millisecond);
extern void delay_us(UINT16 microsecond);

extern void led_on(void);
extern void led_off(void);


#ifdef _SPI_TX
UINT8 SPI_Send_nReceive = 0xFF;    //0xFF: spi send, 0: spi recieve
#else
UINT8 SPI_Send_nReceive = 0x00;    //0xFF: spi send, 0: spi recieve
#endif
#ifdef _SPI_MASTER
UINT8 SPI_Master_nSlave = 0xFF;    //0xFF: master, 0: slave
#else
UINT8 SPI_Master_nSlave = 0x00;    //0xFF: master, 0: slave
#endif


/*******************************************************************************
* Function:     SPI_MasterPort_init()
* Arguments:
* Return:
* Description: SPI port initialization. Set all 4 pins as output. It will automatically change to input when set MASTER/SLAVE
*******************************************************************************/
void SPI_MasterPort_init(void)
{
	//PORTB |= 0x50;//SS(PB4自定义)MOSI(PB5自定义)
	//DDRB |= 0xB0;//MISO(PB6输入)SCK(PB7自定义)
	//DDRB |= (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);     //fit for SPI master and slave
	//Set SCK/nSS/MOSI/MISO as output. When it's set at master/slave, the pins will auto change to input.
	//This settings fit both SPI master and slave.
	Set_Bit(SPI_DDR, PB3); //MISO as output
	Set_Bit(SPI_DDR, PB2); //MOSI as output
	Set_Bit(SPI_DDR, PB1); //SCK as output
	Set_Bit(SPI_DDR, PB0); //Set SS(PB0) as output
}
/*******************************************************************************
* Function:     SPI_MasterRegisters_init()
* Arguments:
* Return:
* Description: 1. SPE: SPI enable, MSTR: master, SPRD:001, CPU_CLK/16m, SPIE: Interrupt Enable
                     DORD:0, MSB first. CPOL:0 start with up-rising
                     2. use SPI_Master_nSlave to control SPI as master or slave
*******************************************************************************/
void SPI_MasterRegisters_init(void)//中断方式定义
{
	SPCR =  (1 << SPE) | (1 << (MSTR & SPI_Master_nSlave)) | (1 << SPR0);
}
void SPI_MasterInit(void)
{
	SPI_MasterPort_init();
	SPI_MasterRegisters_init();
}

/*******************************************************************************
* Function:     SPI_Inquiry_MasterTx()
* Arguments:  data to Tx (byte)
* Return:
* Description: need to Clr_Bit(SPCR, SPIE)
*******************************************************************************/
void SPI_Inquiry_MasterTx(UINT8 cData)
{
	/* need to Clr_Bit(SPCR, SPIE) */
	/* start transmit */
    SPDR = cData;
	/* wait until finished */
	while (!Get_Bit(SPSR, SPIF))
		;
}

/*******************************************************************************
* Function:     SPI_Inquiry_MasterRx()
* Arguments:  
* Return:        Return the received data(byte)
* Description: not verified. need to Clr_Bit(SPCR, SPIE)
*******************************************************************************/
UINT8 SPI_Inquiry_MasterRx(void)
{
    /* need to Clr_Bit(SPCR, SPIE) */
    /* start transmit */
    SPDR = 0x00;    //send 0x00, output SCK
    /* wait until finished */
    while (!Get_Bit(SPSR, SPIF))
        ;
    return SPDR;
}


/*******************************************************************************
* Function:     SPI_Inquiry_SlaveRx()
* Arguments:  
* Return:        data that Rx (byte)
* Description: need to Clr_Bit(SPCR, SPIE)
*******************************************************************************/
UINT8 SPI_Inquiry_SlaveRx(void)
{
    while (!Get_Bit(SPSR, SPIF))
        ;
    return SPDR;
}

/*******************************************************************************
* Function:     SPI_STC_isr()
* Arguments:
* Return:        data that Rx (byte)
* Description: ISR of SPI interrupt
*******************************************************************************/
#pragma interrupt_handler SPI_STC_isr:iv_SPI_STC
void SPI_STC_isr(void) 
{
    CLI();
#ifdef _SPI_TX
    RXbuffer[cnt] = SPDR;
    if (cnt >= 7) 
	{
		cnt = 0;
		M_flag = 1;
	} 
	cnt++;
	SPDR = TXbuffer[cnt];  //byte in SPDR has been sent/received
#else
    RXbuffer[cnt] = SPDR;
    if (cnt >= 7)
    {
        cnt = 0;
        M_flag = 1;
    }
    
    if (RXbuffer[cnt] == 0xA5)
    {
        beep();
        led_on();
    }
    else if (RXbuffer[cnt] == 0x86)
    {
        beep();
        led_off();
    }
        cnt++;
#endif
    SEI();
}

/*******************************************************************************
* Function:     main_spi_interrupt()
* Arguments:
* Return:
* Description: SPI main function
*******************************************************************************/
void main_spi_interrupt(void)
{
    UINT8 i = 0;
    //DDRA = 0xFF;
    CLI();
    SPI_MasterPort_init();
    SPI_MasterRegisters_init();
    //for (i = 0; i < 8; i++)
    //{
    //    PORTA = TXbuffer[i];
    //    delay_us(1);
    //}
    /* LED blinking*/
    SEI();
    SPI_INT_EN();
    SS_0;
    SPDR = TXbuffer[0];
    while (1)
    {
        Clr_Bit(LED_PORT, LED3);
        delay_ms(2000);
        Set_Bit(LED_PORT, LED3);
        delay_ms(2000);
        //if (M_flag == 1) {
        //	for (i = 0; i < 8; i++) {
        //		PORTA = RXbuffer[i];
        //		delay_us(1);
        //	}
        //}
    }//end of while
}

/*******************************************************************************
* Function:     main_spi_inquiry()
* Arguments:
* Return:
* Description: SPI main function
*******************************************************************************/
void main_spi_inquiry(void)
{
    UINT8 i = 0;
    CLI();
    SPI_MasterPort_init();
    SPI_MasterRegisters_init();
    SPI_INT_DISABLE();
    SEI();
    
    SS_0;
    SPI_Inquiry_MasterTx(0xA5);
    //SPI_Inquiry_MasterTx(0x5A);
    SS_1;
}

/*******************************************************************************
* Function:     test_spi_loop_Tx_inquiry()
* Arguments:
* Return:
* Description: output 0xA5 and 0x86 
*******************************************************************************/
void test_spi_loop_Tx_inquiry(void)
{
    UINT8 i = 0;
    UINT8 data[2] = { 0xA5, 0x86 };
    CLI();
    SPI_MasterPort_init();
    SPI_MasterRegisters_init();
    SPI_INT_DISABLE();
    SEI();

    while (1)
    {
        SS_0;
        SPI_Inquiry_MasterTx(data[0]);
        SS_1;
        delay_ms(1);
        SS_0;
        SPI_Inquiry_MasterTx(data[1]);
        //SPI_Inquiry_MasterTx(0x5A);
        SS_1;
        delay_ms(1);
    }
}
void main_spi(void)
{

#ifdef _SPI_Master
    test_spi_loop_Tx_inquiry();
#else
    main_spi_interrupt();
#endif // _SPI_Master
}