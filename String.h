#include <inttypes.h>
#include <avr/pgmspace.h>

//konwertuj liczb� szesnastkow� do tekstu
char *CSHex2V(
  void    *pi_Number //liczba do wy�wietlenia
 ,uint8_t  pi_Bytes  //ilo�� bajt�w z liczby do wy�wietlenia
);

//konwertuj liczb� rzeczywist� do tekstu
char *CSFloat2V(
  float   pn_Number      //liczba do wy�wietlenia
 ,uint8_t pi_IntDigits   //minimalna ilo�� cyfr znacz�cych
 ,uint8_t pi_FractDigits //ilo�� cyfr po przecinku
 ,uint8_t	pb_With0Fract  // czy zostawi� nieznacz�ce zera w u�amku
 ,uint8_t pb_AlwaysSign  //dopisa� z przodu +/ /- czy tylko -
);

//drukuj tekst (zako�czony 0)
void CSPrintV(char* pv_Text);

//konwertuj liczb� dziesi�tn� do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wy�wietlenia
 ,uint8_t pi_IntDigits  //minimalna ilo�� cyfr znacz�cych
 ,uint8_t pb_AlwaysSign //dopisa� z przodu +/ /- czy tylko -
);