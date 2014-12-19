/* Automatically generated at 17.03.2010 12:22:15 */
/* Font: Lucilda Console 8px Size:33 bytes */
#ifndef CHARSETZNAKI09P
#define CHARSETZNAKI09P 1
#include <avr/pgmspace.h>

uint8_t ct_CSZnaki09pXSpaces [] PROGMEM = {0,0,1,1,1,5,6};

uint8_t ct_CSZnaki09pData[] PROGMEM = {
/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4,0x3A,0x0F,0x09,0x0F
,/*0x20  */ /*W,H,YOffXbXa*/3,1,0x0A
  ,0x00,0x00,0x00
};
uint8_t ct_CSZnaki09pAddrOffs[] PROGMEM = {
   /* */0x06};

char ct_CSZnaki09pAddToAddr[] PROGMEM = {'\xFF'};

#define cr_CSZnaki09p { \
  /*vi_Height*/     9 \
 ,/*vi_YSpace*/     0 \
 ,/*vt_XSpaces*/    ct_CSZnaki09pXSpaces \
 ,/*vt_Data*/       ct_CSZnaki09pData \
 ,/*vi_AOCharOffs*/ 32 \
 ,/*vi_AOCount*/    1 \
 ,/*vt_AddrOffs*/   ct_CSZnaki09pAddrOffs \
 ,/*vt_AddToAddr*/  ct_CSZnaki09pAddToAddr \
}
#endif
