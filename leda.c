/* "led.c" - programik do testowania �rodowiska WinAVR */
/* uk�ad ATmega 1MHz */
/* PB0,PB1 - diody LED; PD0 - przycisk */

#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

#define TEST_LCD 0

#define F_CPU 4000000UL
#include "SED1520.h"               
#include "Charset.h"
#include "Time.h"
#include "Button.h"
#include "ds18x20.h"
//#include "graphic.h"
#include "TempUtil.h"
//#include "Banner.h"
#include "bufor.h"
#include "menu.h"

#define VERCOUNT 3
uint8_t ct_SubVerCount[VERCOUNT] PROGMEM = { TEMP_COUNT, 3, TEMP_BUFFCOUNT };
#define CHANGED_VER 1
#define CHANGED_SUBVER 2
uint8_t gi_Ver = 0, gb_IsChanged = CHANGED_VER;
uint8_t gt_SubVer[VERCOUNT];

#if TEST_LCD
static inline void WaitForRead(void) {
	gi_BUTPrevValue = 0;
	while (!gi_BUTPrevValue) {
		TimeSleep();
	}
	uint8_t vi_sreg = SREG;
	cli();
	gb_TempReaded = 0;
	SREG = vi_sreg;

} //WaitForRead
int main(void) {
	GLCD_Init();
	GLCD_ClearScreen();
	GLCD_WriteString("wersja 1.8");
	BUTInit();
	//BANInit();
	TimeInit();
	sei();
	TimeSleep();
	uint8_t x = 10, y = 10;
	while (1) {

		switch (gi_BUTPrevValue) {
			case BUT_LEFT:
			ADVPrintPix(0, 0, 1);
			ADVPrintPix(121, 0, 1);
			ADVPrintPix(121, 31, 1);
			ADVPrintPix(0, 31, 1);
			break;
			case BUT_RIGHT:
			x++;
			break;
			case BUT_BACK:
			y++;
			break;
			case BUT_ENTER:
			GLCD_ClearScreen();
			x = 10;y = 10;
			break;
		}
		//CSPushParams();
		ADVPrintRect(x, y, 10, 10, 1);
		//CSPopParams();
		WaitForRead();
	}
}
#else
//*********************************
//zmieniaj aktualn� wersj�
static inline uint8_t ChangeVer(void) {
	uint8_t vi_NewSubVer;
	switch (gi_BUTPrevValue) {
	//zmien wersje
	case BUT_LEFT:
	case BUT_RIGHT:
		gi_Ver += gi_BUTPrevValue == BUT_RIGHT ? 1 : -1;
		if (gi_Ver == 0xFF)
			gi_Ver = VERCOUNT - 1;
		else if (gi_Ver == VERCOUNT)
			gi_Ver = 0;
		gb_IsChanged = CHANGED_VER;
		break;
	//zmien pod wersje
	case BUT_UP:
	case BUT_DOWN:
		vi_NewSubVer = gt_SubVer[gi_Ver];
		vi_NewSubVer += (gi_BUTPrevValue == BUT_UP ? 1 : -1);
		if (vi_NewSubVer == 0xFF)
			vi_NewSubVer = pgm_read_byte(&ct_SubVerCount[gi_Ver]) - 1;
		else if (vi_NewSubVer == pgm_read_byte(&ct_SubVerCount[gi_Ver]))
			vi_NewSubVer = 0;
		gb_IsChanged = CHANGED_SUBVER;
		gt_SubVer[gi_Ver] = vi_NewSubVer;
		break;
	}
//	if (!vb_IsChanged && gb_IsChanged)
//		LCDSwitchHighlight(LCD_LEDAUTO15);
	return gb_IsChanged;
} //ChangeVer

//*********************************
//�pij z dok�adno�ci� do 1/100sec, rysuj�c w tym czasie zegar gdy trzeba i przewijaj�c baner
static inline void WaitForRead(void) {
	gi_BUTPrevValue = 0;
	while ((!gb_TempReaded) && (!gi_BUTPrevValue)) {
//		BANScroll();
		TimePrint();
		TimeSleep();
		if (gi_BUTValue==BUT_LEFT && gi_BUTSec>=2) {
			//wci�ni�ty center od ponad 2 sekund => wejd� do menu
			MENUExecute(0,0,0);
			gb_IsChanged = CHANGED_VER;
			gi_BUTPrevValue = 0;
			return;
		}
	}

	uint8_t vi_sreg = SREG;
	cli();
	gb_TempReaded = 0;
	SREG = vi_sreg;

//	BANWaitForSroll();
} //WaitForRead

//*********************************
//zainicjalizuj wszystko na ekranie
static inline void PrintFirstTime(void) {
	if (!gb_IsChanged)
		return;
//	BANOff();
	TimeClearSets();
	CSNewPage();

	uint8_t /*vi_CS1,vi_CS2,vi_X1,vi_X2,*/vi_Y1,/*vi_Y2,*/
	vi_H1/*,vi_H2,vb_Mode1,vb_Mode2*/;

	switch (gi_Ver) {
	case 0: //du�a/�rednia temperatura, ma�y/�redni czas
		TimeAddSet(TPRINT_DAY, 0, PSTR("d3"), CS_Z6p, 20, 0, ALIGN_LEFT);
		TimeAddSet(TPRINT_DAY, 1, PSTR("D2.M2"), CS_Z6p, 0, 0, ALIGN_LEFT);
		TimeAddSet(TPRINT_MIN, 0, PSTR("h1:m2"), CS_Z6p, 0, 7, ALIGN_LEFT);
		TimeAddSet(TPRINT_SEC, 0, PSTR("s2"), CS_Z6p, 20, 7, ALIGN_LEFT);
		switch (gt_SubVer[gi_Ver]) {
		case 0:
			break;
		case 1:
		default:
			break;
		}
		break;

	case 1:	//�rednia temperatura, ma�y wykres, ma�y czas, baner z dat�
//		BANSetParams(20, 11, CS_Z9p, 1, 20);
//		BANOn();
		switch (gt_SubVer[gi_Ver]) {
		case 0: /*vi_X1 = 34; */
			vi_Y1 = 3;
			vi_H1 = 19;
			break;
		case 1: /*vi_X1 = 34; */
			vi_Y1 = 2;
			vi_H1 = 10;
			break;
		case 2:
		default: /*vi_X1 = 34; */
			vi_Y1 = 8;
			vi_H1 = 14;
			break;
		}
		TimeAddSet(TPRINT_MIN, 0, PSTR("h1:m2"), CS_Z6p, 19, 33, ALIGN_CENTER);
		TimeAddSet(TPRINT_SEC, 0, PSTR("s2"), CS_Z6p, 19, 47, ALIGN_CENTER);
		TempChartInit(/*vi_X1*/66, vi_Y1, TempBuff(TEMP_BUFF24m,Size) + 2, vi_H1, 0, TEMP_BUFF24m);
		break;

	case 2: //du�y wykres
		TempChartInit((LCD_W - TempBuff(gt_SubVer[gi_Ver],Size) - 4) / 2, 0, TempBuff(gt_SubVer[gi_Ver],Size) + 4, LCD_H, 1, gt_SubVer[gi_Ver]);
		break;
	}

	gb_IsChanged = 0;

} //PrintFirstTime


//*********************************
//wy�wietlaj zmiany na zainicjalizowanym ekranie
static inline void PrintNextTime(void) {
//	BANWaitForSroll();
	int8_t vi_CS1, vi_CS2,/*vi_X1,*//*vi_X2,*//*vi_Y1,*/vi_Y2,/*vi_H1,*/vi_H2, vb_Force;
	//CSSet(CS_Z6p);CSSetXY(100, 0);CSPrintV(CSInt2V(gi_Ver,1,0));CSPrintV(CSInt2V(gt_SubVer[gi_Ver],1,0));//print ver subver
	const uint8_t ci_1l = 0, ci_2l = 6, ci_3l = 13, ci_4l = 20, ci_5l = 26;
	uint8_t vf_tMax;
	vf_tMax = BuforSprawdz();

	switch (gi_Ver) {
	case 0: //d�rednia temperatura, ma�y czas
		switch (gt_SubVer[gi_Ver]) {
		case 0:
			break;
		case 1:
		default:
			break;
		}
		CSPushParams();
		CSSet(CS_Z6p);
		CSSetNegMode(gb_BuforDzien);
		STRInit(gv_TempText, 32);STRAddV("D mx:");STRAddV(CSInt2V(gi_BuforTempMAXGrzalkiDzien, 2, 0));STRAddC('.');
		CSPrintXYu8AV(120, ci_1l, gv_TempText, ALIGN_RIGHT, 35);
		STRInit(gv_TempText, 32);STRAddV("D mn:");STRAddV(CSFloat2V(gi_BuforTempMINGrzalkiDzien, 2, 0, 0, 0));STRAddC('.');
		CSPrintXYu8AV(120, ci_2l, gv_TempText, ALIGN_RIGHT, 35);


		STRInit(gv_TempText, 32);STRAddV("D ");STRAddV(CSInt2V(gi_BuforDzienStartH, 2, 0));STRAddC('-');STRAddV(CSInt2V(gi_BuforDzienEndH, 2, 0));STRAddC('.');
		CSPrintXYu8AV(120, ci_3l, gv_TempText, ALIGN_RIGHT, 35);


		CSSetNegMode(!gb_BuforDzien);
		STRInit(gv_TempText, 32);STRAddV("N mx:");STRAddV(CSFloat2V(gi_BuforTempMAXGrzalkiNoc, 2, 0, 0, 0));STRAddC('.');
		CSPrintXYu8AV(120, ci_4l, gv_TempText, ALIGN_RIGHT, 35);
		STRInit(gv_TempText, 32);STRAddV("N mn:");STRAddV(CSFloat2V(gi_BuforTempMINGrzalkiNoc, 2, 0, 0, 0));STRAddC('.');
		CSPrintXYu8AV(120, ci_5l, gv_TempText, ALIGN_RIGHT, 35);
		CSPopParams();

		CSPushParams();
		CSSet(CS_C13p);
		CSPrintXYu8AV(LCD_W/2, 0, TempGetV(0,/*TEMP_CURR*/TEMP_TICK,0), ALIGN_CENTER, 0);		//g�ra bufora
		CSSet(CS_C8p);
		CSPrintXYu8AV(LCD_W/2, 15, TempGetV(1,/*TEMP_CURR*/TEMP_TICK,0), ALIGN_CENTER, 0);		//srodek bufora
		CSPrintXYu8AV(LCD_W/2, 23, TempGetV(2,/*TEMP_CURR*/TEMP_TICK,0), ALIGN_CENTER, 0);		//d� bufora

		CSSet(CS_Z6p);
		if(gb_BuforStatus == BUFOR_GRZEJE){
			STRInit(gv_TempText, 32);STRAddV("GRZEJ DO:");STRAddV(CSFloat2V(vf_tMax,2,0,0,0));
			CSPrintXYu8AV(0, ci_3l,gv_TempText , ALIGN_LEFT, 0);
		}else{
			CSPrintXYu8AV(0, ci_3l, "-----------", ALIGN_LEFT, 0);
		}
		CSPrintXYu8AV(0, ci_4l, "Pod�:", ALIGN_LEFT, 0);
		STRInit(gv_TempText, 32);STRAddV(CSFloat2V(TempGetF(3,TEMP_TICK),2,1,1,0));STRAddV("->");STRAddV(CSFloat2V(TempGetF(4,TEMP_TICK),2,1,1,0));
		CSPrintXYu8AV(0, ci_5l, gv_TempText, ALIGN_LEFT, 0);

		//CSSetXYu8(0, 15);

		//CSPrintV(CSFloat2V(TempGetF(1,TEMP_TICK),3,1,0,1));
		//CSPrintV(TempGetV(1,TEMP_TICK,0));
		//CSPrintV("0123456789");
		CSPopParams();
//		BANWaitForSroll();
//		STRInit(gv_BANText, BANSIZE);
//		STRAddV(TempGetV(TEMP_FULLNO,TEMP_MIN,vb_Mode2));
//		STRAddC('\n');
//		STRAddV(TempGetV(TEMP_FULLNO,TEMP_AVG,vb_Mode2));
//		STRAddC('\n');
//		STRAddV(TempGetV(TEMP_FULLNO,TEMP_MAX,vb_Mode2));
//		CSPushParams();
//		CSSet(vi_CS2);
//		CSSetNegMode(gt_SubVer[gi_Ver]);
//		CSPrintXYu8AV(LCD_W, vi_Y2, TempGetV(TEMP_FULLNO,TEMP_MIN,vb_Mode2), ALIGN_RIGHT, 0);
//		CSSetNegMode(!gt_SubVer[gi_Ver]);
//		CSPrintXYu8AV(LCD_W, vi_Y2+7, TempGetV(TEMP_FULLNO,TEMP_MIN,vb_Mode2), ALIGN_RIGHT, 0);
//		CSPopParams();
//		CSSet(CS_Z6p);
		//CSPrintXYu8AV(LCD_W, 58, TempGetV(0,TEMP_CURR/*TEMP_TICK*/,0)+1, ALIGN_RIGHT, 14);
///*USUN*/			CSSet(CS_Z6p); CSPrintXYu8AV(128,51,TempGetV(1,TEMP_TICK,0),ALIGN_RIGHT,18);
		break;

	case 1: //�rednia temperatura, ma�y wykres, baner z dat�
		vb_Force = TempChartPrint();
//		BANWaitForSroll();
		switch (gt_SubVer[gi_Ver]) {
		case 0:
			vi_CS1 = CS_C13p;
			vi_CS2 = CS_Z6p; /*vi_X2 = 19;*/
			vi_Y2 = 22;
			vi_H2 = 11;
			break;
		case 1:
			vi_CS1 = CS_C13p;
			vi_CS2 = CS_Z6p; /*vi_X2 = 19;*/
			vi_Y2 = 9;
			vi_H2 = 7;
			break;
		case 2:
		default:
			vi_CS1 = CS_Z6p;
			vi_CS2 = CS_Z6p; /*vi_X2 = 19;*/
			vi_Y2 = 19;
			vi_H2 = 13;
			break;
		}
		CSSet(vi_CS1);
		CSPrintXYAV(LCD_W/2, 0, TempGetV(TEMP_FULLNO,TEMP_CURR/*TEMP_TICK*/,1), ALIGN_CENTER, 0);
//		BANWaitForSroll();
		if (vb_Force) {
			CSSet(vi_CS2);
			int16_t vt_Temps[TEMP_MAX + 1];
			TempBuffGet(TEMP_BUFF24m, TEMP_FULLNO, 0, TempBuff(TEMP_BUFF24m,Size) - 1, vt_Temps, 0);
			for (uint8_t vi_Idx = 0; vi_Idx < 3; vi_Idx++)
				CSPrintXYu8AV(/*vi_X2*/65, vi_Y2+vi_H2*vi_Idx, TempConvert(vt_Temps[TEMP_MAX-vi_Idx],0), ALIGN_RIGHT, 20);
//				CSPrintXYu8AV(vi_X2,vi_Y2,TempGetV(TEMP_FULLNO,TEMP_MAX,1),ALIGN_RIGHT,vi_X2);
//				CSPrintXYu8AV(vi_X2,vi_Y2+vi_H2,TempGetV(TEMP_FULLNO,TEMP_AVG,1),ALIGN_RIGHT,vi_X2);
//				CSPrintXYu8AV(vi_X2,vi_Y2+2*vi_H2,TempGetV(TEMP_FULLNO,TEMP_MIN,1),ALIGN_RIGHT,vi_X2);
		}
//		STRInit(gv_BANText, BANSIZE);
		for (uint8_t vi_Sensor = 0; vi_Sensor < TEMP_COUNT; vi_Sensor++)
			if (vi_Sensor != TEMP_FULLNO) {
				STRAddV_p(ct_TempsData[vi_Sensor].vv_Name);
				STRAddC(':');
				STRAddV(TempGetV(vi_Sensor,TEMP_CURR/*TEMP_TICK*/,1));
				STRAddC(' ');
			}
		STRAddV(TimeGetV(PSTR("d3 D1 M3 Y4 "),gv_TempText));
		break;

	case 2: //du�y wykres
		TempChartExecute();
		break;
	}

} //PrintNextTime
//void debug(int i, int delay) {
//	DDRD = 0xFF;
//	PORTD = 0x00; // clear
//
//	PORTD = 0xFF & _BV(i);
//	_delay_ms(delay);
//	PORTD = 0x00; // clear
//
//}

//void mierzTemp() {
//	DS18Detect();
//	_delay_ms(1000);
//	DS18StartMeas();
//	float vn_Tick = round((DS18ReadMeas(0)) / 1.6) / 10.0; // domyslnie wskazuje ile razy 0,0625
//	GLCD_GoTo(10, 10);
//	//GLCD_WriteString(CSFloat2V(vn_Tick,3,1,1,1));
//
//}
//void fillScreen() {
//	for (int i = 0; i < 32; i++) {
//		for (int j = 0; j < 122; j++) {
//			GLCD_SetPixel(j, i, 2);
//			_delay_ms(5);
//		}
//	}
//}

int main(void) {
	LCDInit();
	CSNewPage();
	TimeInit();
	sei();
	TempInit();
	BUTInit();
//	BANInit();
	TimeDelayHthSec(100);
	//GLCD_Init();
	GLCD_ClearScreen();
	CSSet(CS_Z6p);
	CSSetXY(0, 0);
	DS18Detect();
	//_delay_ms(3000);
	WaitForRead();
	BuforInit();
	TimeSleep();
	while (1) {
		if (ChangeVer())
			PrintFirstTime();
		PrintNextTime();
		WaitForRead();
	}
}
//		DS18StartMeas();
//		float vn_Tick = round((DS18ReadMeas(0)) / 1.6) / 10.0; // domyslnie wskazuje ile razy 0,0625
//		GLCD_GoTo(10, 10);
//		GLCD_WriteString(CSFloat2V(vn_Tick, 3, 1, 1, 1));
//		_delay_ms(500);
//		BUTWaitForPress(1);
//		GLCD_ClearScreen();
//		GLCD_GoTo(0, 0);
//		//GLCD_WriteString(CSInt2V(gi_BUTPrevValue, 1, 0));
//		TimePrint();
//		if (gi_BUTPrevValue == 8) {
//			TimeAddSet(TPRINT_DAY,1,PSTR("h1:m2"),0,0,56,0);
//			_delay_ms(500);
//		} else {
//			_delay_ms(500);
//		}
//	}
#endif

