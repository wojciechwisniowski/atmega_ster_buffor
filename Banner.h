#ifndef BANNER_H
#define BANNER_H

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define BANSIZE   64

int8_t gi_BANY;
uint8_t gb_BANOn, gi_BANH, gi_BANCS, gb_BANNegMode, gi_BANHthSec;

/*volatile*/ uint8_t gi_BANHthSecCounter; //zmienna wewn�trzna

char gv_BANText[BANSIZE+1];


//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//zainicjalizuj modu�
#define BANInit(void) {TimeAddHthCounter(&gi_BANHthSecCounter,0);}

#define BANSetParams(pi_Y,pi_H,pi_CS,pb_NegMode,pi_HthSec) {gi_BANY=pi_Y; gi_BANH=pi_H; gi_BANCS=pi_CS; gb_BANNegMode=pb_NegMode; gi_BANHthSec=pi_HthSec;}

//w��cz wy�wietlanie banera
void BANOn(void);

//wy��cz wy�wietlanie banera
#define BANOff(void) {gb_BANOn = 0; gv_BANText[0] = '\0';}

//przewi� baner (z kontrol� czy ju� nadszed� czas, oraz czy nie przewin�� wi�cej ni� 1px)
uint8_t BANScroll(void);

//czekaj na przewini�cie banera (�eby po wyj�ciu mie� du�o czasu do nast�pnego przewijania => zabezpieczenie przed przycinaniem banera)
void BANWaitForSroll(void);

#endif