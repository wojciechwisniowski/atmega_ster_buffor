//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
#include "SED1520.h"
//-------------------------------------------------------------------------------------------------
unsigned char lcd_x = 0, lcd_y = 0;
//-------------------------------------------------------------------------------------------------
extern void GLCD_WaitForStatus(unsigned char, unsigned char);
extern void GLCD_WriteCommand(unsigned char, unsigned char);
extern void GLCD_WriteDatta(unsigned char);
extern unsigned char GLCD_ReadData(void);
extern void GLCD_InitPorts(void);
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_GoTo(unsigned char x, unsigned char y) {
	lcd_x = x;
	lcd_y = y;

	if (x < (SCREEN_WIDTH / 2)) {
		GLCD_WriteCommand(COLUMN_ADDRESS_SET | lcd_x, 0);
		GLCD_WriteCommand(PAGE_ADDRESS_SET | lcd_y, 0);
		GLCD_WriteCommand(COLUMN_ADDRESS_SET | 0, 1);
		GLCD_WriteCommand(PAGE_ADDRESS_SET | lcd_y, 1);
	} else {
		GLCD_WriteCommand(COLUMN_ADDRESS_SET | (lcd_x - (SCREEN_WIDTH / 2)), 1);
		GLCD_WriteCommand(PAGE_ADDRESS_SET | lcd_y, 1);
	}
}
//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------
void GLCD_ClearScreen(void) {
	char j, i;
	for (j = 0; j < 4; j++) {
		GLCD_GoTo(0, j);
		for (i = 0; i < SCREEN_WIDTH; i++) {
			GLCD_WriteData(0);
		}
	}
	GLCD_GoTo(0, 0);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteChar(char x) {
	char i;
	x -= 32;
	for (i = 0; i < 5; i++)
		GLCD_WriteData(pgm_read_byte(font5x7 + (5 * x) + i));
	GLCD_WriteData(0x00);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteString(char * s) {
	while (*s) {
		GLCD_WriteChar(*s++);
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_SetPixel(unsigned char x, unsigned char y, unsigned char color) {
	unsigned char temp;
	GLCD_GoTo(x, y / 8);
	temp = GLCD_ReadData();
	GLCD_GoTo(x, y / 8);
	if (color)
		GLCD_WriteData(temp | (1 << (y % 8)));
	else
		GLCD_WriteData(temp & ~(1 << (y % 8)));
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Bitmap(char * bmp, unsigned char x, unsigned char y, unsigned char dx,
		unsigned char dy) {
	unsigned char i, j;
	for (j = 0; j < dy / 8; j++) {
		GLCD_GoTo(x, y + j);
		for (i = 0; i < dx; i++)
			GLCD_WriteData(pgm_read_byte(bmp++));
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Init(void) {
	GLCD_InitPorts();
	GLCD_WriteCommand(RESET, 0);
	GLCD_WriteCommand(RESET, 1);
	GLCD_WaitForStatus(0x10, 0);
	GLCD_WaitForStatus(0x10, 1);
	GLCD_WriteCommand(DISPLAY_ON, 0);
	GLCD_WriteCommand(DISPLAY_ON, 1);
	GLCD_WriteCommand(DISPLAY_START_LINE | 0, 0);
	GLCD_WriteCommand(DISPLAY_START_LINE | 0, 1);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
