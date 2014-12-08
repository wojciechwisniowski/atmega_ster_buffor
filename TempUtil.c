#include "TempUtil.h"

#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "MyMath.h"
#include "ds18x20.h"
#include "Time.h"
#include "AdvancedLCD.h"
#include "Charset.h"
#include "Button.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define TEMP_NONE16 ((int16_t)0x8000)
#define TEMP_NONE8 ((int8_t)0x80)

#define TB TempBuff

#define TBAddr(pi_BuffType) ((int8_t *)pgm_read_word(&ct_TempBuffsData[pi_BuffType].Addr))
#define TBAddrFromIdx(pi_BuffType,pi_Idx) (TBAddr(pi_BuffType)+(uint16_t)(pi_Idx)*TB(pi_BuffType,SubSize))

TEMP_DATA;

TEMP_BUFFDATA;

uint8_t gt_CurrTime[TWeekDay+1]; //bie¿¹cy czas zapamiêtywany bezpoœrednio przed odczytem termometrów

#if TEMP_USE_BUFF

uint8_t gb_TempBuffsEEInitialized EEMEM = 0; //czy tablice EEPROM zainicjalizowane?
uint8_t gt_Temp1dLastDate[TDay+2]; //ostatnia data u¿yta do obliczenia indeksu Temps1D

int16_t gt_TempBuffCurrCache[TEMP_MAX+1];
#define TEMP_BCM_CALCULATE 0 //oblicz z mniejszych buforów
#define TEMP_BCM_READFROMEE 1 //odczytaj z EEPROM
#define TEMP_BCM_READFROMCACHE 2 //odczytaj z cache gt_CurrTemps
uint8_t gi_TempBuffCurrMode;
#endif //TEMP_USE_BUFF


#define TEMP_MAX_ERRORS (1000/TEMP_READ_CYCLE)
#define TEMP_STARTMEAS_HthIdx ((TEMP_READ_CYCLE+(TEMP_BUFFCOUNT+1)*TEMP_READ_SUBCYCLE-89)%TEMP_READ_CYCLE)
#if TEMP_READ_CYCLE>256
/*volatile*/ uint16_t gi_TempHthSecCounter = TEMP_STARTMEAS_HthIdx-1; //licznik do procedury TempExecuteHthSec
#else
/*volatile*/ uint8_t gi_TempHthSecCounter = TEMP_STARTMEAS_HthIdx-1; //licznik do procedury TempExecuteHthSec
#endif
/*volatile*/ uint8_t gb_BuffModification; //czy jesteœmy w trakcie modyfikacji buforów przez przerwanie

//==============================================================
// FORWARD DECLARATIONS
//==============================================================

//zapamiêtaj bie¿¹cy czas do gt_CurrTime
void TempStoreCurrTime(void);

//zwróæ wartoœæ indeksu do tabelek z temperaturami na podstawie bie¿¹cego czasu gt_Time
uint8_t TempBuffCurrIdx(uint8_t pi_BuffType);

#if TEMP_USE_BUFF
//zwróæ wartoœæ temperatury jako obliczenie z buforów z mniejsz¹ kompresj¹ czasow¹
void TempBuffGetCurr(uint8_t pi_BuffType,int16_t pt_Temps[]);
//zapisz temperaturê do bufora (konwertuj z int16_t temp/256 do int8_t temp/2)
void TempBuffWrite(uint8_t pi_TempType,uint8_t pi_BuffIdx,int16_t pt_Temp[]) __attribute__((always_inline));
//odczytaj temperaturê z bufora - wersja zwracaj¹ca int16_t, temperatura*256
void TempBuffRead(uint8_t pi_BuffType,uint8_t pi_BuffIdx,int16_t pt_Temps[]);
#endif //TEMP_USE_BUFF

//odczytuj temperatury w przerwaniu z time.h
void TempExecuteHthSec(void);
//odczytaj temperatury
void TempReadMeas(uint8_t pi_Sensor) __attribute__((always_inline));
//zapamiêtaj temperatury w buforach
void TempBuffStore(uint8_t pi_BuffType) __attribute__((always_inline));

//czyszczenie pamiêci
void MemClear_I16(int16_t *px_Addr,uint16_t pi_Size) __attribute__((always_inline));
void MemClear_I16(int16_t *px_Addr,uint16_t pi_Size) {while (pi_Size--) *px_Addr++ = TEMP_NONE16;}
void MemClear_I8(int8_t *px_Addr,uint16_t pi_Size) __attribute__((always_inline));
void MemClear_I8(int8_t *px_Addr,uint16_t pi_Size) {while (pi_Size--) *px_Addr++ = TEMP_NONE8;}
#if TEMP_USE_BUFF
//void EEMemClear_I8(int8_t *px_Addr,uint16_t pi_Size) __attribute__((always_inline));
void EEMemClear_I8(int8_t *px_Addr,uint16_t pi_Size) {while (pi_Size--) eeprom_write_byte((uint8_t *)px_Addr++,TEMP_NONE8);}
#endif //TEMP_USE_BUFF

//normalizowanie indeksów do zakresu (0..pi_Size-1)
//uint8_t TruncIdx16(int16_t pi_Index,uint8_t pi_Size) __attribute__((always_inline));
uint8_t TruncIdx16(int16_t pi_Index,uint8_t pi_Size) {pi_Index %= pi_Size; if (pi_Index<0) pi_Index += pi_Size; return pi_Index;}
uint8_t TruncIdx8(uint8_t pi_Index,uint8_t pi_Size) __attribute__((always_inline));
uint8_t TruncIdx8(uint8_t pi_Index,uint8_t pi_Size) {return pi_Index%pi_Size;}


//==============================================================
// PUBLIC PROCEDURES
//==============================================================
//int16_t gt_TempDebug[TEMP_MAX+1];
//*********************************
//inicjalizuj modu³
void TempInit(void) {
	uint8_t vi_Idx;
	
	TempStoreCurrTime();

	for (vi_Idx=0; vi_Idx<TEMP_COUNT; vi_Idx++) {
		MemClear_I16(gt_TempsVar[vi_Idx].Buff12m,TEMP_12MSIZE);
		gt_TempsVar[vi_Idx].ErrorCount = TEMP_MAX_ERRORS;
	}
	MemClear_I16(gt_TempBuff12m,TB(TEMP_BUFF12m,Size));

	//ustaw indeksy do buforów temperatur
	for (vi_Idx=0; vi_Idx<=TEMP_BUFFCOUNT; vi_Idx++)
		gt_TempBuffsIdx[vi_Idx] = TempBuffCurrIdx(vi_Idx);

#if TEMP_USE_BUFF
	//wyczyœæ wszystkie bufory EEPROM
	for (vi_Idx=1; vi_Idx<TEMP_BUFFCOUNT; vi_Idx++)
		if (TB(vi_Idx,EEMem))
			if (eeprom_read_byte(&gb_TempBuffsEEInitialized)!=1)
				EEMemClear_I8(TBAddr(vi_Idx),TB(vi_Idx,Size)*TB(vi_Idx,SubSize));
			else ;
		else
			MemClear_I8(TBAddr(vi_Idx),TB(vi_Idx,Size)*TB(vi_Idx,SubSize));
	eeprom_write_byte(&gb_TempBuffsEEInitialized,1);
#endif //TEMP_USE_BUFF

#ifdef TEMP_BUFF1D
	//ustaw w temps12m œredni¹/min/max zapisane w gt_Temps1D
	int16_t vt_Temps[TEMP_MAX+1];
	TempBuffRead(TEMP_BUFF1D,gt_TempBuffsIdx[TEMP_BUFF1D],vt_Temps);
	for (vi_Idx=0; vi_Idx<5*24; vi_Idx++)
		gt_TempBuff12m[vi_Idx] = vt_Temps[TEMP_AVG];
	gt_TempBuff12m[1] = vt_Temps[TEMP_MIN];
	gt_TempBuff12m[2] = vt_Temps[TEMP_MAX];
#endif //TEMP_BUFF1D

#if TEMP_USE_BUFF
	gt_Temp1dLastDate[TYear]  = gt_CurrTime[TYear];
	gt_Temp1dLastDate[TMonth] = gt_CurrTime[TMonth];
	gt_Temp1dLastDate[TDay]   = gt_CurrTime[TDay];
	gt_Temp1dLastDate[TDay+1] = gt_CurrTime[TWeekDay];
#endif //TEMP_USE_BUFF

/*	//uzupe³nianie bufora 1W z bufora 1D
	gi_TempBuffCurrMode = TEMP_BCM_READFROMEE;
	gt_TimeCheck[TDay] = 22;
	gt_TimeCheck[TWeekDay] = 0;
	gt_Temp1dLastDate[TDay] = 22;
	gt_Temp1dLastDate[TDay+1] = 0;
	int16_t vt_Temps[10];
	for (vi_Idx=0; vi_Idx<10; vi_Idx++) {
		TempBuffGetCurr(TEMP_BUFF1W,vt_Temps);
		TempBuffWrite(TEMP_BUFF1W,gt_TempBuffsIdx[TEMP_BUFF1W],vt_Temps);
		TimeGetDateFromDayNo(TimeGetDayNo(gt_TimeCheck[TDay],gt_TimeCheck[TMonth],gt_TimeCheck[TYear])-7,gt_Temp1dLastDate);
		gt_TimeCheck[TMonth] = gt_Temp1dLastDate[TMonth];
		gt_TimeCheck[TDay] = gt_Temp1dLastDate[TDay];
		gt_TempBuffsIdx[TEMP_BUFF1D] = TempBuffCurrIdx(TEMP_BUFF1D);
		gt_TempBuffsIdx[TEMP_BUFF1W] = TempBuffCurrIdx(TEMP_BUFF1W);
	}
	while (1);*/

	TimeAddHthProcedure(&TempExecuteHthSec);

} //TempInit


//*********************************
//pobierz temperaturê z bie¿¹cych odczytów
int16_t TempGet(
  uint8_t pi_Sensor   //numer termometru
 ,uint8_t pi_TempType //TEMP_TICK/TEMP_CURR/TEMP_MIN/TEMP_AVG/TEMP_MAX
) {
	int16_t vt_Temps[TEMP_MAX+1];

	if (pi_TempType&0x10) {
		uint8_t vi_sreg = SREG;
		cli();
		if (pi_TempType==TEMP_TICK)
			vt_Temps[0] = gt_TempsVar[pi_Sensor].Tick.I12.I12;
		else
			vt_Temps[0] = gt_TempsVar[pi_Sensor].Curr.I12.I12;
		SREG = vi_sreg;
		pi_TempType = 0;
	}
	else
		TempBuffGet(TEMP_BUFF12m,pi_Sensor,0,(pi_Sensor==TEMP_FULLNO)?TB(TEMP_BUFF12m,Size)-1:TEMP_12MSIZE-1,vt_Temps,1);

	return vt_Temps[pi_TempType];
} //TempGet


//*********************************
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
) {

	int16_t  vt_Temps[TEMP_MAX+1];
	int32_t  vi_TempAVGSum;
	uint8_t  vi_TempCount;
	uint8_t  vi_Buff8Idx,vi_BuffSize,vi_CurrBuffIdx;
	uint16_t vi_BuffIdx,vi_LastIdx;
	int16_t *vt_TempsBuff;
	uint8_t vi_MinTempType,vi_MaxTempType;
	if (pi_Type) {
		vi_MinTempType = TEMP_MIN;
		vi_MaxTempType = TEMP_MAX;
	}
	else {
		vi_MinTempType = TEMP_AVG;
		vi_MaxTempType = TEMP_AVG;
	}

	pt_Temps[TEMP_MIN] = 0x7FFF;
	pt_Temps[TEMP_MAX] = 0x8000;
	vi_TempAVGSum  = 0;
	vi_TempCount   = 0;
	vt_TempsBuff   = 0;
	vi_CurrBuffIdx = gt_TempBuffsIdx[pi_BuffType];

	if (pi_BuffType) {
		vi_BuffSize    = TB(pi_BuffType,Size);
	}
	else if (pi_Sensor==TEMP_FULLNO) {
		vt_TempsBuff = gt_TempBuff12m;
		vi_BuffSize  = TB(TEMP_BUFF12m,Size);
	}
	else {
		vt_TempsBuff = gt_TempsVar[pi_Sensor].Buff12m;
		vi_BuffSize  = TEMP_12MSIZE;
	}

	vi_LastIdx = pi_LastIdx;
	if (vi_LastIdx<pi_FirstIdx)
		vi_LastIdx += TB(pi_BuffType,Size);

	for (vi_BuffIdx = pi_FirstIdx; vi_BuffIdx<=vi_LastIdx; vi_BuffIdx++) {

		vi_Buff8Idx = vi_BuffIdx%vi_BuffSize;
		if (pi_BuffType==TEMP_BUFF24m) {
			uint8_t vi_Buff8Idx2 = vi_Buff8Idx*2;
			TempBuffGet(TEMP_BUFF12m,pi_Sensor,vi_Buff8Idx2,
			  (vi_Buff8Idx!=gt_TempBuffsIdx[TEMP_BUFF24m] || vi_Buff8Idx2<gt_TempBuffsIdx[TEMP_BUFF12m])?vi_Buff8Idx2+1:vi_Buff8Idx2,
			  vt_Temps,0);
			vt_Temps[TEMP_MIN] = vt_Temps[TEMP_AVG];
			vt_Temps[TEMP_MAX] = vt_Temps[TEMP_AVG];
		}
		else
#if TEMP_USE_BUFF
		if (vt_TempsBuff)
#endif //TEMP_USE_BUFF
		{
			uint8_t vi_sreg = SREG;
			cli();
			vt_Temps[TEMP_MIN] = vt_TempsBuff[vi_Buff8Idx];
			SREG = vi_sreg;
			vt_Temps[TEMP_AVG] = vt_Temps[TEMP_MIN];
			vt_Temps[TEMP_MAX] = vt_Temps[TEMP_MIN];
		}
#if TEMP_USE_BUFF
		else
			if (vi_Buff8Idx == vi_CurrBuffIdx && gi_TempBuffCurrMode!=TEMP_BCM_READFROMEE)
				if (gi_TempBuffCurrMode==TEMP_BCM_READFROMCACHE)
					for (uint8_t vi_Idx=TEMP_MIN; vi_Idx<=TEMP_MAX; vi_Idx++)
						vt_Temps[vi_Idx] = gt_TempBuffCurrCache[vi_Idx];
				else
					TempBuffGetCurr(pi_BuffType,vt_Temps);
			else
				TempBuffRead(pi_BuffType,vi_Buff8Idx,vt_Temps);
#endif //TEMP_USE_BUFF

		if (vt_Temps[TEMP_MIN]!=TEMP_NONE16) {
			vi_TempCount++;
			if (vt_Temps[vi_MinTempType]<pt_Temps[TEMP_MIN]) pt_Temps[TEMP_MIN] = vt_Temps[vi_MinTempType];
			vi_TempAVGSum += vt_Temps[TEMP_AVG];
			if (vt_Temps[vi_MaxTempType]>pt_Temps[TEMP_MAX]) pt_Temps[TEMP_MAX] = vt_Temps[vi_MaxTempType];
		}
	}

	if (vi_TempCount)
		pt_Temps[TEMP_AVG] = DivL(vi_TempAVGSum,vi_TempCount);
	else
		MemClear_I16(&pt_Temps[TEMP_MIN],3);

} //TempBuffGet


//*********************************
//konwertuj temperaturê do tekstu
char *TempConvert(
  int16_t pi_Temp    //temperatura do konwersji z dok³adnoœci¹ do 1/256'C
 ,uint8_t pb_WithStC //czy dokleiæ na koniec °C
) {
	int16_t vi_Temp;
	uint8_t vv_Sign;
	char *vv_Text;

	if (pi_Temp==TEMP_NONE16) {
		gv_TempText[0] = '-';
		gv_TempText[1] = '-';
		gv_TempText[2] = '\0';
	}
	else {

		#if TEMPCONV_FRACTDIGITS==1
			vi_Temp = DivL((int32_t)pi_Temp*10,0x100);
		#else
			vi_Temp = DivL((int32_t)pi_Temp*100,0x100);
		#endif
		if (vi_Temp<0) {
			vv_Sign = '-';
			vi_Temp = -vi_Temp;
		}
		else if (vi_Temp>0)
			vv_Sign = '+';
		else
			#if TEMPCONV_ALWAYSSIGN
				vv_Sign = ' ';
			#else
				vv_Sign = '\x0';
			#endif

		vv_Text = gv_TempText;
		if (vv_Sign) *vv_Text++ = vv_Sign;
		#if TEMPCONV_FRACTDIGITS==1
			if (vi_Temp>99) {
				*vv_Text++ = '0'+(vi_Temp/100);
				vi_Temp %= 100;
			}
			*vv_Text++ = '0'+(vi_Temp/10);
			*vv_Text++ = ',';
			*vv_Text++ = '0'+vi_Temp%10;
		#else
			if (vi_Temp>999) {
				*vv_Text++ = '0'+(vi_Temp/1000);
				vi_Temp %= 1000;
			}
			*vv_Text++ = '0'+(vi_Temp/100);
			vi_Temp %= 100;
			*vv_Text++ = ',';
			*vv_Text++ = '0'+(vi_Temp/10);
			*vv_Text++ = '0'+vi_Temp%10;
		#endif
		if (pb_WithStC) {
			*vv_Text++ = '°';
			*vv_Text++ = 'C';
		}
		*vv_Text = 0;
	}

	return gv_TempText;

} //TempConvert


//==============================================================
// PRIVATE PROCEDURES
//==============================================================


//*********************************
//zapamiêtaj bie¿¹cy czas do gt_CurrTime
void TempStoreCurrTime(void) {
	uint8_t vi_sreg = SREG;
	cli();
	for (uint8_t vi_Idx=0; vi_Idx<=TWeekDay; vi_Idx++)
		gt_CurrTime[vi_Idx] = gt_Time[vi_Idx];
	SREG = vi_sreg;
} //TempStoreCurrTime


//*********************************
//zwróæ wartoœæ indeksu do tabelek z temperaturami na podstawie bie¿¹cego czasu gt_Time
uint8_t TempBuffCurrIdx(uint8_t pi_BuffType) {
	uint16_t vi_Idx;
#if TEMP_USE_BUFF
	uint16_t vi_DayNo = TimeGetDayNo(gt_CurrTime[TDay],gt_CurrTime[TMonth],gt_CurrTime[TYear]);
#endif
	switch (pi_BuffType) {
		case TEMP_BUFF12m: vi_Idx = (gt_CurrTime[THour]*5)+(gt_CurrTime[TMin]/12); break;
#ifdef TEMP_BUFF1h5
		case TEMP_BUFF1h5: vi_Idx = gt_CurrTime[TWeekDay]*16+(gt_CurrTime[THour]*2+gt_CurrTime[TMin]/30)/3; break;
#endif
#ifdef TEMP_BUFF6h
		case TEMP_BUFF6h: vi_Idx = vi_DayNo*4+gt_CurrTime[THour]/4; break;
#endif
#ifdef TEMP_BUFF1D
		case TEMP_BUFF1D: vi_Idx = vi_DayNo; break;
#endif
#ifdef TEMP_BUFF1W
		case TEMP_BUFF1W: vi_Idx = (vi_DayNo-2/*TimeGetDayNo(3,1,00)*//*poniedzia³ek*/)/7; break;
#endif
#ifdef TEMP_BUFF1M
		case TEMP_BUFF1M: vi_Idx = gt_CurrTime[TYear]*12+gt_CurrTime[TMonth]; break;
#endif
#ifdef TEMP_BUFF3M
		case TEMP_BUFF3M: vi_Idx = gt_CurrTime[TYear]*4+TimeSeason(gt_CurrTime[TDay],gt_CurrTime[TMonth],gt_CurrTime[TYear]); break;
#endif
		default: case TEMP_BUFF24m: vi_Idx = ((gt_CurrTime[THour]*5)+(gt_CurrTime[TMin]/12))/2; break;
	}
	return TruncIdx16(vi_Idx,TB(pi_BuffType,Size));
} //TempBuffCurrIdx


#if TEMP_USE_BUFF
//*********************************
//zwróæ wartoœæ temperatury jako obliczenie z buforów z mniejsz¹ kompresj¹ czasow¹
void TempBuffGetCurr(
  uint8_t pi_BuffType //>12m
 ,int16_t pt_Temps[]
) {
	uint8_t vi_SubBuffType;
	int16_t vi_FirstIdx;
	uint8_t vi_Idx;

	switch (pi_BuffType) {
#ifdef TEMP_BUFF1h5
		case TEMP_BUFF1h5: vi_SubBuffType = TEMP_BUFF12m; vi_FirstIdx = ((gt_TempBuffsIdx[TEMP_BUFF12m]*2)-(gt_TempBuffsIdx[TEMP_BUFF12m]*2)%15)/2; break;
#endif
#ifdef TEMP_BUFF6h
		case TEMP_BUFF6h: vi_SubBuffType = TEMP_BUFF12m; vi_FirstIdx = gt_TempBuffsIdx[TEMP_BUFF12m]-gt_TempBuffsIdx[TEMP_BUFF12m]%30; break;
#endif
#ifdef TEMP_BUFF1D
		case TEMP_BUFF1D: vi_SubBuffType = TEMP_BUFF12m; vi_FirstIdx = 0; break;
#endif
#ifdef TEMP_BUFF1W
		case TEMP_BUFF1W: vi_SubBuffType = TEMP_BUFF1D; vi_FirstIdx = gt_TempBuffsIdx[TEMP_BUFF1D]-(gt_Temp1dLastDate[TDay+1]+6)%7; break;
#endif
#ifdef TEMP_BUFF1M
		case TEMP_BUFF1M: vi_SubBuffType = TEMP_BUFF1D; vi_FirstIdx = TimeGetDayNo(1,gt_Temp1dLastDate[TMonth],gt_Temp1dLastDate[TYear]); break;
#endif
#ifdef TEMP_BUFF3M
		case TEMP_BUFF3M:
			vi_SubBuffType = TEMP_BUFF1D;
			uint8_t vt_Time[TDay+1];
			vt_Time[TYear]  = gt_Temp1dLastDate[TYear];
			vt_Time[TMonth] = gt_Temp1dLastDate[TMonth];
			vt_Time[TDay]   = gt_Temp1dLastDate[TDay];
			TimeSeasonFirstDay(vt_Time);
			vi_FirstIdx = TimeGetDayNo(vt_Time[TDay],vt_Time[TMonth],vt_Time[TYear]);
			break;
#endif
		default: vi_SubBuffType = 0; vi_FirstIdx = 0; break;
	}

	TempBuffGet(vi_SubBuffType,TEMP_FULLNO,TruncIdx16(vi_FirstIdx,TB(vi_SubBuffType,Size)),gt_TempBuffsIdx[vi_SubBuffType],pt_Temps,1);
	if (TB(pi_BuffType,SubSize)==1) {
		pt_Temps[TEMP_MIN] = pt_Temps[TEMP_AVG];
		pt_Temps[TEMP_MAX] = pt_Temps[TEMP_AVG];
	}
	for (vi_Idx=TEMP_MIN; vi_Idx<=TEMP_MAX; vi_Idx++)
		gt_TempBuffCurrCache[vi_Idx] = pt_Temps[vi_Idx];

} //TempBuffGetCurr


//temperatury w EEPROM zapamiêtane w 1bajt wed³ug tabeli (dla ujemnych identycznie):
//rozdz minT maxT minV maxV Mul  Div
// 0,1     0    5    0   50 100 2560
// 0,25    5   15   50   90  40 2560
// 0,5    15   25   90  110  20 2560
// 1      25   34  110  119  10 2560
// 2      34   50  119  127   5 2560
#define TempEERangesCount 5
struct {
	uint8_t vi_Mul,vi_MaxTemp,vi_MaxValue;
} ct_TempEERanges[TempEERangesCount] PROGMEM = {
  {100, 5,50}
 ,{ 40,15,90}
 ,{ 20,25,110}
 ,{ 10,34,119}
 ,{  5,50,127}
};
#define TempEERange(pi_Idx,pv_Name) (pgm_read_byte(&ct_TempEERanges[pi_Idx].pv_Name))

//*********************************
//zapisz temperaturê do bufora (konwertuj z int16_t temp/256 do int8_t temp/2)
void TempBuffWrite(
  uint8_t pi_BuffType
 ,uint8_t pi_BuffIdx
 ,int16_t pt_Temp[]
) {
	uint8_t vi_RangeIdx,vi_MinValue,vi_MinTemp,vi_TempTypeId,vb_ChangeSign;
	int16_t vi_InTemp,vi_OutValue;
	int8_t *vx_Addr = TBAddrFromIdx(pi_BuffType,pi_BuffIdx);

	vi_TempTypeId = TB(pi_BuffType,SubSize);
	while (vi_TempTypeId--) {
		vi_InTemp = *pt_Temp;
		if(vi_InTemp==TEMP_NONE16)
			vi_OutValue = TEMP_NONE8;
		else {
			vb_ChangeSign = vi_InTemp<0;
			if (vb_ChangeSign)
				vi_InTemp = -vi_InTemp;
			vi_MinValue = 0;
			vi_MinTemp  = 0;
			for (vi_RangeIdx = 0; vi_RangeIdx<TempEERangesCount; vi_RangeIdx++) {
				vi_OutValue = DivL((int32_t)(vi_InTemp-vi_MinTemp*0x100)
				                   *TempEERange(vi_RangeIdx,vi_Mul)
				                  ,2560)
				             +vi_MinValue;
				if (vi_OutValue<=TempEERange(vi_RangeIdx,vi_MaxValue))
					break;
				vi_MinValue = TempEERange(vi_RangeIdx,vi_MaxValue);
				vi_MinTemp  = TempEERange(vi_RangeIdx,vi_MaxTemp);
			}
			if (vi_OutValue>0x7F)
				vi_OutValue = 0x7F;
			if (vb_ChangeSign)
				vi_OutValue = -vi_OutValue;
		}

		if TB(pi_BuffType,EEMem)
			eeprom_write_byte((uint8_t *)vx_Addr,(int8_t)vi_OutValue);
		else
			*((int8_t *)vx_Addr) = (int8_t)vi_OutValue;
		pt_Temp++; vx_Addr++;
	}
} //TempBuffWrite


//*********************************
//odczytaj temperaturê z bufora - wersja zwracaj¹ca int16_t
void TempBuffRead(
  uint8_t pi_BuffType
 ,uint8_t pi_BuffIdx
 ,int16_t pt_Temps[]
) {
	int8_t vi_InValue;
	uint8_t vi_RangeIdx,vi_MinValue,vi_MinTemp,vb_ChangeSign;
	uint8_t vi_TempType;
	int16_t vi_OutTemp;
	int8_t *vx_Addr = TBAddrFromIdx(pi_BuffType,pi_BuffIdx);

	vi_OutTemp = TEMP_NONE16;
	for (vi_TempType=TEMP_MIN; vi_TempType<=TEMP_MAX; vi_TempType++) {
		if (vi_TempType<TB(pi_BuffType,SubSize)) {
			if TB(pi_BuffType,EEMem)
				vi_InValue = eeprom_read_byte((uint8_t *)vx_Addr);
			else
				vi_InValue = *((int8_t *)vx_Addr);
			if (vi_InValue==TEMP_NONE8)
				vi_OutTemp = TEMP_NONE16;
			else {
				vb_ChangeSign = vi_InValue<0;
				if (vb_ChangeSign)
					vi_InValue = -vi_InValue;
				vi_MinTemp  = 0;
				vi_MinValue = 0;
				for (vi_RangeIdx=0; 1; vi_RangeIdx++) {
					if (vi_InValue<=TempEERange(vi_RangeIdx,vi_MaxValue))
						break;
					vi_MinTemp  = TempEERange(vi_RangeIdx,vi_MaxTemp);
					vi_MinValue = TempEERange(vi_RangeIdx,vi_MaxValue);
				}
				vi_OutTemp = DivL((vi_InValue-vi_MinValue)*2560L
												  ,TempEERange(vi_RangeIdx,vi_Mul)
												  )
										+vi_MinTemp*0x100;
				if (vb_ChangeSign)
					vi_OutTemp = -vi_OutTemp;
			}
			vx_Addr ++;
		}
		pt_Temps[vi_TempType] = vi_OutTemp;
	}

} //TempBuffRead
#endif //TEMP_USE_BUFF


//*********************************
//odczytuj temperatury w przerwaniu z time.h
void TempExecuteHthSec(void) {
#if TEMP_READ_CYCLE/TEMP_READ_SUBCYCLE-1>127
	int16_t vi_Idx;
#else
	int8_t vi_Idx;
#endif
#if TEMP_READ_CYCLE>256	
	uint16_t vi_TempHthSecCounter;
#else
	uint8_t vi_TempHthSecCounter;
#endif
	uint8_t vi_sreg;

	vi_sreg = SREG;
	cli();
	if (++gi_TempHthSecCounter==TEMP_READ_CYCLE) gi_TempHthSecCounter = 0;
	vi_TempHthSecCounter = gi_TempHthSecCounter;
	SREG = vi_sreg;

	if (vi_TempHthSecCounter==TEMP_STARTMEAS_HthIdx)
		DS18StartMeas();

	if (vi_TempHthSecCounter%TEMP_READ_SUBCYCLE)
		return;

	vi_Idx = vi_TempHthSecCounter/TEMP_READ_SUBCYCLE-1;
	if (vi_Idx<0)
		return;

	if (vi_Idx<TEMP_BUFFCOUNT) {
		//1..TEMP_BUFFLAST+1
		TempBuffStore(vi_Idx+1);
		return;
	}

	vi_Idx -= TEMP_BUFFCOUNT;

	if (vi_Idx<TEMP_COUNT) {
		//0..(TEMP_COUNT-1)
		TempReadMeas(vi_Idx);
		return;
	}

} //TempExecuteHthSec


//*********************************
//odczytaj temperatury
void TempReadMeas(uint8_t pi_Sensor) {
	int8_t   vi_Diff;
	uint16_t vi_EMASize;
	uint8_t  vb_EMASize1;
	Long     vi_Tick;
	TempVar *vr_Temp = &gt_TempsVar[pi_Sensor];

	if (pi_Sensor>=TEMP_COUNT)
		return;

	//odczytaj, uwzglêdnij zapisany offset
	#define TD              ct_TempsData[pi_Sensor]
	#define vi_EMASize10sec (pgm_read_byte(&TD.vi_EMASize10sec))
	#define vi_Offs         ((int8_t)pgm_read_byte(&TD.vi_Offs))
	#define vi_Offs0        ((int8_t)pgm_read_byte(&TD.vi_Offs0))
	#define vi_Offs20       ((int8_t)pgm_read_byte(&TD.vi_Offs20))

	//w B3-nic B2-ca³kowite stopnie B1-u³amki stopni(1/256'C) B0-mikro u³amki (1/65536'C)
	//DIV(Temp*10,16)/10 => zaokr¹glij do 1/10 sec
//	vi_Tick.I.H = (DS18ReadMeas(TD.vt_ID,1)+(int16_t)vi_Offs)*10/16;
//	vi_Tick.I.L = 0;
//	vi_Tick.L = DivL(vi_Tick.L,10);
	vi_Tick.L = DivL(DivL(DS18ReadMeas(TD.vt_ID,1)*10,16)*0x10000L,10);


	//koryguj
	int8_t vi_CorrRange;
	if (vi_Tick.B.B2>20) vi_CorrRange = 20;
	else if (vi_Tick.B.B2<0) vi_CorrRange = 0;
	else vi_CorrRange = vi_Tick.B.B2;
	vi_Tick.L += ((20-vi_CorrRange)*vi_Offs0+vi_CorrRange*vi_Offs20)*328L;// /20*65536/10


	//nie akceptuj skoków wiêkszych ni¿ 3'C
	vi_Diff = vi_Tick.B.B2-vr_Temp->Tick.B.B2;
	vb_EMASize1 = vr_Temp->ErrorCount==TEMP_MAX_ERRORS;
	if (vb_EMASize1 || (vi_Diff>-3 && vi_Diff<3)) {
		vr_Temp->Tick = vi_Tick;
		vr_Temp->ErrorCount = 0;
	}
	else
		vr_Temp->ErrorCount++;

	vi_EMASize = vb_EMASize1? 1: DivUI((uint16_t)vi_EMASize10sec*250,TEMP_READ_CYCLE/4);
	vr_Temp->Curr.L = EmaL(vr_Temp->Tick.L,vr_Temp->Curr.L,vi_EMASize);
	vr_Temp->Sum12m += vr_Temp->Curr.I12.I12;
	vr_Temp->Sum12mCount ++;
	vr_Temp->Buff12m[gi_Temps12mIdx] = DivL(vr_Temp->Sum12m,vr_Temp->Sum12mCount);

	if (pi_Sensor==TEMP_FULLNO)
		gt_TempBuff12m[gt_TempBuffsIdx[TEMP_BUFF12m]] = vr_Temp->Buff12m[gi_Temps12mIdx];
	if (pi_Sensor==(TEMP_COUNT-1))
		gb_TempReaded = 1;

} //TempReadMeas


//*********************************
//zapamiêtaj temperatury do EEPROM
void TempBuffStore(uint8_t pi_BuffType) { //1..TEMP_BUFFLAST licz dany bufor i zapisuj  TEMP_BUFFLAST+1 przewiñ indeksy  inne wartoœci wyjdŸ
	uint8_t vi_NewTempsIdx,vi_BuffType,vi_Sensor;
#if TEMP_USE_BUFF
	uint8_t vi_MaxTypeToWrite;
	int16_t vt_Temps[TEMP_MAX+1];
#endif

	if (!pi_BuffType || pi_BuffType>TEMP_BUFFCOUNT)
		return;
	if (pi_BuffType==1)
		TempStoreCurrTime();

	for (vi_BuffType=TEMP_BUFF12m; vi_BuffType<TEMP_BUFFCOUNT; vi_BuffType++)
		if (gt_TempBuffsIdx[vi_BuffType]!=TempBuffCurrIdx(vi_BuffType)) {
			gb_BuffModification = 1;
			break;
		}
	if (!gb_BuffModification)
		return;

	//zmieni³ siê indeks któregokolwiek bufora - dzia³aj
	if (pi_BuffType<TEMP_BUFFCOUNT) {
#if TEMP_USE_BUFF

#if defined(TEMP_BUFF1W) || defined(TEMP_BUFF1M) || defined(TEMP_BUFF3M)
		if (!(gt_CurrTime[THour]+gt_CurrTime[TMin])) /*na koniec dnia: 1W,1M,3M*/ vi_MaxTypeToWrite = TEMP_BUFFCOUNT-1;
		else
#endif
#if defined(TEMP_BUFF1D)
		if (!gt_CurrTime[TMin]) /*raz na godzinê 1D*/ vi_MaxTypeToWrite = TEMP_BUFF1D;
		else
#endif
#if defined(TEMP_BUFF6h)
			/*5 razy na godzinê 1h5 6h*/ vi_MaxTypeToWrite = TEMP_BUFF6h;
#elif defined(TEMP_BUFF1h5)
			/*5 razy na godzinê 1h5 6h*/ vi_MaxTypeToWrite = TEMP_BUFF1h5;
#else
			vi_MaxTypeToWrite = 0;
#endif
#if defined(TEMP_BUFF1D)
			if (pi_BuffType<=TEMP_BUFF1D)
#endif
				gi_TempBuffCurrMode = TEMP_BCM_CALCULATE;
#if defined(TEMP_BUFF1D)
			else
				gi_TempBuffCurrMode = TEMP_BCM_READFROMEE;
#endif

		vi_NewTempsIdx = TempBuffCurrIdx(pi_BuffType);
		if ((gt_TempBuffsIdx[pi_BuffType]!=vi_NewTempsIdx) || (pi_BuffType<=vi_MaxTypeToWrite)) {
			TempBuffGetCurr(pi_BuffType,vt_Temps);
			TempBuffWrite(pi_BuffType,gt_TempBuffsIdx[pi_BuffType],vt_Temps);
		}
#endif //TEMP_USE_BUFF
	}

	else { //pi_BuffType==TEMP_BUFFLAST+1
		// przewiñ indeksy

#if TEMP_USE_BUFF
		gt_Temp1dLastDate[TYear]  = gt_CurrTime[TYear];
		gt_Temp1dLastDate[TMonth] = gt_CurrTime[TMonth];
		gt_Temp1dLastDate[TDay]   = gt_CurrTime[TDay];
		gt_Temp1dLastDate[TDay+1] = gt_CurrTime[TWeekDay];
#endif //TEMP_USE_BUFF
		for (vi_BuffType=TEMP_BUFF12m; vi_BuffType<=TEMP_BUFF24m; vi_BuffType++) {
			vi_NewTempsIdx = TempBuffCurrIdx(vi_BuffType);
			if (gt_TempBuffsIdx[vi_BuffType]!=vi_NewTempsIdx) {
				gt_TempBuffsIdx[vi_BuffType] = vi_NewTempsIdx;
#if TEMP_USE_BUFF
				if (TB(vi_BuffType,EEMem))
					EEMemClear_I8(TBAddrFromIdx(vi_BuffType,vi_NewTempsIdx),TB(vi_BuffType,SubSize));
#endif //TEMP_USE_BUFF
			}
		}
		gt_TempBuff12m[gt_TempBuffsIdx[TEMP_BUFF12m]] = TEMP_NONE16;

		//zresetuj Temp12mSum i przesuñ Temps12mShort indeks
		vi_Sensor = TEMP_COUNT;
		while (vi_Sensor--) {
			gt_TempsVar[vi_Sensor].Sum12m = 0;
			gt_TempsVar[vi_Sensor].Sum12mCount = 0;
			gt_TempsVar[vi_Sensor].Buff12m[gi_Temps12mIdx] = TEMP_NONE16;
		}
		gi_Temps12mIdx = TruncIdx8(gi_Temps12mIdx+1,TEMP_12MSIZE);

		gb_BuffModification = 0;
	}

} //TempBuffStore


//==============================================================
// CHARTS
//==============================================================

#if TEMP_USE_CHART

typedef struct {
	uint8_t vi_X,vi_Y,vi_W,vi_H;
	uint8_t vi_BuffType;
	int16_t vt_TempsScale[TEMP_MAX+1];
#if TEMP_USE_CHART==2
	int16_t vt_TempsPrint[TEMP_MAX+1];
#endif
	uint8_t vi_LastBuffIdx;
	int8_t  vi_PrevYOffs;
	uint8_t vi_Space; //odstêp od krawêdzi zewnêtrznych do skrajnego punktu wykresu)
	uint8_t vi_SelXOffs; //przesuniêcie bie¿¹cego podœwietlenia w stosunku do koñca linii
} ChartParams;

ChartParams gr_ChartParams;
#define CP gr_ChartParams
#endif


#if TEMP_USE_CHART==2
//*********************************
//pobierz datê dla okreœlonego indeksu na wykresie
void TempGetDateFromBuffIdx(uint8_t pi_BuffType,uint8_t pi_BuffIdx,uint8_t pt_Time[],uint8_t pb_Last) {
	uint8_t vi_BuffOffs;
#ifdef TEMP_BUFF1h5
	int16_t vi_1h5;
#endif
#ifdef TEMP_BUFF6h
	int16_t vi_6h;
#endif
#if defined(TEMP_BUFF1h5) || defined(TEMP_BUFF6h)
	int16_t vi_DecDays;
#endif
	uint16_t vi_DayNo;
#if TEMP_USE_BUFF
	vi_DayNo = TimeGetDayNo(gt_Temp1dLastDate[TDay],gt_Temp1dLastDate[TMonth],gt_Temp1dLastDate[TYear]);
#else
	vi_DayNo = TimeGetDayNo(gt_CurrTime[TDay],gt_CurrTime[TMonth],gt_CurrTime[TYear]);
#endif
	if (pi_BuffIdx<=gt_TempBuffsIdx[pi_BuffType])
		vi_BuffOffs = gt_TempBuffsIdx[pi_BuffType]-pi_BuffIdx;
	else
		vi_BuffOffs = gt_TempBuffsIdx[pi_BuffType]+TB(pi_BuffType,Size)-pi_BuffIdx;

	switch (pi_BuffType) {
/*		case TEMP_BUFF24m:
			pt_Time[THour] = (pi_BuffIdx*24)/60;
			pt_Time[TMin]  = (pi_BuffIdx*24)%60;
			TimeGetDateFromDayNo(vi_DayNo-(pi_BuffIdx>gt_TempBuffsIdx[pi_BuffType]?-1:0),pt_Time);
			break;*/
#ifdef TEMP_BUFF1h5
		case TEMP_BUFF1h5:
			vi_1h5     = (gt_CurrTime[THour]*2+gt_CurrTime[TMin]/30)/3-vi_BuffOffs;
			vi_DecDays = vi_1h5/16;
			vi_1h5    %= 16;
			if (vi_1h5<0) {
				vi_DecDays--;
				vi_1h5 += 16;
			}
			TimeGetDateFromDayNo(vi_DayNo+vi_DecDays,pt_Time);
			pt_Time[THour] = vi_1h5*3/2;
			pt_Time[TMin] = ((vi_1h5*3)%2)*30;
			break;
#endif
#ifdef TEMP_BUFF6h
		case TEMP_BUFF6h:
			vi_6h      = gt_CurrTime[THour]/6-vi_BuffOffs;
			vi_DecDays = vi_6h/4;
			vi_6h     %= 4;
			if (vi_6h<0) {
				vi_DecDays--;
				vi_6h += 4;
			}
			TimeGetDateFromDayNo(vi_DayNo+vi_DecDays,pt_Time);
			pt_Time[THour] = vi_6h*6;
			pt_Time[TMin]  = 0;
			break;
#endif
#ifdef TEMP_BUFF1D
		case TEMP_BUFF1D:
			TimeGetDateFromDayNo(vi_DayNo-vi_BuffOffs,pt_Time);
			break;
#endif
#ifdef TEMP_BUFF1W
		case TEMP_BUFF1W:
			TimeGetDateFromDayNo(vi_DayNo-gt_Temp1dLastDate[TDay+1]+1-vi_BuffOffs*7,pt_Time);
			break;
#endif
#ifdef TEMP_BUFF1M
		case TEMP_BUFF1M:
			pt_Time[TYear]  = gt_Temp1dLastDate[TYear]-vi_BuffOffs/12;
			if (vi_BuffOffs%12 >= gt_Temp1dLastDate[TMonth]) {
				pt_Time[TYear]--;
				pt_Time[TMonth] = gt_Temp1dLastDate[TMonth]+12-vi_BuffOffs%12;
			}
			else
				pt_Time[TMonth] = gt_Temp1dLastDate[TMonth]-vi_BuffOffs%12;
			break;
#endif
#ifdef TEMP_BUFF3M
		case TEMP_BUFF3M:
			pt_Time[TYear]  = gt_Temp1dLastDate[TYear];
			pt_Time[TMonth] = gt_Temp1dLastDate[TMonth];
			pt_Time[TDay]   = gt_Temp1dLastDate[TDay];
			TimeSeasonFirstDay(pt_Time);
			pt_Time[TDay]    = 30;
			pt_Time[TYear]  += -1-vi_BuffOffs/4;
			pt_Time[TMonth] += +12-(vi_BuffOffs%4)*3;
			while (pt_Time[TMonth]>12) {
				pt_Time[TYear] ++;
				pt_Time[TMonth] -= 12;
			}
			TimeSeasonFirstDay(pt_Time);
			break;
#endif
		default: case TEMP_BUFF12m:
			pt_Time[THour] = (pi_BuffIdx*12)/60;
			pt_Time[TMin]  = (pi_BuffIdx*12)%60;
			TimeGetDateFromDayNo(vi_DayNo-(pi_BuffIdx>gt_TempBuffsIdx[pi_BuffType]?1:0),pt_Time);
			break;
	}
	if (pb_Last)
		switch (pi_BuffType) {
/*			case TEMP_BUFF24m:
				pt_Time[TMin] += 23;
				break;*/
#ifdef TEMP_BUFF1h5
			case TEMP_BUFF1h5:
				pt_Time[THour] += 1;
				pt_Time[TMin]  += 29;
				break;
#endif
#ifdef TEMP_BUFF6h
			case TEMP_BUFF6h:
				pt_Time[THour] += 5;
				pt_Time[TMin]  += 59;
				break;
#endif
#ifdef TEMP_BUFF1D
			case TEMP_BUFF1D:
				break;
#endif
#ifdef TEMP_BUFF1W
			case TEMP_BUFF1W:
				pt_Time[TDay] += 6;
				uint8_t vi_LastDay = TimeLastDay(pt_Time[TMonth],pt_Time[TYear]);
				if (pt_Time[TDay]>vi_LastDay) {
					pt_Time[TDay] -= vi_LastDay;
					pt_Time[TMonth]++;
					if (pt_Time[TMonth]==13) {
						pt_Time[TMonth] = 1;
						pt_Time[TYear] ++;
					}
				}
				break;
#endif
#ifdef TEMP_BUFF1M
			case TEMP_BUFF1M:
				pt_Time[TDay] = TimeLastDay(pt_Time[TMonth],pt_Time[TYear]);
				break;
#endif
#ifdef TEMP_BUFF3M
			case TEMP_BUFF3M:
				if (pt_Time[TMonth]==12) {
					pt_Time[TYear]++;
					pt_Time[TMonth] = 3;
				}
				else
					pt_Time[TMonth] += 3;
				pt_Time[TDay] = 30;
				TimeSeasonFirstDay(pt_Time);
				pt_Time[TDay] --;
				break;
#endif
			default: case TEMP_BUFF12m:
				pt_Time[TMin] += 11;
				break;
		}
	pt_Time[TWeekDay] = TimeDayOfWeek(pt_Time[TDay],pt_Time[TMonth],pt_Time[TYear]);

} //TempGetDateFromBuffIdx
#endif


#if TEMP_USE_CHART==2
//*********************************
//zwróæ 1 jeœli narysowaæ zwyk³¹ podzia³kê, 2 jeœli wiêksz¹ podzia³kê
uint8_t TempChartScale(uint8_t pi_BuffIdx) {
	uint8_t vt_Time[TWeekDay+1];
#if defined(TEMP_BUFF6h)
	if (CP.vi_BuffType>TEMP_BUFF6h)
#elif defined(TEMP_BUFF1h5)
	if (CP.vi_BuffType>TEMP_BUFF1h5)
#else
	if (CP.vi_BuffType>TEMP_BUFF12m)
#endif
		TempGetDateFromBuffIdx(CP.vi_BuffType,pi_BuffIdx,vt_Time,0);

	switch (CP.vi_BuffType) {
#ifdef TEMP_BUFF1h5
		case TEMP_BUFF1h5:
			if (pi_BuffIdx==16) return 2;
			if (!(pi_BuffIdx%16)) return 1;
			break;
#endif
#ifdef TEMP_BUFF6h
#endif
#ifdef TEMP_BUFF1D
		case TEMP_BUFF1D:
			if (vt_Time[TDay]==1) return 2;
			if (vt_Time[TDay]==10 || vt_Time[TDay]==20) return 1;
			break;
#endif
#ifdef TEMP_BUFF1W
		case TEMP_BUFF1W:
			if (vt_Time[TDay]<=7) {
				if (vt_Time[TMonth]==1 || vt_Time[TMonth]==7) return 2;
				if (!((vt_Time[TMonth]-1)%3)) return 1;
			}
			break;
#endif
#ifdef TEMP_BUFF1M
		case TEMP_BUFF1M:
			if (vt_Time[TMonth]==1) {
				if (!(((int8_t)vt_Time[TYear]-1)%5)) return 2;
				return 1;
			}
			break;
#endif
#ifdef TEMP_BUFF3M
		case TEMP_BUFF3M:
			if (vt_Time[TMonth]==12) {
				if (!((int8_t)vt_Time[TYear]%10)) return 2;
				if (!((int8_t)vt_Time[TYear]%5)) return 1;
			}
			break;
#endif
		default: case TEMP_BUFF12m:
			if (!(pi_BuffIdx%60)) return 2;
			if (!(pi_BuffIdx%15)) return 1;
			break;
	}

	return 0;
} //TempChartScale
#endif


#if TEMP_USE_CHART
//*********************************
//inicjalizuj wykres
void TempChartInit(
  uint8_t pi_X        //X lewego górnego rogu
 ,uint8_t pi_Y        //Y lewego górnego rogu
 ,uint8_t pi_W        //szerokoœæ wykresu (bez obramowañ i cyferek) - jednoczeœnie iloœæ branych danych
 ,uint8_t pi_H        //wysokoœæ wykresu (bez obramowañ i cyferek)
 ,uint8_t pi_Space    //odstêp od krawêdzi zewnêtrznych do skrajnych punktów wykresu
 ,uint8_t pi_BuffType //TEMP_BUFF12m/TEMP_BUFF1h5/TEMP_BUFF6H/TEMP_BUFF1D/TEMP_BUFF1W/TEMP_BUFF1M/TEMP_BUFF3M/TEMP_BUFF24m
) {
	uint8_t vi_ChangeXY,vi_ChangeWH;
	vi_ChangeXY       = 1+pi_Space;
	vi_ChangeWH       = 2*vi_ChangeXY;
	CP.vi_X           = pi_X+vi_ChangeXY;
	CP.vi_Y           = pi_Y+vi_ChangeXY;
	CP.vi_W           = pi_W-vi_ChangeWH;
	CP.vi_H           = pi_H-vi_ChangeWH;
	CP.vi_Space       = pi_Space;
	CP.vi_BuffType    = pi_BuffType;
	if (TB(pi_BuffType,Size)<CP.vi_W)
		CP.vi_W         = TB(pi_BuffType,Size);
	CP.vi_LastBuffIdx = 0xFF;
	CP.vi_SelXOffs    = 0;
} //TempChartInit


#else
//*********************************
void TempChartInit(
  uint8_t pi_X        //X lewego górnego rogu
 ,uint8_t pi_Y        //Y lewego górnego rogu
 ,uint8_t pi_W        //szerokoœæ wykresu (bez obramowañ i cyferek) - jednoczeœnie iloœæ branych danych
 ,uint8_t pi_H        //wysokoœæ wykresu (bez obramowañ i cyferek)
 ,uint8_t pi_Space    //odstêp od krawêdzi zewnêtrznych do skrajnych punktów wykresu
 ,uint8_t pi_BuffType //TEMP_BUFF12m/TEMP_BUFF1h5/TEMP_BUFF6H/TEMP_BUFF1D/TEMP_BUFF1W/TEMP_BUFF1M/TEMP_BUFF3M/TEMP_BUFF24m
) {}
#endif


#if TEMP_USE_CHART
//*********************************
//pobierz temperaturê dla wykresu
//static inline void TempGetForChart2(int16_t pi_FirstIdx,int16_t pi_LastIdx,int16_t pt_Temps[]) __attribute__((always_inline));
void TempGetForChart2(int16_t pi_FirstIdx,int16_t pi_LastIdx,int16_t pt_Temps[],uint8_t pi_Type){
	TempBuffGet(
	  CP.vi_BuffType
	 ,TEMP_FULLNO
	 ,TruncIdx16(pi_FirstIdx,TB(CP.vi_BuffType,Size))
	 ,TruncIdx16(pi_LastIdx,TB(CP.vi_BuffType,Size))
	 ,pt_Temps
	 ,pi_Type
	 );
}
static inline void TempGetForChart1(int16_t pi_Idx,int16_t *pt_Temps,uint8_t pi_Type) __attribute__((always_inline));
void TempGetForChart1(int16_t pi_Idx,int16_t *pt_Temps,uint8_t pi_Type) {TempGetForChart2(pi_Idx,pi_Idx,pt_Temps,pi_Type);}
#endif


#if TEMP_USE_CHART
//*********************************
//rysuj wykres, zwróæ tekst z temperatur¹ min/avg/max
//zwraca 1, gdy nast¹pi³ pe³ny wydruk, 0, gdy tylko zaktualizowana bie¿¹ca temp (w ostatniej kolumnie)
uint8_t TempChartPrint(void) {
	#define TC_Margin 1

	uint8_t vb_Force;
	uint8_t vi_XOffs,vi_BuffIdx;
	int8_t  vi_YOffs;
//	uint8_t vi_TempType,vi_MinTempType,vi_MaxTempType;
	int16_t vt_Temps[TEMP_MAX+1];

	while (gb_BuffModification) TimeSleep();

#if TEMP_USE_BUFF
	gi_TempBuffCurrMode = TEMP_BCM_CALCULATE;
#endif
	TempGetForChart1(gt_TempBuffsIdx[CP.vi_BuffType],vt_Temps,0);
#if TEMP_USE_BUFF
	gi_TempBuffCurrMode = TEMP_BCM_READFROMCACHE;
#endif

	vb_Force =   (CP.vi_LastBuffIdx!=gt_TempBuffsIdx[CP.vi_BuffType])
	          || (vt_Temps[TEMP_MIN]<CP.vt_TempsScale[TEMP_MIN]-TC_Margin)
	          || (vt_Temps[TEMP_MAX]>CP.vt_TempsScale[TEMP_MAX]+TC_Margin);
#if TEMP_USE_CHART==2
	if (!vb_Force) {
		TempGetForChart1(gt_TempBuffsIdx[CP.vi_BuffType],vt_Temps,1);
		vb_Force =   (vt_Temps[TEMP_MIN]<CP.vt_TempsPrint[TEMP_MIN])
		          || (vt_Temps[TEMP_MAX]>CP.vt_TempsPrint[TEMP_MAX]);
	}
#endif

	if (vb_Force) {
		CP.vi_LastBuffIdx = gt_TempBuffsIdx[CP.vi_BuffType];
		//znajdŸ minimaln¹, œredni¹ i maksymaln¹ temperaturê
		TempGetForChart2(CP.vi_LastBuffIdx-CP.vi_W+1,CP.vi_LastBuffIdx,CP.vt_TempsScale,0);
#if TEMP_USE_CHART==2
		TempGetForChart2(CP.vi_LastBuffIdx-CP.vi_W+1,CP.vi_LastBuffIdx,CP.vt_TempsPrint,1);
#endif
		//czyœæ obszar wykresu
		uint8_t vi_TmpSpace = CP.vi_Space+1;
		uint8_t vi_X = CP.vi_X-vi_TmpSpace;
		uint8_t vi_Y = CP.vi_Y-vi_TmpSpace;
		uint8_t vi_W = CP.vi_W+vi_TmpSpace*2;
		uint8_t vi_H = CP.vi_H+vi_TmpSpace*2;
		ADVPrintRect(vi_X,vi_Y,vi_W,vi_H,1);
		vi_X++; vi_Y++; vi_W -= 2; vi_H -= 2;
		ADVPrintRect(vi_X,vi_Y,vi_W,vi_H,0);

		vi_BuffIdx = TruncIdx16((int16_t)CP.vi_LastBuffIdx-CP.vi_W,TB(CP.vi_BuffType,Size));
		CP.vi_PrevYOffs   = -1;
	}
	else {
		//czyœæ ostatni¹ liniê wykresu
		ADVPrintRect(CP.vi_X+CP.vi_W-1,CP.vi_Y,1,CP.vi_H,0);
		vi_BuffIdx = CP.vi_LastBuffIdx-1;
	}

/*	if (TB(CP.vi_BuffType,SubSize)==1) {
		vi_MinTempType=TEMP_AVG;
		vi_MaxTempType=TEMP_AVG;
	}
	else {
		vi_MinTempType=TEMP_MIN;
		vi_MaxTempType=TEMP_MAX;
	}*/
	//rysuj kreski/kropki odpowiadaj¹ce kolejnym temperaturom
	for (vi_XOffs=vb_Force?0:CP.vi_W-1; vi_XOffs<CP.vi_W; vi_XOffs++) {
		vi_BuffIdx = TruncIdx8(vi_BuffIdx+1,TB(CP.vi_BuffType,Size));

		TempGetForChart1(vi_BuffIdx,vt_Temps,0);

//		for (vi_TempType=vi_MinTempType; vi_TempType<=vi_MaxTempType; vi_TempType++) {
#define vi_TempType TEMP_AVG
			if (vt_Temps[vi_TempType]!=TEMP_NONE16) {

				vi_YOffs = DivL((int32_t)((CP.vt_TempsScale[TEMP_MAX]+TC_Margin)-vt_Temps[vi_TempType])*(int32_t)(CP.vi_H-1)
				               ,(CP.vt_TempsScale[TEMP_MAX]+TC_Margin)-(CP.vt_TempsScale[TEMP_MIN]-TC_Margin));

				if (   vi_YOffs<0 || vi_YOffs>=CP.vi_H
				    || CP.vi_LastBuffIdx!=gt_TempBuffsIdx[CP.vi_BuffType]
					#if TEMP_USE_BUFF
				    || gb_BuffModification
					#endif
				   )
					//w trakcie przetwarzania mamy nowy odczyt - rysuj od nowa
					return TempChartPrint();

				uint8_t vi_X = CP.vi_X+vi_XOffs;
				if (vi_TempType!=TEMP_AVG || CP.vi_PrevYOffs==-1 || CP.vi_PrevYOffs==vi_YOffs)
					ADVPrintPix(vi_X,CP.vi_Y+vi_YOffs,1);
				else if (CP.vi_PrevYOffs<vi_YOffs)
					ADVPrintRect(vi_X,CP.vi_Y+CP.vi_PrevYOffs+1,1,vi_YOffs-CP.vi_PrevYOffs,1);
				else
					ADVPrintRect(vi_X,CP.vi_Y+vi_YOffs,1,CP.vi_PrevYOffs-vi_YOffs,1);
			}
			else
				vi_YOffs = -1;
			if (vi_XOffs<CP.vi_W-1 && vi_TempType==TEMP_AVG) CP.vi_PrevYOffs = vi_YOffs;
//		}

#if TEMP_USE_CHART==2
		if (CP.vi_Space && vb_Force) {
			uint8_t vi_Scale = TempChartScale(vi_BuffIdx);
			if (vi_Scale) {
				ADVPrintRect(CP.vi_X+vi_XOffs-vi_Scale+1,CP.vi_Y-CP.vi_Space,vi_Scale*3/2,1,1);
				ADVPrintRect(CP.vi_X+vi_XOffs-vi_Scale+1,CP.vi_Y+CP.vi_H-1+CP.vi_Space,vi_Scale*3/2,1,1);
			}
		}
#endif

	}

	return vb_Force;

} //TempChartPrint


#else
//*********************************
uint8_t TempChartPrint(void) {return 0;}
#endif


#if TEMP_USE_CHART==2
//*********************************
//drukuj tekst dla wykresu (czas i temperatury) dla pe³nego zakresu lub wybranego piksela
void TempChartPrintText(uint8_t pb_Full, uint8_t pi_Y) {
	uint8_t vi_FirstBuffIdx,vi_LastBuffIdx;
	int16_t vt_Temps[TEMP_MAX+1];
	uint8_t vt_FirstTime[TWeekDay+1],vt_LastTime[TWeekDay+1];
	char    vv_TextFull[60],vv_Text1[20],vv_Text2[20], *vv_Format;

	if (pb_Full) {
		vi_FirstBuffIdx = TruncIdx16((int16_t)CP.vi_LastBuffIdx-(CP.vi_W-1),TB(CP.vi_BuffType,Size));
		vi_LastBuffIdx  = CP.vi_LastBuffIdx;
	}
	else {
		vi_FirstBuffIdx = TruncIdx16(CP.vi_LastBuffIdx-CP.vi_SelXOffs,TB(CP.vi_BuffType,Size));
		vi_LastBuffIdx  = vi_FirstBuffIdx;
	}

	CSSet(CS_Z6p);

	//tekst z zakresem czasu
	TempGetDateFromBuffIdx(CP.vi_BuffType,vi_FirstBuffIdx,vt_FirstTime,0);
	TempGetDateFromBuffIdx(CP.vi_BuffType,vi_LastBuffIdx,vt_LastTime,1);
	vv_Format = ct_TempBuffsData[CP.vi_BuffType].TimeFormatFull;
	TimeConvert(vv_Format,vv_Text1,vt_FirstTime);
	if (!pb_Full)
		vv_Format = ct_TempBuffsData[CP.vi_BuffType].TimeFormatOne;
	TimeConvert(vv_Format,vv_Text2,vt_LastTime);
	STRInit(vv_TextFull,60);
	STRAddV(vv_Text1);
	if (*vv_Text2) {
		STRAddV_p(PSTR(Sp4pxS"-"Sp4pxS));
		STRAddV(vv_Text2);
	}
	STRAddV_p(PSTR(" ("));
	if (pb_Full)
		STRAddV_p((char *)ct_TempBuffsData[CP.vi_BuffType].RangeFull);
	else
		STRAddV_p((char *)ct_TempBuffsData[CP.vi_BuffType].RangeOne);
	STRAddC(')');
	CSPrintXYu8AV(CP.vi_X+CP.vi_W/2,pi_Y,vv_TextFull,ALIGN_CENTER,LCD_W);

	//tekst z temperaturami
	STRInit(vv_TextFull,60);
	if (pb_Full) {
		vt_Temps[TEMP_MIN] = CP.vt_TempsPrint[TEMP_MIN];
		vt_Temps[TEMP_AVG] = CP.vt_TempsPrint[TEMP_AVG];
		vt_Temps[TEMP_MAX] = CP.vt_TempsPrint[TEMP_MAX];
	}
	else
		TempGetForChart2(vi_FirstBuffIdx,vi_LastBuffIdx,vt_Temps,1);
	if ((!pb_Full) && TB(CP.vi_BuffType,SubSize)==1) {
		STRAddV_p(PSTR("Œrednia:"));
		STRAddV(TempConvert(vt_Temps[TEMP_MIN],1));
	}
	else {
		STRAddV_p(PSTR("Mi:")); STRAddV(TempConvert(vt_Temps[TEMP_MIN],1));
		STRAddV_p(PSTR(" Œr:")); STRAddV(TempConvert(vt_Temps[TEMP_AVG],1));
		STRAddV_p(PSTR(" Ma:")); STRAddV(TempConvert(vt_Temps[TEMP_MAX],1));
	}
	CSPrintXYu8AV(CP.vi_X+CP.vi_W/2,pi_Y+6,vv_TextFull,ALIGN_CENTER,LCD_W);

} //TempChartPrintText
#endif


#if TEMP_USE_CHART==2
//*********************************
//rysuj/wyczyœæ zaznaczenie na wykresie
void TempChartPrintSel(uint8_t pb_Print) {
	uint8_t vi_X,vi_Y;
	vi_X = CP.vi_X+CP.vi_W-1-CP.vi_SelXOffs;
	vi_Y = CP.vi_Y-CP.vi_Space-2;
	ADVPrintRect(vi_X-1,vi_Y,3,1,pb_Print);
	ADVPrintPix(vi_X,vi_Y+1,!pb_Print);
	vi_Y = CP.vi_Y+CP.vi_H+CP.vi_Space;
	ADVPrintPix(vi_X,vi_Y,!pb_Print);
	ADVPrintRect(vi_X-1,vi_Y+1,3,1,pb_Print);
} //TempChartPrintSel
#endif


#if TEMP_USE_CHART==2
//*********************************
//drukuj wykres jeœli nie wciœniêty Center
//po wciœniêciu Center => wejdŸ do trybu przesuwania siê po wykresie (a¿ do czasu wciœniêcia ponownego Center)
void TempChartExecute(void) {
	uint8_t vb_Force,vb_ComplexMode,vi_Button,vi_FullSpace;
	int16_t vi_ExitHthSec;

	//vb_ComplexMode = (gi_BUTPrevValue==BUT_CENTER);
	vb_ComplexMode = (gi_BUTPrevValue==BUT_DOWN);
	vi_FullSpace = CP.vi_Space+1;

	if (vb_ComplexMode) {
		ADVPrintRect(0,CP.vi_Y-vi_FullSpace+12,LCD_W,CP.vi_H+vi_FullSpace*2-12,0);
		CP.vi_Y += 13;
		CP.vi_H -= 27;
		CP.vi_LastBuffIdx = 0xFF;
		//LCDSwitchHighlight(LCD_LEDHIGH);
		//gi_BUTAutoRepeatPerSec = 25;
	}
	else {
		CP.vi_Y += 12;
		CP.vi_H -= 12;
	}

	do {
		if (vb_ComplexMode)
			TempChartPrintSel(1);

		vb_Force = TempChartPrint();

		if (vb_Force) {
			if (vb_ComplexMode)
				TempChartPrintSel(1);
			//drukuj tekst dla pe³nego zakresu
			TempChartPrintText(1,CP.vi_Y-12-vb_ComplexMode-vi_FullSpace);
		}

		if (vb_ComplexMode) {
			//drukuj tekst dla wybranego piksela
			TempChartPrintText(0,CP.vi_Y+CP.vi_H+vi_FullSpace+2);

			//jeœli przycisk wciœniêty => odczekaj 0.25sec
			vi_ExitHthSec = (gi_TempHthSecCounter+25)%TEMP_READ_CYCLE;
			while (vi_ExitHthSec<gi_TempHthSecCounter && gi_BUTValue) TimeSleep();
			while (vi_ExitHthSec>gi_TempHthSecCounter && gi_BUTValue) TimeSleep();

			//czekaj na przyciski
			do
				vi_Button = BUTWaitForPress(1);
			while ((!vi_Button) && (!gb_TempReaded));

			//czyœæ poprzednie zaznaczenie
			TempChartPrintSel(0);

			uint8_t vi_sreg = SREG;
			cli();
			switch (vi_Button) {
				case BUT_LEFT:
					CP.vi_SelXOffs = TruncIdx16((int16_t)CP.vi_SelXOffs+gi_BUTRepeatCount+1,CP.vi_W);
					break;
				case BUT_RIGHT:
					CP.vi_SelXOffs = TruncIdx16((int16_t)CP.vi_SelXOffs-gi_BUTRepeatCount-1,CP.vi_W);
					break;
			}
			gi_BUTRepeatCount = 0;
			gb_TempReaded = 0;
			SREG = vi_sreg;
		}

	} while (vb_ComplexMode && vi_Button!=BUT_DOWN && gi_BUTSec<60);
//} while (vb_ComplexMode && vi_Button!=BUT_CENTER && gi_BUTSec<60);

	if (vb_ComplexMode) {
		BUTWaitForUnpress();
		gi_BUTPrevValue = 0;
		CP.vi_Y -= 13;
		CP.vi_H += 27;
		CP.vi_LastBuffIdx = 0xFF;
		ADVPrintRect(0,CP.vi_Y-vi_FullSpace+12,LCD_W,CP.vi_H+vi_FullSpace*2-12,0);
		//odrysuj bez complex mode
		TempChartExecute();
		//LCDSwitchHighlight(vi_Button?LCD_LEDAUTO15:LCD_LEDAUTO1);
//		BUTSetDefaultAutoRepeat();
	}
	else {
		CP.vi_Y -= 12;
		CP.vi_H += 12;
	}

} //TempChartExecute


#else //TEMP_USE_CHAT==1/0
//*********************************
//drukuj wykres
void TempChartExecute(void) {TempChartPrint();}
#endif

// zwraca 1 jezeli h1 <= currentH <= h2
int8_t TimeIsBetween(uint8_t h1, uint8_t h2){
	if( h1 <= gt_CurrTime[THour] && gt_CurrTime[THour] <= h2){
		return 1;
	}else{
		return 0;
	}
}

