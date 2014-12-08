#include "Menu.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "Charset.h"
#include "Time.h"
#include "BasicLCD.h"
#include "Button.h"
#include "bufor.h"


//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//**************
char gv_DataICzas[] PROGMEM = "Data i czas";
char gv_ZmienDate[] PROGMEM = "Zmieñ datê";
char gv_ZmienCzas[] PROGMEM = "Zmieñ czas";
char gv_CzasLetniZimowy[] PROGMEM = "Czas letni-zimowy";
char gv_KorygujPredkosc[] PROGMEM = "Koryguj prêdkoœæ";

char gv_Rok[] PROGMEM = "Rok";
char gv_Miesiac[] PROGMEM = "Miesi¹c";
char gv_Dzien[] PROGMEM = "Dzieñ";
char gv_Godzina[] PROGMEM = "Godzina";
char gv_Minuta[] PROGMEM = "Minuta";
char gv_Sekunda[] PROGMEM = "Sekunda";
char gv_SetnaSekundy[] PROGMEM = "Setna sekundy";
char gv_SekundaZmien[] PROGMEM = "Sekunda: zmieñ";
char gv_SekundaZeruj[] PROGMEM = "Sekunda: zeruj";

char gv_JasnoscLCD[] PROGMEM = "Jasnoœæ LCD";
char gv_Minimalna[] PROGMEM = "Minimalna";
char gv_Maksymalna[] PROGMEM = "Maksymalna";
char gv_Automatyczna[] PROGMEM = "Automatyczna";

char gv_Bufor[] PROGMEM = "Zmien temperatury grzalek";
char gv_MinD[] PROGMEM = "Min Dzinna";
char gv_MaxD[] PROGMEM = "Max Dzienna";
char gv_MinN[] PROGMEM = "Min Nocna";
char gv_MaxN[] PROGMEM = "Max Nocna";

char gv_Strefe[] PROGMEM = "Zmieñ godziny strefy dnia";
char gv_DStart[] PROGMEM = "Godzina rozpoczêcia dnia";
char gv_DEnd[] PROGMEM = "Ostatnia godzina dnia";

MENURow gt_MENUZmienDate [] PROGMEM = {
	{gv_Rok,0,0,&TimeChange,TYear},
	{gv_Miesiac,0,0,&TimeChange,TMonth},
	{gv_Dzien,0,0,&TimeChange,TDay},
	{0,0,0,0,0}
};

MENURow gt_MENUZmienCzas [] PROGMEM = {
	{gv_Godzina,0,0,&TimeChange,THour},
	{gv_Minuta,0,0,&TimeChange,TMin},
	{gv_SekundaZmien,0,0,&TimeChange,TSec},
	{gv_SekundaZeruj,0,0,&TimeChange,TClearSec},
	{0,0,0,0,0}
};

MENURow gt_MENUZmienBufor [] PROGMEM = {
	{gv_MinD,0,0,&BuforChange,MIND},
	{gv_MaxD,0,0,&BuforChange,MAXD},
	{gv_MinN,0,0,&BuforChange,MINN},
	{gv_MaxN,0,0,&BuforChange,MAXN},
	{0,0,0,0,0}
};

MENURow gt_MENUZmienStrefe [] PROGMEM = {
	{gv_DStart,0,0,&BuforChange,DSTART},
	{gv_DEnd,0,0,&BuforChange,DEND},
	{0,0,0,0,0}
};

MENURow gt_MENUKorygujPredkosc [] PROGMEM = {
//	{gv_Godzina,0,0,&TimeChangeCor,THour},
//	{gv_Minuta,0,0,&TimeChangeCor,TMin},
	{gv_Sekunda,0,0,&TimeChangeCor,TSec},
	{gv_SetnaSekundy,0,0,&TimeChangeCor,THthSec},
	{0,0,0,0,0}
};

MENURow gt_MENUDataICzas [] PROGMEM = {
	{gv_ZmienDate,gt_MENUZmienDate,0,0,0},
	{gv_ZmienCzas,gt_MENUZmienCzas,0,0,0},
	{gv_CzasLetniZimowy,0,0,&TimeChangeDaylightSavingTime,0},
	{gv_KorygujPredkosc,gt_MENUKorygujPredkosc,0,0,0},
	{0,0,0,0,0}
};

//MENURow gt_MENUJasnoscLCD [] PROGMEM = {
//	{gv_Minimalna,0,0,LCDChangeHighlight,0},
//	{gv_Maksymalna,0,0,LCDChangeHighlight,1},
//	{gv_Automatyczna,0,0,LCDChangeHighlight,2},
//	{0,0,0,0,0}
//};

MENURow gt_MENUMain[] PROGMEM = {
	{gv_DataICzas,gt_MENUDataICzas,0,0,0},
	{gv_Bufor,gt_MENUZmienBufor,0,0,0},
	{gv_Strefe,gt_MENUZmienStrefe,0,0,0},
	{0,0,0,0,0}
};


//==============================================================
// FORWARD DECLARATIONS
//==============================================================

//drukuj pojedynczy wiersz menu
void MENUPrintRow(MENURow *pr_Menu,uint8_t pi_Y,uint8_t pi_Level,uint8_t pb_Selected);
//drukuj wszystkie wiersze danego menu
void MENUPrint(MENURow *pt_Menu,uint8_t pi_Y,uint8_t pi_Level,uint8_t pi_SelectedIdx);
//uruchamiaj procedurê zapisan¹ we wierszu menu
void MENUExecuteProc(void (*pp_Procedure)(uint8_t,uint8_t,int8_t),uint8_t pi_Y,uint8_t pi_Param) __attribute__((always_inline));
#if MENU_WITH_MATRIX
//uruchamiaj modyfikacje tablicy
void MENUExecuteMatrix(MENUMatrix *px_Matrix,uint8_t pi_Y) __attribute__((always_inline));
#endif


//==============================================================
// PUBLIC PROCEDURES
//==============================================================


//*********************************
//uruchamiaj menu - dla wejœcia z zwen¹trz wywo³aj MENUExecute(0,0,0)
void MENUExecute(
  MENURow *pt_Menu //tablica z menu
 ,uint8_t pi_Y     //na jakiej pozycji
 ,uint8_t pi_Level //jaki poziom menu
) {
	BUTSetDefaultAutoRepeat();

	if (!pt_Menu)
		pt_Menu = gt_MENUMain;

	int8_t  vi_SelectedIdx;
	uint8_t vi_MaxIdx;
	void   *vx_Addr;
	uint8_t vi_BUTValue;

	vi_SelectedIdx = 0;
	vi_MaxIdx = 0;
	while (pgm_read_word(&pt_Menu[vi_MaxIdx].vv_Text)) vi_MaxIdx++;

	MENUPrint(pt_Menu,pi_Y,pi_Level,vi_SelectedIdx);

//	LCDSwitchHighlight(LCD_LEDHIGH);
	BUTWaitForUnpress();

	while(1) {
		//czekaj na wciœniêcie przycisku lub automatyczny repeat
//		LCDSwitchHighlight(LCD_LEDHIGH);
		vi_BUTValue = BUTWaitForPress(MENU_INACTIVE_MENU_EXIT_SEC);
		switch(vi_BUTValue) {
			case BUT_RIGHT: //case BUT_CENTER:
				//wejdŸ do podrzêdnego menu lub do obs³ugi procedury na najni¿szym poziomie lub do obs³gi tablicy
				ADVPrintRect(0,pi_Y,LCD_W,LCD_H-pi_Y,0);
				MENUPrintRow(&pt_Menu[vi_SelectedIdx],pi_Y,pi_Level,0);
				if ((vx_Addr=(void *)pgm_read_word(&pt_Menu[vi_SelectedIdx].vt_SubMenu)))
					MENUExecute((MENURow *)vx_Addr,pi_Y/*+MENU_HEIGHT*/,pi_Level+1);
#if MENU_WITH_MATRIX
				else if ((vx_Addr=(void *)pgm_read_word(&pt_Menu[vi_SelectedIdx].vx_Matrix)))
					MENUExecuteMatrix((MENUMatrix *)vx_Addr,pi_Y+MENU_HEIGHT+1);
#endif
				else if ((vx_Addr=(void *)pgm_read_word(&pt_Menu[vi_SelectedIdx].vp_Procedure)))
					MENUExecuteProc((void (*))vx_Addr,pi_Y/*+MENU_HEIGHT+5*/,pgm_read_byte(&pt_Menu[vi_SelectedIdx].vi_Param));
				MENUPrint(pt_Menu,pi_Y,pi_Level,vi_SelectedIdx);
				break;

			case BUT_UP: case BUT_DOWN:
				//przewijaj belkê po aktualnym menu
				MENUPrintRow(&pt_Menu[vi_SelectedIdx],pi_Y+vi_SelectedIdx*MENU_HEIGHT,pi_Level,0);
				vi_SelectedIdx += vi_BUTValue==BUT_UP?-1:1;
				if (vi_SelectedIdx<0) vi_SelectedIdx = vi_MaxIdx-1;
				if (vi_SelectedIdx==vi_MaxIdx) vi_SelectedIdx = 0;
				MENUPrintRow(&pt_Menu[vi_SelectedIdx],pi_Y+vi_SelectedIdx*MENU_HEIGHT,pi_Level,1);
				break;

default: case BUT_LEFT: case 0:
				//wciœniêty lewy lub nikt nic nie wciska => wychodŸ
				BUTWaitForUnpress();
//				LCDSwitchHighlight(vi_BUTValue?LCD_LEDAUTO15:LCD_LEDAUTO1);
				return;
		}

	}
} //MENUExecute

//==============================================================
// PRIVATE PROCEDURES
//==============================================================

//*********************************
//drukuj pojedynczy wiersz menu
void MENUPrintRow(
  MENURow *pr_Menu    //rekord menu
 ,uint8_t pi_Y        //na jakiej pozycji
 ,uint8_t pi_Level    //jaki poziom menu
 ,uint8_t pb_Selected //czy pozycja wybrana
) {
	CSSet(MENU_CS);
	CSSetXYu8(pi_Level*6,pi_Y);
	CSSetNegMode(pb_Selected);
	CSPrintC(Sp2px);
	CSPrintV_p((char *)pgm_read_word(&pr_Menu->vv_Text));
	CSPrintC(Sp2px);
	CSSetNegMode(0);
} //MENUPrintRow


//*********************************
//drukuj wszystkie wiersze danego menu
void MENUPrint(
  MENURow *pt_Menu       //tablica z menu
 ,uint8_t pi_Y           //na jakiej pozycji
 ,uint8_t pi_Level       //jaki poziom menu
 ,uint8_t pi_SelectedIdx //numer wybranego wiersza
) {
	uint8_t vi_Idx=0;

	ADVPrintRect(0,pi_Y,LCD_W,LCD_H-pi_Y,0);

	while (pgm_read_word(&pt_Menu->vv_Text)) {
		MENUPrintRow(pt_Menu,pi_Y+vi_Idx*MENU_HEIGHT,pi_Level,pi_SelectedIdx==vi_Idx?1:0);
		pt_Menu++;
		vi_Idx++;
	}
}	//MENUPrint


//*********************************
//uruchamiaj procedurê zapisan¹ we wierszu menu
void MENUExecuteProc(
  void  (*pp_Procedure)(uint8_t,uint8_t,int8_t)
 ,uint8_t pi_Y
 ,uint8_t pi_Param
) {
	uint8_t vi_BUTRepeatCount;

	BUTWaitForUnpress();

	while(gi_BUTValue || gi_BUTSec<MENU_INACTIVE_PROC_EXIT_SEC) {
		uint8_t vi_BUTHthSec=gi_BUTHthSec;
		do {TimeSleep();} while (!gi_BUTValue && vi_BUTHthSec+2>gi_BUTHthSec);
//		for (uint8_t vi_Idx=0; vi_Idx<10; vi_Idx++) sleep_mode_noinline();
		uint8_t vi_sreg = SREG;
		cli();
		vi_BUTRepeatCount = gi_BUTRepeatCount;
		gi_BUTRepeatCount = 0;
		SREG = vi_sreg;
		switch(gi_BUTValue) {
			case BUT_UP:
				pp_Procedure(pi_Y,pi_Param,+vi_BUTRepeatCount);
				break;
			case BUT_DOWN:
				pp_Procedure(pi_Y,pi_Param,-vi_BUTRepeatCount);
				break;
			case BUT_LEFT: //case BUT_CENTER:
				BUTWaitForUnpress();
				return;
			default:
				pp_Procedure(pi_Y,pi_Param,0);
				break;
		}
	}

} //MENUExecProc


#if MENU_WITH_MATRIX
#define MatrixByte(pv_Field) (pgm_read_byte(&px_Matrix->pv_Field))
#define MatrixWord(pv_Field) (pgm_read_word(&px_Matrix->pv_Field))
#define MatrixCols ((MENUMatrixCol *)MatrixWord(vx_MatrixCols))
#define MatrixColByte(pi_ColNo,pv_Field) (pgm_read_byte(&MatrixCols[pi_ColNo].pv_Field))
#define MatrixColWord(pi_ColNo,pv_Field) (pgm_read_word(&MatrixCols[pi_ColNo].pv_Field))
//#define MatrixGetValue(px_Matrix,pi_Row,pi_Col) (MatrixWord(vx_Values)?((uint8_t *)MatrixWord(vx_Values))[pi_Row*MatrixByte(vi_Cols)+pi_Col]:eeprom_read_byte((uint8_t *)MatrixWord(vx_EEValues)+pi_Row*MatrixByte(vi_Cols)+pi_Col))
uint8_t MatrixGetValue(MENUMatrix *px_Matrix, uint8_t pi_Row, uint8_t pi_Col) {
	return MatrixWord(vx_Values)?
	       ((uint8_t *)MatrixWord(vx_Values))[pi_Row*MatrixByte(vi_Cols)+pi_Col]:
			 eeprom_read_byte((uint8_t *)MatrixWord(vx_EEValues)+pi_Row*MatrixByte(vi_Cols)+pi_Col);
}
#define MatrixSetValue(pi_Row,pi_Col,pi_Value) {\
	if (MatrixWord(vx_Values))\
		((uint8_t *)MatrixWord(vx_Values))[pi_Row*MatrixByte(vi_Cols)+pi_Col] = pi_Value;\
	if (MatrixWord(vx_EEValues))\
		eeprom_write_byte((uint8_t *)MatrixWord(vx_EEValues)+pi_Row*MatrixByte(vi_Cols)+pi_Col,pi_Value);\
}
//*********************************
//drukuj pojedynczy element tablicy
void MENUPrintMatrixEl(
  MENUMatrix *px_Matrix
 ,uint8_t pi_Y
 ,uint8_t pi_Row
 ,uint8_t pi_Col
 ,uint8_t pi_Selected
) {
	uint8_t vi_Idx,vi_MaxRows,vi_X,vi_Value,vi_Align;
	char vv_Text[40];

	vi_Value = MatrixGetValue(px_Matrix,pi_Row,pi_Col);

	STRInit(vv_Text,40);
	if (pi_Selected) STRAddC(pi_Selected==1?ULOn:NEGOn);
	if (vi_Value<100 && MatrixColByte(pi_Col,vi_Digits)>=3 && MatrixColByte(pi_Col,vb_With0))
		STRAddC('0');
	if (vi_Value<10 && MatrixColByte(pi_Col,vi_Digits)>=2 && MatrixColByte(pi_Col,vb_With0))
		STRAddC('0');
	STRAddV(CSInt2V(vi_Value,0,0));
	STRAddV_p(PSTR(ULOffS NEGOffS));
	if (MatrixColWord(pi_Col,vv_TextAfter)) STRAddV_p((char *)MatrixColWord(pi_Col,vv_TextAfter));
	CSSet(MatrixByte(vi_CS));

	vi_MaxRows = ((LCD_H-pi_Y)/MatrixByte(vi_RowHeight));
	if (MatrixByte(vi_Cols)+MatrixByte(vi_Rows)>1) {
		vi_X = 0;
		for (vi_Idx = 0; vi_Idx<=pi_Col; vi_Idx++)
			vi_X += MatrixColByte(vi_Idx,vi_Width);
		vi_X += MatrixByte(vi_Width)*(pi_Row/vi_MaxRows);
		vi_Align = ALIGN_RIGHT;
	}
	else {
		vi_X = LCD_W/2;
		vi_Align = ALIGN_CENTER;
	}
	pi_Y += MatrixByte(vi_RowHeight)*(pi_Row%vi_MaxRows);
	if (MatrixColWord(pi_Col,vv_TextBefore))
		CSPrintXYu8AV_p(vi_X-MatrixColByte(pi_Col,vi_Width),pi_Y,(char *)MatrixColWord(pi_Col,vv_TextBefore),ALIGN_LEFT,0);
	CSPrintXYu8AV(vi_X,pi_Y,vv_Text,vi_Align,MatrixColByte(pi_Col,vi_Width));

} //MENUPrintMatrixEl


//*********************************
//uruchamiaj modyfikacjê tablicy
void MENUExecuteMatrix(MENUMatrix *px_Matrix, uint8_t pi_Y) {
	uint8_t vb_Exit;
	uint8_t vi_Row,vi_Col,vi_Selected,vi_Value;
	uint8_t vi_BUTValue;

	#if MENU_MATRIX_ONEVALUE
		uint8_t vb_OneValue;
		vb_OneValue = (MatrixByte(vi_Cols)+MatrixByte(vi_Rows))==1;
	#else
		#define vb_OneValue 0
	#endif

	vb_Exit = 0;

	//drukuj ca³oœæ
	for (vi_Col=0; vi_Col<MatrixByte(vi_Cols); vi_Col++)
		for (vi_Row=0; vi_Row<MatrixByte(vi_Rows); vi_Row++)
			MENUPrintMatrixEl(px_Matrix,pi_Y,vi_Row,vi_Col,(vi_Col+vi_Row)?0:1);

	vi_Row = 0;
	vi_Col = 0;
	vi_Selected = vb_OneValue;
	BUTWaitForUnpress();

	while(!vb_Exit && (gi_BUTValue || gi_BUTSec<MENU_INACTIVE_PROC_EXIT_SEC)) {

		if (vi_Selected) {
			gi_BUTAutoRepeatPerSec = MatrixColByte(vi_Col,vi_MaxValue)/5;
			if (gi_BUTAutoRepeatPerSec<10)
				gi_BUTAutoRepeatPerSec = 10;
		}
		else
			BUTSetDefaultAutoRepeat();

		vi_BUTValue = BUTWaitForPress(MENU_INACTIVE_PROC_EXIT_SEC);

		if (vb_OneValue && (vi_BUTValue==BUT_CENTER || vi_BUTValue==BUT_LEFT))
			vb_Exit = 1;
		else if (vi_BUTValue==BUT_CENTER) {
			//zmiana zaznaczenia
			BUTWaitForUnpress();
			vi_Selected = !vi_Selected;
		}
		else if (!vi_Selected && vi_BUTValue) {
			//chodzenie po menu
			MENUPrintMatrixEl(px_Matrix,pi_Y,vi_Row,vi_Col,0); //odznacz poprzedni element
			switch (vi_BUTValue) {
				case BUT_LEFT:
					if (!(vi_Col+vi_Row)) vb_Exit = 1;
					else if (vi_Col) vi_Col--;
					else {
						vi_Col=MatrixByte(vi_Cols)-1;
						vi_Row--;
					}
					break;
				case BUT_RIGHT:
					if (vi_Col+vi_Row==MatrixByte(vi_Cols)+MatrixByte(vi_Rows)-2) vb_Exit = 1;
					else if (vi_Col<MatrixByte(vi_Cols)-1)	vi_Col++;
					else {
						vi_Col=0;
						vi_Row++;
					}
					break;
				case BUT_UP:
					if (!vi_Row) vb_Exit = 1;
					else vi_Row--;
					break;
				case BUT_DOWN:
					if (vi_Row==MatrixByte(vi_Rows)-1) vb_Exit = 1;
					else vi_Row++;
					break;
				default:
					break;
			}
		}
		else if (((!vb_OneValue) && vi_BUTValue) || (vb_OneValue && (vi_BUTValue==BUT_UP || vi_BUTValue==BUT_DOWN))) {
			//zmieniaj wartoœæ
			vi_Value = MatrixGetValue(px_Matrix,vi_Row,vi_Col);
			switch (vi_BUTValue) {
				case BUT_LEFT: case BUT_DOWN:
					if (vi_Value>MatrixColByte(vi_Col,vi_MinValue))
						vi_Value--;
					break;
				case BUT_RIGHT: case BUT_UP:
					if (vi_Value<MatrixColByte(vi_Col,vi_MaxValue))
						vi_Value++;
					break;
				default:
					break;
			}
			MatrixSetValue(vi_Row,vi_Col,vi_Value);
		}

		if (vi_BUTValue)
			MENUPrintMatrixEl(px_Matrix,pi_Y,vi_Row,vi_Col,vi_Selected+(!vb_OneValue)); //zaznacz nowy element
	}

	BUTWaitForUnpress();
	BUTSetDefaultAutoRepeat();

} //MENUExecuteMatrix
#endif

