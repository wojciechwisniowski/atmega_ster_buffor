#ifndef BANNER_H
#define BANNER_H

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define BANSIZE   64

int8_t gi_BANY;
uint8_t gb_BANOn, gi_BANH, gi_BANCS, gb_BANNegMode, gi_BANHthSec;

/*volatile*/ uint8_t gi_BANHthSecCounter; //zmienna wewnêtrzna

char gv_BANText[BANSIZE+1];


//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//zainicjalizuj modu³
#define BANInit(void) {TimeAddHthCounter(&gi_BANHthSecCounter,0);}

#define BANSetParams(pi_Y,pi_H,pi_CS,pb_NegMode,pi_HthSec) {gi_BANY=pi_Y; gi_BANH=pi_H; gi_BANCS=pi_CS; gb_BANNegMode=pb_NegMode; gi_BANHthSec=pi_HthSec;}

//w³¹cz wyœwietlanie banera
void BANOn(void);

//wy³¹cz wyœwietlanie banera
#define BANOff(void) {gb_BANOn = 0; gv_BANText[0] = '\0';}

//przewiñ baner (z kontrol¹ czy ju¿ nadszed³ czas, oraz czy nie przewin¹æ wiêcej ni¿ 1px)
uint8_t BANScroll(void);

//czekaj na przewiniêcie banera (¿eby po wyjœciu mieæ du¿o czasu do nastêpnego przewijania => zabezpieczenie przed przycinaniem banera)
void BANWaitForSroll(void);

#endif