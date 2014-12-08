#ifndef	CHARSET_H
#define CHARSET_H 1

#include <inttypes.h>

#include "BasicLCD.h"
#include "AdvancedLCD.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//znaki specjalne
//odstêp w pikselach
#define Sp1px  '\x01'
#define Sp2px  '\x02'
#define Sp3px  '\x03'
#define Sp4px  '\x04'
//podkreœlenie
#define ULOn      '\x10'
#define ULOff     '\x11'
#define ULSwitch  '\x12'
//negacja
#define NEGOn     '\x13'
#define NEGOff    '\x14'
#define NEGSwitch '\x15'
//odstêp w pikselach
#define Sp1pxS  "\x01"
#define Sp2pxS  "\x02"
#define Sp3pxS  "\x03"
#define Sp4pxS  "\x04"
//podkreœlenie
#define ULOnS      "\x10"
#define ULOffS     "\x11"
#define ULSwitchS  "\x12"
//negacja
#define NEGOnS     "\x13"
#define NEGOffS    "\x14"
#define NEGSwitchS "\x15"

#define CLRMODE_NONE 0 //nie czyœæ, po prostu nadpisuj
#define CLRMODE_CHAR 1 //czyœæ w ramach wydruku pojedynczego znaku, w trakcie new line doczyœæ do koñca
#define CLRMODE_LINE 2 //czyœæ po przejœciu do nowej linii
#define CLRMODE_PAGE 3 //czyœæ po przejœciu do nowej strony (czyli powrót kursora do góry)

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

//PARAMETERS
#define CSPCurr 0           //bie¿¹cy charset
#define CSPMaxH 1           //maksymalna wysokoœæ znaków wydrukowanych w danej linii
#define CSPPrevCharXSpace 2 //pobrane z ostatnio drukowanego znaku w celu obliczenia przestrzeni przed nastêpnym
#define CSPClrMode 3        //tryb czyszczenia
#define CSPNewLine 4        //czy przenosiæ automatycznie do nowej linii
#define CSPNegMode 5        //czy negowaæ wyœwietlanie
#define CSPUnderline 6      //czy podkreœlaæ
#define CSPAddSpacePx 7     //czy dodawaæ dodatkowe przerwy miêdzy znakami
uint8_t gt_CSParams[CSPAddSpacePx+1],gt_CSParamsStack[CSPAddSpacePx+1];
#define CSPCurrX 0
#define CSPCurrY 1
int16_t gt_CSXY[2],gt_CSXYStack[2];

//CHARACTER SETS
#define CS_CNT 3       //iloœæ zestawów znaków
#define CS_Z6p 0
#define CS_Z9p CS_Z6p
//#define CS_Z9p  1
//#define CS_Z16p 2
//#define CS_Z20p 3
//#define CS_C6p  CS_Z6p  //cyfry z zestawu znaków
//#define CS_C7p  4
//#define CS_C8p  CS_Z9p  //Cyfry08p=Znaki9p liniê wy¿ej
#define CS_C8p  1  //Cyfry08p=Znaki9p liniê wy¿ej
//#define CS_C13p CS_Z16p //Cyfry13p=Znaki16p liniê wy¿ej
#define CS_C13p 2 //Cyfry13p=Znaki16p liniê wy¿ej
#define CS_Z16p CS_Z6p
//#define CS_C16p 5//CS_Z20p //Cyfry16p=Znaki20p liniê wy¿ej
//#define CS_C21p 6
//#define CS_C31p 7
//#define CS_C40p 8
typedef struct {
	uint8_t vi_Height;    //maksymalna wysokoœæ znaków w charsecie
	uint8_t vi_YSpace;    //odstêp po linii
	uint8_t *vt_XSpaces;  //mapowania odstêpów zapisanych przy obrazkach na iloœæ pikseli
	uint8_t *vt_Data;     //dane z obrazkami
	uint8_t vi_AOCharOffs;//o ile przesuniete dane w tabelce gt_AddrOffs w stosunku do ASCII(char)
	uint8_t vi_AOCount;   //iloœæ rekordów w gt_AddrOffs
	uint8_t *vt_AddrOffs; //ofsety kolejnych znaków (indeksy od 0 do vi_AOCount-1); jeœli 0 znaczy ¿e nie ma znaczka
	char    *vt_AddToAddr;//od którego znaczka dodawaæ do adresu kolejne +256
} CharSet;
CharSet ct_CS[CS_CNT];

//TEMPORARY BUFFOR
#define TEMPTEXTSIZE 32
char gv_TempText[TEMPTEXTSIZE];

#ifndef __OPTIMIZE__
	uint8_t gi_OutBufforIdx;
	char gv_OutBuffor[128];
#endif

//==============================================================
// PUBLIC PROCEDURES
//==============================================================


//ustaw bie¿¹cy zestaw znaków
#define CSSet(pi_CSNo) {gt_CSParams[CSPCurr] = pi_CSNo;}
//ustaw wspó³rzêdne kursora
void CSSetXY(int16_t pi_X, int16_t pi_Y);
//ustaw wspó³rzêdne kursora - wersja 1bajt
void CSSetXYu8(uint8_t pi_X, uint8_t pi_Y);
//ustaw tryb czyszczenia wyœwietlacza
#define CSSetClrMode(pi_ClrMode) {gt_CSParams[CSPClrMode]=pi_ClrMode;}
//ustaw tryb przenoszenia kursora do nowej linii
#define CSSetNewLine(pb_NewLine) {gt_CSParams[CSPNewLine]=pb_NewLine;}
//ustaw tryb negowania wyœwietlacza tekstu
#define CSSetNegMode(pb_NegMode) {gt_CSParams[CSPNegMode]=pb_NegMode; gb_LCDNegMode=pb_NegMode;}
//ustaw tryb podkreœlenia tekstu
#define CSSetUnderline(pb_Underline) {gt_CSParams[CSPUnderline]=pb_Underline;}
//ustaw dodatkowy odstêp miêdzy znakami
#define CSSetAddSpacePx(pi_AddSpacePx) {gt_CSParams[CSPAddSpacePx]=pi_AddSpacePx;}

//zapamiêtaj wszystkie parametry
void CSPushParams(void);

//odtwórz zapamiêtane parametry
void CSPopParams(void);

//przesuñ kursor do nowej linii
void CSNewLine(void);

//wyczyœæ wyœwietlacz, przesuñ kursor na górê
void CSNewPage(void);

//drukuj pojedynczy znak w miejscu kursora (gi_CurrX,gi_CurrY); po wydrukowaniu przesuñ kursor
void CSPrintC(char pv_Char);

//drukuj tekst
#define CSPrintV(pv_Text) {CSPrintV_dual(pv_Text,0);}
//drukuj tekst (z pamiêci programu)
#define CSPrintV_p(pv_Text) {CSPrintV_dual(pv_Text,1);}
//drukuj tekst (z pamiêci operacyjnej lub programu)
void CSPrintV_dual(
  char*   pv_Text     //tekst do wydrukowania
 ,uint8_t pb_ProgMem  //0:pamiêæ operacyjna 1:pamiêæ programu
);

//zwróæ szerokoœæ tekstu w pikselach
#define CSWidthV(pv_Text) (CSWidthV_dual(pv_Text,0))
//zwróæ szerokoœæ tekstu w pikselach (z pamiêci programu)
#define CSWidthV_p(pv_Text) (CSWidthV_dual(pv_Text,1))
//zwróæ szerokoœæ tekstu w pikselach (z pamiêci operacyjnej lub programu)
uint16_t CSWidthV_dual(
  char*   pv_Text     //tekst do obliczenia szerokoœci
 ,uint8_t pb_ProgMem  //0:pamiêæ operacyjna 1:pamiêæ programu
	);

//drukuj tekst z wyrównywaniem
#define CSPrintAV(pv_Text,pi_Align,pi_Width) {CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//drukuj tekst z wyrównywaniem (z pamiêci programu)
#define CSPrintAV_p(pv_Text,pi_Align,pi_Width) {CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//ustaw wspó³rzêdne, drukuj tekst z wyrównywaniem
#define CSPrintXYAV(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXY(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//ustaw wspó³rzêdne, drukuj tekst z wyrównywaniem (z pamiêci programu)
#define CSPrintXYAV_p(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXY(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//ustaw wspó³rzêdne, drukuj tekst z wyrównywaniem - wersja 1 bajt
#define CSPrintXYu8AV(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXYu8(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//ustaw wspó³rzêdne, drukuj tekst z wyrównywaniem (z pamiêci programu) - wersja 1 bajt
#define CSPrintXYu8AV_p(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXYu8(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//drukuj tekst z wyrównywaniem (z pamiêci operacyjnej lub programu)
void CSPrintAV_dual(
  char   *pv_Text    //tekst do wydrukowania
 ,uint8_t pi_Align   //ALIGN_LEFT/ALIGN_CENTER/ALIGN_RIGHT
 ,uint8_t pi_Width   //jeœli d³ugoœæ wiêksza od CSWidthV(pv_Text) to obszar przed/po tekœcie czyszczony
 ,uint8_t pb_ProgMem //0:pamiêæ operacyjna 1:pamiêæ programu
);

//konwertuj liczbê dziesiêtn¹ do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits  //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pb_AlwaysSign //dopisaæ z przodu +/ /- czy tylko -
);

//konwertuj liczbê rzeczywist¹ do tekstu
char *CSFloat2V(
  float   pn_Number      //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits   //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pi_FractDigits //iloœæ cyfr po przecinku
 ,uint8_t	pb_With0Fract  // czy zostawiæ nieznacz¹ce zera w u³amku
 ,uint8_t pb_AlwaysSign  //dopisaæ z przodu +/ /- czy tylko -
);

//konwertuj liczbê szesnastkow¹ do tekstu
char *CSHex2V(
  void    *pi_Number //liczba do wyœwietlenia
 ,uint8_t  pi_Bytes  //iloœæ bajtów z liczby do wyœwietlenia
);

//konwertuj liczbê binarn¹ (1 bajt) do tekstu
char *CSBin2V(uint8_t pi_Byte);

//operacje sklejania stringów
char *gv_strBuffor; uint8_t gi_strLen,gi_strIdx;
void STRInit(char *pv_Buffor,uint8_t vi_Length);
void STRAddC(char pc_Char);
void STRAddV(char *pv_Text);
void STRAddV_p(char *pv_Text);

#endif
