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

//procedura wywo�ywana w przerwaniu zegara czasu (time.h) co 1/100 sekundy
void BUTExecute(void);
//przerwanie na zako�czenie konwersji DAC => tylko po to, �eby umo�liwi� nast�pn� konwersj�
//SIGNAL(ADC_vect);

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//*********************************
//inicjalizuj modu�
void BUTInit(void) {
	DDRB  = 0x00; //port C jest wejsciem dla przyciskow
	PORTB = 0x0F; // PB0..PB3 wejscie podciagniete do Vcc czyli przycisk zwiera z masa
	TimeAddHthProcedure(&BUTExecute);
	BUTSetDefaultAutoRepeat();
} //BUTInit

//*********************************
//czekaj na wci�ni�cie przycisku lub automatyczny repeat przy wci�ni�tym d�u�ej
//zwr�� numer pobranego przycisku i automatycznie zmniejsz gi_BUTRepeatCount o 1
uint8_t BUTWaitForPress(uint8_t pi_MaxSec) {
	uint8_t vi_BUTValue,vi_sreg;

	do
		TimeSleep(); //zawsze odczekaj chwilk�
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
		TimeSleep(); //zawsze odczekaj chwilk�
	while (gi_BUTValue);
}

//==============================================================
// PRIVATE PROCEDURES
//==============================================================

//czasy (w 1/100sec) kolejnych powt�rze�
uint8_t ct_ButRepeatOffs[] PROGMEM = {100,25,20,16,13,11,10,0};

//*********************************
//procedura wywo�ywana w przerwaniu zegara czasu (time.h) co 1/100 sekundy
void BUTExecute(void) {

	static uint8_t vi_ReadCount = 0;
	static uint8_t vi_PrevValue = 0;
	static uint8_t vi_RepeatOffsIdx = 0;
	static uint16_t vi_NextRepeatTime = 0;
	uint8_t vi_ReadedValue;
	uint8_t vi_NewValue;
	uint8_t vi_RepeatOffs;

	//znajd�, od kt�rego przycisku wysz�o takie napi�cie
	vi_NewValue = PINB & 0x0f;
	vi_NewValue = vi_NewValue^0x0f;

	if (vi_NewValue != vi_PrevValue) {
		//zmieni�a si� warto�� przycisku => wyzeruj licznik
		vi_ReadCount = 1;
		vi_PrevValue = vi_NewValue;
	} else if (vi_ReadCount < 0xFF)
		//nie zmieni�a si� warto�� przycisku => zwi�ksz licznik
		vi_ReadCount++;

	//je�li up�yn�� odpowiedni czas od zmiany (w celu wyeliminowania drga�) to wypisz do globali nowe warto�ci
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
