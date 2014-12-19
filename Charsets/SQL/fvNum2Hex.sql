CREATE OR REPLACE FUNCTION
fvNum2Hex(pi_Number IN INTEGER) RETURN VARCHAR2 IS
  vi_Digit INTEGER;
  vv_Out   VARCHAR2(8);
BEGIN
  FOR vi_Idx IN REVERSE 0..7 LOOP
    vi_Digit := MOD(TRUNC(pi_Number/POWER(16,vi_Idx)),16);
    IF vi_Digit<10 THEN
      vv_Out := vv_Out||CHR(ASCII('0')+vi_Digit);
    ELSE
      vv_Out := vv_Out||CHR(ASCII('A')+vi_Digit-10);
    END IF;
  END LOOP;
  RETURN NVL(LTRIM(vv_Out,'0'),'0');
END fvNum2Hex;
/
