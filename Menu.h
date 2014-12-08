#ifndef MENU_H
#define MENU_H 1

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define MENU_WITH_MATRIX 0 //czy dorzuciæ kod do obs³ugi edycji tablic
#define MENU_MATRIX_ONEVALUE 0 //czy w obs³udze tablic obs³u¿yæ ekstra przypadek 1x1

#define MENU_HEIGHT 9
#define MENU_CS CS_Z9p
#define MENU_INACTIVE_PROC_EXIT_SEC 90 //wychodŸ z procedury po 90 sekundach od ostatniego wciœniêcia przycisku
#define MENU_INACTIVE_MENU_EXIT_SEC 30 //wychodŸ z menu po 60 sekundach od ostatniego wciœniêcia przycisku

#if MENU_WITH_MATRIX
typedef struct {
	char *vv_TextBefore,*vv_TextAfter; //tekst do wydrukowania na kolumnie
	uint8_t vi_Width,vi_Digits,vb_With0; //szerokoœæ kolumny, iloœæ cyfr,czy poprzedzaæ zerami
	uint8_t vi_MinValue,vi_MaxValue; //minimalna i maksymalna wartoœæ dla zmiennej
} MENUMatrixCol;

typedef struct {
	uint8_t vi_Cols,vi_Rows; //iloœæ kolumn, wierszy, szerokoœæ
	uint8_t vi_CS,vi_Width,vi_RowHeight; //charset, szeroœæ ca³oœci, wysokoœæ wiersza
	MENUMatrixCol *vx_MatrixCols; //lista definicji kolumn
	uint8_t *vx_EEValues; //wskaŸnik do tablicy dwuwymiarowej przechowywanej w EEPROM [ROWS][COLS]
	uint8_t *vx_Values; //wskaŸnik do tablicy dwuwymiarowej przechowywanej w RAM [ROWS][COLS]
} MENUMatrix;
#endif

typedef struct {
	char   *vv_Text; //tekst w pamiêci programu
	void   *vt_SubMenu; //wskaŸnik do podrzêdnego menu
	void   *vx_Matrix; //wskaŸnik do tablicy elementów do modyfikacji
	void  (*vp_Procedure)(uint8_t,uint8_t,int8_t); //procedura do wywo³ania
	//procedura w formacie void ChangeValue(uint8_t pi_Y,uint8_t pi_Param,int8_t pi_Value)
	//pi_Y: wyznaczany przez modu³
	//pi_Param: co zmieniamy, wartoœæ statyczna zapisana w kolejnym polu
	//pi_Value: o ile zmieniamy; jeœli tylko wydrukowaæ bez zmiany to przekazane 0
	uint8_t vi_Param;
} MENURow;

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//uruchamiaj menu - dla wejœcia z zwen¹trz wywo³aj MENUExecute(0,0,0)
void MENUExecute(
  MENURow *pt_Menu //tablica z menu
 ,uint8_t pi_Y     //na jakiej pozycji
 ,uint8_t pi_Level //jaki poziom menu
);

#endif