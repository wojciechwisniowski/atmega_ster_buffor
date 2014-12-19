CREATE OR REPLACE PACKAGE BODY
Raw2Cpp
AS

gi_Bytes     INTEGER;
gi_MaxXSpace INTEGER;

/*
----------------------------------------------------------------------------------------------------
*/

PROCEDURE rReadFile IS
  v_FileHandle sys.utl_file.file_type;
  vv_Raw       RAW(32767);
  vv_Text      VARCHAR2(32000);
BEGIN
  gt_Image   .DELETE;
  gt_ToOutput.DELETE;
  gt_Width   .DELETE;
  v_FileHandle := sys.utl_file.FOpen(gv_Path, gv_SetName||'.raw', 'rb', 32767);
  sys.utl_file.get_raw(v_FileHandle, vv_Raw, 32000);
  vv_Text := sys.utl_raw.cast_to_varchar2(vv_Raw);
  sys.utl_file.fclose(v_FileHandle);
  IF LENGTH(vv_Text)<>gi_Width*gi_Height THEN
    raise_application_error(-20000,'Odczytano '||LENGTH(vv_Text)||' znakow, a powinno byc '||gi_Width||'*'||gi_Height||'='||TO_CHAR(gi_Width*gi_Height));
  END IF;
  FOR vi_X IN 1..gi_Width LOOP
    FOR vi_Y IN 1..gi_Height LOOP
--      IF (vi_Y-1)*pi_Width+(vi_X-1)+1>gi_Width*gi_Height THEN
--        raise_application_error(-20000,'Za wysoki indeks');
--      END IF;
      gt_Image(vi_X)(vi_Y) := ASCII(SUBSTR(vv_Text,(vi_Y-1)*gi_Width+(vi_X-1)+1,1));
    END LOOP;
  END LOOP;
END rReadFile;

/*
----------------------------------------------------------------------------------------------------
*/

PROCEDURE rWriteFile IS
  v_FileHandle sys.utl_file.file_type;
  vv_Text      CLOB := fvGetAllCpp;
BEGIN
  v_FileHandle := sys.utl_file.FOpen(gv_Path, 'CS'||gv_SetName||gv_SubSetName||'.h', 'w', 32767);
  FOR vi_Idx IN 0..TRUNC(dbms_lob.getLength(vv_Text)/32000) LOOP
    sys.utl_file.put(v_FileHandle,dbms_lob.substr(vv_Text,32000,vi_Idx*32000+1));
    sys.utl_file.fflush(v_FileHandle);
  END LOOP;
  sys.utl_file.fclose(v_FileHandle);
END rWriteFile;

/*
----------------------------------------------------------------------------------------------------
*/

FUNCTION fvGetAllCpp
RETURN CLOB IS
  vv_Text CLOB;
BEGIN
  gi_MaxXSpace := 0;
  gi_Bytes     := gt_XSpaces.COUNT;
  vv_Text :=
    '/* Automatically generated at '||TO_CHAR(SYSDATE,'DD.MM.YYYY HH24:MI:SS')||' */'||CHR(10)||
    '/* '||gv_Comment||' Size:$size$ bytes */'||CHR(10)||
    '#ifndef CHARSET'||upper(gv_SetName)||CHR(10)||
    '#define CHARSET'||upper(gv_SetName)||' 1'||CHR(10)||
    '#include <avr/pgmspace.h>'||CHR(10)||CHR(10)||
    'uint8_t ct_CS'||gv_SetName||'XSpaces [] PROGMEM = {';
    FOR vi_Idx IN gt_XSpaces.FIRST .. gt_XSpaces.LAST LOOP
      vv_Text := vv_Text||gt_XSpaces(vi_Idx);
      IF vi_Idx<gt_XSpaces.LAST THEN
        vv_Text := vv_Text||',';
      ELSE
        vv_Text := vv_Text||'};'||CHR(10);
      END IF;
    END LOOP;
    vv_Text := vv_Text||
    '#define ci_CS'||gv_SetName||'YSpace '||gi_YSpace||CHR(10)||
    '#define ci_CS'||gv_SetName||'LastIdx 0x$LastIdx$-0x20'||CHR(10)||
    '#define ci_CS'||gv_SetName||'Height 0x'||fvNum2Hex(gi_Height)||CHR(10)||CHR(10)||
    fvGetImagesCpp||CHR(10)||
    fvGetImagesSetCpp||CHR(10)||
    '#endif'||CHR(10);
    vv_Text := REPLACE(vv_Text,'$size$',gi_Bytes);
    vv_Text := REPLACE(vv_Text,'$LastIdx$',fvNum2Hex(gt_ToOutput.LAST+1));
  RETURN vv_Text;
END fvGetAllCpp;

/*
----------------------------------------------------------------------------------------------------
*/

FUNCTION fvGetImagesCpp
RETURN CLOB IS
  vi_Idx    INTEGER;
  vi_ImgNo  INTEGER := 0;
  vi_CurrX  INTEGER := 1;
  vi_Char   INTEGER;
  vi_Byte   INTEGER;
  vv_Text   VARCHAR2(32000);
  vi_Height INTEGER;
  vi_YOffs  INTEGER;
  vi_XSpace INTEGER;
BEGIN
  IF gt_ToOutput.FIRST IS NULL THEN
    vi_Idx := gt_Width.FIRST;
    LOOP
      EXIT WHEN vi_Idx IS NULL;
      gt_ToOutput(vi_Idx) := vi_Idx;
      vi_Idx := gt_Width.NEXT(vi_Idx);
    END LOOP;
  END IF;
  vi_Char := gt_Width.FIRST;
  LOOP
    EXIT WHEN vi_Char IS NULL;

    vi_Height := 0;
    vi_YOffs  := 255;
    FOR vi_Y IN 1..gi_Height LOOP
      FOR vi_X IN vi_CurrX .. vi_CurrX+gt_Width(vi_Char)-1 LOOP
        IF vi_YOffs=255 AND gt_Image(vi_x)(vi_Y)>0 THEN
          vi_YOffs := vi_Y-1;
        END IF;
        IF gt_Image(vi_X)(vi_Y)>0 THEN
          vi_Height := vi_Y;
        END IF;
      END LOOP;
    END LOOP;
    IF vi_Height=0 THEN
      vi_YOffs  := 0;
      vi_Height := 1;
    ELSE
      vi_Height := vi_Height-vi_YOffs;
    END IF;

    IF gt_ToOutput.EXISTS(vi_Char) AND gt_ToOutput(vi_Char)=vi_Char THEN
      BEGIN
        vi_XSpace := gt_CharXSpaces(vi_Char);
      EXCEPTION WHEN no_data_found THEN
        vi_XSpace := 16+1;
      END;
      gi_MaxXSpace := GREATEST(gi_MaxXSpace,vi_XSpace);
      vv_Text := vv_Text||
        'uint8_t cv_CS'||gv_SetName||fvNum2Hex(vi_Char)||'[] /*'||CHR(vi_Char)||'*/ PROGMEM = {/*W,H,YOffs,XSpace*/'||gt_Width(vi_Char)||','||vi_Height||','||vi_YOffs||',0x'||fvNum2Hex(vi_XSpace);
      gi_Bytes := gi_Bytes+4;
    END IF;

    FOR vi_Y8 IN 0..TRUNC((vi_Height-1)/8) LOOP
      IF gt_ToOutput.EXISTS(vi_Char) AND gt_ToOutput(vi_Char)=vi_Char THEN
        vv_Text := vv_Text||CHR(10)||'  ';
      END IF;
      FOR vi_X IN vi_CurrX .. vi_CurrX+gt_Width(vi_Char)-1 LOOP
        vi_Byte := 0;
        FOR vi_Y IN 0 .. 7 LOOP
          IF vi_YOffs+vi_Y8*8+vi_Y+1<=gi_Height THEN
            vi_Byte := vi_Byte+gt_Image(vi_X)(vi_YOffs+vi_Y8*8+vi_Y+1)*POWER(2,vi_Y);
          END IF;
        END LOOP;
        IF gt_ToOutput.EXISTS(vi_Char) AND gt_ToOutput(vi_Char)=vi_Char THEN
          vv_Text := vv_Text||',0x'||LPAD(fvNum2Hex(vi_Byte),2,'0');
          gi_Bytes := gi_Bytes+1;
        END IF;
      END LOOP;
    END LOOP;

    IF gt_ToOutput.EXISTS(vi_Char) AND gt_ToOutput(vi_Char)=vi_Char THEN
      vv_Text := vv_Text||'};'||CHR(10);
    END IF;

    vi_CurrX := vi_CurrX+gt_Width(vi_Char)+1;
    vi_ImgNo := vi_ImgNo+1;

    vi_Char := gt_Width.NEXT(vi_Char);
  END LOOP;

  vv_Text := vv_Text||
    'uint8_t cv_CS'||gv_SetName||'None [] /*brak znaku w charsecie*/ PROGMEM = {/*W,H,YOffs,XSpace*/3,4,'||(gi_Height-6)||',0x'||fvNum2Hex(gi_MaxXSpace)||',15,9,15};'||CHR(10);
  gi_Bytes := gi_Bytes+7;

  RETURN vv_Text;
END fvGetImagesCpp;

/*
----------------------------------------------------------------------------------------------------
*/

FUNCTION fvGetImagesSetCpp
RETURN CLOB IS
  vv_Text VARCHAR2(32000);
BEGIN
  vv_Text :=
    '// pod indeksem ASCII(znak)-0x20 adres do obrazka danego znaku'||CHR(10)||
    'uint8_t *cv_CS'||gv_SetName||'[] PROGMEM = {';
  gi_Bytes := gi_Bytes+2*(gt_ToOutput.LAST-30);
  FOR vi_Char IN 32 .. gt_ToOutput.LAST LOOP
    IF MOD(vi_Char,8)=0 THEN
      vv_Text := vv_Text||CHR(10)||'  ';
    END IF;
    IF vi_Char=32 THEN
      vv_Text := vv_Text||' ';
    ELSE
      vv_Text := vv_Text||',';
    END IF;
    IF gt_ToOutput.EXISTS(vi_Char) THEN
      vv_Text := vv_Text||'cv_CS'||gv_SetName||fvNum2Hex(gt_ToOutput(vi_Char));
    ELSE
      vv_Text := vv_Text||LPAD('0',LENGTH(gv_SetName)+7);
    END IF;
  END LOOP;
  vv_Text := vv_Text||',cv_CS'||gv_SetName||'None};'||CHR(10);

  RETURN vv_Text;
END fvGetImagesSetCpp;

/*
----------------------------------------------------------------------------------------------------
*/

END Raw2Cpp;
/
