#include "w25q80.h"

// 注：W25Q80由256 BYTE 组成一个 PAGE，不可PGAE擦除，可以进行BYTE PROGRAM 或者 PAGE PROGRAM

// 由16 PAGE 组成一个 SECTOR， 可 SECTOR擦除

// 由16 SECTOR组成一个 BLOCK， 可 BLOCK 擦除

// 由16 BLOCK 组成一个 FULL MEMEORY，可 FULL MEMORY 擦除

// 所以，总容量是1M bytes

// W25Q80主要命令字

#define READ_ARRAY 0x03

#define SECTOR_ERASE 0x20

#define BYTE_OR_PAGE_PROGRAM 0x02

#define WRITE_ENABLE 0x06

#define WRITE_DISABLE 0x04

#define READ_STATUS_REGISTER 0x05

#define Manufacturer_DeviceID 0x9F

// 定义W25Q80的CS脚对应MCU的IO

#define W25Q80_CS P1_2

// SPI硬件初始化

void Spi_Init(void)

{

PERCFG |= 0x02; // SPI1映射到P1口

P1SEL |= 0xE0; // P15~P17作复用功能(clk mosi miso)

P1SEL &= ~0x04; // P12作GPIO

P1DIR |= 0x04; // P12作输出

P1_2 = 1; // P12输出高电平

U1CSR &= ~0xA0; // SPI主方式

U1GCR &= ~0xC0; // CPOL=0 CPHA=0

U1GCR |= 0x20; // MSB

U1BAUD = 0; // 波特率设为sysclk/8

U1GCR |= 0x11;

}

// SPI发送与接收字节

static u8 Spi_ReadWriteByte(u8 TxData)

{

U1DBUF = TxData; // 发送数据

while( !(U1CSR&0x02) );	// 等待发送完成

U1CSR &= 0xFD; // 清除发送完成标志

return U1DBUF;

}

// CS线拉低，使能芯片

static void W25Q80_Enable( void )

{

volatile u8 i;

W25Q80_CS = 0;

for ( i=5; i>0; i-- ); // 延时

}

// CS线拉高，禁能芯片

#define W25Q80_Disable() ( W25Q80_CS = 1 )

// 设置FLASH芯片"写使能"

static void SetW25Q80WriteEnable(void)

{

W25Q80_Enable();

Spi_ReadWriteByte(WRITE_ENABLE);

W25Q80_Disable();

}

// 设置FLASH芯片"写禁能"

//static void ClearW25Q80WriteEnable(void)

//{

// W25Q80_Enable();

// Spi_ReadWriteByte(WRITE_DISABLE);

// W25Q80_Disable();

//}

// 读取FLASH芯片的状态字节，可判断芯片是否busy

static u8 ReadW25Q80StatusRegister(void)

{

u8 temp;

W25Q80_Enable();

Spi_ReadWriteByte(READ_STATUS_REGISTER);

temp = Spi_ReadWriteByte(0xF0);

W25Q80_Disable();

return temp;

}

// 读取FLASH的内容，读取的字节数没有限制

void ReadW25Q80Operation(u32 addr,u8 *databuf,u32 len)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // 等待FLASH芯片结束BUSY状态

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

// 写入FLASH，一次最多写256字节，注意不要越界（不要超过页边界）

void WriteW25Q80Operation(u32 addr,u8 *databuf,u16 len)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // 等待FLASH芯片结束BUSY状态

{

temp = ReadW25Q80StatusRegister();

if ( ++i>1000000 ) return;

}

SetW25Q80WriteEnable(); // 设置芯片写使能

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

// 对FLASH进行SECTOR擦除

void EraseW25Q80Operation(u32 addr)

{

u32 i = 0;

u8 temp;

temp = ReadW25Q80StatusRegister();

while( temp&0x01 ) // 等待FLASH芯片结束BUSY状态

{

temp = ReadW25Q80StatusRegister();

if ( ++i>1000000 ) return;

}

SetW25Q80WriteEnable(); // 设置芯片写使能

W25Q80_Enable();

Spi_ReadWriteByte(SECTOR_ERASE);

Spi_ReadWriteByte((u8)(addr >>16));

Spi_ReadWriteByte((u8)(addr >>8));

Spi_ReadWriteByte((u8)addr);

W25Q80_Disable();

}

// 读取FLASH芯片的JEDEC ID信息

void ReadW25Q80DeviceID(u8 *buf)

{

W25Q80_Enable();

Spi_ReadWriteByte(Manufacturer_DeviceID);

buf[0] = Spi_ReadWriteByte(0xF0);

buf[1] = Spi_ReadWriteByte(0xF0);

buf[2] = Spi_ReadWriteByte(0xF0);

W25Q80_Disable();

}