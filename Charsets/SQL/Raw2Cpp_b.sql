CREATE OR REPLACE PACKAGE BODY
Raw2Cpp
AS

gi_Bytes     INTEGER;
gi_MaxXSpace INTEGER;
gt_ImgSize   ITable;

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

PROCEDURE rWriteCyfry IS
BEGIN
  Raw2Cpp.gv_SubSetName := 'Pelny';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE(/* */fiHex2Num('20'));
  Raw2Cpp.gt_ToOutput.DELETE(/*A*/fiHex2Num('41'));
  Raw2Cpp.gt_ToOutput.DELETE(/*B*/fiHex2Num('42'));
  Raw2Cpp.gt_ToOutput.DELETE(/*D*/fiHex2Num('44'));
  Raw2Cpp.gt_ToOutput.DELETE(/*E*/fiHex2Num('45'));
  Raw2Cpp.gt_ToOutput.DELETE(/*F*/fiHex2Num('46'));
  Raw2Cpp.gv_SubSetName := 'Standard';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE(/*+*/fiHex2Num('2B'));
  Raw2Cpp.gt_ToOutput.DELETE(/*,*/fiHex2Num('2C'));
  Raw2Cpp.gt_ToOutput.DELETE(/*-*/fiHex2Num('2D'));
  Raw2Cpp.gt_ToOutput.DELETE(/*C*/fiHex2Num('43'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B0'));
  Raw2Cpp.gv_SubSetName := 'Czas';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE(/*.*/fiHex2Num('2E'));
  Raw2Cpp.gt_ToOutput.DELETE(/*:*/fiHex2Num('3A'));
  Raw2Cpp.gt_ToOutput(/*+*/fiHex2Num('2B')) := /*+*/fiHex2Num('2B');
  Raw2Cpp.gt_ToOutput(/*,*/fiHex2Num('2C')) := /*,*/fiHex2Num('2C');
  Raw2Cpp.gt_ToOutput(/*-*/fiHex2Num('2D')) := /*-*/fiHex2Num('2D');
  Raw2Cpp.gt_ToOutput(/*C*/fiHex2Num('43')) := /*C*/fiHex2Num('43');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('B0')) := /*�*/fiHex2Num('B0');
  Raw2Cpp.gv_SubSetName := 'Temp';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE(/*C*/fiHex2Num('43'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B0'));
  Raw2Cpp.gv_SubSetName := 'Min';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE;
  Raw2Cpp.gt_ToOutput(/* */fiHex2Num('20')) := fiHex2Num('20');

  Raw2Cpp.gv_SubSetName := 'Nic';
  Raw2Cpp.rWriteFile;
END rWriteCyfry;

/*
----------------------------------------------------------------------------------------------------
*/

PROCEDURE rWriteZnaki IS
BEGIN
  Raw2Cpp.gv_SubSetName := 'Pelny';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE(/*!*/fiHex2Num('21'));
  Raw2Cpp.gt_ToOutput.DELETE(/*"*/fiHex2Num('22'));
  Raw2Cpp.gt_ToOutput.DELETE(/*#*/fiHex2Num('23'));
  Raw2Cpp.gt_ToOutput.DELETE(/*$*/fiHex2Num('24'));
  Raw2Cpp.gt_ToOutput.DELETE(/*%*/fiHex2Num('25'));
  Raw2Cpp.gt_ToOutput.DELETE(/*&*/fiHex2Num('26'));
  Raw2Cpp.gt_ToOutput.DELETE(/*'*/fiHex2Num('27'));
--  Raw2Cpp.gt_ToOutput.DELETE(/*(*/fiHex2Num('28'));
--  Raw2Cpp.gt_ToOutput.DELETE(/*)*/fiHex2Num('29'));
  Raw2Cpp.gt_ToOutput.DELETE(/***/fiHex2Num('2A'));
  Raw2Cpp.gt_ToOutput.DELETE(/*/*/fiHex2Num('2F'));
  Raw2Cpp.gt_ToOutput.DELETE(/*<*/fiHex2Num('3C'));
  Raw2Cpp.gt_ToOutput.DELETE(/*=*/fiHex2Num('3D'));
  Raw2Cpp.gt_ToOutput.DELETE(/*>*/fiHex2Num('3E'));
  Raw2Cpp.gt_ToOutput.DELETE(/*?*/fiHex2Num('3F'));
  Raw2Cpp.gt_ToOutput.DELETE(/*@*/fiHex2Num('40'));
  Raw2Cpp.gt_ToOutput.DELETE(/*[*/fiHex2Num('5B'));
  Raw2Cpp.gt_ToOutput.DELETE(/*\*/fiHex2Num('5C'));
  Raw2Cpp.gt_ToOutput.DELETE(/*]*/fiHex2Num('5D'));
  Raw2Cpp.gt_ToOutput.DELETE(/*^*/fiHex2Num('5E'));
  Raw2Cpp.gt_ToOutput.DELETE(/*_*/fiHex2Num('5F'));
  Raw2Cpp.gt_ToOutput.DELETE(/*`*/fiHex2Num('60'));
  Raw2Cpp.gt_ToOutput.DELETE(/*{*/fiHex2Num('7B'));
  Raw2Cpp.gt_ToOutput.DELETE(/*|*/fiHex2Num('7C'));
  Raw2Cpp.gt_ToOutput.DELETE(/*}*/fiHex2Num('7D'));
  Raw2Cpp.gt_ToOutput.DELETE(/*~*/fiHex2Num('7E'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('80'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('A9'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('AE'));
--  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B0'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B1'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B5'));

  Raw2Cpp.gv_SubSetName := 'Standard';
  Raw2Cpp.rWriteFile;

  FOR vi_Idx IN ASCII('a') .. ASCII('z') LOOP
    Raw2Cpp.gt_ToOutput(vi_Idx) := ASCII(UPPER(CHR(vi_Idx)));
  END LOOP;
  Raw2Cpp.gt_ToOutput.DELETE(/*(*/fiHex2Num('28'));
  Raw2Cpp.gt_ToOutput.DELETE(/*)*/fiHex2Num('29'));
  Raw2Cpp.gt_ToOutput.DELETE(/*�*/fiHex2Num('B0'));
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('B9')) := /*�*/fiHex2Num('A5');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('E6')) := /*�*/fiHex2Num('C6');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('EA')) := /*�*/fiHex2Num('CA');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('B3')) := /*�*/fiHex2Num('A3');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('F1')) := /*�*/fiHex2Num('D1');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('F3')) := /*�*/fiHex2Num('D3');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('9C')) := /*�*/fiHex2Num('8C');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('BF')) := /*�*/fiHex2Num('AF');
  Raw2Cpp.gt_ToOutput(/*�*/fiHex2Num('9F')) := /*�*/fiHex2Num('8F');

  Raw2Cpp.gv_SubSetName := 'Duze';
  Raw2Cpp.rWriteFile;

  Raw2Cpp.gt_ToOutput.DELETE;
  Raw2Cpp.gt_ToOutput(/* */fiHex2Num('20')) := fiHex2Num('20');

  Raw2Cpp.gv_SubSetName := 'Nic';
  Raw2Cpp.rWriteFile;
END rWriteZnaki;

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
    vv_Text := vv_Text||CHR(10)||
    fvGetImagesCpp||CHR(10)||
    fvGetImagesSetCpp||CHR(10);
--    '#define ci_CS'||gv_SetName||'YSpace '||gi_YSpace||CHR(10)||
--    '#define ci_CS'||gv_SetName||'LastIdx (0x$LastIdx$-0x20)'||CHR(10)||
--    '#define ci_CS'||gv_SetName||'Height 0x'||fvNum2Hex(gi_Height)||CHR(10)||CHR(10)||
--    '// jak obliczy� adres konkretnego znaku vv_Char?'||CHR(10)||
--    '//uint8_t vi_Idx = vv_Char-vi_IdxOffs;'||CHR(10)||
--    '//if (vi_Idx>vi_LastIdx || (!vt_AddrOffs[vi_Idx]))'||CHR(10)||
--    '//  vp_Addr = vt_Data;'||CHR(10)||
--    '//else {'||CHR(10)||
--    '//  vp_Addr = vt_Data+vt_AddrOffs[vi_Idx];'||CHR(10)||
--    '//  vi_TmpIdx=0; while (vt_AddToAddr[vi_TmpIdx]<=vv_Char) vp_Addr += 0x100;'||CHR(10)||
--    '//}'||CHR(10);

    vv_Text := vv_Text||CHR(10)||
    '#define cr_CS'||gv_SetName||' { \'||CHR(10)||
    '  /*vi_Height*/     '||gi_Height||' \'||CHR(10)||
    ' ,/*vi_YSpace*/     '||gi_YSpace||' \'||CHR(10)||
    ' ,/*vt_XSpaces*/    ct_CS'||gv_SetName||'XSpaces \'||CHR(10)||
    ' ,/*vt_Data*/       ct_CS'||gv_SetName||'Data \'||CHR(10)||
    ' ,/*vi_AOCharOffs*/ '||TO_CHAR(gt_ToOutput.FIRST)||' \'||CHR(10)||
    ' ,/*vi_AOCount*/    '||TO_CHAR(gt_ToOutput.LAST-gt_ToOutput.FIRST+1)||' \'||CHR(10)||
    ' ,/*vt_AddrOffs*/   ct_CS'||gv_SetName||'AddrOffs \'||CHR(10)||
    ' ,/*vt_AddToAddr*/  ct_CS'||gv_SetName||'AddToAddr \'||CHR(10)||
    '}'||CHR(10)||
    '#endif'||CHR(10);
    gi_Bytes := gi_Bytes+12;
    vv_Text := REPLACE(vv_Text,'$size$',gi_Bytes);
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
  vi_Bytes  INTEGER := 1;
  vi_PrevChar INTEGER := 0;
BEGIN
  vi_YOffs := gi_Height-6;
  IF vi_YOffs>15 THEN
    vi_YOffs := 15;
  END IF;
  vv_Text := 'uint8_t ct_CS'||gv_SetName||'Data[] PROGMEM = {'||CHR(10)||
    '/*brak znaku w charsecie*/ /*W,H,YOffXbXa*/3,4'||
    ',0x'||fvNum2Hex(vi_YOffs) || '$XSpaceNone$,0x0F,0x09,0x0F'||CHR(10);
  gi_Bytes := gi_Bytes+6;
  vi_Bytes := 6;
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
      IF vi_YOffs>15 THEN
        vi_YOffs := 15;
      END IF;
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
        ',/*0x'||fvNum2Hex(vi_Char)||' '||CHR(vi_Char)||'*/ /*W,H,YOffXbXa*/'||gt_Width(vi_Char)||','||vi_Height||
          ',0x'||fvNum2Hex(vi_YOffs) || fvNum2Hex(TRUNC(vi_XSpace/4)+MOD(vi_XSpace,4));
      gi_Bytes := gi_Bytes+3;
      vi_Bytes := vi_Bytes+3;
      gt_ImgSize(vi_Char) := 3;
    ELSIF gt_ToOutput.EXISTS(vi_Char) THEN
      vv_Text := vv_Text||',/*0x'||fvNum2Hex(vi_Char)||' '||CHR(vi_Char)||'*/ /*przekieruj pod znak*/0xFF,0x'||fvNum2Hex(gt_ToOutput(vi_Char));
      gi_Bytes := gi_Bytes+2;
      vi_Bytes := vi_Bytes+2;
      gt_ImgSize(vi_Char) := 2;
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
          vi_Bytes := vi_Bytes+1;
          gt_ImgSize(vi_Char) := gt_ImgSize(vi_Char)+1;
        END IF;
      END LOOP;
    END LOOP;

    IF gt_ToOutput.EXISTS(vi_Char) THEN
      vv_Text := vv_Text||CHR(10);
      IF MOD(vi_Bytes,256)=0 THEN
        vv_Text := vv_Text||',/*temporary*/0'||CHR(10);
        gt_ImgSize(vi_Char) := gt_ImgSize(vi_Char)+1;
        gi_Bytes := gi_Bytes+1;
        vi_Bytes := vi_Bytes+1;
      END IF;
      vi_PrevChar := vi_Char;
    END IF;

    vi_CurrX := vi_CurrX+gt_Width(vi_Char)+1;
    vi_ImgNo := vi_ImgNo+1;

    vi_Char := gt_Width.NEXT(vi_Char);
  END LOOP;

  vv_Text := REPLACE(vv_Text,'$XSpaceNone$',fvNum2Hex(TRUNC(gi_MaxXSpace/4)+MOD(gi_MaxXSpace,4)))||'};';

  RETURN vv_Text;
END fvGetImagesCpp;

/*
----------------------------------------------------------------------------------------------------
*/

FUNCTION fvGetImagesSetCpp
RETURN CLOB IS
  vv_Text VARCHAR2(32000);
  vi_Bytes INTEGER := 6;
  vv_Addresses VARCHAR2(32000);
BEGIN
  vv_Text := 'uint8_t ct_CS'||gv_SetName||'AddrOffs[] PROGMEM = {';

  vv_Addresses := 'char ct_CS'||gv_SetName||'AddToAddr[] PROGMEM = {';
  gi_Bytes := gi_Bytes+(gt_ToOutput.LAST-gt_ToOutput.FIRST+1);
  FOR vi_Char IN gt_ToOutput.FIRST .. gt_ToOutput.LAST LOOP
    IF MOD(vi_Char-gt_ToOutput.FIRST,8)=0 THEN
      vv_Text := vv_Text||CHR(10)||'  ';
    END IF;
    IF vi_Char=gt_ToOutput.FIRST THEN
      vv_Text := vv_Text||' ';
    ELSE
      vv_Text := vv_Text||',';
    END IF;
    IF gt_ToOutput.EXISTS(vi_Char) THEN
--      vv_Text := vv_Text||'cv_CS'||gv_SetName||fvNum2Hex(gt_ToOutput(vi_Char))||'-ci_'||gv_SetName||'Addr';
      vv_Text := vv_Text||'/*'||CHR(vi_Char)||'*/0x'||LPAD(fvNum2Hex(MOD(vi_Bytes,256)),2,'0');

      IF TRUNC(vi_Bytes/256)<>TRUNC((vi_Bytes+gt_ImgSize(vi_Char))/256) AND gt_ToOutput.NEXT(vi_Char) IS NOT NULL THEN
        IF gt_ToOutput.NEXT(vi_Char)=ASCII('\') THEN
          vv_Addresses := vv_Addresses||'''\\'',';
        ELSE
          vv_Addresses := vv_Addresses||''''||CHR(gt_ToOutput.NEXT(vi_Char))||''',';
        END IF;
        gi_Bytes := gi_Bytes+1;
      END IF;
      vi_Bytes := vi_Bytes + gt_ImgSize(vi_Char);
    ELSE
--      vv_Text := vv_Text||LPAD('0',LENGTH(gv_SetName)+7);
      vv_Text := vv_Text||LPAD('0',9);
    END IF;
  END LOOP;
--  vv_Text := vv_Text||',cv_CS'||gv_SetName||'None-ci_'||gv_SetName||'Addr};'||CHR(10);
  vv_Text := vv_Text||'};';
  vv_Addresses := vv_Addresses||'''\xFF''};';
  gi_Bytes := gi_Bytes+1;

  RETURN vv_Text||CHR(10)||CHR(10)||vv_Addresses;
END fvGetImagesSetCpp;

/*
----------------------------------------------------------------------------------------------------
*/

END Raw2Cpp;
/
