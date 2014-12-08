#ifndef MYMATH_H
#define MYMATH_H 1

#include <inttypes.h>

//dziel wersja unsigned long
#define DivUL_INLINE 0
#define DivUI_INLINE 1

//dziel wersja unsigned long
#define DivULUL(pi_Value,pi_Div) (((uint32_t)(pi_Value)+(pi_Div)/2)/(pi_Div))
#if DivUL_INLINE
#define DivUL(pi_Value,pi_Div) (((uint32_t)(pi_Value)+(pi_Div)/2)/(pi_Div))
#else
uint32_t DivUL(
  uint32_t pi_Value
 ,uint16_t pi_Div
);
#endif

//EMA wersja unsigned long
uint32_t EmaUL(
  uint32_t pi_NewValue
 ,uint32_t pi_PrevEMA
 ,uint16_t pi_Period
);

//dziel wersja long
int32_t DivL(
  int32_t  pi_Value
 ,uint16_t pi_Div
);

//EMA wersja long
int32_t EmaL(
  int32_t  pi_NewValue
 ,int32_t  pi_PrevEMA
 ,uint16_t pi_Period
);

//dziel wersja unsigned integer
#if DivUI_INLINE
#define DivUI(pi_Value,pi_Div) (((uint16_t)(pi_Value)+(pi_Div)/2)/(pi_Div))
#else
uint32_t DivUI(
  uint16_t pi_Value
 ,uint8_t  pi_Div
);
#endif

//EMA wersja unsigned integer
uint16_t EmaUI(
  uint16_t pi_NewValue
 ,uint16_t pi_PrevEMA
 ,uint8_t  pi_Period
);

//dziel wersja integer
int16_t DivI(
  int16_t pi_Value
 ,uint8_t pi_Div
);

//EMA wersja integer
int16_t EmaI(
  int16_t pi_NewValue
 ,int16_t pi_PrevEMA
 ,uint8_t pi_Period
);

#endif
