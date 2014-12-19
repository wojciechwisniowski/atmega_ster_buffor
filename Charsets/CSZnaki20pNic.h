/* Automatically generated at 17.03.2010 12:22:17 */
/* Font: Lucilda Console 16px Size:37 bytes */
#ifndef CHARSETZNAKI20P
#define CHARSETZNAKI20P 1
#include <avr/pgmspace.h>

uint8_t ct_CSZnaki20pXSpaces [] PROGMEM = {0,1,1,2,3,5,6};

uint8_t ct_CSZnaki20pData[] PROGMEM = {
/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4,0xEA,0x0F,0x09,0x0F
,/*0x20  */ /*W,H,YOffXbXa*/7,1,0x0A
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uint8_t ct_CSZnaki20pAddrOffs[] PROGMEM = {
   /* */0x06};

char ct_CSZnaki20pAddToAddr[] PROGMEM = {'\xFF'};

#define cr_CSZnaki20p { \
  /*vi_Height*/     20 \
 ,/*vi_YSpace*/     0 \
 ,/*vt_XSpaces*/    ct_CSZnaki20pXSpaces \
 ,/*vt_Data*/       ct_CSZnaki20pData \
 ,/*vi_AOCharOffs*/ 32 \
 ,/*vi_AOCount*/    1 \
 ,/*vt_AddrOffs*/   ct_CSZnaki20pAddrOffs \
 ,/*vt_AddToAddr*/  ct_CSZnaki20pAddToAddr \
}
#endif
