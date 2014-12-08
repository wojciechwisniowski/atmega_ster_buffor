#include "Banner.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "BasicLCD.h"
#include "AdvancedLCD.h"
#include "Charset.h"
#include "Time.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

int16_t gi_BANCurrX;

//==============================================================
// PUBLIC PROCEDURES
//==============================================================


//*********************************
//w��cz wy�wietlanie banera
void BANOn(void) {
	CSPushParams();
	CSSetNegMode(gb_BANNegMode);
	ADVPrintRect(0,gi_BANY-1,LCD_W,gi_BANH,0);
	CSPopParams();
	gi_BANCurrX = 0;
	gi_BANHthSecCounter = 0;
	BANScroll();
	gb_BANOn = 1;
} //BANOn


//*********************************
//przewi� baner (z kontrol� czy ju� nadszed� czas, oraz czy nie przewin�� wi�cej ni� 1px)
uint8_t BANScroll(void) {
	
	if ((!gb_BANOn) || (!gv_BANText[0]))
		return gi_BANHthSecCounter%2;

	if (gi_BANHthSecCounter<gi_BANHthSec)
		return 0;
		
	uint8_t vi_sreg = SREG;
	cli();
	while (gi_BANHthSecCounter>=gi_BANHthSec) {
		gi_BANCurrX--;
		gi_BANHthSecCounter -= gi_BANHthSec;
	}
	SREG = vi_sreg;

	CSPushParams();
	CSSetNewLine(0);
	CSSetNegMode(gb_BANNegMode);
	CSSetAddSpacePx(1);
	CSSet(gi_BANCS);
	CSSetXY(gi_BANCurrX,gi_BANY);
	
	//drukuj przewini�te na ekran
	while(gt_CSXY[CSPCurrX]<LCD_W) {
		CSPrintV(gv_BANText);
		gt_CSParams[CSPPrevCharXSpace]=0;
		if (gt_CSXY[CSPCurrX]<=0)
			gi_BANCurrX = gt_CSXY[CSPCurrX];
	}
	
	CSPopParams();
	
	return 1;
	
} //BANScroll


//*********************************
//czekaj na przewini�cie banera (�eby po wyj�ciu mie� du�o czasu do nast�pnego przewijania => zabezpieczenie przed przycinaniem banera)
void BANWaitForSroll(void) {
	while (BANScroll()) TimeSleep(); //czekaj a� przestanie drukowa�
	while (!BANScroll()) TimeSleep(); //czekaj a� nast�pi wydruk
} //BANWaitForSroll