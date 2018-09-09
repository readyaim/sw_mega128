#include "w25q80.h"

// ע��W25Q80��256 BYTE ���һ�� PAGE������PGAE���������Խ���BYTE PROGRAM ���� PAGE PROGRAM

// ��16 PAGE ���һ�� SECTOR�� �� SECTOR����

// ��16 SECTOR���һ�� BLOCK�� �� BLOCK ����

// ��16 BLOCK ���һ�� FULL MEMEORY���� FULL MEMORY ����

// ���ԣ���������1M bytes

// W25Q80��Ҫ������

#define READ_ARRAY 0x03

#define SECTOR_ERASE 0x20

#define BYTE_OR_PAGE_PROGRAM 0x02

#define WRITE_ENABLE 0x06

#define WRITE_DISABLE 0x04

#define READ_STATUS_REGISTER 0x05

#define Manufacturer_DeviceID 0x9F

// ����W25Q80��CS�Ŷ�ӦMCU��IO

#define W25Q80_CS P1_2

// SPIӲ����ʼ��

void Spi_Init(void)

{

PERCFG |= 0x02; // SPI1ӳ�䵽P1��

P1SEL |= 0xE0; // P15~P17�����ù���(clk mosi miso)

P1SEL &= ~0x04; // P12��GPIO

P1DIR |= 0x04; // P12�����

P1_2 = 1; // P12����ߵ�ƽ

U1CSR &= ~0xA0; // SPI����ʽ

U1GCR &= ~0xC0; // CPOL=0 CPHA=0

U1GCR |= 0x20; // MSB

U1BAUD = 0; // ��������Ϊsysclk/8

U1GCR |= 0x11;

}

// SPI����������ֽ�

static u8 Spi_ReadWriteByte(u8 TxData)

{

U1DBUF = TxData; // ��������

while( !(U1CSR&0x02) );	// �ȴ��������

U1CSR &= 0xFD; // ���������ɱ�־

return U1DBUF;

}

// CS�����ͣ�ʹ��оƬ

static void W25Q80_Enable( void )

{

volatile u8 i;

W25Q80_CS = 0;

for ( i=5; i>0; i-- ); // ��ʱ

}

// CS�����ߣ�����оƬ

#define W25Q80_Disable() ( W25Q80_CS = 1 )

// ����FLASHоƬ"дʹ��"

static void SetW25Q80WriteEnable(void)

{

W25Q80_Enable();

Spi_ReadWriteByte(WRITE_ENABLE);

W25Q80_Disable();

}

// ����FLASHоƬ"д����"

//static void ClearW25Q80WriteEnable(void)

//{

// W25Q80_Enable();

// Spi_ReadWriteByte(WRITE_DISABLE);

// W25Q80_Disable();

//}

// ��ȡFLASHоƬ��״̬�ֽڣ����ж�оƬ�Ƿ�busy

static u8 ReadW25Q80StatusRegister(void)

{

u8 temp;

W25Q80_Enable();

Spi_ReadWriteByte(READ_STATUS_REGISTER);

temp = Spi_ReadWriteByte(0xF0);

W25Q80_Disable();

return temp;

}

// ��ȡFLASH�����ݣ���ȡ���ֽ���û������

void ReadW25Q80Operation(u32 addr,u8 *databuf,u32 len)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // �ȴ�FLASHоƬ����BUSY״̬

{

temp = ReadW25Q80StatusRegister();

if ( ++i>1000000 ) return;

}

W25Q80_Enable();

Spi_ReadWriteByte(READ_ARRAY);

Spi_ReadWriteByte((u8)(addr >>16));

Spi_ReadWriteByte((u8)(addr >>8));

Spi_ReadWriteByte((u8)addr);

for ( i=0; i<len; i++ )

{

databuf[i] = Spi_ReadWriteByte(0xF0);

}

W25Q80_Disable();

}

// д��FLASH��һ�����д256�ֽڣ�ע�ⲻҪԽ�磨��Ҫ����ҳ�߽磩

void WriteW25Q80Operation(u32 addr,u8 *databuf,u16 len)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // �ȴ�FLASHоƬ����BUSY״̬

{

temp = ReadW25Q80StatusRegister();

if ( ++i>1000000 ) return;

}

SetW25Q80WriteEnable(); // ����оƬдʹ��

W25Q80_Enable();

Spi_ReadWriteByte(BYTE_OR_PAGE_PROGRAM);

Spi_ReadWriteByte((u8)(addr >>16));

Spi_ReadWriteByte((u8)(addr >>8));

Spi_ReadWriteByte((u8)addr);

for ( i=0; i<len; i++ )

{

Spi_ReadWriteByte(databuf[i]);

}

W25Q80_Disable();

}

// ��FLASH����SECTOR����

void EraseW25Q80Operation(u32 addr)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // �ȴ�FLASHоƬ����BUSY״̬

{

temp = ReadW25Q80StatusRegister();

if ( ++i>1000000 ) return;

}

SetW25Q80WriteEnable(); // ����оƬдʹ��

W25Q80_Enable();

Spi_ReadWriteByte(SECTOR_ERASE);

Spi_ReadWriteByte((u8)(addr >>16));

Spi_ReadWriteByte((u8)(addr >>8));

Spi_ReadWriteByte((u8)addr);

W25Q80_Disable();

}

// ��ȡFLASHоƬ��JEDEC ID��Ϣ

void ReadW25Q80DeviceID(u8 *buf)

{

W25Q80_Enable();

Spi_ReadWriteByte(Manufacturer_DeviceID);

buf[0] = Spi_ReadWriteByte(0xF0);

buf[1] = Spi_ReadWriteByte(0xF0);

buf[2] = Spi_ReadWriteByte(0xF0);

W25Q80_Disable();

}