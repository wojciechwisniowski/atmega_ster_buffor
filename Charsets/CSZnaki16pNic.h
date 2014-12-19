/* Automatically generated at 17.03.2010 12:22:16 */
/* Font: Lucilda Console 13px Size:35 bytes */
#ifndef CHARSETZNAKI16P
#define CHARSETZNAKI16P 1
#include <avr/pgmspace.h>

uint8_t ct_CSZnaki16pXSpaces [] PROGMEM = {0,0,1,2,2,5,6};

uint8_t ct_CSZnaki16pData[] PROGMEM = {
/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4,0xAA,0x0F,0x09,0x0F
,/*0x20  */ /*W,H,YOffXbXa*/5,1,0x0A
  ,0x00,0x00,0x00,0x00,0x00
};
uint8_t ct_CSZnaki16pAddrOffs[] PROGMEM = {
   /* */0x06};

char ct_CSZnaki16pAddToAddr[] PROGMEM = {'\xFF'};

#define cr_CSZnaki16p { \
  /*vi_Height*/     16 \
 ,/*vi_YSpace*/     0 \
 ,/*vt_XSpaces*/    ct_CSZnaki16pXSpaces \
 ,/*vt_Data*/       ct_CSZnaki16pData \
 ,/*vi_AOCharOffs*/ 32 \
 ,/*vi_AOCount*/    1 \
 ,/*vt_AddrOffs*/   ct_CSZnaki16pAddrOffs \
 ,/*vt_AddToAddr*/  ct_CSZnaki16pAddToAddr \
}
#endif
