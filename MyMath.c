#include "mymath.h"


//*********************************
//dziel wersja unsigned long
#if !DivUL_INLINE
uint32_t DivUL(
  uint32_t pi_Value
 ,uint16_t pi_Div
) {
	return (pi_Value+pi_Div/2)/pi_Div;
} //DivUL
#endif


//*********************************
//EMA wersja unsigned long
uint32_t EmaUL(
  uint32_t pi_NewValue
 ,uint32_t pi_PrevEMA
 ,uint16_t pi_Period
) {
	uint32_t vi_OutValue = DivUL(pi_NewValue+pi_PrevEMA*(pi_Period-1),pi_Period);
	if (vi_OutValue==pi_PrevEMA) {
		if (pi_NewValue>vi_OutValue) vi_OutValue++;
		else if (pi_NewValue<vi_OutValue) vi_OutValue--;
	}
	return vi_OutValue;
} //EmaUL


//*********************************
//dziel wersja long
int32_t DivL(
  int32_t  pi_Value
 ,uint16_t pi_Div
) {
	int32_t vi_OutValue = pi_Value+pi_Div/2;
	if (pi_Value<0) vi_OutValue -= pi_Div;
	return vi_OutValue/pi_Div;
} //DivL


//*********************************
//EMA wersja long
int32_t EmaL(
  int32_t  pi_NewValue
 ,int32_t  pi_PrevEMA
 ,uint16_t pi_Period
) {
	int32_t vi_OutValue = DivL(pi_NewValue+pi_PrevEMA*(pi_Period-1),pi_Period);
	if (vi_OutValue==pi_PrevEMA) {
		if (pi_NewValue>vi_OutValue) vi_OutValue++;
		else if (pi_NewValue<vi_OutValue) vi_OutValue--;
	}
	return vi_OutValue;
} //EmaL


//*********************************
//dziel wersja unsigned integer
#if !DivUI_INLINE
uint32_t DivUI(
  uint16_t pi_Value
 ,uint8_t  pi_Div
) {
	return (pi_Value+pi_Div/2)/pi_Div;
} //DivUI
#endif


//*********************************
//EMA wersja unsigned integer
uint16_t EmaUI(
  uint16_t pi_NewValue
 ,uint16_t pi_PrevEMA
 ,uint8_t  pi_Period
) {
	uint16_t vi_OutValue = DivUL(pi_NewValue+(uint32_t)pi_PrevEMA*(pi_Period-1),pi_Period);
	if (vi_OutValue==pi_PrevEMA) {
		if (pi_NewValue>vi_OutValue) vi_OutValue++;
		else if (pi_NewValue<vi_OutValue) vi_OutValue--;
	}
	return vi_OutValue;
} //EmaUI


//*********************************
//dziel wersja integer
int16_t DivI(
  int16_t pi_Value
 ,uint8_t pi_Div
) {
	if (pi_Value<0)
		return (pi_Value+pi_Div/2)/pi_Div;
	else
		return (pi_Value-pi_Div/2)/pi_Div;
} //DivI


//*********************************
//EMA wersja integer
int16_t EmaI(
  int16_t pi_NewValue
 ,int16_t pi_PrevEMA
 ,uint8_t pi_Period
) {
	int16_t vi_OutValue = DivL((int32_t)pi_NewValue+(int32_t)pi_PrevEMA*(pi_Period-1),pi_Period);
	if (vi_OutValue==pi_PrevEMA) {
		if (pi_NewValue>vi_OutValue) vi_OutValue++;
		else if (pi_NewValue<vi_OutValue) vi_OutValue--;
	}
	return vi_OutValue;
} //EmaI
