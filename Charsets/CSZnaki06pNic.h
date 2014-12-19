/* Automatically generated at 17.03.2010 12:22:13 */
/* Font: Sam narysowa³em Size:33 bytes */
#ifndef CHARSETZNAKI06P
#define CHARSETZNAKI06P 1
#include <avr/pgmspace.h>

uint8_t ct_CSZnaki06pXSpaces [] PROGMEM = {0,0,1,3,4,5,6};

uint8_t ct_CSZnaki06pData[] PROGMEM = {
/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4,0x05,0x0F,0x09,0x0F
,/*0x20  */ /*W,H,YOffXbXa*/3,1,0x05
  ,0x00,0x00,0x00
};
uint8_t ct_CSZnaki06pAddrOffs[] PROGMEM = {
   /* */0x06};

char ct_CSZnaki06pAddToAddr[] PROGMEM = {'\xFF'};

#define cr_CSZnaki06p { \
  /*vi_Height*/     6 \
 ,/*vi_YSpace*/     0 \
 ,/*vt_XSpaces*/    ct_CSZnaki06pXSpaces \
 ,/*vt_Data*/       ct_CSZnaki06pData \
 ,/*vi_AOCharOffs*/ 32 \
 ,/*vi_AOCount*/    1 \
 ,/*vt_AddrOffs*/   ct_CSZnaki06pAddrOffs \
 ,/*vt_AddToAddr*/  ct_CSZnaki06pAddToAddr \
}
#endif
