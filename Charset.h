#ifndef	CHARSET_H
#define CHARSET_H 1

#include <inttypes.h>

#include "BasicLCD.h"
#include "AdvancedLCD.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//znaki specjalne
//odst�p w pikselach
#define Sp1px  '\x01'
#define Sp2px  '\x02'
#define Sp3px  '\x03'
#define Sp4px  '\x04'
//podkre�lenie
#define ULOn      '\x10'
#define ULOff     '\x11'
#define ULSwitch  '\x12'
//negacja
#define NEGOn     '\x13'
#define NEGOff    '\x14'
#define NEGSwitch '\x15'
//odst�p w pikselach
#define Sp1pxS  "\x01"
#define Sp2pxS  "\x02"
#define Sp3pxS  "\x03"
#define Sp4pxS  "\x04"
//podkre�lenie
#define ULOnS      "\x10"
#define ULOffS     "\x11"
#define ULSwitchS  "\x12"
//negacja
#define NEGOnS     "\x13"
#define NEGOffS    "\x14"
#define NEGSwitchS "\x15"

#define CLRMODE_NONE 0 //nie czy��, po prostu nadpisuj
#define CLRMODE_CHAR 1 //czy�� w ramach wydruku pojedynczego znaku, w trakcie new line doczy�� do ko�ca
#define CLRMODE_LINE 2 //czy�� po przej�ciu do nowej linii
#define CLRMODE_PAGE 3 //czy�� po przej�ciu do nowej strony (czyli powr�t kursora do g�ry)

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

//PARAMETERS
#define CSPCurr 0           //bie��cy charset
#define CSPMaxH 1           //maksymalna wysoko�� znak�w wydrukowanych w danej linii
#define CSPPrevCharXSpace 2 //pobrane z ostatnio drukowanego znaku w celu obliczenia przestrzeni przed nast�pnym
#define CSPClrMode 3        //tryb czyszczenia
#define CSPNewLine 4        //czy przenosi� automatycznie do nowej linii
#define CSPNegMode 5        //czy negowa� wy�wietlanie
#define CSPUnderline 6      //czy podkre�la�
#define CSPAddSpacePx 7     //czy dodawa� dodatkowe przerwy mi�dzy znakami
uint8_t gt_CSParams[CSPAddSpacePx+1],gt_CSParamsStack[CSPAddSpacePx+1];
#define CSPCurrX 0
#define CSPCurrY 1
int16_t gt_CSXY[2],gt_CSXYStack[2];

//CHARACTER SETS
#define CS_CNT 3       //ilo�� zestaw�w znak�w
#define CS_Z6p 0
#define CS_Z9p CS_Z6p
//#define CS_Z9p  1
//#define CS_Z16p 2
//#define CS_Z20p 3
//#define CS_C6p  CS_Z6p  //cyfry z zestawu znak�w
//#define CS_C7p  4
//#define CS_C8p  CS_Z9p  //Cyfry08p=Znaki9p lini� wy�ej
#define CS_C8p  1  //Cyfry08p=Znaki9p lini� wy�ej
//#define CS_C13p CS_Z16p //Cyfry13p=Znaki16p lini� wy�ej
#define CS_C13p 2 //Cyfry13p=Znaki16p lini� wy�ej
#define CS_Z16p CS_Z6p
//#define CS_C16p 5//CS_Z20p //Cyfry16p=Znaki20p lini� wy�ej
//#define CS_C21p 6
//#define CS_C31p 7
//#define CS_C40p 8
typedef struct {
	uint8_t vi_Height;    //maksymalna wysoko�� znak�w w charsecie
	uint8_t vi_YSpace;    //odst�p po linii
	uint8_t *vt_XSpaces;  //mapowania odst�p�w zapisanych przy obrazkach na ilo�� pikseli
	uint8_t *vt_Data;     //dane z obrazkami
	uint8_t vi_AOCharOffs;//o ile przesuniete dane w tabelce gt_AddrOffs w stosunku do ASCII(char)
	uint8_t vi_AOCount;   //ilo�� rekord�w w gt_AddrOffs
	uint8_t *vt_AddrOffs; //ofsety kolejnych znak�w (indeksy od 0 do vi_AOCount-1); je�li 0 znaczy �e nie ma znaczka
	char    *vt_AddToAddr;//od kt�rego znaczka dodawa� do adresu kolejne +256
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


//ustaw bie��cy zestaw znak�w
#define CSSet(pi_CSNo) {gt_CSParams[CSPCurr] = pi_CSNo;}
//ustaw wsp�rz�dne kursora
void CSSetXY(int16_t pi_X, int16_t pi_Y);
//ustaw wsp�rz�dne kursora - wersja 1bajt
void CSSetXYu8(uint8_t pi_X, uint8_t pi_Y);
//ustaw tryb czyszczenia wy�wietlacza
#define CSSetClrMode(pi_ClrMode) {gt_CSParams[CSPClrMode]=pi_ClrMode;}
//ustaw tryb przenoszenia kursora do nowej linii
#define CSSetNewLine(pb_NewLine) {gt_CSParams[CSPNewLine]=pb_NewLine;}
//ustaw tryb negowania wy�wietlacza tekstu
#define CSSetNegMode(pb_NegMode) {gt_CSParams[CSPNegMode]=pb_NegMode; gb_LCDNegMode=pb_NegMode;}
//ustaw tryb podkre�lenia tekstu
#define CSSetUnderline(pb_Underline) {gt_CSParams[CSPUnderline]=pb_Underline;}
//ustaw dodatkowy odst�p mi�dzy znakami
#define CSSetAddSpacePx(pi_AddSpacePx) {gt_CSParams[CSPAddSpacePx]=pi_AddSpacePx;}

//zapami�taj wszystkie parametry
void CSPushParams(void);

//odtw�rz zapami�tane parametry
void CSPopParams(void);

//przesu� kursor do nowej linii
void CSNewLine(void);

//wyczy�� wy�wietlacz, przesu� kursor na g�r�
void CSNewPage(void);

//drukuj pojedynczy znak w miejscu kursora (gi_CurrX,gi_CurrY); po wydrukowaniu przesu� kursor
void CSPrintC(char pv_Char);

//drukuj tekst
#define CSPrintV(pv_Text) {CSPrintV_dual(pv_Text,0);}
//drukuj tekst (z pami�ci programu)
#define CSPrintV_p(pv_Text) {CSPrintV_dual(pv_Text,1);}
//drukuj tekst (z pami�ci operacyjnej lub programu)
void CSPrintV_dual(
  char*   pv_Text     //tekst do wydrukowania
 ,uint8_t pb_ProgMem  //0:pami�� operacyjna 1:pami�� programu
);

//zwr�� szeroko�� tekstu w pikselach
#define CSWidthV(pv_Text) (CSWidthV_dual(pv_Text,0))
//zwr�� szeroko�� tekstu w pikselach (z pami�ci programu)
#define CSWidthV_p(pv_Text) (CSWidthV_dual(pv_Text,1))
//zwr�� szeroko�� tekstu w pikselach (z pami�ci operacyjnej lub programu)
uint16_t CSWidthV_dual(
  char*   pv_Text     //tekst do obliczenia szeroko�ci
 ,uint8_t pb_ProgMem  //0:pami�� operacyjna 1:pami�� programu
	);

//drukuj tekst z wyr�wnywaniem
#define CSPrintAV(pv_Text,pi_Align,pi_Width) {CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//drukuj tekst z wyr�wnywaniem (z pami�ci programu)
#define CSPrintAV_p(pv_Text,pi_Align,pi_Width) {CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//ustaw wsp�rz�dne, drukuj tekst z wyr�wnywaniem
#define CSPrintXYAV(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXY(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//ustaw wsp�rz�dne, drukuj tekst z wyr�wnywaniem (z pami�ci programu)
#define CSPrintXYAV_p(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXY(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//ustaw wsp�rz�dne, drukuj tekst z wyr�wnywaniem - wersja 1 bajt
#define CSPrintXYu8AV(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXYu8(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,0);}
//ustaw wsp�rz�dne, drukuj tekst z wyr�wnywaniem (z pami�ci programu) - wersja 1 bajt
#define CSPrintXYu8AV_p(pi_X,pi_Y,pv_Text,pi_Align,pi_Width) {CSSetXYu8(pi_X,pi_Y); CSPrintAV_dual(pv_Text,pi_Align,pi_Width,1);}
//drukuj tekst z wyr�wnywaniem (z pami�ci operacyjnej lub programu)
void CSPrintAV_dual(
  char   *pv_Text    //tekst do wydrukowania
 ,uint8_t pi_Align   //ALIGN_LEFT/ALIGN_CENTER/ALIGN_RIGHT
 ,uint8_t pi_Width   //je�li d�ugo�� wi�ksza od CSWidthV(pv_Text) to obszar przed/po tek�cie czyszczony
 ,uint8_t pb_ProgMem //0:pami�� operacyjna 1:pami�� programu
);

//konwertuj liczb� dziesi�tn� do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wy�wietlenia
 ,uint8_t pi_IntDigits  //minimalna ilo�� cyfr znacz�cych
 ,uint8_t pb_AlwaysSign //dopisa� z przodu +/ /- czy tylko -
);

//konwertuj liczb� rzeczywist� do tekstu
char *CSFloat2V(
  float   pn_Number      //liczba do wy�wietlenia
 ,uint8_t pi_IntDigits   //minimalna ilo�� cyfr znacz�cych
 ,uint8_t pi_FractDigits //ilo�� cyfr po przecinku
 ,uint8_t	pb_With0Fract  // czy zostawi� nieznacz�ce zera w u�amku
 ,uint8_t pb_AlwaysSign  //dopisa� z przodu +/ /- czy tylko -
);

//konwertuj liczb� szesnastkow� do tekstu
char *CSHex2V(
  void    *pi_Number //liczba do wy�wietlenia
 ,uint8_t  pi_Bytes  //ilo�� bajt�w z liczby do wy�wietlenia
);

//konwertuj liczb� binarn� (1 bajt) do tekstu
char *CSBin2V(uint8_t pi_Byte);

//operacje sklejania string�w
char *gv_strBuffor; uint8_t gi_strLen,gi_strIdx;
void STRInit(char *pv_Buffor,uint8_t vi_Length);
void STRAddC(char pc_Char);
void STRAddV(char *pv_Text);
void STRAddV_p(char *pv_Text);

#endif
