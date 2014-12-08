#ifndef BASICLCD_H
#define BASICLCD_H 1

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//wielkoœæ wyœwietlacza LCD
#define LCD_W 122
#define LCD_H 32
#define LCD_H8 (LCD_H/8)

#define LCD_CODESIZE 0 //0:najmniejszy/najwolniejszy 1:œredni 2:najwiêkszy/najszybszy

/*#ifndef __OPTIMIZE__
	//tryb bez optymalizacji dla debugera - wrzuæ najmniejszy kod
	#define LCDCODESIZE 0
#else
	#ifdef __OPTIMIZE_SIZE__
		//tryb z optymalizacj¹ rozmiaru kodu - wrzuæ najmniejszy kod
		#define LCDCODESIZE 0
	#endif
#endif
#ifndef LCDCODESIZE
	#define LCDCODESIZE 2 //0:najwolniejsze/najmniejsze  1:œrednie  2:najszybsze/najwiêksze
#endif*/

uint8_t gi_LCDLedHigh, gi_LCDLedLow; //maksymalne,minimalne podœwietlenie
volatile uint8_t gi_LCDLed; //bie¿¹ce podœwietlenie
volatile uint8_t gi_LCDLedTime; //czas (10/sec) pozosta³y do rozpoczêcia œciemniania 1:œciemnianie 0:œciemnione


uint8_t gb_LCDGlobalNegMode; //do negowania ca³oœci wyœwietlania
uint8_t gb_LCDNegMode; //czy zanegowaæ bie¿¹ce wyœwietlanie

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//zresetuj wyœwietlacz po starcie
void LCDInit(void);

//wyœlij dane do wyœwietlacza
void LCDWriteData(uint8_t pi_Data);

//odczytaj tekst z wyœwietlacza
void LCDReadRow(
  uint8_t *pt_Data
 ,uint8_t  pi_Bytes
);

//wype³nij lub wyczyœæ wiersz
void LCDFillClrRow(
  uint8_t pb_Fill
 ,uint8_t pi_Bytes
 ,uint8_t pi_Mask
);

//wyœlij liniê na wyœwietlacz z pamiêci programu
void LCDWriteRow_p(
  uint8_t *pv_Data1       //adres do pamiêci programu linii Y-1
 ,uint8_t *pv_Data2       //adres do pamiêci programu linii Y
 ,uint8_t  pi_MoveBits    //ile przesun¹æ bity dla pv_Text2 (np: pv_Text1+2:10010111 01010101 pi_MoveBits:1 out:1001011(1 0101010)1
 ,uint8_t  pi_Bytes       //iloœæ bajtów do wys³ania
 ,uint8_t  pi_SpaceXBefore//ile odstêpu dodaæ (wyczyœciæ) przed wydrukiem
 ,uint8_t  pi_SpaceXAfter //ile odstêpu dodaæ (wyczyœciæ) po wydruku
 ,uint8_t  pi_Mask        //z jak¹ mask¹ wysy³aæ
);

//ustaw wspó³rzêdne x,y na wyœwietlaczu
void LCDSetXY8(
  int16_t pi_X
 ,int8_t  pi_Y8
);

////ustaw jasnoœæ podœwietlania LED
//void LCDSetHighlight(
//  uint8_t pi_LedLow  //0 - wartoœæ ignorowana
// ,uint8_t pi_LedHigh //0 - wartoœæ ignorowana
//);
//
////zmieñ jasnoœæ podœwietlania LED: : procedura do wywo³ania w menu
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
