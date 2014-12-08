#ifndef BASICLCD_H
#define BASICLCD_H 1

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//wielko�� wy�wietlacza LCD
#define LCD_W 122
#define LCD_H 32
#define LCD_H8 (LCD_H/8)

#define LCD_CODESIZE 0 //0:najmniejszy/najwolniejszy 1:�redni 2:najwi�kszy/najszybszy

/*#ifndef __OPTIMIZE__
	//tryb bez optymalizacji dla debugera - wrzu� najmniejszy kod
	#define LCDCODESIZE 0
#else
	#ifdef __OPTIMIZE_SIZE__
		//tryb z optymalizacj� rozmiaru kodu - wrzu� najmniejszy kod
		#define LCDCODESIZE 0
	#endif
#endif
#ifndef LCDCODESIZE
	#define LCDCODESIZE 2 //0:najwolniejsze/najmniejsze  1:�rednie  2:najszybsze/najwi�ksze
#endif*/

uint8_t gi_LCDLedHigh, gi_LCDLedLow; //maksymalne,minimalne pod�wietlenie
volatile uint8_t gi_LCDLed; //bie��ce pod�wietlenie
volatile uint8_t gi_LCDLedTime; //czas (10/sec) pozosta�y do rozpocz�cia �ciemniania 1:�ciemnianie 0:�ciemnione


uint8_t gb_LCDGlobalNegMode; //do negowania ca�o�ci wy�wietlania
uint8_t gb_LCDNegMode; //czy zanegowa� bie��ce wy�wietlanie

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//zresetuj wy�wietlacz po starcie
void LCDInit(void);

//wy�lij dane do wy�wietlacza
void LCDWriteData(uint8_t pi_Data);

//odczytaj tekst z wy�wietlacza
void LCDReadRow(
  uint8_t *pt_Data
 ,uint8_t  pi_Bytes
);

//wype�nij lub wyczy�� wiersz
void LCDFillClrRow(
  uint8_t pb_Fill
 ,uint8_t pi_Bytes
 ,uint8_t pi_Mask
);

//wy�lij lini� na wy�wietlacz z pami�ci programu
void LCDWriteRow_p(
  uint8_t *pv_Data1       //adres do pami�ci programu linii Y-1
 ,uint8_t *pv_Data2       //adres do pami�ci programu linii Y
 ,uint8_t  pi_MoveBits    //ile przesun�� bity dla pv_Text2 (np: pv_Text1+2:10010111 01010101 pi_MoveBits:1 out:1001011(1 0101010)1
 ,uint8_t  pi_Bytes       //ilo�� bajt�w do wys�ania
 ,uint8_t  pi_SpaceXBefore//ile odst�pu doda� (wyczy�ci�) przed wydrukiem
 ,uint8_t  pi_SpaceXAfter //ile odst�pu doda� (wyczy�ci�) po wydruku
 ,uint8_t  pi_Mask        //z jak� mask� wysy�a�
);

//ustaw wsp�rz�dne x,y na wy�wietlaczu
void LCDSetXY8(
  int16_t pi_X
 ,int8_t  pi_Y8
);

////ustaw jasno�� pod�wietlania LED
//void LCDSetHighlight(
//  uint8_t pi_LedLow  //0 - warto�� ignorowana
// ,uint8_t pi_LedHigh //0 - warto�� ignorowana
//);
//
////zmie� jasno�� pod�wietlania LED: : procedura do wywo�ania w menu
//void LCDChangeHighlight(
//  uint8_t pi_Y
// ,uint8_t pi_What
// ,int8_t  pi_Value
//);
//
//void LCDSwitchHighlight(uint8_t pb_Mode);
//
////void LCDCallInterrupt(void);

#endif
