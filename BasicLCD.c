#include "BasicLCD.h"

#include <avr/IO.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "Charset.h"
#include "SED1520.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================
int16_t gi_LCDCurrX;
int8_t gi_LCDCurrY8;

//==============================================================
// FORWARD DECLARATIONS
//==============================================================

void LCDWriteCmd(uint8_t pi_Data);

#if LCD_CODESIZE>=1
//wy�lij dane do wy�wietlacza
static inline void LCDWriteCmdInternal(uint8_t pi_Data) __attribute__((always_inline));
//wy�lij dane do wy�wietlacza
static inline void LCDWriteDataInternal(uint8_t pi_Data) __attribute__((always_inline));
#else
//wy�lij dane do wy�wietlacza
void LCDWriteCmdInternal(uint8_t pi_Data);
//wy�lij dane do wy�wietlacza
void LCDWriteDataInternal(uint8_t pi_Data);
#endif

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//*********************************
//zresetuj wy�wietlacz po starcie
void LCDInit(void) {
	GLCD_Init();
} //LCDInit

//*********************************
//wy�lij dane do wy�wietlacza
void LCDWriteData(uint8_t pi_Data) {
	LCDWriteDataInternal(pi_Data);
} //LCDWriteData

//*********************************
//odczytaj tekst z wy�wietlacza
void LCDReadRow(uint8_t *pt_Data, uint8_t pi_Bytes) {
	if (gi_LCDCurrY8 < 0 && gi_LCDCurrY8 >= LCD_H8)
		return;
	int16_t vi_X = gi_LCDCurrX;
	int8_t vi_Y8 = gi_LCDCurrY8;

	while (pi_Bytes--) {
		if (gi_LCDCurrX >= 0 && gi_LCDCurrX < LCD_W) {
			*pt_Data++ = GLCD_ReadData();
		} else {
			pt_Data++;
		}
		gi_LCDCurrX++;
		LCDSetXY8(gi_LCDCurrX, gi_LCDCurrY8);
	}
	LCDSetXY8(vi_X, vi_Y8);
} //LCDReadRow

//*********************************
//wype�nij lub wyczy�� wiersz
void LCDFillClrRow(uint8_t pb_Fill, uint8_t pi_Bytes, uint8_t pi_Mask) {
	uint8_t vi_InByte, vi_OutByte, vi_Idx;
	vi_InByte =
			(pb_Fill && (!(gb_LCDNegMode ^ gb_LCDGlobalNegMode)))
					|| ((!pb_Fill) && (gb_LCDNegMode ^ gb_LCDGlobalNegMode)) ?
					pi_Mask : 0;

	if (pi_Mask != 0xFF) {
		//odczytaj dane z wy�wietlacza i wy�lij po��czone mask�
		uint8_t vv_FromLCDData[LCD_W/*pi_Bytes*/];
		LCDReadRow(vv_FromLCDData, pi_Bytes);
		vi_Idx = 0;
		while (pi_Bytes--) {
			vi_OutByte = vi_InByte | (vv_FromLCDData[vi_Idx++] & ~pi_Mask);
			LCDWriteDataInternal(vi_OutByte);
		}
	} else
		//zapisz dane bez odczytywania
		while (pi_Bytes--)
			LCDWriteDataInternal(vi_InByte);
} //LCDFillClrRow

//*********************************
//wy�lij lini� na wy�wietlacz z pami�ci programu
void LCDWriteRow_p(uint8_t *pv_Data1       //adres do pami�ci programu linii Y-1
		, uint8_t *pv_Data2       //adres do pami�ci programu linii Y
		, uint8_t pi_MoveBits //ile przesun�� bity dla pv_Data2 (np: pv_Data1+2:10010111 01010101 pi_MoveBits:1 out:1001011(1 0101010)1
		, uint8_t pi_Bytes       //ilo�� bajt�w do wys�ania
		, uint8_t pi_SpaceXBefore //ile odst�pu doda� (wyczy�ci�) przed wydrukiem
		, uint8_t pi_SpaceXAfter //ile odst�pu doda� (wyczy�ci�) po wydruku
		, uint8_t pi_Mask        //z jak� mask� wysy�a�
		) {
	uint8_t vi_Idx, vi_LCDIdx;
	uint8_t vi_MoveBits1, vi_NegMask;
	uint8_t vi_Negative;
#if LCD_CODESIZE<=1
	uint8_t vi_Byte;
#endif

	vi_Negative = (gb_LCDNegMode ^ gb_LCDGlobalNegMode) ? 0xFF : 0;

	vi_MoveBits1 = 8 - pi_MoveBits;
	vi_Idx = 0;

	if (gi_LCDCurrY8
			< 0|| gi_LCDCurrY8>=LCD_H8 || gi_LCDCurrX+pi_Bytes+pi_SpaceXBefore+pi_SpaceXAfter<0 || gi_LCDCurrX>=LCD_W)return
;	if (pi_Mask != 0xFF) {
		//odczytaj dane z wy�wietlacza i wy�lij po��czone mask�
		uint8_t vv_FromLCDData[LCD_W/*pi_Bytes+pi_SpaceXBefore+pi_SpaceXAfter*/];
		LCDReadRow(vv_FromLCDData, pi_Bytes + pi_SpaceXBefore + pi_SpaceXAfter);
		vi_NegMask = ~pi_Mask;
		vi_LCDIdx = 0;
		while (pi_SpaceXBefore--)
			LCDWriteDataInternal(
					(vv_FromLCDData[vi_LCDIdx++] & vi_NegMask)
							| (vi_Negative & pi_Mask));
#if LCD_CODESIZE<=1
		while (pi_Bytes--) {
			if (pv_Data1)
				vi_Byte = pgm_read_byte(&pv_Data1[vi_Idx]) >> vi_MoveBits1;
			else
				vi_Byte = 0;
			if (pv_Data2)
				vi_Byte |= pgm_read_byte(&pv_Data2[vi_Idx]) << pi_MoveBits;
			LCDWriteDataInternal(
					(vv_FromLCDData[vi_LCDIdx] & vi_NegMask)
							| ((vi_Byte ^ vi_Negative) & pi_Mask));
			vi_Idx++;
			vi_LCDIdx++;
		}
#else
		if (pv_Data1 && pv_Data2)
		while(pi_Bytes--) {
			LCDWriteDataInternal((vv_FromLCDData[vi_LCDIdx]&vi_NegMask)
					|((((pgm_read_byte(&pv_Data1[vi_Idx])>>vi_MoveBits1)
											|(pgm_read_byte(&pv_Data2[vi_Idx])<<pi_MoveBits))^vi_Negative)&pi_Mask)
			);
			vi_Idx++; vi_LCDIdx++;
		}
		else if (pv_Data1)
		while(pi_Bytes--) {
			LCDWriteDataInternal((vv_FromLCDData[vi_LCDIdx]&vi_NegMask)
					|(((pgm_read_byte(&pv_Data1[vi_Idx])>>vi_MoveBits1)^vi_Negative)&pi_Mask));
			vi_Idx++; vi_LCDIdx++;
		}
		else if (pv_Data2)
		while(pi_Bytes--) {
			LCDWriteDataInternal((vv_FromLCDData[vi_LCDIdx]&vi_NegMask)
					|(((pgm_read_byte(&pv_Data2[vi_Idx])<<pi_MoveBits)^vi_Negative)&pi_Mask));
			vi_Idx++; vi_LCDIdx++;
		}
		else
		while(pi_Bytes--) {
			LCDWriteDataInternal((vv_FromLCDData[vi_LCDIdx]&vi_NegMask)|(vi_Negative&pi_Mask));
			vi_Idx++; vi_LCDIdx++;
		}
#endif
		while (pi_SpaceXAfter--)
			LCDWriteDataInternal(
					(vv_FromLCDData[vi_LCDIdx++] & vi_NegMask)
							| (vi_Negative & pi_Mask));
	}

	else {
		//zapisz dane bez odczytywania
		while (pi_SpaceXBefore--)
			LCDWriteDataInternal(vi_Negative);
#if LCD_CODESIZE<=1
		while (pi_Bytes--) {
			if (pv_Data1)
				vi_Byte = pgm_read_byte(&pv_Data1[vi_Idx]) >> vi_MoveBits1;
			else
				vi_Byte = 0;
			if (pv_Data2)
				vi_Byte |= pgm_read_byte(&pv_Data2[vi_Idx]) << pi_MoveBits;
			LCDWriteDataInternal(vi_Byte ^ vi_Negative);
			vi_Idx++;
		}
#else
		if (pv_Data1 && pv_Data2)
		while(pi_Bytes--) {
			LCDWriteDataInternal(((pgm_read_byte(&pv_Data1[vi_Idx])>>vi_MoveBits1)
							|(pgm_read_byte(&pv_Data2[vi_Idx])<<pi_MoveBits))^vi_Negative);
			vi_Idx++;
		}
		else if (pv_Data1)
		while(pi_Bytes--) {
			LCDWriteDataInternal((pgm_read_byte(&pv_Data1[vi_Idx])>>vi_MoveBits1)^vi_Negative);
			vi_Idx++;
		}
		else if (pv_Data2)
		while(pi_Bytes--) {
			LCDWriteDataInternal((pgm_read_byte(&pv_Data2[vi_Idx])<<pi_MoveBits)^vi_Negative);
			vi_Idx++;
		}
		else
		while(pi_Bytes--) {
			LCDWriteDataInternal(vi_Negative);
		}
#endif
		while (pi_SpaceXAfter--)
			LCDWriteDataInternal(vi_Negative);
	}

} //LCDWriteRow_p

//*********************************
//ustaw wsp�rz�dne x,y na wy�wietlaczu
void LCDSetXY8(int16_t pi_X, int8_t pi_Y8) {
	gi_LCDCurrX  = pi_X;
	gi_LCDCurrY8 = pi_Y8;

	if (pi_Y8<0 || pi_Y8>=LCD_H8 || pi_X<0 || pi_X>=LCD_W)
		return;

	GLCD_GoTo(pi_X, pi_Y8);
} //LCDSetXY8

//*********************************
//ustaw jasno�� pod�wietlania LED
//void LCDSetHighlight(
//  uint8_t pi_LedLow  //0 - warto�� ignorowana
// ,uint8_t pi_LedHigh //0 - warto�� ignorowana
//){
//	} //LCDSetHighlight

//*********************************
//zmie� jasno�� pod�wietlania LED: : procedura do wywo�ania w menu
//	void LCDChangeHighlight(  uint8_t pi_Y
//			 ,uint8_t pi_What
//			 ,int8_t  pi_Value){
//		} //LCDChangeHighlight

//*********************************
//w��cz/wy��cz pod�wietlanie LED
//		void LCDSwitchHighlight(uint8_t pb_Mode) {
//		} //LCDOnOffHighlight

//==============================================================
// PRIVATE PROCEDURES
//==============================================================

//*********************************
//wy�lij komend� do wy�wietlacza
//		void LCDWriteCmd(uint8_t pi_Cmd) {
//
//		} //LCDWriteCmd

//*********************************
//wy�lij komend� do wy�wietlacza
//		void LCDWriteCmdInternal(uint8_t pi_Cmd) {
//
//		} //LCDWriteCmdInternal

//*********************************
//wy�lij dane do wy�wietlacza
void LCDWriteDataInternal(uint8_t pi_Data) {
	GLCD_WriteData(pi_Data);
} //LCDWriteDataInternal
