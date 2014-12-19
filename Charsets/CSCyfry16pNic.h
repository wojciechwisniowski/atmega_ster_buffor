/* Automatically generated at 17.03.2010 12:22:09 */
/* Font: Lucilda Console 16px Size:36 bytes */
#ifndef CHARSETCYFRY16P
#define CHARSETCYFRY16P 1
#include <avr/pgmspace.h>

uint8_t ct_CSCyfry16pXSpaces [] PROGMEM = {0,1,2,3,4};

uint8_t ct_CSCyfry16pData[] PROGMEM = {
/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4,0xAA,0x0F,0x09,0x0F
,/*0x20  */ /*W,H,YOffXbXa*/8,1,0x0A
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uint8_t ct_CSCyfry16pAddrOffs[] PROGMEM = {
   /* */0x06};

char ct_CSCyfry16pAddToAddr[] PROGMEM = {'\xFF'};

#define cr_CSCyfry16p { \
  /*vi_Height*/     16 \
 ,/*vi_YSpace*/     0 \
 ,/*vt_XSpaces*/    ct_CSCyfry16pXSpaces \
 ,/*vt_Data*/       ct_CSCyfry16pData \
 ,/*vi_AOCharOffs*/ 32 \
 ,/*vi_AOCount*/    1 \
 ,/*vt_AddrOffs*/   ct_CSCyfry16pAddrOffs \
 ,/*vt_AddToAddr*/  ct_CSCyfry16pAddToAddr \
}
#endif
