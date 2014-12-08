#include "Button.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#include "Time.h"
#include "Charset.h"
//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================
//uint8_t gt_BUTMap[BUTCount] PROGMEM = {BUTMap};

//==============================================================
// FORWARD DECLARATIONS
//==============================================================

//procedura wywo³ywana w przerwaniu zegara czasu (time.h) co 1/100 sekundy
void BUTExecute(void);
//przerwanie na zakoñczenie konwersji DAC => tylko po to, ¿eby umo¿liwiæ nastêpn¹ konwersjê
//SIGNAL(ADC_vect);

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//*********************************
//inicjalizuj modu³
void BUTInit(void) {
	DDRB  = 0x00; //port C jest wejsciem dla przyciskow
	PORTB = 0x0F; // PB0..PB3 wejscie podciagniete do Vcc czyli przycisk zwiera z masa
	TimeAddHthProcedure(&BUTExecute);
	BUTSetDefaultAutoRepeat();
} //BUTInit

//*********************************
//czekaj na wciœniêcie przycisku lub automatyczny repeat przy wciœniêtym d³u¿ej
//zwróæ numer pobranego przycisku i automatycznie zmniejsz gi_BUTRepeatCount o 1
uint8_t BUTWaitForPress(uint8_t pi_MaxSec) {
	uint8_t vi_BUTValue,vi_sreg;

	do
		TimeSleep(); //zawsze odczekaj chwilkê
	while (!gi_BUTRepeatCount && gi_BUTSec<pi_MaxSec);

	vi_sreg = SREG;
	cli();
	if (gi_BUTRepeatCount) {
		gi_BUTRepeatCount--;
		vi_BUTValue = gi_BUTValue;
	}
	else
		vi_BUTValue = 0;
	SREG = vi_sreg;

	return vi_BUTValue;
} //BUTWaitForPress

//*********************************
//czekaj na puszczenie przycisku
void BUTWaitForUnpress(void) {
	do
		TimeSleep(); //zawsze odczekaj chwilkê
	while (gi_BUTValue);
}

//==============================================================
// PRIVATE PROCEDURES
//==============================================================

//czasy (w 1/100sec) kolejnych powtórzeñ
uint8_t ct_ButRepeatOffs[] PROGMEM = {100,25,20,16,13,11,10,0};

//*********************************
//procedura wywo³ywana w przerwaniu zegara czasu (time.h) co 1/100 sekundy
void BUTExecute(void) {

	static uint8_t vi_ReadCount = 0;
	static uint8_t vi_PrevValue = 0;
	static uint8_t vi_RepeatOffsIdx = 0;
	static uint16_t vi_NextRepeatTime = 0;
	uint8_t vi_ReadedValue;
	uint8_t vi_NewValue;
	uint8_t vi_RepeatOffs;

	//znajdŸ, od którego przycisku wysz³o takie napiêcie
	vi_NewValue = PINB & 0x0f;
	vi_NewValue = vi_NewValue^0x0f;

	if (vi_NewValue != vi_PrevValue) {
		//zmieni³a siê wartoœæ przycisku => wyzeruj licznik
		vi_ReadCount = 1;
		vi_PrevValue = vi_NewValue;
	} else if (vi_ReadCount < 0xFF)
		//nie zmieni³a siê wartoœæ przycisku => zwiêksz licznik
		vi_ReadCount++;

	//jeœli up³yn¹³ odpowiedni czas od zmiany (w celu wyeliminowania drgañ) to wypisz do globali nowe wartoœci
	if ((vi_ReadCount >= (vi_NewValue ? BUTRepeatPress : BUTRepeatUnpress))
			&& (vi_NewValue != gi_BUTValue)) {
		gi_BUTPrevValue = gi_BUTValue;
		gi_BUTValue = vi_NewValue;
		gi_BUTHthSec = 0;
		gi_BUTRepeatCount = 0;
		vi_RepeatOffsIdx = 0;
		vi_NextRepeatTime = 1;
	}

	if (gi_BUTHthSec < 0xFFFF) {
		gi_BUTHthSec++;
		if (gi_BUTHthSec < 25599)
			gi_BUTSec = gi_BUTHthSec / 100;
	}

	if (gi_BUTValue) {
		//repeaty automatyczne
		if (gi_BUTHthSec == vi_NextRepeatTime) {
			gi_BUTRepeatCount++;
			vi_RepeatOffs = pgm_read_byte(&ct_ButRepeatOffs[vi_RepeatOffsIdx]);
			if (vi_RepeatOffs) {
				vi_NextRepeatTime += vi_RepeatOffs;
				vi_RepeatOffsIdx++;
			} else
				vi_NextRepeatTime += (100 + gi_BUTAutoRepeatPerSec / 2)
						/ gi_BUTAutoRepeatPerSec;
		}
	}

} //BUTExecute
