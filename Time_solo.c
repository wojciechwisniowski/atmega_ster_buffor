#include "Time.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "SED1520.h"
#include "AdvancedLCD.h"
#include "Charset.h"
#include "MyMath.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//zmienne do korekcji czasu
int16_t  gi_TCorHthSecPerDay; //o ile setnych sekundy korygowaæ dziennie
int8_t   gi_TCorSign; //aktualny kierunek korekcji -1: zwolnij +1: przypspiesz
//int8_t   gi_TCorT1TOP; //GRUBA KOREKCJA: modyfikator timera => o ile zmieniæ pojedyncz¹ d³ugoœæ w stosunku do oryginalnie obliczonej wartoœci
uint32_t gi_TDayTicks; //licznik tików od pocz¹tku doby/korekcji czasu
uint16_t gi_THoursFromChange; //ile godzin up³ynê³o od restartu/zmiany czasu
uint32_t gi_TCorTickOffs; //co ile tików korygowaæ czas
uint32_t gi_TCorNextTick; //nastêpna wartoœæ tiku do korekcji
volatile uint8_t gi_IntRecurrLevel; //zmienna wewnêtrzna do przechowywania poziomu rekurencji w przerwaniu
uint8_t gi_IntCorHthSecCnt; //zmienna wewnêtrzna licznik dla korekcji setnej sekundy w przerwaniu
#if SUBINT_COUNT>1
uint8_t gi_IntSubHthSec; //licznik czêœci setnych sekundy w przerwaniach
#endif

//register uint32_t gi_TDayTicks asm("r2");
//register int8_t gi_TCorSign asm("r2");
//register uint32_t gi_TCorNextTick asm("r3");
//register uint8_t gi_IntRecurrLevel asm("r3"); //zmienna wewnêtrzna do przechowywania poziomu rekurencji
//register uint8_t gi_IntCorHthSecCnt asm("r4"); //zmienna wewnêtrzna licznik dla korekcji setnej sekundy w przerwaniu

#define COMPILE_HOUR (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0'))
#define COMPILE_MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define COMPILE_SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))
#define COMPILE_YEAR ((((__DATE__ [7]-'0')*10+(__DATE__ [8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))
#define COMPILE_MONTH ((__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) \
              : __DATE__ [2] == 'b' ? 1 \
              : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
              : __DATE__ [2] == 'y' ? 4 \
              : __DATE__ [2] == 'l' ? 6 \
              : __DATE__ [2] == 'g' ? 7 \
              : __DATE__ [2] == 'p' ? 8 \
              : __DATE__ [2] == 't' ? 9 \
              : __DATE__ [2] == 'v' ? 10 : 11)+1)
#define COMPILE_DAY ((__DATE__ [4]==' ' ? 0 : __DATE__ [4]-'0')*10+(__DATE__[5]-'0'))

#define EEHourCount 4

uint8_t gt_EETime[THour+EEHourCount] EEMEM = {0xFF,0xFF,0xFF};// = {COMPILE_YEAR-2000,COMPILE_MONTH,COMPILE_DAY,COMPILE_HOUR};
#define GetEETime(pi_Index) {gt_Time[pi_Index] = eeprom_read_byte((uint8_t *)&gt_EETime[pi_Index]);}
#define SetEETime(pi_Index) {eeprom_write_byte((uint8_t *)&gt_EETime[pi_Index],gt_Time[pi_Index]);}
static inline void GetEEHour(void) __attribute__((always_inline));
void GetEEHour(void) {gt_Time[THour]=0; for (uint8_t vi_Idx=0; vi_Idx<EEHourCount; vi_Idx++) if (eeprom_read_byte((uint8_t *)&gt_EETime[THour+vi_Idx])>gt_Time[THour]) gt_Time[THour]=eeprom_read_byte((uint8_t *)&gt_EETime[THour+vi_Idx]);}
//static inline void SetEEOneHour(uint8_t pi_Index) __attribute__((always_inline));
void SetEEOneHour(uint8_t pi_Index) {eeprom_write_byte((uint8_t *)&gt_EETime[THour+pi_Index],gt_Time[THour]);}
//static inline void SetEEAllHour(void) __attribute__((always_inline));
void SetEEAllHour(void) {for (uint8_t vi_Idx=0; vi_Idx<EEHourCount; vi_Idx++) SetEEOneHour(vi_Idx);}
static inline void SetEEHour(void) __attribute__((always_inline));
void SetEEHour(void) {if (!gt_Time[THour]) SetEEAllHour(); else SetEEOneHour(gt_Time[THour]%EEHourCount);}

uint8_t  /*volatile*/ *gt_THthSecCnt8 [THth8CountersCnt];
uint16_t /*volatile*/ *gt_THthSecCnt16[THth16CountersCnt];
void (*gt_THthProcedures[THthProceduresCnt])(void);

int16_t gi_EETCorHthSecPerDay EEMEM = 0xFA19; //o ile setnych sekundy korygowaæ dziennie
uint8_t gb_EEDaylightSavingTime EEMEM = 0;
#define gb_DaylightSavingTime (eeprom_read_byte(&gb_EEDaylightSavingTime))

#define  TCorTickSize 497UL//rozmiar jednego tiku (o ile siê zmienia w trakcie 1/100s) : (0xFFFFFFFFUL/24/3600/100)

//==============================================================
// FORWARD DECLARATIONS
//==============================================================

//zmieñ podan¹ jednostkê czasu o okreœlon¹ wartoœæ
void TimeChangeInt(uint8_t pi_What,int8_t  pi_ChangeValue);
//zmieñ czas letni/zimowy
void TimeChangeDaylightSavingTimeInt(void);

//==============================================================
// PUBLIC PROCEDURES
//==============================================================


//*********************************
//forsuj przerwanie timera - aby nie gubiæ impulsów
//wywo³aj w przerwaniach o wy¿szym priorytecie (INT0,1,2,TIMER2)
void TimeForceINT(void) {
	if (TIFR&_BV(OCF1A)) {
		uint8_t vi_sreg = SREG;
		uint8_t vi_TIMSK = TIMSK;
		uint8_t vi_GICR = GICR;
		TIMSK = _BV(OCIE1A);
		GICR = 0;
		sei();
		SREG = vi_sreg;
		TIMSK = vi_TIMSK;
		GICR = vi_GICR;
	}
} //TimeForceINT


//*********************************
//œpij do nastêpnego przerwania - wersja noinline
void TimeSleep(void) {
	set_sleep_mode(SLEEP_MODE_IDLE); //ustaw sleep mode
	sleep_mode();
} //TimeSleep


//*********************************
//odczekaj n setnych sekundy
void TimeDelayHthSec(uint8_t pi_HthSec) {
	uint8_t vi_PrevHthSec,vi_CurrHthSec;
	vi_PrevHthSec = gt_Time[THthSec];
	while (pi_HthSec) {
		TimeSleep();
		vi_CurrHthSec = gt_Time[THthSec];
		if (vi_CurrHthSec!=vi_PrevHthSec) pi_HthSec --;
		vi_PrevHthSec = vi_CurrHthSec;
	}
} //TimeDelayHthSec


//*********************************
//odczekaj n dziesi¹tych sekundy
void TimeDelay01Sec(uint8_t pi_01Sec) {
	for (uint8_t vi_Idx=0; vi_Idx<pi_01Sec; vi_Idx++)
		TimeDelayHthSec(10);
} //TimeDelay01Sec


//*********************************
//zainicjalizuj modu³ czasu po starcie
void TimeInit(void) {
//	gi_IntRecurrLevel  = 0;
//	gi_IntCorHthSecCnt = 0;
	//u¿ywany licznik 1
	//przerwanie (1/100sec)
	//u¿yty preskaler, zale¿ny od czêstotliwoœci taktowania

	//preskaler (bity w TCCR1B):
	//CS1:100 -> F_CPU/256
	//CS1:011 -> F_CPU/64
	//CS1:010 -> F_CPU/8
	//CS1:001 -> F_CPU/1

	//(bity 0,1 w TCCR1A, bity 2,3 w TCCR1B)
	//WGM1:1111 => tryb fastPWM 0=>OCR1A
	#define T1_WGMA _BV(WGM11)|_BV(WGM10)
	#define T1_WGMB _BV(WGM13)|_BV(WGM12)

	//FCPU F_CPU/x F-TIMER  IRQ(0.01sec) IRQ(05msec)
	//16M   8      2000kHz  20000 ck    1000 ck
	// 8M   8      1000kHz  10000 ck     500 ck
	// 4M   8       500kHz   5000 ck     250 ck
	// 2M   1      2000kHz  20000 ck    1000 ck
	// 1M   1      1000kHz  10000 ck     500 ck
//#if   (F_CPU>=16384000)
//	#define T1_DIV 0x40
//	#define T1_CS _BV(CS11)|_BV(CS10)
//#elif (F_CPU>=2048000)
#if (F_CPU>=4000000)
	#define T1_DIV 0x8
	#define T1_CS _BV(CS11)
#else
	#define T1_DIV 0x1
	#define T1_CS _BV(CS10)
#endif
	#define DIV(a,b) (((a)+(b)/2)/(b))
	#define LASTPARTDIV(a,b) ((a)-DIV(a,b)*((b)-1))
	#define ABS(a) ((a)>=0?(a):-(a))

	#define T1_DIV100 (T1_DIV*100)
	#define T1_SIZE DIV(F_CPU,T1_DIV100)
	#define T1_HTHSEC_ERROR_F ((float)(F_CPU-T1_SIZE*T1_DIV100)/(float)F_CPU*THthSecPerDay)
	#define T1_HTHSEC_ERROR ((int16_t)(T1_HTHSEC_ERROR_F+(T1_HTHSEC_ERROR_F<0?-0.5:+0.5)))
	#define T1_COR_MANY DIV(T1_SIZE,TCorTimerDiv)
	#define T1_COR_ONE  LASTPARTDIV(T1_SIZE,TCorTimerDiv)

	#if (ABS(LASTPARTDIV(T1_SIZE,255))<DIV(T1_SIZE,255) && ABS(LASTPARTDIV(T1_SIZE,255))<ABS(LASTPARTDIV(T1_SIZE,254)))
		#define TCorTimerDiv 255
		#warning TCorTimerDiv 255
	#elif (ABS(LASTPARTDIV(T1_SIZE,254))<DIV(T1_SIZE,254) && ABS(LASTPARTDIV(T1_SIZE,254))<ABS(LASTPARTDIV(T1_SIZE,253)))
		#define TCorTimerDiv 254
		#warning TCorTimerDiv 254
	#elif (ABS(LASTPARTDIV(T1_SIZE,253))<DIV(T1_SIZE,253) && ABS(LASTPARTDIV(T1_SIZE,253))<ABS(LASTPARTDIV(T1_SIZE,252)))
		#define TCorTimerDiv 253
		#warning TCorTimerDiv 253
	#elif (ABS(LASTPARTDIV(T1_SIZE,252))<DIV(T1_SIZE,252) && ABS(LASTPARTDIV(T1_SIZE,252))<ABS(LASTPARTDIV(T1_SIZE,251)))
		#define TCorTimerDiv 252
		#warning TCorTimerDiv 252
	#elif (ABS(LASTPARTDIV(T1_SIZE,251))<DIV(T1_SIZE,251) && ABS(LASTPARTDIV(T1_SIZE,251))<ABS(LASTPARTDIV(T1_SIZE,250)))
		#define TCorTimerDiv 251
		#warning TCorTimerDiv 251
	#elif (ABS(LASTPARTDIV(T1_SIZE,250))<DIV(T1_SIZE,250) && ABS(LASTPARTDIV(T1_SIZE,250))<ABS(LASTPARTDIV(T1_SIZE,249)))
		#define TCorTimerDiv 250
		#warning TCorTimerDiv 250
	#elif (ABS(LASTPARTDIV(T1_SIZE,249))<DIV(T1_SIZE,249) && ABS(LASTPARTDIV(T1_SIZE,249))<ABS(LASTPARTDIV(T1_SIZE,248)))
		#define TCorTimerDiv 249
		#warning TCorTimerDiv 249
	#elif (ABS(LASTPARTDIV(T1_SIZE,248))<DIV(T1_SIZE,248) && ABS(LASTPARTDIV(T1_SIZE,248))<ABS(LASTPARTDIV(T1_SIZE,247)))
		#define TCorTimerDiv 248
		#warning TCorTimerDiv 248
	#elif (ABS(LASTPARTDIV(T1_SIZE,247))<DIV(T1_SIZE,247) && ABS(LASTPARTDIV(T1_SIZE,247))<ABS(LASTPARTDIV(T1_SIZE,246)))
		#define TCorTimerDiv 247
		#warning TCorTimerDiv 247
	#elif (ABS(LASTPARTDIV(T1_SIZE,246))<DIV(T1_SIZE,246) && ABS(LASTPARTDIV(T1_SIZE,246))<ABS(LASTPARTDIV(T1_SIZE,245)))
		#define TCorTimerDiv 246
		#warning TCorTimerDiv 246
	#elif (ABS(LASTPARTDIV(T1_SIZE,245))<DIV(T1_SIZE,245) && ABS(LASTPARTDIV(T1_SIZE,245))<ABS(LASTPARTDIV(T1_SIZE,244)))
		#define TCorTimerDiv 245
		#warning TCorTimerDiv 245
	#else
		#error Nie udalo sie ustalic TCorTimerDiv
	#endif

	#define T1_SIZE_1BYTE (\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_MANY-1<=255 &&\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_MANY-1<=255 &&\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_ONE-1<=255 &&\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_ONE-1<=255\
	)
#if (T1_SIZE/SUBINT_COUNT<20)
		#error("zmniejsz SUBINT_COUNT -> nie wyrobiê siê")
	#endif
	#if (\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_MANY-1<=10 ||\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_MANY-1<=10 ||\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_ONE-1<=10 ||\
		(T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_ONE-1<=10\
		)
		#error("skorygowane d³ugoœci timer1 za ma³e")
	#endif

	for (uint8_t vi_Idx=TYear; vi_Idx<THour; vi_Idx++)
		GetEETime(vi_Idx);
	GetEEHour();
	if (gt_Time[TYear]==0xFF ||
			((((gt_Time[TYear]*12L+gt_Time[TMonth])*31L)+gt_Time[TDay])*24L+gt_Time[THour]-1)<=
			(((((COMPILE_YEAR-2000L)*12L+COMPILE_MONTH)*31L)+COMPILE_DAY)*24L+COMPILE_HOUR)
     ) {
		gt_Time[TYear]  = COMPILE_YEAR-2000;
		gt_Time[TMonth] = COMPILE_MONTH;
		gt_Time[TDay]   = COMPILE_DAY;
		gt_Time[THour]  = COMPILE_HOUR;
		gt_Time[TMin]   = COMPILE_MINUTE;
		gt_Time[TSec]   = COMPILE_SECOND;
		for (uint8_t vi_Idx=TYear; vi_Idx<THour; vi_Idx++)
			SetEETime(vi_Idx);
		SetEEAllHour();
	}
	else
		gt_Time[TMin] = 30;

	gt_Time[TWeekDay] = TimeDayOfWeek(gt_Time[TDay],gt_Time[TMonth],gt_Time[TYear]);

	gi_TCorHthSecPerDay = eeprom_read_word((uint16_t *)&gi_EETCorHthSecPerDay);
	TimeSetCor(gi_TCorHthSecPerDay);
	#if T1_SIZE_1BYTE
		OCR1AL = (T1_SIZE/SUBINT_COUNT-1);
	#else
OCR1A = (T1_SIZE/SUBINT_COUNT-1);
	#endif


	TCCR1A |= T1_WGMA;
	TCCR1B |= T1_CS|T1_WGMB;



	TIMSK |= _BV(OCIE1A); //Odblokowanie przerwania T1 compare A


} //TimeInit


#if TIME_DEBUG
char cv_ZaMalaStala[] PROGMEM = "Za ma³a sta³a time.h:";
#endif


//*********************************
//dodaj licznik do zwiêkszania co 1/100 sec
void TimeAddHthCounter(
  uint8_t  /*volatile*/ *pt_8tVariable
 ,uint16_t /*volatile*/ *pt_16tVariable
) {
#if THth8CountersCnt+THth16CountersCnt
	uint8_t vi_Cnt;
#endif
	if (pt_8tVariable) {
#if THth8CountersCnt
		vi_Cnt = 0;
		while ((vi_Cnt<THth8CountersCnt) && gt_THthSecCnt8[vi_Cnt]) vi_Cnt++;
		if (vi_Cnt<THth8CountersCnt) gt_THthSecCnt8[vi_Cnt] = pt_8tVariable;
#if TIME_DEBUG
		else {CSSet(CS_Z9p); CSSetXY(0,0); CSPrintV_p(cv_ZaMalaStala); CSPrintV_p(PSTR("THth8CountersCnt")); while (1);}
#endif
#endif
	}
	if (pt_16tVariable) {
#if THth16CountersCnt
		vi_Cnt = 0;
		while ((vi_Cnt<THth16CountersCnt) && gt_THthSecCnt16[vi_Cnt]) vi_Cnt++;
		if (vi_Cnt<THth16CountersCnt) gt_THthSecCnt16[vi_Cnt] = pt_16tVariable;
#if TIME_DEBUG
		else {CSSet(CS_Z9p); CSSetXY(0,0); CSPrintV_p(cv_ZaMalaStala); CSPrintV_p(PSTR("THth16CountersCnt")); while (1);}
#endif
#endif
	}
} //TimeAddHthCounter


//*********************************
//dodaj procedurê do wywo³ywania co 1/100 sec
void TimeAddHthProcedure(
  void *pt_Procedure
) {
#if THthProceduresCnt
	uint8_t vi_Cnt=0;
	while ((vi_Cnt<THthProceduresCnt) && gt_THthProcedures[vi_Cnt]) vi_Cnt++;
	if (vi_Cnt<THthProceduresCnt) gt_THthProcedures[vi_Cnt] = pt_Procedure;
#endif
#if TIME_DEBUG
	else {CSSet(CS_Z9p); CSSetXY(0,0); CSPrintV_p(cv_ZaMalaStala); CSPrintV_p(PSTR("THth8ProceduresCnt")); while (1);}
#endif
} //TimeAddHthProcedure


//*********************************
//pobierz czas jako string
// w pv_Format dostêpne:
//  Y2 - rok 2-cyfrowo Y4 - rok 4-cyfrowo
//  S1 - pora roku cyfr¹ 1,2,3,4 S2 - pora roku 2 cyfrowo I II III IV S3 - pora roku literowo zim wio lat jes
//  M1 - miesi¹c 1/2-cyfrowo M2 - miesi¹c 2-cyfrowo M3 - miesi¹c literowo (sty,lut...)
//  D1 - dzieñ 1/2-cyfrowo D2 - dzieñ 2-cyfrowo
//  d1 - dzieñ tygodnia 1-cyfrowo (1-7) d3 - dzieñ tygodnia literowo (pon,wto...)
//  h1 - godzina 1/2-cyfrowo h2 - godzina 2-cyfrowo
//  m1 - minuta 1/2-cyfrowo m2-minuta 2-cyfrowo
//  s1 - sekunda 1/2-cyfrowo s2-sekunda 2-cyfrowo
//  11 - dziesi¹ta sekundy 1-cyfrowo
//  21 - setna sekundy 1/2-cyfrowo 22 - setna sekundy 2-cyfrowo
char ct_Seasons2[4][4] PROGMEM = {"I","II","III","IV"};
char ct_Seasons3[4][4] PROGMEM = {"zim","wio","lat","jes"};
char ct_Months[12][4] PROGMEM = {"sty","lut","mar","kwi","maj","cze","lip","sie","wrz","paŸ","lis","gru"};
char ct_WeekDays[7][4] PROGMEM = {"nie","pon","wto","œro","czw","pi¹","sob"};
char *TimeConvert(
  char    *pv_Format //adres z pamiêci programu
 ,char    *pv_Buffor
 ,uint8_t *pt_Time
) {

	char   *vv_Format;
char    vi_Char1, vi_Char2;
	int8_t  vi_Value;
	char   *vv_Buffor;
	char   *vv_Text;
	uint8_t vb_IsText;
	uint8_t vi_Sec;

	do {
		vv_Format = pv_Format;
		vv_Buffor = pv_Buffor;
		vi_Sec = pt_Time[TSec];

		while ((vi_Char1 = pgm_read_byte(vv_Format++))) {
			vi_Char2 = pgm_read_byte(vv_Format);
			vv_Text  = 0;
			switch (vi_Char1) {
				case 'Y': vi_Value = pt_Time[TYear]; break;
				case 'S': vi_Value = TimeSeason(pt_Time[TDay],pt_Time[TMonth],pt_Time[TYear]);
					if (vi_Value==4) {
						//zima koñcówka roku => zwróæ ¿e zima nastêpnego roku
						pt_Time[TYear]++;
						vi_Value = 0;
					}
					if (vi_Char2=='2') vv_Text = ct_Seasons2[vi_Value];
					if (vi_Char2=='3') vv_Text = ct_Seasons3[vi_Value];
					vi_Value++;
					break;
				case 'M': vi_Value = pt_Time[TMonth]; if (vi_Char2=='3') vv_Text = ct_Months[vi_Value-1]; break;
				case 'D': vi_Value = pt_Time[TDay]; break;
				case 'd': vi_Value = pt_Time[TWeekDay]; if (vi_Char2=='3') vv_Text = ct_WeekDays[vi_Value]; if (!vi_Value) vi_Value = 7; break;
				case 'h': vi_Value = pt_Time[THour]; break;
				case 'm': vi_Value = pt_Time[TMin]; break;
				case 's': vi_Value = pt_Time[TSec]; break;
				case '1': vi_Value = pt_Time[THthSec]/10; break;
				case '2': vi_Value = pt_Time[THthSec]; break;
				default: vi_Value = -127;
			}

			if (vi_Value==-127 || vi_Char2<'1' || vi_Char2>'4')
				*vv_Buffor++ = vi_Char1;
			else {
				vv_Format++;
				vb_IsText = 0;
				if (vv_Text)
					for (uint8_t vi_Idx=0; pgm_read_byte(vv_Text); vi_Idx++)
						*vv_Buffor++ = pgm_read_byte(vv_Text++);

				if (vi_Char1=='Y' && vi_Char2=='4') {
					if (vi_Value<0) {
						*vv_Buffor++ = '1';
						*vv_Buffor++ = '9';
					}
					else {
						*vv_Buffor++ = '2';
						*vv_Buffor++ = '0';
					}
					vi_Char2 = '2';
				}

				if (!vv_Text) {
					if (vi_Value<0)
						//rok przed 2000
						vi_Value += 100;
					if (vi_Value>=10 || vi_Char2>'1')
						*vv_Buffor++ = '0'+(vi_Value/10);
					*vv_Buffor++ = '0'+(vi_Value%10);
				}
			}
		}

	} while (vi_Sec != pt_Time[TSec]);

	*vv_Buffor = 0;

	return pv_Buffor;
} //TimeConvert


//*********************************
//ustaw korekcjê czasu
void TimeSetCor(
  int16_t pi_CorHthSecPerDay //iloœæ setnych sekundy na dobê; -:zwolnij; +:przyspiesz
) {

	int32_t vi_ChangeVal;
	#define vi_CorHthSecPerDay (pi_CorHthSecPerDay-T1_HTHSEC_ERROR)


/*	gi_TCorT1TOP = (-pi_CorHthSecPerDay)/((int32_t)THthSecPerDay/(int32_t)T1_SIZE);
	vi_Modulo    = (-pi_CorHthSecPerDay)%((int32_t)THthSecPerDay/(int32_t)T1_SIZE);
	if (vi_Modulo<0) {
		gi_TCorT1TOP--;
vi_Modulo += THthSecPerDay/T1_SIZE;
		gi_TCorTickOffs = (THthSecPerDay*TCorTickSize)/vi_Modulo;
	}
*/
	if (vi_CorHthSecPerDay>0) {
		gi_TCorSign = 1;
		gi_TCorTickOffs = DivUL(THthSecPerDay*TCorTickSize,vi_CorHthSecPerDay);
//		gi_TCorTickOffs = (THthSecPerDay*TCorTickSize)/(int32_t)vi_CorHthSecPerDay;
	}
	else if (vi_CorHthSecPerDay<0) {
		gi_TCorSign = -1;
		gi_TCorTickOffs = DivUL(THthSecPerDay*TCorTickSize,-vi_CorHthSecPerDay);
//		gi_TCorTickOffs = (THthSecPerDay*TCorTickSize)/(int32_t)(-vi_CorHthSecPerDay);
	}
	else
		gi_TCorSign = 0;

	uint8_t vi_sreg = SREG;
	cli();
	gi_TDayTicks    = 0;
	gi_TCorNextTick = gi_TCorTickOffs/2;
	SREG = vi_sreg;

	if (pi_CorHthSecPerDay!=gi_TCorHthSecPerDay) {
		//zmieñ bie¿¹cy czas na podstawie czasu, jaki up³yn¹³ od poprzedniej zmiany

		vi_ChangeVal = DivL((int32_t)(pi_CorHthSecPerDay-gi_TCorHthSecPerDay)*(int32_t)gi_THoursFromChange,24);
		int8_t vi_Sign;
		if (vi_ChangeVal<0) {
			vi_Sign = -1;
			vi_ChangeVal = -vi_ChangeVal;
		}
		else
			vi_Sign = 1;
		while (vi_ChangeVal>=100) {
			TimeChangeInt(TSec,vi_Sign);
			vi_ChangeVal -= 100;
		}
		while (vi_ChangeVal) {
			TimeChangeInt(THthSec,vi_Sign);
			vi_ChangeVal--;
		}
//		TimeChangeInt(THthSec,vi_ChangeVal%6000);

//		TimeChangeInt(TMin,vi_ChangeVal/6000);

		gi_TCorHthSecPerDay = pi_CorHthSecPerDay;
		eeprom_write_word((uint16_t *)&gi_EETCorHthSecPerDay,gi_TCorHthSecPerDay);
	}

//	#if T1_SIZE_1BYTE
//		OCR1AL = (T1_SIZE/SUBINT_COUNT+gi_TCorT1TOP-1); //z jak¹ wartoœci¹ porównywaæ licznik
//	#else
//		uint8_t vi_sreg = SREG;
//		cli();
//		OCR1A = (T1_SIZE/SUBINT_COUNT+gi_TCorT1TOP-1); //z jak¹ wartoœci¹ porównywaæ licznik
//		SREG = vi_sreg;
//	#endif
}


//*********************************
//wyczyœæ wszystkie sety
void TimeClearSets(void) {
	uint8_t vi_SetNo,vi_SubIdx;
	for (vi_SetNo=0; vi_SetNo<=TPRINT_DAY; vi_SetNo++)
		for (vi_SubIdx=0; vi_SubIdx<2; vi_SubIdx++)
			gt_TimeSets[vi_SetNo][vi_SubIdx].vv_Format = 0;
} //TimeClearSets


//*********************************
//ustaw pojedynczy set
void TimeAddSet(
	uint8_t pi_When    //kiedy drukowaæ: TPRINT_SEC/TPRINT_MIN/TPRINT_HOUR/TPRINT_DAY
 ,uint8_t pi_SubIdx  //0/1 - dwa alternatywne wydruki na ten sam czas
 ,char   *pv_Format  //z pamiêci programu
 ,uint8_t pi_CS      //charset
 ,uint8_t pi_X       //po³o¿enie X
 ,uint8_t pi_Y       //po³o¿enie Y
 ,uint8_t pi_Align   //wyrównywanie: ALIGN_LEFT/ALIGN_CENTER/ALIGN_RIGHT
) {
	TimeSet *vr_TS = &gt_TimeSets[pi_When][pi_SubIdx];

	vr_TS->vv_Format  = pv_Format;
	vr_TS->vi_CS      = pi_CS;
	vr_TS->vi_X       = pi_X;
	vr_TS->vi_Y       = pi_Y;
	vr_TS->vi_Align   = pi_Align;
	gt_TPrint[pi_When] = 1;

	//oblicz maksymaln¹ d³ugoœæ

/*	uint8_t vt_Time[TWeekDay+1];
	uint8_t vi_MaxWidth,vi_OrgWidth,vi_CurrWidth;

	CSPushParams(); CSSet(pi_CS);
	vt_Time[TYear]    = 10;
	vt_Time[TMonth]   = 1;
	vt_Time[TDay]     = 10;
	vt_Time[THour]    = 10;
	vt_Time[TMin]     = 10;
	vt_Time[TSec]     = 10;
	vt_Time[THthSec]  = 10;
	vt_Time[TWeekDay] = 0;
	vi_OrgWidth = CSWidthV(TimeConvert(pv_Format,gv_TempText,vt_Time));

	//oblicz maksymaln¹ d³ugoœæ iteruj¹c po dniach tygodnia
	vi_MaxWidth = 0;
	for (vt_Time[TWeekDay]=0; vt_Time[TWeekDay]<7; vt_Time[TWeekDay]++) {
		vi_CurrWidth = CSWidthV(TimeConvert(pv_Format,gv_TempText,vt_Time));
		if (vi_CurrWidth>vi_MaxWidth)
			vi_MaxWidth = vi_CurrWidth;
	}
	vt_Time[TWeekDay] = 0;
	vr_TS->vi_Width = vi_MaxWidth-vi_OrgWidth;

	//oblicz maksymaln¹ d³ugoœæ iteruj¹c po miesi¹cach
	vi_MaxWidth = 0;
	for (vt_Time[TMonth]=2; vt_Time[TMonth]<=12; vt_Time[TMonth]++) {
		vi_CurrWidth = CSWidthV(TimeConvert(pv_Format,gv_TempText,vt_Time));
		if (vi_CurrWidth>vi_MaxWidth)
			vi_MaxWidth = vi_CurrWidth;
	}
	vr_TS->vi_Width += vi_MaxWidth;

	CSPopParams();*/

} //TimeAddSet


void debug(char * s){
	GLCD_ClearScreen();
	GLCD_GoTo(0, 0);
	GLCD_WriteString(s);
	_delay_ms(500);
}
//*********************************
//drukuj czas (wed³ug setów ustawionych w TimeAddSet)
void TimePrint(void) {
	uint8_t vb_Print;
	int8_t 	vi_SetNo,vi_SubIdx;
	TimeSet *vr_TS;

	vi_SetNo=TPRINT_DAY+1;
	while(vi_SetNo--)
		if (gt_TPrint[vi_SetNo]) {
			vb_Print = 1;
			break;
		}
	if (!vb_Print)
		return;

	CSPushParams();

	for (vi_SetNo=TPRINT_DAY; vi_SetNo>=0; vi_SetNo--) {
		vb_Print = gt_TPrint[vi_SetNo];
		if (vb_Print) gt_TPrint[vi_SetNo] = 0;
		for (vi_SubIdx=0; vi_SubIdx<2; vi_SubIdx++) {
			vr_TS = &gt_TimeSets[vi_SetNo][vi_SubIdx];
			if (vb_Print && vr_TS->vv_Format) {
				CSSet(vr_TS->vi_CS);
				CSPrintXYu8AV(vr_TS->vi_X,vr_TS->vi_Y,TimeGetV(vr_TS->vv_Format,gv_TempText),vr_TS->vi_Align,vr_TS->vi_Width);
			}
		}
	}

	CSPopParams();
} //TimePrint


//*********************************
//zmieñ czas: procedura do wywo³ania w menu
void TimeChange(
  uint8_t pi_Y
 ,uint8_t pi_What //TYear,TMonth,TDay,THour,TMin,TSec,TClearSec
 ,int8_t pi_Value
) {
//	char *vv_DFormat=0, *vv_HFormat=0;
//	uint8_t	vi_CSd,vi_CSt,vi_Yt;

	if (pi_Value) {
		if (pi_What==TClearSec) {
			if (gt_Time[TSec]>=30)
				TimeChangeInt(TMin,1);
			gt_Time[TSec] = 0;
			gt_Time[THthSec] = 0;
		}
		else
			TimeChangeInt(pi_What,pi_Value);
		gi_THoursFromChange = 0;
	}

/*	TimeClearSets();
	switch(pi_What) {
		case TYear:  vv_DFormat = PSTR("d3 D1.M2."ULOnS"Y4"ULOffS); break;
		case TMonth: vv_DFormat = PSTR("d3 D1."ULOnS"M2"ULOffS".Y4"); break;
		case TDay:   vv_DFormat = PSTR("d3 "ULOnS"D1"ULOffS".M2.Y4"); break;
		case THour:  vv_HFormat = PSTR(ULOnS"h1"ULOffS":m2:s2.11"); break;
		case TMin:   vv_HFormat = PSTR("h1:"ULOnS"m2"ULOffS":s2.11"); break;
		default: case TSec: case TClearSec: vv_HFormat = PSTR("h1:m2:"ULOnS"s2"ULOffS".11"); break;
	}
	if (!vv_DFormat) vv_DFormat = PSTR("d3 D1.M2.Y4");
	if (!vv_HFormat) vv_HFormat = PSTR("h1:m2:s2.11");
	if (pi_What<=TDay) {
		vi_CSd = CS_Z16p;
		vi_CSt = CS_Z9p;  vi_Yt = pi_Y+17;
	}
	else {
		vi_CSd = CS_Z9p;
		vi_CSt = CS_Z16p; vi_Yt = pi_Y+10;
	}
	TimeAddSet(TPRINT_DAY,0,vv_DFormat,vi_CSd,LCD_W/2,pi_Y,ALIGN_CENTER);
	TimeAddSet(TPRINT_SEC,0,vv_HFormat,vi_CSt,LCD_W/2,vi_Yt,ALIGN_CENTER);
	TimePrint();
	TimeClearSets();*/
} //TimeChange


//*********************************
//zmieñ korekcjê prêdkoœci czasu: procedura do wywo³ania w menu
void TimeChangeCor(
  uint8_t pi_Y
 ,uint8_t pi_What
 ,int8_t  pi_Value
) {
	int16_t vi_HthSecPerDay;
//	uint8_t vt_Time[TWeekDay+1];

//	char vv_Buffor[40];
//	char *vv_Format,*vv_TFormat;

	switch (pi_What) {
		case TSec:    vi_HthSecPerDay = 100*pi_Value; /*vv_Format = PSTR(ULOnS"s2"ULOffS Sp2pxS"sec 22"Sp2pxS"ssec");*/ break;
		default: case THthSec: vi_HthSecPerDay = pi_Value; /*vv_Format = PSTR("s2"Sp2pxS"sec "ULOnS"22"ULOffS Sp2pxS"ssec");*/ break;
	}
	vi_HthSecPerDay += gi_TCorHthSecPerDay;
	if (pi_Value && vi_HthSecPerDay>-6000 && vi_HthSecPerDay<6000)
		TimeSetCor(vi_HthSecPerDay);

/*	STRInit(vv_Buffor,40);
	if (gi_TCorHthSecPerDay<0) {
		STRAddC('-');
		vv_TFormat = PSTR("h1:m2:s2.11 zwolnij");
	}
	else if (gi_TCorHthSecPerDay>0) {
		STRAddC('+');
		vv_TFormat = PSTR("h1:m2:s2.11 przyspiesz");
	}
	else
		vv_TFormat = PSTR("h1:m2:s2.11");

	if (gi_TCorHthSecPerDay>=0)
		vi_HthSecPerDay = gi_TCorHthSecPerDay;
	else
		vi_HthSecPerDay = -gi_TCorHthSecPerDay;
	vt_Time[TSec] = vi_HthSecPerDay/100;
	vt_Time[THthSec] = vi_HthSecPerDay%100;
	STRAddV(TimeConvert(vv_Format,gv_TempText,vt_Time));

	CSPushParams();
	CSSet(CS_Z9p); CSPrintXYu8AV(LCD_W/2,pi_Y,TimeGetV(vv_TFormat,gv_TempText),ALIGN_CENTER,LCD_W);
	CSSet(CS_Z16p); CSPrintXYu8AV(LCD_W/2,pi_Y+9,vv_Buffor,ALIGN_CENTER,LCD_W);
	CSSet(CS_Z6p); CSPrintXYu8AV_p(LCD_W/2,pi_Y+26,PSTR("na dobê"),ALIGN_CENTER,LCD_W);
	CSPopParams();*/

} //TimeChangeCor


//*********************************
//zmieñ czas letni/zimowy
void TimeChangeDaylightSavingTime(
  uint8_t pi_Y
 ,uint8_t pi_What
 ,int8_t pi_Value
) {
	if (pi_Value)
		TimeChangeDaylightSavingTimeInt();

//	CSSet(CS_Z9p);
//	CSPrintXYu8AV(LCD_W/2,pi_Y,TimeGetV(PSTR("h1:m2:s2"),gv_TempText),ALIGN_CENTER,LCD_W);
//	CSSet(CS_Z16p);
//	CSPrintXYu8AV_p(LCD_W/2,pi_Y+10,gb_DaylightSavingTime?PSTR("letni"):PSTR("zimowy"),ALIGN_CENTER,LCD_W);

} //TimeChangeDaylightSavingTime


//==============================================================
// PUBLIC PROCEDURES - UTILITIES
//==============================================================


//*********************************
//zwróæ numer dnia, pocz¹wszy od 1.01.2000=>0, 2.02.2000=>1 itd
uint16_t TimeGetDayNo(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
) {
	uint16_t vi_DayNo;
	uint8_t  vi_Month;
	vi_DayNo = ((uint16_t)(pi_YearXX+3)*1461)/4-3*365;
	for (vi_Month=1; vi_Month<pi_Month; vi_Month++)
		vi_DayNo += TimeLastDay(vi_Month,pi_YearXX);
	vi_DayNo += pi_Day-1;
	return vi_DayNo;
} //TimeGetDayNo


//*********************************
//zwróæ datê (TYear,TMonth,TDay,TWeekDay) z numeru dnia pocz¹wszy od 1.01.2000=>0, 2.02.2000=>1 itd
void TimeGetDateFromDayNo(
  uint16_t pi_DayNo
 ,uint8_t  pt_Date[]
) {
	pi_DayNo++;
	pt_Date[TYear]  = (pi_DayNo/1461)*4;
	pt_Date[TMonth] = 1;
	pi_DayNo -= pt_Date[TYear]/4*1461;
	while (pi_DayNo>((pt_Date[TYear]%4)?365:366)) {
		pi_DayNo -= (pt_Date[TYear]%4)?365:366;
		pt_Date[TYear]++;
	}
	while (pi_DayNo>TimeLastDay(pt_Date[TMonth],pt_Date[TYear])) {
		pi_DayNo -= TimeLastDay(pt_Date[TMonth],pt_Date[TYear]);
		pt_Date[TMonth]++;
	}
	pt_Date[TDay] = pi_DayNo;
	pt_Date[TWeekDay] = TimeDayOfWeek(pt_Date[TDay],pt_Date[TMonth],pt_Date[TYear]);
} //TimeGetDateFromDayNo


//*********************************
//zwróæ dzieñ tygodnia dla podanej daty
//http://pl.wikipedia.org/wiki/Wieczny_kalendarz
//http://everything2.com/title/How+to+calculate+the+day+of+the+week+for+a+given+date
uint8_t TimeDayOfWeek(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
) {

	uint16_t vi_Year;
	vi_Year = 2000+(uint16_t)pi_YearXX;
	if (pi_Month < 3) {
		pi_Month += 12;
		vi_Year --;
	}

	return (
			(uint16_t)pi_Day
		+ (2 * (uint16_t)pi_Month)
		+ (6 * ((uint16_t)pi_Month + 1) / 10)
		+  vi_Year
		+ (vi_Year / 4)
		- (vi_Year / 100)
		+ (vi_Year / 400)
		+ 1
	) % 7;
} //TimeDayOfWeek


//*********************************
//zwróæ ostatni dzieñ danego miesi¹ca
uint8_t TimeLastDay(
  uint8_t pi_Month
 ,uint8_t pi_YearXX
) {
	switch (pi_Month) {
		case 2: if (pi_YearXX%4) return 28; else return 29;
		case 4: case 6: case 9: case 11: return 30;
		default: return 31;
	}
} //TimeLastDay


//*********************************
//zwróæ porê roku (0:zima 1:wiosna 2:lato 3:jesieñ 4:zima 22.12-31.12)
uint8_t TimeSeason(
  uint8_t pi_Day
 ,uint8_t pi_Month
 ,uint8_t pi_YearXX
) {
	if (pi_Month%3)
		//1,2, 4,5, 7,8 10,11
		return pi_Month/3;
	switch (pi_Month) {
		case 3:	return pi_Day<21?0:1;
		case 6: return pi_Day<22?1:2;
		case 9: return pi_Day<23?2:3;
		default: case 12: return pi_Day<22?3:4;
	}
} //TimeSeason


//*********************************
//zwróæ pierwszy dzieñ pory roku
void TimeSeasonFirstDay(
  uint8_t pt_Time[] //in-out
) {
	uint8_t vi_Season = TimeSeason(pt_Time[TDay],pt_Time[TMonth],pt_Time[TYear]);
	switch (vi_Season) {
		case 1: pt_Time[TMonth] = 3; pt_Time[TDay] = 21; break;
		case 2: pt_Time[TMonth] = 6; pt_Time[TDay] = 22; break;
		case 3: pt_Time[TMonth] = 9; pt_Time[TDay] = 23; break;
		default: case 0: case 4: if (!vi_Season) pt_Time[TYear]--; pt_Time[TMonth] = 12; pt_Time[TDay] = 22; break;
	}
} //TimeSeasonFirstDay


//*********************************
//porównaj dwa czasy -1: pierwszy mniejszy 0: równe +1:pierwszy wiêkszy
int8_t TimeCompare(
  uint8_t *pt_FirstTime //[0]:hour [1]:min [2]:sec
 ,uint8_t *pt_LastTime //[0]:hour [1]:min [2]:sec
 ,uint8_t pi_Bytes //1: tylko godziny, 2: godziny i minuty, 3: godziny, minuty, sekundy
) {
	uint8_t vi_Idx;
	for (vi_Idx=0; vi_Idx<pi_Bytes; vi_Idx++)
		if (pt_FirstTime[vi_Idx]<pt_LastTime[vi_Idx])
			return -1;
		else if (pt_FirstTime[vi_Idx]>pt_LastTime[vi_Idx])
			return +1;
	return 0;
} //TimeCompare


//*********************************
//zwróæ iloœæ sekund pomiêdzy dwoma czasami
int32_t TimeSecBetween(
  uint8_t *pt_FirstTime //[0]:hour [1]:min [2]:sec
 ,uint8_t *pt_LastTime //[0]:hour [1]:min [2]:sec
 ,uint8_t pi_Bytes //1: tylko godziny, 2: godziny i minuty, 3: godziny, minuty, sekundy
) {
	int32_t vi_Result;
	vi_Result = ((int32_t)pt_LastTime[0]-(int32_t)pt_FirstTime[0])*3600L;
	if (pi_Bytes>=2)
		vi_Result += ((int32_t)pt_LastTime[1]-(int32_t)pt_FirstTime[1])*60L;
	if (pi_Bytes>=3)
		vi_Result += ((int32_t)pt_LastTime[2]-(int32_t)pt_FirstTime[2]);
	return vi_Result>=0 ? vi_Result : vi_Result+86400L;
} //TimeSecBetween


//==============================================================
// PRIVATE PROCEDURES
//==============================================================


//*********************************
//licz subcykle (dla SUBINT_COUNT=1 sklejone z korekcj¹ prêdkoœci zegara)
SIGNAL(TIMER1_COMPA_vect)
{
#if SUBINT_COUNT>1
	if ((++gi_IntSubHthSec)==SUBINT_COUNT) gi_IntSubHthSec = 0;
	if (gi_IntSubHthSec<=1)
		TIMSK |= _BV(OCIE1B);
	else if (gi_IntSubHthSec==(SUBINT_COUNT+1)/2)
		TIMSK |= _BV(TOIE1);
#else
	TIMSK |= _BV(TOIE1);
#endif
#if SUBINT_COUNT>1
} //SIGNAL(TIMER1_COMPA_vect)


//*********************************
//zmieniaj d³ugoœæ licznika - korekcja prêdkoœci zegara (2 razy w ka¿dej 1/100sec)
SIGNAL(TIMER1_COMPB_vect)
{
	TIMSK &= ~_BV(OCIE1B); //wy³¹cz to przerwanie - w³¹czane w OVF
#endif
	#if T1_SIZE_1BYTE
		#warning D³ugoœæ Timer1 w jednym bajcie
		typedef union{uint8_t W; struct {uint8_t L;} B;} WordOrByte;
	#else
		#warning D³ugoœæ Timer1 w dwóch bajtach
		typedef union {uint16_t W; struct {uint8_t L,H;} B;} WordOrByte;
	#endif
	typedef union {uint16_t W; struct {uint8_t L,H;} B;} Word;
	WordOrByte vi_Size = {W:0};

#if SUBINT_COUNT>1
	if (gi_IntSubHthSec)
		vi_Size.W = T1_SIZE/SUBINT_COUNT-1;
	else
#endif
	if (gi_TCorSign) {
		//koryguj czas
		gi_TDayTicks += TCorTickSize;
		if (gi_TDayTicks>=gi_TCorNextTick) {
			//zosta³ osi¹gniêty czas korekcji
			gi_TCorNextTick += gi_TCorTickOffs;
			if (gi_TCorNextTick < gi_TCorTickOffs)
				gi_TCorNextTick = 0xFFFFFFFF;

			//zwolnij/przyspiesz o 1/n setnej sekundy na n najbli¿szych wywo³añ czyli skoryguj o 1/100 sec przez n wywo³añ
			gi_IntCorHthSecCnt = TCorTimerDiv;
		}
		if (gi_IntCorHthSecCnt) {
			if (--gi_IntCorHthSecCnt) {
				if (gi_TCorSign<0)
					vi_Size.W = (T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_MANY-1; //ZWIÊKSZ rozmiar licznika=>ZWOLNIJ
				else
					vi_Size.W = (T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_MANY-1; //ZMNIEJSZ rozmiar licznika=>PRZYSPIESZ
			}
			else {
				if (gi_TCorSign<0)
					vi_Size.W = (T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)+T1_COR_ONE-1; //ZWIÊKSZ rozmiar licznika=>ZWOLNIJ
				else
					vi_Size.W = (T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-T1_COR_ONE-1; //ZMNIEJSZ rozmiar licznika=>PRZYSPIESZ
			}
		}
	}

	if (!vi_Size.W)
		vi_Size.W = (T1_SIZE/SUBINT_COUNT+T1_SIZE%SUBINT_COUNT)-1; //normalna prêdkoœæ

#if WITH_OCR1B
	//ustaw PWM na pinie OC1B
	if (gb_OCR1BMode) {
		uint8_t vi_PWM;
		if (gb_OCR1BMode==1) vi_PWM = gi_OCR1BPWM8curr;
		else vi_PWM = 0xFF-gi_OCR1BPWM8curr;

		//wersja bardzo wolna dok³adna 692ck
//		OCR1B = DivUL((uint32_t)vi_Size.W*vi_PWM,0xFF);;

		//wersja size*pwm/255 - wolna dok³adna 444ck
//		Word vi_B12,vi_B0,vi_Out;
//		vi_B0.W = vi_Size.B.L*vi_PWM;
//		vi_B12.W = vi_Size.B.H*vi_PWM;
//		vi_B0.W += 0x7F;
//		vi_B12.W += vi_B0.B.H;
//		vi_Out.B.H = vi_B12.W/0xFF;
//		vi_B0.B.H = vi_B12.W-vi_Out.B.H*0xFF;
//		vi_Out.B.L = vi_B0.W/0xFF;
//		OCR1B = vi_Out.W;

		//wersja size*pwm/256 - bardzo szybka prawie dok³adna - 10ck/39ck lub 10ck/45ck
		Word vi_B0;
		vi_B0.W = vi_Size.B.L*vi_PWM;
		vi_B0.W += 0xFF;
		#if T1_SIZE_1BYTE
			OCR1B = vi_B0.B.H;
		#else
			if (vi_PWM&0x80) vi_B0.B.H += vi_Size.B.H; //ta linijka zwiêksza dok³adnoœæ i wyd³u¿a czas do 45ck
			#if T1_SIZE_1BYTE
				OCR1BL = vi_Size.B.H*vi_PWM+vi_B0.B.H;
			#else
				OCR1B = vi_Size.B.H*vi_PWM+vi_B0.B.H;
			#endif
		#endif
	}
	else
		#if T1_SIZE_1BYTE
			OCR1BL = 0;
		#else
			OCR1B = 0;
		#endif
#endif
	#if T1_SIZE_1BYTE
		OCR1AL = vi_Size.W;
	#else
		OCR1A = vi_Size.W;
	#endif

#if WITH_OCR1B
		//zmieniaj wype³nienie OCR1B powoli co 1/100sec o 2/256 -> 1.3sec od 0 do max
		if (gi_OCR1BPWM8curr<gi_OCR1BPWM8dest) gi_OCR1BPWM8curr++;
		if (gi_OCR1BPWM8curr>gi_OCR1BPWM8dest) gi_OCR1BPWM8curr--;
#endif


} //SIGNAL(TIMER1_COMPB_vect)


//*********************************
//zwiêkszaj czas w przerwaniu, wywo³uj procedury, aktualizuj liczniki - co 1/100 sec
SIGNAL(TIMER1_OVF_vect)
{
	TIMSK &= ~_BV(TOIE1); //wy³¹cz to przerwanie - w³¹czane w OVF

	gi_IntRecurrLevel++;

	if ((++gt_Time[THthSec])==100) {
		gt_Time[THthSec] = 0;
		TimeChangeInt(TSec,1);
		if ((!gt_Time[TSec]) && (!gt_Time[TMin])) {
			//pe³na godzina
			if (gi_THoursFromChange<0xFFFF)
				gi_THoursFromChange ++;
			if (!gt_Time[THour]) {
				//pó³noc
				gi_TDayTicks    = 0;
				gi_TCorNextTick = gi_TCorTickOffs-TCorTickSize*100;
			}
		}
	}
//	if (!(gt_Time[THthSec]%50))	gt_TPrint[TPRINT_HALFSEC] = 1;

	//zwiêkszaj liczniki
	uint8_t vi_Cnt;
	vi_Cnt = 0;
	while ((vi_Cnt<THth8CountersCnt) && gt_THthSecCnt8[vi_Cnt]) {
		(*gt_THthSecCnt8[vi_Cnt])++;
		vi_Cnt++;
	}
	vi_Cnt = 0;
	while ((vi_Cnt<THth16CountersCnt) && gt_THthSecCnt16[vi_Cnt]) {
		(*gt_THthSecCnt16[vi_Cnt])++;
		vi_Cnt++;
	}

	if (gi_IntRecurrLevel<3) {
		sei(); //umo¿liw wchodzenie rekurencyjne - ale tylko 2 razy (d³ugotrwa³o trwaj¹ce procedury ale rzadko nie blokuj¹ przerwania czasowego)
		//wywo³uj procedury
		vi_Cnt = 0;
		while ((vi_Cnt<THthProceduresCnt) && gt_THthProcedures[vi_Cnt]) {
			gt_THthProcedures[vi_Cnt]();
			vi_Cnt++;
		}
		cli();
	}

	gi_IntRecurrLevel--;

} //SIGNAL(TIMER1_OVF_vect)


//*********************************
//zmieñ podan¹ jednostkê czasu o okreœlon¹ wartoœæ
void TimeChangeInt(
  uint8_t pi_What
 ,int8_t  pi_ChangeValue
) {

	int8_t  vi_NextChangeValue;
	uint8_t vi_Size;
	uint8_t vi_sreg = SREG;

	cli(); //zablokuj przerwania na czas modyfikacji gt_Time

	gt_Time[pi_What] += pi_ChangeValue;
	switch (pi_What) {
		case THthSec: vi_Size = 100; break;
		case TSec: gt_TPrint[TPRINT_SEC] = 1; vi_Size = 60; break;
		case TMin: gt_TPrint[TPRINT_MIN] = 1; vi_Size = 60; break;
		case THour: gt_TPrint[TPRINT_HOUR] = 1; vi_Size = 24; break;
		case TDay:
			gt_TPrint[TPRINT_DAY] = 1;
			vi_Size = TimeLastDay(gt_Time[TMonth],gt_Time[TYear]);
			break;
		default: case TMonth: vi_Size = 12; break;
	}

	vi_NextChangeValue = 0;
	if (pi_What>=THour) {
		if ((int8_t)gt_Time[pi_What]<0) {
			gt_Time[pi_What] += vi_Size;
			vi_NextChangeValue = -1;
		}
		else if (gt_Time[pi_What]>=vi_Size) {
			gt_Time[pi_What] -= vi_Size;
			vi_NextChangeValue = 1;
		}
	}
	else if (pi_What != TYear) {
		if (gt_Time[pi_What]>vi_Size) {
			gt_Time[pi_What] = 1;
			vi_NextChangeValue = 1;
		}
		else if (!gt_Time[pi_What]) {
			vi_NextChangeValue = -1;
			if (pi_What == TDay)
				gt_Time[pi_What] = TimeLastDay(gt_Time[TMonth]-1,gt_Time[TYear]);
			else
				gt_Time[pi_What] = vi_Size;
		}
	}


	sei(); //odblokuj przerwania na czas modyfikacji reszty oraz zapisu EEPROM
	if (vi_NextChangeValue)
		TimeChangeInt(pi_What-1,vi_NextChangeValue);
	else if (pi_What<=TDay)
		gt_Time[TWeekDay] = TimeDayOfWeek(gt_Time[TDay],gt_Time[TMonth],gt_Time[TYear]);

	if (pi_What<=TDay)
		SetEETime(pi_What)
	else if (pi_What==THour) {
		if (pi_ChangeValue<0)
			SetEEAllHour();
		else
			SetEEHour();
		if (    pi_ChangeValue==1 && gt_Time[THour]==2
		    &&  gt_Time[TWeekDay]==0 && gt_Time[TDay]>=25
		    && (   (gt_Time[TMonth]==3 && !gb_DaylightSavingTime)
		        || (gt_Time[TMonth]==10 && gb_DaylightSavingTime)
	    	   )
		   )
			//o godzinie 2giej ostatniej niedzieli marca lub paŸdziernika zmieñ czas letni/zimowy
			TimeChangeDaylightSavingTimeInt();
	}


	SREG = vi_sreg;

} //TimeChangeInt


//*********************************
//zmieñ czas letni/zimowy
void TimeChangeDaylightSavingTimeInt(void)
{
	eeprom_write_byte(&gb_EEDaylightSavingTime,!gb_DaylightSavingTime);
	TimeChangeInt(THour,gb_DaylightSavingTime?+1:-1);
} //TimeSetDaylightSavingTime
