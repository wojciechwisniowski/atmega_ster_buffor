#include "Charset.h"

#include <avr/pgmspace.h>

#include "BasicLCD.h"
#include "AdvancedLCD.h"

#include "Charsets/CSZnaki06pPelny.h"
//#include "Charsets/CSZnaki09pStandard.h"
//#include "Charsets/CSZnaki16pStandard.h"
//#include "Charsets/CSZnaki20pDuze.h"
//#include "Charsets/CSCyfry07pPelny.h" //nie u¿ywane
#include "Charsets/CSCyfry08pPelny.h"
#include "Charsets/CSCyfry13pPelny.h"
//#include "Charsets/CSCyfry16pStandard.h"
//#include "Charsets/CSCyfry21pStandard.h"
//#include "Charsets/CSCyfry31pStandard.h"
//#include "Charsets/CSCyfry40pMin.h"


//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define CS ct_CS[gt_CSParams[CSPCurr]]
#define CS_Height (pgm_read_byte(&CS.vi_Height))
#define CS_YSpace (pgm_read_byte(&CS.vi_YSpace))
#define CS_XSpaces ((uint8_t *)pgm_read_word(&CS.vt_XSpaces))
#define CS_Data ((uint8_t *)pgm_read_word(&CS.vt_Data))
#define CS_AOCharOffs (pgm_read_byte(&CS.vi_AOCharOffs))
#define CS_AOCount (pgm_read_byte(&CS.vi_AOCount))
#define CS_AddrOffs ((uint8_t *)pgm_read_word(&CS.vt_AddrOffs))
#define CS_AddToAddr ((char *)pgm_read_word(&CS.vt_AddToAddr))

#define CS_XSpaces_el(pi_XSpaceIdx) (pgm_read_byte(&CS_XSpaces[pi_XSpaceIdx]))
#define CS_Data_el(pi_Idx) (pgm_read_byte(&CS_Data[pi_Idx]))
#define CS_AddrOffs_el(pi_Idx) (pgm_read_byte(&CS_AddrOffs[pi_Idx]))
#define CS_AddToAddr_el(pi_Idx) ((char)pgm_read_byte(&CS_AddToAddr[pi_Idx]))

#define CS_ImgXSpace(pv_Img) (pgm_read_byte(&pv_Img[2]))
//#define CS_ImgXSpBefore(pv_Img) ((CS_ImgXSpace(pv_Img)>>2)&0x3)
//#define CS_ImgXSpAfter (pv_Img) (CS_ImgXSpace(pv_Img)&0x3)
#define CS_XSpaceValue(pi_PrevXSpace,pi_CurrXSpace) (CS_XSpaces_el( ((pi_PrevXSpace)&0x3)+(((pi_CurrXSpace)>>2)&0x3) ) )

//==============================================================
// FORWARD DECLARATIONS
//==============================================================

//zwróæ adres do obrazka dla przekazanego znaku
//static inline uint8_t* CSGetImg(char pc_Char) __attribute__((always_inline));
uint8_t *CSGetImg(char pc_Char);

CharSet ct_CS[] PROGMEM = {
  cr_CSZnaki06p
 //,cr_CSZnaki09p
 //,cr_CSZnaki16p
 //,cr_CSZnaki20p
 //,cr_CSCyfry07p
 ,cr_CSCyfry08p
 ,cr_CSCyfry13p
 //,cr_CSCyfry16p
// ,cr_CSCyfry21p
// ,cr_CSCyfry31p
// ,cr_CSCyfry40p
};

uint8_t gt_CSParams[CSPAddSpacePx+1]={/*CSPCurr*/0,/*CSPMaxH*/0,/*CSPPrevCharXSpace*/0,/*CSPClrMode*/CLRMODE_CHAR,/*CSPNewLine*/1,/*CSPNegMode*/0,/*CSPUnderline*/0,/*CSPAddSpacePx*/0};

//==============================================================
// PUBLIC PROCEDURES
//==============================================================


//*********************************
//ustaw wspó³rzêdne kursora
void CSSetXY(int16_t pi_X, int16_t pi_Y) {
	gt_CSParams[CSPPrevCharXSpace] = 0;
	gt_CSParams[CSPMaxH] = 0;
	gt_CSXY[CSPCurrX] = pi_X;
	gt_CSXY[CSPCurrY] = pi_Y;
} //CSSetXY


//*********************************
//ustaw wspó³rzêdne kursora - wersja 1bajt
void CSSetXYu8(uint8_t pi_X, uint8_t pi_Y) {
	CSSetXY(pi_X,pi_Y);
} //CSSetXYu8


//*********************************
//zapamiêtaj wszystkie parametry
void CSPushParams(void) {
	uint8_t vi_Idx;
	for(vi_Idx=0; vi_Idx<=CSPAddSpacePx; vi_Idx++) gt_CSParamsStack[vi_Idx]=gt_CSParams[vi_Idx];
	for(vi_Idx=0; vi_Idx<2; vi_Idx++) gt_CSXYStack[vi_Idx]=gt_CSXY[vi_Idx];
	CSSetClrMode(CLRMODE_CHAR);
	CSSetNewLine(1);
	CSSetNegMode(0);
	CSSetUnderline(0);
	CSSetAddSpacePx(0);
}


//*********************************
//odtwórz zapamiêtane parametry
void CSPopParams(void) {
	uint8_t vi_Idx;
	for(vi_Idx=0; vi_Idx<=CSPAddSpacePx; vi_Idx++) gt_CSParams[vi_Idx]=gt_CSParamsStack[vi_Idx];
	for(vi_Idx=0; vi_Idx<2; vi_Idx++) gt_CSXY[vi_Idx]=gt_CSXYStack[vi_Idx];
	gb_LCDNegMode = gt_CSParams[CSPNegMode];
}


//*********************************
//przesuñ kursor do nowej linii
void CSNewLine(void) {
	gt_CSParams[CSPPrevCharXSpace] = 0;
	if (gt_CSParams[CSPClrMode]==CLRMODE_CHAR && gt_CSXY[CSPCurrX]<LCD_W)
		//wyczyœæ liniê do koñca
		ADVPrintRect(gt_CSXY[CSPCurrX],gt_CSXY[CSPCurrY],LCD_W-gt_CSXY[CSPCurrX],CS_Height+CS_YSpace,0);
	gt_CSXY[CSPCurrX] = 0;
	if (gt_CSParams[CSPMaxH]==0)
		gt_CSParams[CSPMaxH] = CS_Height;
	gt_CSXY[CSPCurrY] += gt_CSParams[CSPMaxH]+CS_YSpace;
	if (gt_CSXY[CSPCurrY]+gt_CSParams[CSPMaxH]>LCD_H)
		gt_CSXY[CSPCurrY] = 0;
	gt_CSParams[CSPMaxH] = 0;
	if ((!gt_CSXY[CSPCurrY]) && gt_CSParams[CSPClrMode]==CLRMODE_PAGE)
		CSNewPage();
} //CSNewLine


//*********************************
//wyczyœæ wyœwietlacz, przesuñ kursor na górê
void CSNewPage(void) {
	//wyczyœæ ca³y ekran
	gt_CSParams[CSPPrevCharXSpace] = 0;
	ADVPrintRect(0,0,LCD_W,LCD_H,0);
	CSSetXYu8(0,0);
} //CSNewPage


//*********************************
//drukuj pojedynczy znak w miejscu kursora (gt_CSParams[CSPCurrX,gt_CSParams[CSPCurrY); po wydrukowaniu przesuñ kursor
void CSPrintC(char pv_Char) {
	uint8_t *vt_Img;
	uint8_t vi_W,vi_CharH,vi_CSH;
	uint8_t vi_CurrCharXSpace,vi_XSpaceBefore,vi_XSpaceAfter;

	#ifndef __OPTIMIZE__
		gi_OutBufforIdx = (gi_OutBufforIdx+1)%128;
		gv_OutBuffor[gi_OutBufforIdx] = pv_Char;
		gv_OutBuffor[(gi_OutBufforIdx+1)%128] = 0;
	#endif

	if (pv_Char=='\n') {
		CSNewLine();
		return;
	}

	vi_CSH = CS_Height+CS_YSpace;

	//ustal adres obrazka
	if (pv_Char<'\x20') {
		if (pv_Char<'\x10') {
			ADVPrintRect(gt_CSXY[CSPCurrX],gt_CSXY[CSPCurrY],pv_Char,vi_CSH,0);
			vi_W = pv_Char;
		}
		else {
			switch (pv_Char) {
				case ULOn:      gt_CSParams[CSPUnderline] = 1; break;
				case ULOff:     gt_CSParams[CSPUnderline] = 0; break;
				case ULSwitch:  gt_CSParams[CSPUnderline] ^= 1; break;
				case NEGOn:     gt_CSParams[CSPNegMode] = 1; break;
				case NEGOff:    gt_CSParams[CSPNegMode] = 0; break;
				case NEGSwitch: gt_CSParams[CSPNegMode] ^= 1; break;
			}
			gb_LCDNegMode = gt_CSParams[CSPNegMode];
			vi_W = 0;
		}
	}
	
	else {

		vt_Img = CSGetImg(pv_Char);
		vi_CurrCharXSpace = CS_ImgXSpace(vt_Img);

		//sprawdŸ czy siê zmieœci w poziomie (czy nie przenieœæ kursora do nowej linii)
		while(1) {
			vi_XSpaceBefore = CS_XSpaceValue(gt_CSParams[CSPPrevCharXSpace],vi_CurrCharXSpace)
			                 -CS_XSpaceValue(gt_CSParams[CSPPrevCharXSpace],0);
			vi_W = ADV_ImgWidth(vt_Img)+vi_XSpaceBefore;
				if (gt_CSParams[CSPNewLine] && gt_CSXY[CSPCurrX]+vi_W>LCD_W)
					CSNewLine();
				else
					break;
		}
		vi_XSpaceAfter = CS_XSpaceValue(vi_CurrCharXSpace,0)+gt_CSParams[CSPAddSpacePx];
		vi_W          += vi_XSpaceAfter;
		vi_CharH       = ADV_ImgHeight(vt_Img)+ADV_ImgYOffs(vt_Img);

		if (!gt_CSXY[CSPCurrX] && gt_CSParams[CSPClrMode]==CLRMODE_LINE)
			ADVPrintRect(0,gt_CSXY[CSPCurrY],LCD_W,vi_CSH,0);

		//drukuj obrazek
		if (gt_CSParams[CSPClrMode]==CLRMODE_CHAR)
			//czyszczenie w obrêbie drukowanego znaku
			ADVPrintImg(gt_CSXY[CSPCurrX]
			           ,gt_CSXY[CSPCurrY]
			           ,vi_XSpaceBefore
			           ,vi_XSpaceAfter
			           ,vi_CSH
			           ,vt_Img);
		else
			//miejsce wydruku zosta³o wyczyszczone wczeœniej lub tryb bez czyszczenia
			ADVPrintImg(gt_CSXY[CSPCurrX]+vi_XSpaceBefore,gt_CSXY[CSPCurrY],0,0,0,vt_Img);

		//aktualizuj globale
		if (gt_CSParams[CSPMaxH]<vi_CharH) gt_CSParams[CSPMaxH] = vi_CharH;
		gt_CSParams[CSPPrevCharXSpace] = vi_CurrCharXSpace;
	}
	
	if (gt_CSParams[CSPUnderline])
		ADVPrintRect(gt_CSXY[CSPCurrX],gt_CSXY[CSPCurrY]+vi_CSH-1,vi_W,1,1);

	gt_CSXY[CSPCurrX] += vi_W;
	
} //CSPrintC


//*********************************
//drukuj tekst (zakoñczony 0)
void CSPrintV_dual(
  char*   pv_Text     //tekst do wydrukowania
 ,uint8_t pb_ProgMem  //0:pamiêæ operacyjna 1:pamiêæ programu
) {
	char vc_Char;

	while (1) {
		if (pb_ProgMem)
			vc_Char = pgm_read_byte(pv_Text++);
		else
			vc_Char = *pv_Text++;
		if (vc_Char)
			CSPrintC(vc_Char);
		else
			return;
	}
} //CSPrintV_dual


//*********************************
//zwróæ szerokoœæ tekstu w pikselach (z pamiêci operacyjnej lub programu)
uint16_t CSWidthV_dual(
  char*   pv_Text     //tekst do obliczenia szerokoœci
 ,uint8_t pb_ProgMem  //0:pamiêæ operacyjna 1:pamiêæ programu
	) {
	char vc_Char;
	uint16_t vi_Width = 0;
	uint8_t vi_PrevCharXSpace = 0,vi_CurrCharXSpace;
	uint8_t *vt_Img;

	while (1) {
		if (pb_ProgMem)
			vc_Char = pgm_read_byte(pv_Text++);
		else
			vc_Char = *pv_Text++;

		if (!vc_Char)
			return vi_Width+CS_XSpaceValue(vi_PrevCharXSpace,0);
		else if (vc_Char<'\x20') {
			if (vc_Char<'\x10')
				vi_Width += vc_Char;
		}
		else if (vc_Char=='\n')
			vi_PrevCharXSpace = 0;
		else {
		
			vt_Img = CSGetImg(vc_Char);
			vi_CurrCharXSpace = CS_ImgXSpace(vt_Img);

			vi_Width += 
					CS_XSpaceValue(vi_PrevCharXSpace,vi_CurrCharXSpace)
				+ gt_CSParams[CSPAddSpacePx]
				+ ADV_ImgWidth(vt_Img);
			vi_PrevCharXSpace = vi_CurrCharXSpace;
		}
	
	}

} //CSWidthV_dual


//*********************************
//drukuj tekst z wyrównywaniem (z pamiêci operacyjnej lub programu)
void CSPrintAV_dual(
  char   *pv_Text    //tekst do wydrukowania
 ,uint8_t pi_Align   //ALIGN_LEFT/ALIGN_CENTER/ALIGN_RIGHT
 ,uint8_t pi_Width   //jeœli d³ugoœæ wiêksza od CSWidthV(pv_Text) to obszar przed/po tekœcie czyszczony
 ,uint8_t pb_ProgMem //0:pamiêæ operacyjna 1:pamiêæ programu
) {
	uint16_t vi_Width, vi_MaxWidth;
	uint8_t vi_NewLine, vi_ClrMode;
	uint8_t vi_WidthL, vi_WidthR;
	uint8_t vi_Height;

	if (pi_Width || pi_Align!=ALIGN_LEFT)
		vi_Width = CSWidthV_dual(pv_Text,pb_ProgMem);
	else
		vi_Width = 0;
	vi_NewLine = gt_CSParams[CSPNewLine];
	vi_ClrMode = gt_CSParams[CSPClrMode];
	vi_Height = CS_Height+CS_YSpace;

	CSSetNewLine(0);
	gt_CSParams[CSPPrevCharXSpace]=0;
	gt_CSParams[CSPMaxH]=0;
	
	//czyœæ przed, ustaw X
	if (pi_Width>vi_Width) {
		vi_WidthL = pi_Width-vi_Width;
		vi_WidthR = vi_WidthL;
		vi_MaxWidth = pi_Width;
	}
	else {
		vi_WidthL = 0;
		vi_WidthR = 0;
		vi_MaxWidth = vi_Width;
	}
	switch(pi_Align) {
		case ALIGN_CENTER:
			vi_WidthL /=2;
			gt_CSXY[CSPCurrX] -= vi_MaxWidth/2;
			break;
		case ALIGN_RIGHT:
			gt_CSXY[CSPCurrX] -= vi_MaxWidth;
			break;
default: case ALIGN_LEFT:
			vi_WidthL = 0;
	}
	vi_WidthR -= vi_WidthL;
	
	//czyœæ przed
	if (vi_WidthL) {
		ADVPrintRect(gt_CSXY[CSPCurrX],gt_CSXY[CSPCurrY],vi_WidthL,vi_Height,0);
		gt_CSXY[CSPCurrX] += vi_WidthL;
	}

	//drukuj tekst
	CSPrintV_dual(pv_Text,pb_ProgMem);
	
	//czyœæ po
	if (vi_WidthR) {
		ADVPrintRect(gt_CSXY[CSPCurrX],gt_CSXY[CSPCurrY],vi_WidthR,vi_Height,0);
		gt_CSXY[CSPCurrX] += vi_WidthR;
	}
	
	CSSetNewLine(vi_NewLine);
	CSSetClrMode(vi_ClrMode);
} //CSPrintAV_dual


//*********************************
//konwertuj liczbê dziesiêtn¹ do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits  //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pb_AlwaysSign //dopisaæ z przodu +/ /- czy tylko -
) {
	uint8_t  vi_OutIdx,vi_Digit;
	uint32_t vi_Number;
	uint8_t  vb_Equal0,vi_InIntDigits,vi_OutIntDigits;

	if (pi_Number>=0) vi_Number = pi_Number;
	else vi_Number = -pi_Number;
	vi_OutIdx = TEMPTEXTSIZE-2;
	vb_Equal0 = 1;
	if (!pi_IntDigits) vi_InIntDigits = 1;
	else vi_InIntDigits = pi_IntDigits;
	vi_OutIntDigits = 0;

	do {
		if ((vi_Digit=vi_Number%10))
			vb_Equal0 = 0;
		gv_TempText[vi_OutIdx--] = '0'+vi_Digit;
		vi_OutIntDigits++;
		if (vi_InIntDigits) vi_InIntDigits--;
		vi_Number /= 10;
	} while (vi_Number);
	
	if (pi_Number<0 && !vb_Equal0)
		gv_TempText[vi_OutIdx--] = '-';
	else if (pb_AlwaysSign/* || pi_IntDigits*/) {
		if (vb_Equal0/* || !pb_AlwaysSign*/)
			gv_TempText[vi_OutIdx--] = ' ';
		else
			gv_TempText[vi_OutIdx--] = '+';
	}

	while (vi_InIntDigits--) gv_TempText[vi_OutIdx--] = ' ';

	gv_TempText[TEMPTEXTSIZE-1] = 0;
	
	return &gv_TempText[vi_OutIdx+1];
} //CSInt2V


//*********************************
//konwertuj liczbê rzeczywist¹ do tekstu
#include <math.h>
char *CSFloat2V(
  float   pn_Number      //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits   //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pi_FractDigits //iloœæ cyfr po przecinku
 ,uint8_t pb_With0Fract  // czy zostawiæ nieznacz¹ce zera w u³amku
 ,uint8_t pb_AlwaysSign  //dopisaæ z przodu +/ /- czy tylko -
) {
	uint8_t vi_Idx,vi_OutIdx,vi_Digit;
	uint32_t vi_Number;
	uint8_t  vb_Equal0,vi_InIntDigits,vi_OutIntDigits;
	
	if (!pi_FractDigits) pi_FractDigits = 1;
	
	vi_Number = lround(fabs(pn_Number)*pow(10,pi_FractDigits));
	vi_OutIdx = TEMPTEXTSIZE/2;
	vb_Equal0 = 1;
	if (!pi_IntDigits) vi_InIntDigits = 1;
	else vi_InIntDigits = pi_IntDigits;
	vi_OutIntDigits = 0;

	do {
		if ((vi_Digit=vi_Number%10))
			vb_Equal0 = 0;
		gv_TempText[vi_OutIdx--] = '0'+vi_Digit;
		if (pi_FractDigits) {
			if (!--pi_FractDigits)
				gv_TempText[vi_OutIdx--] = ',';
		}
		else {
			vi_OutIntDigits++;
			if (vi_InIntDigits) vi_InIntDigits--;
		}
		vi_Number /= 10;
	} while (!vi_OutIntDigits || vi_Number);

	if (pn_Number<0 && !vb_Equal0)
		gv_TempText[vi_OutIdx--] = '-';
	else if (pb_AlwaysSign/* || pi_IntDigits*/) {
		if (vb_Equal0/* || !pb_AlwaysSign*/)
			gv_TempText[vi_OutIdx--] = ' ';
		else
			gv_TempText[vi_OutIdx--] = '+';
	}

	while (vi_InIntDigits--) gv_TempText[vi_OutIdx--] = ' ';

	if (!pb_With0Fract) {
		vi_Idx = TEMPTEXTSIZE/2;
		while (gv_TempText[vi_Idx]=='0') vi_Idx--;
			if (gv_TempText[vi_Idx]==',') vi_Idx--;
		gv_TempText[vi_Idx+1] = 0;	
	}
	else
		gv_TempText[TEMPTEXTSIZE/2+1] = 0;
	
	return &gv_TempText[vi_OutIdx+1];
} //CSFloat2V


//*********************************
//konwertuj liczbê szesnastkow¹ do tekstu
char *CSHex2V(
  void    *pi_Number //liczba do wyœwietlenia
 ,uint8_t  pi_Bytes  //iloœæ bajtów z liczby do wyœwietlenia
) {
	uint8_t vi_Byte;
	uint8_t vi_OutIdx,vi_Digit;
	uint8_t *vi_Number;

	vi_Number = (uint8_t *)pi_Number+pi_Bytes-1;
	vi_OutIdx = 0;
	gv_TempText[pi_Bytes*2] = 0;

	while (pi_Bytes--) {
		vi_Byte = *vi_Number--;
		vi_Digit = vi_Byte>>4;
		gv_TempText[vi_OutIdx++] = vi_Digit+(vi_Digit<10 ? '0' : 'A'-10);
		vi_Digit = vi_Byte&0xF;
		gv_TempText[vi_OutIdx++] = vi_Digit+(vi_Digit<10 ? '0' : 'A'-10);
	}
	return gv_TempText;
} //CSHex2V


//*********************************
//konwertuj liczbê binarn¹ (1 bajt) do tekstu
char *CSBin2V(uint8_t pi_Byte) {
	uint8_t vi_Idx;
	uint8_t vi_Bit;

	vi_Idx=8;
	vi_Bit=1;
	
	gv_TempText[vi_Idx]=0;
	while (vi_Idx--) {
		gv_TempText[vi_Idx] = (pi_Byte&vi_Bit ? '1' : '0');
		vi_Bit <<= 1;
	}
	return gv_TempText;
} //CSBin2V


//*********************************
//operacje sklejania stringów
void STRInit(char *pv_Buffor,uint8_t pi_Length) {
	gv_strBuffor = pv_Buffor;
	gi_strLen = pi_Length-1;
	gi_strIdx = 0;
	*gv_strBuffor = '\0';
} //STRInit

//*********************************
void STRAddC(char pc_Char) {
	if (gi_strIdx<gi_strLen) {
		*gv_strBuffor++ = pc_Char;
		*gv_strBuffor = '\0';
		gi_strIdx++;
	}
} //STRAddC

//*********************************
void STRAddV(char *pv_Text) {
	char vc_Char;
	while ((vc_Char=*pv_Text++)) STRAddC(vc_Char);
} //STRAddV

//*********************************
void STRAddV_p(char *pv_Text) {
	char vc_Char;
	while ((vc_Char=pgm_read_byte(pv_Text++))) STRAddC(vc_Char);
} //STRAddV_p


//==============================================================
// PRIVATE PROCEDURES
//==============================================================


//*********************************
//zwróæ adres do obrazka dla przekazanego znaku
uint8_t *CSGetImg(char pc_Char) {
	uint8_t vi_Idx,*vp_Addr;

	do {
		vi_Idx  = pc_Char-CS_AOCharOffs;
		vp_Addr = CS_Data;
	
		if ((vi_Idx<CS_AOCount) && (CS_AddrOffs_el(vi_Idx))) {
			vp_Addr += CS_AddrOffs_el(vi_Idx);
			vi_Idx = 0;
			while (CS_AddToAddr_el(vi_Idx++)<=pc_Char)
				vp_Addr += 0x100;
		}
	
		if (pgm_read_byte(vp_Addr)==0xFF)
			//wskazanie na inn¹ literkê -> podmieñ pc_Char i licz od nowa
			pc_Char = pgm_read_byte(vp_Addr+1);
		else
			return vp_Addr;
	} while(1);
} //CSGetImg
