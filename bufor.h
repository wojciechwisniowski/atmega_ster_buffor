#include <avr/io.h>

#include "Time.h"

#define GRZALKA_PORT PORTD
#define GRZALKA_DDR DDRD
#define GRZALKA_1 (1 << 6) //d6
#define GRZALKA_2 (1 << 5) //d5
#define GRZALKA_3 (1 << 4) //d4

#define BUFOR_GRZEJE 1
#define BUFOR_NIE_GRZEJE 0

#define MIND    0
#define MAXD	1
#define MINN    2
#define MAXN    3
#define DSTART  4
#define DEND  	5

uint8_t gb_BuforStatus;
uint8_t gi_BuforTempMINGrzalkiDzien;
uint8_t gi_BuforTempMAXGrzalkiDzien;

uint8_t gi_BuforTempMINGrzalkiNoc;
uint8_t gi_BuforTempMAXGrzalkiNoc;

uint8_t gi_BuforDzienStartH;
uint8_t gi_BuforDzienEndH;

uint8_t gb_BuforDzien;




void BuforGrzej(uint8_t pb_grzej);
void BuforInit(void);
void BuforChange(
		  uint8_t pi_Y
		 ,uint8_t pi_What //MIN_d MAX_d MIN_N MAX_N
		 ,int8_t pi_Value
		);

int8_t BuforSprawdz(void);

