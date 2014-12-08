#ifndef TIME_H
#define TIME_H 1

#include <inttypes.h>
#include <util/delay.h>


//TEMPORARY BUFFOR
#define TEMPTEXTSIZE 32
char gv_TempText[TEMPTEXTSIZE];
//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//WYKORZYSTANY LICZNIK T1 liczacy co 1/100 sec

#define WITH_OCR1B 0 //czy w��czona obs�uga OCR1B
#if WITH_OCR1B
	#define SUBINT_COUNT 41 //SUBINT_COUNT*100 (4.1kHz) - czestotliwo�� dla OCR1B
#else
	#define SUBINT_COUNT 1 //wy��czone OCR1B - nie dziel na drobniejsze cz�ci (100Hz)
#endif
#define TIME_DEBUG 0 //ustaw 1 �eby uzyska� komunikat b��du o zbyt ma�ych warto�ciach poni�szych sta�ych

#define THth8CountersCnt 1 // maksymalna ilo�� licznik�w 8-bitowych
#define THth16CountersCnt 0 // maksymalna ilo�� licznik�w 16-bitowych
#define THthProceduresCnt 3 // maksymalna ilo�� procedur wywo�ywanych co 1/100 sec

#define THthSecPerDay 8640000UL //24*3600*100

#define TYear    0
#define TMonth   1
#define TDay     2
#define THour    3
#define TMin     4
#define TSec     5
#define THthSec  6
#define TWeekDay 7
/*volatile*/ uint8_t gt_Time[TWeekDay+1]; //aktualny czas

//zmienne do drukowania czasu
//#define TPRINT_HALFSEC 0
#define TPRINT_SEC 0
#define TPRINT_MIN 1
#define TPRINT_HOUR 2
#define TPRINT_DAY 3
typedef struct {
	char *vv_Format;
	uint8_t vi_CS,vi_X,vi_Y,vi_Align,vi_Width;
} TimeSet;
TimeSet gt_TimeSets[TPRINT_DAY+1][2]; //0:co sec 1:co min 2:co dzie�
/*volatile*/ uint8_t gt_TPrint[TPRINT_DAY+1]; //ustawiane przez przerwanie co 0.5sec/1sec/1min/1godzina/1dzie�, czyszczone przez TimePrint

#if WITH_OCR1B
	uint8_t gb_OCR1BMode; //0:wy��czone 1:non inverting mode 2:inverting mode
	uint8_t gi_OCR1BPWM8; //warto�� 8-bitowa dla OCR1B
	#define SetOCR1BMode(pi_Mode) {\
		gb_OCR1BMode=pi_Mode;\
		if (!pi_Mode) TCCR1A &= ~(_BV(COM1B1)|_BV(COM1B0));\
		if (pi_Mode==1) {TCCR1A &= ~(_BV(COM1B0)); TCCR1A |= _BV(COM1B1);}\
		if (pi_Mode==2) TCCR1A |= _BV(COM1B1)|_BV(COM1B0);\
	}
	#define SetOCR1B(pi_PWM) {gi_OCR1BPWM8 = pi_PWM;}
#endif

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//�pij do nast�pnego przerwania - wersja noinline
void TimeSleep(void) __attribute((noinline));

//odczekaj n setnych sekundy
void TimeDelayHthSec(uint8_t pi_HthSec);

//odczekaj n dziesi�tych sekundy
void TimeDelay01Sec(uint8_t pi_01Sec);

//zainicjalizuj modu� czasu po starcie
void TimeInit(void);

//dodaj licznik do zwi�kszania co 1/100 sec
void TimeAddHthCounter(
  uint8_t  /*volatile*/ *pt_8tVariable
 ,uint16_t /*volatile*/ *pt_16tVariable
);

//dodaj procedur� do wywo�ywania co 1/100 sec
void TimeAddHthProcedure(
  void *pt_Procedure
);

//pobierz czas jako string
// w pv_Format dost�pne:
//  Y2 - rok 2-cyfrowo Y4 - rok 4-cyfrowo
//  S1 - pora roku cyfr� 1,2,3,4 S2 - pora roku 2 cyfrowo I II III IV S3 - pora roku literowo zim wio lat jes
//  M1 - miesi�c 1/2-cyfrowo M2 - miesi�c 2-cyfrowo M3 - miesi�c literowo (sty,lut...)
//  D1 - dzie� 1/2-cyfrowo D2 - dzie� 2-cyfrowo
//  d1 - dzie� tygodnia 1-cyfrowo (1-7) d3 - dzie� tygodnia literowo (pon,wto...)
//  h1 - godzina 1/2-cyfrowo h2 - godzina 2-cyfrowo
//  m1 - minuta 1/2-cyfrowo m2 - minuta 2-cyfrowo
//  s1 - sekunda 1/2-cyfrowo s2 - sekunda 2-cyfrowo
//  11 - dziesi�ta sekundy 1-cyfrowo
//  21 - setna sekundy 1/2-cyfrowo 22 - setna sekundy 2-cyfrowo
char *TimeConvert(
  char    *pv_Format //adres z pami�ci programu
 ,char    *pv_Buffor
 ,uint8_t *pt_Time
);

//pobierz czas jako string
#define TimeGetV(pv_Format,pv_Buffor) (TimeConvert(pv_Format,pv_Buffor,(uint8_t *)gt_Time))

//ustaw korekcj� czasu
void TimeSetCor(
  int16_t pi_CorHthSecPerDay //ilo�� setnych sekundy na dob�; -:zwolnij; +:przyspiesz
);

//ustaw czas letni/zimowy
void TimeSetDaylightSavingTime(
	uint8_t pb_DaylightSavingTime
);

//wyczy�� wszystkie sety
void TimeClearSets(void);

//ustaw pojedynczy set
void TimeAddSet(
	uint8_t pi_When    //kiedy drukowa�: TPRINT_SEC/TPRINT_MIN/TPRINT_HOUR/TPRINT_DAY
 ,uint8_t pi_SubIdx  //0/1 - dwa alternatywne wydruki na ten sam czas
 ,char   *pv_Format  //z pami�ci programu
 ,uint8_t pi_CS      //charset
 ,uint8_t pi_X       //po�o�enie X
 ,uint8_t pi_Y       //po�o�enie Y
 ,uint8_t pi_Align   //wyr�wnywanie: ALIGN_LEFT/ALIGN_CENTER/ALIGN_RIGHT
);

//drukuj czas (wed�ug set�w ustawionych w TimeAddSet)
void TimePrint(void);

//zmie� czas: procedura do wywo�ania w menu
#define TClearSec 10
void TimeChange(
  uint8_t pi_Y
 ,uint8_t pi_What //TYear,TMonth,TDay,THour,TMin,TSec,TClearSec
 ,int8_t pi_Value
);

//zmie� korekcj� pr�dko�ci czasu: procedura do wywo�ania w menu
void TimeChangeCor(
  uint8_t pi_Y
 ,uint8_t pi_What
 ,int8_t pi_Value
);

//zmie� czas letni/zimowy
void TimeChangeDaylightSavingTime(
  uint8_t pi_Y
 ,uint8_t pi_What
 ,int8_t pi_Value
);


//==============================================================
// PUBLIC PROCEDURES - UTILITIES
//==============================================================

//zwr�� numer dnia, pocz�wszy od 1.01.2000=>0, 2.02.2000=>1 itd
uint16_t TimeGetDayNo(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
);

//zwr�� dat� (TYear,TMonth,TDay,TWeekDay) z numeru dnia pocz�wszy od 1.01.2000=>0, 2.02.2000=>1 itd
void TimeGetDateFromDayNo(
  uint16_t pi_DayNo
 ,uint8_t  pt_Date[]
);

//zwr�� dzie� tygodnia dla podanej daty
uint8_t TimeDayOfWeek(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
);

//zwr�� ostatni dzie� danego miesi�ca
uint8_t TimeLastDay(
  uint8_t pi_Month
 ,uint8_t pi_YearXX
);

//zwr�� por� roku (0:zima 1:wiosna 2:lato 3:jesie� 4:zima 22.12-31.12)
uint8_t TimeSeason(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
);

//zwr�� pierwszy dzie� pory roku
void TimeSeasonFirstDay(
  uint8_t pt_Time[] //in-out
);

//por�wnaj dwa czasy -1: pierwszy mniejszy 0: r�wne +1:pierwszy wi�kszy
int8_t TimeCompare(
  uint8_t *pt_FirstTime //[0]:hour [1]:min [2]:sec
 ,uint8_t *pt_LastTime //[0]:hour [1]:min [2]:sec
 ,uint8_t pi_Bytes //1: tylko godziny, 2: godziny i minuty, 3: godziny, minuty, sekundy
);

//zwr�� ilo�� sekund pomi�dzy dwoma czasami
int32_t TimeSecBetween(
  uint8_t *pt_FirstTime //[0]:hour [1]:min [2]:sec
 ,uint8_t *pt_LastTime //[0]:hour [1]:min [2]:sec
 ,uint8_t pi_Bytes //1: tylko godziny, 2: godziny i minuty, 3: godziny, minuty, sekundy
);

static inline void _delay_ns(double __ns) __attribute__((always_inline));
void _delay_ns(double __ns) {
	double __tmp = ((F_CPU) / 1e9) * __ns;
	uint16_t __ticks16 = (uint16_t)(__tmp+0.5);
//	uint8_t  __ticks8;
	if (__ns<=0)
		return;
	if (__ticks16>=256*3) {
		_delay_us(__ns/1000.0);
		return;
	}
//	__ticks8 = (uint8_t)__ticks16;
	if (__ticks16/3>0) _delay_loop_1(__ticks16/3);
	if (__ticks16%3) __asm__ volatile ("nop");
	if (__ticks16%3==2) __asm__ volatile ("nop");
} //_delay_ns

#endif
