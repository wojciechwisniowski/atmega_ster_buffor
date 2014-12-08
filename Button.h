#ifndef	BUTTON_H
#define BUTTON_H 1

#include <inttypes.h>

#define BUTCount 4 //ilo�� przycisk�w
#define BUTRepeatPress 2 //ile kolejnych (co 1/100sec) identycznych odczyt�w uzna� za znacz�ce wci�ni�cie przycisku
#define BUTRepeatUnpress 7 //ile kolejnych (co 1/100sec) identycznych odczyt�w uzna� za znacz�ce puszczenie przycisku

//#define BUTMap 0,BMap(1),BMap(2),BMap(3)
//nazwy przycisk�w
#define BUT_BACK 1
#define BUT_UP 1

#define BUT_LEFT 2
#define BUT_RIGHT 4

#define BUT_ENTER 8
#define BUT_DOWN 8

/*volatile*/ uint8_t gi_BUTPrevValue; //poprzednia warto��
/*volatile*/ uint8_t gi_BUTValue; //bie��ca warto�� wci�ni�tego przycisku
/*volatile*/ uint16_t gi_BUTHthSec; //od ilu setnych sekundy obecna warto�� gi_BUTValue
/*volatile*/ uint16_t gi_BUTRepeatCount; //po wci�ni�ciu ustawiane 1, nast�pnie zwi�kszane po up�ywie 1sec coraz szybciej (a� do 10razy na sec)
/*volatile*/ uint8_t gi_BUTSec;     //od ilu sekund obecna warto�� gi_BUTValue

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

uint8_t gi_BUTAutoRepeatPerSec; //ile na sekund� automatycznego powtarzania

//ustaw domy�lne automatyczne powtarzanie (10 razy na sekund�)
#define BUTSetDefaultAutoRepeat(void) {gi_BUTAutoRepeatPerSec = 10;}

//inicjalizuj modu�
void BUTInit(void);

//czekaj na wci�ni�cie przycisku lub automatyczny repeat przy wci�ni�tym d�u�ej
//zwr�� numer pobranego przycisku i automatycznie zmniejsz gi_BUTRepeatCount o 1
uint8_t BUTWaitForPress(uint8_t pi_MaxSec);

//czekaj na puszczenie przycisku
void BUTWaitForUnpress(void);

#endif
