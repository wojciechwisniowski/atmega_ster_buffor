#ifndef TEMPUTIL_H
#define TEMPUTIL_H 1

#include <avr/pgmspace.h>
#include "onewire.h"

//WSZÊDZIE GDZIE NIE JEST NAPISANE INACZEJ, TEMPERATURA PRZECHOWYWANA JEST JAKO int16_t (Temp w 'C)*0x100

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define TEMP_USE_BUFF 1 //czy w³¹czone bufory; po wy³¹czeniu dostêpny tylko TEMP_BUFF12m - 12min ostatnia doba
#define TEMP_USE_CHART 1 //0: wy³¹czone 1: w³¹czone w trybie prostym 2: w³¹czone w trybie zaawansowanym (chodzenie po wykresie)

#define TEMP_COUNT  5 //iloœæ termometrów

#define TEMP_FULLNO 0 //numer czujnika do pe³nego zapisu

#define TEMP_12MSIZE 2/*h*/*5 //wielkoœæ bufora 12-minutowych odczytów (gt_TempsVar[pi_Sensor].Buff12m)

#define TEMP_READ_CYCLE 247 //co ile setnych sekundy cykl odczytu
#define TEMP_READ_SUBCYCLE 8 //co ile setnych sekundy kolejne wywo³ania zapisu EEPROM/odczytu termometrów w ramach cyklu
//TEMP_READ_SUBCYCLE ZDECYDOWANIE NIE ZALECANE = 1 (sam odczyt po 1wire trwa 8ms), rekomendowane>2, wtedy g³ówny program nie bêdzie siê przycina³ przy zapisie do EEPROM/odczycie temperatur
//TEMP_READ_CYCLE powinno byæ >= 89+TEMP_COUNT*TEMP_READ_SUBCYCLE
#if TEMP_READ_SUBCYCLE<2
	#error Za ma³a wartoœæ TEMP_READ_SUBCYCLE
#elif F_CPU<4000000 && TEMP_READ_SUBCYCLE<3
	#error Za ma³a wartoœæ TEMP_READ_SUBCYCLE
#elif TEMP_READ_SUBCYCLE<3
	#warning Rekomendowane zwiêkszenie TEMP_READ_SUBCYCLE
#elif TEMP_READ_CYCLE<89+TEMP_COUNT*TEMP_READ_SUBCYCLE
	#error "Za ma³a wartoœæ TEMP_READ_CYCLE"
#endif

/*volatile*/ uint8_t gb_TempReaded; //czy nast¹pi³ odczyt 1: ustawiane przez przerwanie 0: czyœæ w g³ównym programie


////////////////////////////
//STATYCZNE DANE TERMOMETRÓW

typedef struct {
	uint8_t  vt_ID[OW_ROMCODE_SIZE]; //identyfikator termometru
	int8_t   vi_Offs0,vi_Offs20;     //o ile (1/10'C) korygowaæ odczyt termometru przy 20'C,0'C
	char     vv_Name[4];             //nazwa termometru
	uint8_t  vi_EMASize10sec;        //wielkoœæ EMA dla TEMP_READ_SSEC=10*100 -> dla innych interwa³ów automatycznie korygowane
} TempData;

TempData ct_TempsData[TEMP_COUNT] /*PROGMEM = TEMP_DATA*/;

//fibo: 1 2 3 5 8 13 21 34 55 89 144 233 377
#define TEMP_DATA /**/\
TempData ct_TempsData[TEMP_COUNT] PROGMEM = {\
  { {0x28,0x51,0x57,0x0E,0x02,0x00,0x00,0x99}\
   ,0,0,"TOP",5}\
 ,{ {0x28,0xDB,0xC1,0x71,0x02,0x00,0x00,0xA5}\
   ,0,-4,"MID",21}\
 ,{ {0x28,0x7C,0xBB,0xEA,0x03,0x00,0x00,0x72}\
  ,0,3,"BOT",34}\
 ,{ {0x28,0x32,0xD0,0xEA,0x03,0x00,0x00,0x75}\
  ,0,0,"PZA",55}\
 ,{ {0x28,0x6E,0x61,0x0E,0x02,0x00,0x00,0x48}\
  ,0,1,"PPO",89}\
}


////////////////////////////
//ZMIENNE ROBOCZE TERMOMETRÓW

typedef union {
	int32_t L;
	struct {int8_t B0; int16_t I12; int8_t B3;} I12;
	struct {int8_t B0,B1,B2,B3;} B;
	struct {int16_t L,H;} I;
} Long;

typedef struct {
	Long    Tick;
	Long    Curr;
	int32_t Sum12m;
#if 12*60*100/TEMP_READ_CYCLE>254
	uint16_t Sum12mCount;
#else
	uint8_t Sum12mCount;
#endif
	int16_t Buff12m[TEMP_12MSIZE];
	uint8_t ErrorCount;
} TempVar;

TempVar gt_TempsVar[TEMP_COUNT];
uint8_t gi_Temps12mIdx; //indeks bufora gt_TempsVar[Sensor].Buff12m


////////////////////////
//BUFORY Z TEMPERATURAMI
//temperatury w d³u¿szych buforach ni¿ 12min zapamiêtane w 1bajt wed³ug tabeli (dla ujemnych identycznie):
//rozdz minT maxT minV maxV Mul  Div
// 0,1     0    5    0   50 100 2560
// 0,25    5   15   50   90  40 2560
// 0,5    15   25   90  110  20 2560
// 1      25   34  110  119  10 2560
// 2      34   50  119  127   5 2560

typedef struct {
	void *Addr;
	uint8_t Size,SubSize,EEMem;
	char TimeFormatFull[12],TimeFormatOne[9],RangeFull[8],RangeOne[9];
} TempBuffData;

#if TEMP_USE_BUFF

	#define TEMP_BUFFCOUNT 5

	#define TEMP_BUFF12m 0
	#define TEMP_BUFF1h5 1
//	#define TEMP_BUFF6h
	#define TEMP_BUFF1D  2
	#define TEMP_BUFF1W  3
//	#define TEMP_BUFF1M
	#define TEMP_BUFF3M  4
	#define TEMP_BUFF24m TEMP_BUFFCOUNT

	#define TEMP_BUFF1h5SIZE (16*7)     //tydzieñ
	#define TEMP_BUFF1h5SUBSIZE 1       //tylko œrednia
	#define TEMP_BUFF6hSIZE (4*31)      //miesi¹c
	#define TEMP_BUFF6hSUBSIZE 1        //tylko œrednia
	#define TEMP_BUFF1DSIZE (30*2+31*2) //4 miesi¹ce
	#define TEMP_BUFF1DSUBSIZE 3        //min/avg/max
	#define TEMP_BUFF1WSIZE (52*2)      //2 lata
	#define TEMP_BUFF1WSUBSIZE 3        //min/avg/max
	#define TEMP_BUFF1MSIZE (12*10)     //10 lat
	#define TEMP_BUFF1MSUBSIZE 3        //min/avg/max
	#define TEMP_BUFF3MSIZE (4*27)      //27 lat
	#define TEMP_BUFF3MSUBSIZE 3        //min/avg/max

	TempBuffData ct_TempBuffsData[TEMP_BUFFCOUNT+1];

	#define TEMP_BUFFDATA \
		/**/\
		int16_t gt_TempBuff12m[5*24/*dzieñ*/];\
		int8_t gt_TempBuff1h5[TEMP_BUFF1h5SIZE][TEMP_BUFF1h5SUBSIZE];\
		/*int8_t gt_TempBuff6h[TEMP_BUFF6hSIZE][TEMP_BUFF6hSUBSIZE];*/\
		int8_t gt_TempBuff1D[TEMP_BUFF1DSIZE][TEMP_BUFF1DSUBSIZE] EEMEM;\
		int8_t gt_TempBuff1W[TEMP_BUFF1WSIZE][TEMP_BUFF1WSUBSIZE] EEMEM;\
		/*int8_t gt_TempBuff1M[TEMP_BUFF1MSIZE][TEMP_BUFF1MSUBSIZE] EEMEM;*/\
		int8_t gt_TempBuff3M[TEMP_BUFF3MSIZE][TEMP_BUFF3MSUBSIZE] EEMEM;\
		\
		TempBuffData ct_TempBuffsData[TEMP_BUFFCOUNT+1] PROGMEM = {\
		  {(void *)gt_TempBuff12m,5*24/*dzieñ*/   ,1                  ,0,"d3"Sp2pxS"h1:m2"       ,"h1:m2"                 ,"doba"         ,"12"Sp2pxS"min"  }\
		 ,{(void *)gt_TempBuff1h5,TEMP_BUFF1h5SIZE,TEMP_BUFF1h5SUBSIZE,0,"D1.M2"Sp2pxS"h1:m2"    ,"h1:m2"                 ,"tydzieñ"      ,"1,5"Sp2pxS"godz"}\
		 /*,{(void *)gt_TempBuff6h ,TEMP_BUFF6hSIZE ,TEMP_BUFF6hSUBSIZE ,0,"D1.M2"Sp2pxS"h1:m2"    ,"h1:m2"                 ,"miesi¹c"      ,"6"Sp2pxS"godz"  }*/\
		 ,{(void *)gt_TempBuff1D ,TEMP_BUFF1DSIZE ,TEMP_BUFF1DSUBSIZE ,1,"D1"Sp2pxS"M3"Sp2pxS"Y2",""                      ,"4"Sp2pxS"mies","doba"           }\
		 ,{(void *)gt_TempBuff1W ,TEMP_BUFF1WSIZE ,TEMP_BUFF1WSUBSIZE ,1,"D1"Sp2pxS"M3"Sp2pxS"Y2","D1"Sp2pxS"M3"Sp2pxS"Y2","2"Sp2pxS"lata","tydzieñ"        }\
		 /*,{(void *)gt_TempBuff1M ,TEMP_BUFF1MSIZE ,TEMP_BUFF1MSUBSIZE ,1,"M3"Sp2pxS"Y4"          ,""                      ,"10"Sp2pxS"lat","mies"           }*/\
		 ,{(void *)gt_TempBuff3M ,TEMP_BUFF3MSIZE ,TEMP_BUFF3MSUBSIZE ,1,"S3"Sp3pxS"Y4"          ,""                      ,"27"Sp2pxS"lat","3"Sp2pxS"mies"  }\
		 ,{(void *)0             ,5*12/*dzieñ*/   ,1                  ,0                         ,""                      ,""             ,""               }\
		};

#else //TEMP_USE_BUFF==0

	#define TEMP_BUFFCOUNT 1

	#define TEMP_BUFF12m 0
	#define TEMP_BUFF24m TEMP_BUFFCOUNT

	TempBuffData ct_TempBuffsData[TEMP_BUFFCOUNT+1];

	#define TEMP_BUFFDATA \
		/**/\
		int16_t gt_TempBuff12m[5*24/*dzieñ*/];\
		\
		TempBuffData ct_TempBuffsData[TEMP_BUFFCOUNT+1] PROGMEM = {\
		  {(void *)gt_TempBuff12m,5*24/*dzieñ*/,1,0,"d3"Sp2pxS"h1:m2","dzieñ","12"Sp2pxS"min"}\
		 ,{(void *)0,5*12/*dzieñ*/,1,0,"","",""}\
		};

#endif //TEMP_USE_BUFF

#define TempBuff(pi_BuffType,pv_Field) (pgm_read_byte(&ct_TempBuffsData[pi_BuffType].pv_Field))

//bie¿¹ce indeksy do buforów temperatur
uint8_t gt_TempBuffsIdx[TEMP_BUFFCOUNT+1];

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//inicjalizuj modu³
void TempInit(void);

//pobierz temperaturê z bie¿¹cych odczytów
#define TEMP_MIN  0x00 //minimalna z 12-minutówek
#define TEMP_AVG  0x01 //œrednia z 12-minutówek
#define TEMP_MAX  0x02 //maksymalna z 12-minutówek
#define TEMP_TICK 0x10 //bie¿¹ca temperatura szybka: Read
#define TEMP_CURR 0x11 //bie¿¹ca temperatura wolna: EMA(Read)
int16_t TempGet(
  uint8_t pi_Sensor   //numer termometru
 ,uint8_t pi_TempType //TEMP_TICK/TEMP_CURR/TEMP_MIN/TEMP_AVG/TEMP_MAX
);

//pobierz temperaturê z bie¿¹cych odczytów - wersja zwracaj¹ca float
#define TempGetF(pi_Sensor,pi_TempType) ((float)TempGet(pi_Sensor,pi_TempType)/256.0)

//zwróæ temperaturê z bie¿¹cych odczytów jako tekst
#define TempGetV(pi_Sensor,pi_TempType,pb_WithStC) (TempConvert(TempGet(pi_Sensor,pi_TempType),pb_WithStC))

//pobierz trzy temperatury z dowolnego przedzia³u dowolnego bufora
void TempBuffGet(
  uint8_t pi_BuffType //typ bufora TEMP_BUFF12m/...
 ,uint8_t pi_Sensor   //numer termometru
//dla pi_Sensor==TEMP_FULLNO
//bie¿¹cy indeks: gt_TempBuffsIdx[pi_BuffType], rozmiar bufora: TempBuff(pi_BuffType,Size)
//dla pi_Sensor!=TEMP_FULLNO
//bie¿¹cy indeks: gi_Temps12mIdx, rozmiar bufora: TEMP_12MSIZE
 ,uint8_t pi_FirstIdx //pocz¹tkowy indeks
 ,uint8_t pi_LastIdx  //koñcowy indeks
 ,int16_t pt_Temps[]  //zwrócone temperatury [TEMP_MIN] [TEMP_AVG] [TEMP_MAX] tabelka MUSI mieæ 3 elementy
 ,uint8_t pi_Type //0: min/max liczone z buff[TEMP_AVG] 1: min/max liczone z buff[TEMP_MIN/TEMP_MAX]
);

//konwertuj temperaturê do tekstu
#define TEMPCONV_ALWAYSSIGN 0 //1: dla temperatury 0 poprzedŸ spacj¹
#define TEMPCONV_FRACTDIGITS 1 //iloœæ cyfr po przecinku mo¿liwe 1,2
char *TempConvert(
  int16_t pi_Temp    //temperatura do konwersji
 ,uint8_t pb_WithStC //czy dokleiæ na koniec °C
);


//inicjalizuj wykres
void TempChartInit(
  uint8_t pi_X        //X lewego górnego rogu
 ,uint8_t pi_Y        //Y lewego górnego rogu
 ,uint8_t pi_W        //szerokoœæ wykresu (bez obramowañ i cyferek) - jednoczeœnie iloœæ branych danych
 ,uint8_t pi_H        //wysokoœæ wykresu (bez obramowañ i cyferek)
 ,uint8_t pi_Space    //odstêp od krawêdzi zewnêtrznych do skrajnych punktów wykresu
 ,uint8_t pi_BuffType //TEMP_BUFF12m/TEMP_BUFF1h5/TEMP_BUFF6H/TEMP_BUFF1D/TEMP_BUFF1W/TEMP_BUFF1M/TEMP_BUFF3M/TEMP_BUFF24m
);

//rysuj wykres
//zwraca 1, gdy nast¹pi³ pe³ny wydruk, 0, gdy tylko zaktualizowana bie¿¹ca temp (w ostatniej kolumnie)
uint8_t TempChartPrint(void);

//po wciœniêciu Center => wejdŸ do trybu przesuwania siê po wykresie (a¿ do czasu wciœniêcia ponownego Center)
void TempChartExecute(void);


#endif

// zwraca 1 jezeli h1 <= currentH <= h2
int8_t TimeIsBetween(uint8_t h1, uint8_t h2);
