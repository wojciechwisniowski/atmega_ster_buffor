// jm12232.h
//
//
#include "font.h"



#define DISPLAY_ON 0xAF
#define DISPLAY_OFF 0xAE
#define DISPLAY_START_LINE 0xC0
#define PAGE_ADDRESS_SET 0xB8
#define COLUMN_ADDRESS_SET 0x00
#define ADC_CLOCKWISE 0xA0
#define ADC_COUNTERCLOCKWISE 0xA1
#define STATIC_DRIVE_ON 0xA5
#define STATIC_DRIVE_OFF 0xA4
#define DUTY_RATIO_16 0xA8
#define DUTY_RATIO_32 0xA9
#define READ_MODIFY_WRITE 0xE0
#define END_READ_MODIFY 0xEE
#define RESET 0xE2


#define SCREEN_WIDTH	122
#define SCREEN_HEIGHT	32

void GLCD_WaitForStatus(unsigned char,unsigned char);
void GLCD_WriteCommand(unsigned char,unsigned char);
void GLCD_GoTo(unsigned char,unsigned char);
void GLCD_WriteData(unsigned char);
unsigned char GLCD_ReadData(void);
void GLCD_ClearScreen(void);
void GLCD_WriteChar(char);
void GLCD_WriteString(char *);
void GLCD_SetPixel(unsigned char, unsigned char, unsigned char);
void GLCD_Init(void);
void GLCD_Bitmap(char * , unsigned char, unsigned char, unsigned char, unsigned char);
