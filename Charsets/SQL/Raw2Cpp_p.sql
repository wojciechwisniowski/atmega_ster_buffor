CREATE OR REPLACE PACKAGE
Raw2Cpp
AS

TYPE ITable  IS TABLE OF INTEGER       INDEX BY BINARY_INTEGER;
TYPE I2Table IS TABLE OF ITable        INDEX BY BINARY_INTEGER;
TYPE VTable IS TABLE  OF VARCHAR2(100) INDEX BY BINARY_INTEGER;

gt_Width       ITable;  -- szerokosci kolejnych znakow => indeksowane znaczkiem ascii
gi_Width       INTEGER; -- sumaryczna szerokosc
gi_Height      INTEGER; -- sumaryczna wysokosc
gt_Image       I2Table; -- odczytany obrazek (X)(Y)
gt_XSpaces     ITable; -- odstêp w poziomie po znaku w zaleznosci od zapisanych wartosci w gt_CharXSpaces
gi_YSpace      INTEGER := 1; -- odstêp w pionie po wierszu
gt_CharXSpaces ITable;  -- 0: brak odstêpu; 1: odstêp opcjonalny 2: odstêp obowi¹zkowy
gt_ToOutput    ITable;  -- obrazki do wrzucenia do plików wyjœciowych (Idx-znaku) => Idx-znaku do wydrukowania
gv_Comment     VARCHAR2(200);
gv_Path        VARCHAR2(200);
gv_SetName     VARCHAR2(100);
gv_SubSetName  VARCHAR2(100);

PROCEDURE rReadFile;

PROCEDURE rWriteFile;

PROCEDURE rWriteCyfry;

PROCEDURE rWriteZnaki;

FUNCTION fvGetAllCpp
RETURN CLOB;

FUNCTION fvGetImagesCpp
RETURN CLOB;

FUNCTION fvGetImagesSetCpp
RETURN CLOB;

END Raw2Cpp;
/
