#ifndef MENU_H
#define MENU_H 1

#include <inttypes.h>

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define MENU_WITH_MATRIX 0 //czy dorzuci� kod do obs�ugi edycji tablic
#define MENU_MATRIX_ONEVALUE 0 //czy w obs�udze tablic obs�u�y� ekstra przypadek 1x1

#define MENU_HEIGHT 9
#define MENU_CS CS_Z9p
#define MENU_INACTIVE_PROC_EXIT_SEC 90 //wychod� z procedury po 90 sekundach od ostatniego wci�ni�cia przycisku
#define MENU_INACTIVE_MENU_EXIT_SEC 30 //wychod� z menu po 60 sekundach od ostatniego wci�ni�cia przycisku

#if MENU_WITH_MATRIX
typedef struct {
	char *vv_TextBefore,*vv_TextAfter; //tekst do wydrukowania na kolumnie
	uint8_t vi_Width,vi_Digits,vb_With0; //szeroko�� kolumny, ilo�� cyfr,czy poprzedza� zerami
	uint8_t vi_MinValue,vi_MaxValue; //minimalna i maksymalna warto�� dla zmiennej
} MENUMatrixCol;

typedef struct {
	uint8_t vi_Cols,vi_Rows; //ilo�� kolumn, wierszy, szeroko��
	uint8_t vi_CS,vi_Width,vi_RowHeight; //charset, szero�� ca�o�ci, wysoko�� wiersza
	MENUMatrixCol *vx_MatrixCols; //lista definicji kolumn
	uint8_t *vx_EEValues; //wska�nik do tablicy dwuwymiarowej przechowywanej w EEPROM [ROWS][COLS]
	uint8_t *vx_Values; //wska�nik do tablicy dwuwymiarowej przechowywanej w RAM [ROWS][COLS]
} MENUMatrix;
#endif

typedef struct {
	char   *vv_Text; //tekst w pami�ci programu
	void   *vt_SubMenu; //wska�nik do podrz�dnego menu
	void   *vx_Matrix; //wska�nik do tablicy element�w do modyfikacji
	void  (*vp_Procedure)(uint8_t,uint8_t,int8_t); //procedura do wywo�ania
	//procedura w formacie void ChangeValue(uint8_t pi_Y,uint8_t pi_Param,int8_t pi_Value)
	//pi_Y: wyznaczany przez modu�
	//pi_Param: co zmieniamy, warto�� statyczna zapisana w kolejnym polu
	//pi_Value: o ile zmieniamy; je�li tylko wydrukowa� bez zmiany to przekazane 0
	uint8_t vi_Param;
} MENURow;

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//uruchamiaj menu - dla wej�cia z zwen�trz wywo�aj MENUExecute(0,0,0)
void MENUExecute(
  MENURow *pt_Menu //tablica z menu
 ,uint8_t pi_Y     //na jakiej pozycji
 ,uint8_t pi_Level //jaki poziom menu
);

#endif