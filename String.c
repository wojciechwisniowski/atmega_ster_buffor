#include <math.h>
#include "String.h"
#include "HD44780.h" 

#define CSOUTTEXTSIZE 16
char gv_CSOutText[CSOUTTEXTSIZE];

char *CSHex2V(
  void    *pi_Number //liczba do wyœwietlenia
 ,uint8_t  pi_Bytes  //iloœæ bajtów z liczby do wyœwietlenia
) {
	uint8_t vi_Byte;
	uint8_t vi_OutIdx,vi_Digit;
	uint8_t *vi_Number;

	vi_Number = (uint8_t *)pi_Number+pi_Bytes-1;
	vi_OutIdx = 0;
	gv_CSOutText[pi_Bytes*2] = 0;

	while (pi_Bytes--) {
		vi_Byte = *vi_Number--;
		vi_Digit = vi_Byte>>4;
		gv_CSOutText[vi_OutIdx++] = vi_Digit+(vi_Digit<10 ? '0' : 'A'-10);
		vi_Digit = vi_Byte&0xF;
		gv_CSOutText[vi_OutIdx++] = vi_Digit+(vi_Digit<10 ? '0' : 'A'-10);
	}
	
	return gv_CSOutText;
} //CSHex2V

void CSPrintV(char *Str){
	GLCD_WriteString(Str);
  }
  
  
  //konwertuj liczbê rzeczywist¹ do tekstu
char *CSFloat2V(
  float   pn_Number      //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits   //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pi_FractDigits //iloœæ cyfr po przecinku
 ,uint8_t pb_With0Fract  // czy zostawiæ nieznacz¹ce zera w u³amku
 ,uint8_t pb_AlwaysSign  //dopisaæ z przodu +/ /- czy tylko -
) {
	uint8_t vi_Idx,vi_OutIdx,vi_Digit;
	uint32_t vi_Number;
	uint8_t  vb_Equal0,vi_InIntDigits,vi_OutIntDigits;
	
	if (!pi_FractDigits) pi_FractDigits = 1;
	
//	for (vi_Idx=0; vi_Idx<pi_FractDigits; vi_Idx++) vn_Number *= 10.0;
	vi_Number = lround(fabs(pn_Number)*pow(10,pi_FractDigits));
	vi_OutIdx = CSOUTTEXTSIZE-2;
	vb_Equal0 = 1;
	if (!pi_IntDigits) vi_InIntDigits = 1;
	else vi_InIntDigits = pi_IntDigits;
	vi_OutIntDigits = 0;

	do {
		if ((vi_Digit=vi_Number%10))
			vb_Equal0 = 0;
		gv_CSOutText[vi_OutIdx--] = '0'+vi_Digit;
		if (pi_FractDigits) {
			if (!--pi_FractDigits)
				gv_CSOutText[vi_OutIdx--] = ',';
		}
		else {
			vi_OutIntDigits++;
			if (vi_InIntDigits) vi_InIntDigits--;
		}
		vi_Number /= 10;
	} while (!vi_OutIntDigits || vi_Number);

	if (pn_Number<0 && !vb_Equal0)
		gv_CSOutText[vi_OutIdx--] = '-';
	else if (pb_AlwaysSign/* || pi_IntDigits*/) {
		if (vb_Equal0/* || !pb_AlwaysSign*/)
			gv_CSOutText[vi_OutIdx--] = ' ';
		else
			gv_CSOutText[vi_OutIdx--] = '+';
	}

	while (vi_InIntDigits--) gv_CSOutText[vi_OutIdx--] = ' ';

	if (!pb_With0Fract) {
		vi_Idx = CSOUTTEXTSIZE-2;
		while (gv_CSOutText[vi_Idx]=='0') vi_Idx--;
			if (gv_CSOutText[vi_Idx]==',') vi_Idx--;
		gv_CSOutText[vi_Idx+1] = 0;	
	}
	else
		gv_CSOutText[CSOUTTEXTSIZE-1] = 0;
	
	return &gv_CSOutText[vi_OutIdx+1];

} //CSFloat2V

//konwertuj liczbê dziesiêtn¹ do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits  //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pb_AlwaysSign //dopisaæ z przodu +/ /- czy tylko -
) {
	uint8_t  vi_OutIdx,vi_Digit;
	uint32_t vi_Number;
	uint8_t  vb_Equal0,vi_InIntDigits,vi_OutIntDigits;

	if (pi_Number>=0) vi_Number = pi_Number;
	else vi_Number = -pi_Number;
	vi_OutIdx = CSOUTTEXTSIZE-2;
	vb_Equal0 = 1;
	if (!pi_IntDigits) vi_InIntDigits = 1;
	else vi_InIntDigits = pi_IntDigits;
	vi_OutIntDigits = 0;

	do {
		if ((vi_Digit=vi_Number%10))
			vb_Equal0 = 0;
		gv_CSOutText[vi_OutIdx--] = '0'+vi_Digit;
		vi_OutIntDigits++;
		if (vi_InIntDigits) vi_InIntDigits--;
		vi_Number /= 10;
	} while (vi_Number);
	
	if (pi_Number<0 && !vb_Equal0)
		gv_CSOutText[vi_OutIdx--] = '-';
	else if (pb_AlwaysSign/* || pi_IntDigits*/) {
		if (vb_Equal0/* || !pb_AlwaysSign*/)
			gv_CSOutText[vi_OutIdx--] = ' ';
		else
			gv_CSOutText[vi_OutIdx--] = '+';
	}

	while (vi_InIntDigits--) gv_CSOutText[vi_OutIdx--] = ' ';

	gv_CSOutText[CSOUTTEXTSIZE-1] = 0;
	
	return &gv_CSOutText[vi_OutIdx+1];

} //CSInt2V
