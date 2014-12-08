//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>

#define SCREEN_WIDTH	122

#define SED1520_DATA_PORT 	PORTC
#define SED1520_DATA_DDR 	DDRC
#define SED1520_DATA_PIN 	PINC

#define SED1520_CONTROL_PORT 	PORTA
#define SED1520_CONTROL_DDR 	DDRA

#define SED1520_A0 (1 << 0)         // a0
#define SED1520_E1 (1 << 1)         // a1
#define SED1520_E2 (1 << 2)         // a2
#define SED1520_RW (1 << 3)         // a3
#define SED1520_RES (1 << 7)        //a7

extern unsigned char lcd_x, lcd_y;
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

void GLCD_InitPorts(void)
{
SED1520_DATA_DDR = 0xFF;

SED1520_CONTROL_DDR |= (SED1520_E2 | SED1520_E1 | SED1520_RW | SED1520_A0 | SED1520_RES);
_delay_ms(10);
SED1520_CONTROL_PORT |= SED1520_RES;

}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WaitForStatus(unsigned char status, unsigned char controller)
{
char tmp;
SED1520_CONTROL_PORT &= ~SED1520_A0; 
SED1520_CONTROL_PORT |= SED1520_RW; 
SED1520_DATA_DDR = 0x00; 
do
  {
  if(controller == 0) 
    {
	SED1520_CONTROL_PORT |= SED1520_E1; 
	asm("nop");asm("nop"); 
	tmp = SED1520_DATA_PIN; 
	SED1520_CONTROL_PORT &= ~SED1520_E1; 
	} 
  else 
    {
	SED1520_CONTROL_PORT |= SED1520_E2; 
	asm("nop");asm("nop"); 
	tmp = SED1520_DATA_PIN; 
	SED1520_CONTROL_PORT &= ~SED1520_E2; 
	}
  }while(tmp & status);
SED1520_DATA_DDR = 0xFF; 
}
//-------------------------------------------------------------------------------------------------
// Write command
//-------------------------------------------------------------------------------------------------
void GLCD_WriteCommand(unsigned char commandToWrite,unsigned char ctrl)
{
GLCD_WaitForStatus(0x80, ctrl);

SED1520_CONTROL_PORT &= ~SED1520_A0;
SED1520_CONTROL_PORT &= ~SED1520_RW;

SED1520_DATA_PORT = commandToWrite;

if(ctrl)
  {
  SED1520_CONTROL_PORT |= SED1520_E2;
  asm("nop");asm("nop");
  SED1520_CONTROL_PORT &= ~SED1520_E2;
  }
else
  {
  SED1520_CONTROL_PORT |= SED1520_E1;
  asm("nop");asm("nop");
  SED1520_CONTROL_PORT &= ~SED1520_E1;
  }
}
//-------------------------------------------------------------------------------------------------
// Write data
//-------------------------------------------------------------------------------------------------
void GLCD_WriteData(unsigned char dataToWrite)
{
GLCD_WaitForStatus(0x80, 0);
GLCD_WaitForStatus(0x80, 1);
SED1520_CONTROL_PORT |= SED1520_A0; 
SED1520_CONTROL_PORT &= ~SED1520_RW; 
SED1520_DATA_PORT = dataToWrite; 
if(lcd_x < 61) 
  {
  SED1520_CONTROL_PORT |= SED1520_E1;
  asm("nop");asm("nop");
  SED1520_CONTROL_PORT &= ~SED1520_E1;
  }
else
  {
  SED1520_CONTROL_PORT |= SED1520_E2;
  asm("nop");asm("nop");
  SED1520_CONTROL_PORT &= ~SED1520_E2;
  }
lcd_x++;
if(lcd_x >= SCREEN_WIDTH)
	lcd_x = 0;
}
//-------------------------------------------------------------------------------------------------
// Read data
//-------------------------------------------------------------------------------------------------
unsigned char GLCD_ReadData(void)
{
unsigned char tmp;

GLCD_WaitForStatus(0x80, 0); 
GLCD_WaitForStatus(0x80, 1); 
SED1520_CONTROL_PORT |= SED1520_A0; 
SED1520_CONTROL_PORT |= SED1520_RW; 
SED1520_DATA_DDR = 0x00; 
SED1520_DATA_PORT = 0xFF;
if(lcd_x < 61)
    {
  	SED1520_CONTROL_PORT |= SED1520_E1;
  	asm("nop");asm("nop");
  	SED1520_CONTROL_PORT &= ~SED1520_E1;
	asm("nop");asm("nop");
	SED1520_CONTROL_PORT |= SED1520_E1; 
	asm("nop");asm("nop");
	tmp = SED1520_DATA_PIN; 
	SED1520_CONTROL_PORT &= ~SED1520_E1; 
	}
else 
    {	
    SED1520_CONTROL_PORT |= SED1520_E2;
    asm("nop");asm("nop");
    SED1520_CONTROL_PORT &= ~SED1520_E2;
	asm("nop");asm("nop");
	SED1520_CONTROL_PORT |= SED1520_E2;
	asm("nop");asm("nop");
	tmp = SED1520_DATA_PIN;	
	SED1520_CONTROL_PORT &= ~SED1520_E2; 
	}
SED1520_DATA_DDR = 0xFF; 
lcd_x++; 
if(lcd_x > 121)
  lcd_x = 0;
return tmp;
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------