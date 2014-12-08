#ifndef DS18X20_H
#define DS18X20_H 1

#include <inttypes.h>

#include "onewire.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

#define DS18MAXCOUNT 0 //maksymalna iloœæ termometrów do wykrycia
uint8_t gt_DS18IDs[DS18MAXCOUNT][OW_ROMCODE_SIZE]; //identyfikatory czujników
uint8_t gi_DS18Count;
//wykryj wszystkie termometry i wyœwietl ich IDki na LCD
void DS18Detect(void);

//rozpocznij pomiar temperatury
void DS18StartMeas(void);

//odczytaj temperaturê pojedynczego termometru
int16_t DS18ReadMeas(
  uint8_t pt_TempId[]
 ,uint8_t pb_Progmem
);

#endif
