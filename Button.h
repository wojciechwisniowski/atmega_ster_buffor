#ifndef	BUTTON_H
#define BUTTON_H 1

#include <inttypes.h>

#define BUTCount 4 //iloœæ przycisków
#define BUTRepeatPress 2 //ile kolejnych (co 1/100sec) identycznych odczytów uznaæ za znacz¹ce wciœniêcie przycisku
#define BUTRepeatUnpress 7 //ile kolejnych (co 1/100sec) identycznych odczytów uznaæ za znacz¹ce puszczenie przycisku

//#define BUTMap 0,BMap(1),BMap(2),BMap(3)
//nazwy przycisków
#define BUT_BACK 1
#define BUT_UP 1

#define BUT_LEFT 2
#define BUT_RIGHT 4

#define BUT_ENTER 8
#define BUT_DOWN 8

/*volatile*/ uint8_t gi_BUTPrevValue; //poprzednia wartoœæ
/*volatile*/ uint8_t gi_BUTValue; //bie¿¹ca wartoœæ wciœniêtego przycisku
/*volatile*/ uint16_t gi_BUTHthSec; //od ilu setnych sekundy obecna wartoœæ gi_BUTValue
/*volatile*/ uint16_t gi_BUTRepeatCount; //po wciœniêciu ustawiane 1, nastêpnie zwiêkszane po up³ywie 1sec coraz szybciej (a¿ do 10razy na sec)
/*volatile*/ uint8_t gi_BUTSec;     //od ilu sekund obecna wartoœæ gi_BUTValue

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

uint8_t gi_BUTAutoRepeatPerSec; //ile na sekundê automatycznego powtarzania

//ustaw domyœlne automatyczne powtarzanie (10 razy na sekundê)
#define BUTSetDefaultAutoRepeat(void) {gi_BUTAutoRepeatPerSec = 10;}

//inicjalizuj modu³
void BUTInit(void);

//czekaj na wciœniêcie przycisku lub automatyczny repeat przy wciœniêtym d³u¿ej
//zwróæ numer pobranego przycisku i automatycznie zmniejsz gi_BUTRepeatCount o 1
uint8_t BUTWaitForPress(uint8_t pi_MaxSec);

//czekaj na puszczenie przycisku
void BUTWaitForUnpress(void);

#endif
