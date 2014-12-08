#include <inttypes.h>
#include <avr/pgmspace.h>

//konwertuj liczbê szesnastkow¹ do tekstu
char *CSHex2V(
  void    *pi_Number //liczba do wyœwietlenia
 ,uint8_t  pi_Bytes  //iloœæ bajtów z liczby do wyœwietlenia
);

//konwertuj liczbê rzeczywist¹ do tekstu
char *CSFloat2V(
  float   pn_Number      //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits   //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pi_FractDigits //iloœæ cyfr po przecinku
 ,uint8_t	pb_With0Fract  // czy zostawiæ nieznacz¹ce zera w u³amku
 ,uint8_t pb_AlwaysSign  //dopisaæ z przodu +/ /- czy tylko -
);

//drukuj tekst (zakoñczony 0)
void CSPrintV(char* pv_Text);

//konwertuj liczbê dziesiêtn¹ do tekstu
char *CSInt2V(
  int32_t pi_Number     //liczba do wyœwietlenia
 ,uint8_t pi_IntDigits  //minimalna iloœæ cyfr znacz¹cych
 ,uint8_t pb_AlwaysSign //dopisaæ z przodu +/ /- czy tylko -
);