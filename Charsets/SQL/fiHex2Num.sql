CREATE OR REPLACE FUNCTION
fiHex2Num(pv_Text VARCHAR2) RETURN INTEGER IS
  vv_Text VARCHAR2(100) := pv_Text;
  vi_Out  INTEGER       := 0;
BEGIN
  LOOP
    EXIT WHEN vv_Text IS NULL;
    vi_Out := vi_Out*16;
    IF SUBSTR(vv_Text,1,1)<='9' THEN
      vi_Out := vi_Out+ASCII(SUBSTR(vv_Text,1,1))-ASCII('0');
    ELSE
      vi_Out := vi_Out+ASCII(UPPER(SUBSTR(vv_Text,1,1)))-ASCII('A')+10;
    END IF;
    vv_Text := SUBSTR(vv_Text,2);
  END LOOP;
  RETURN vi_Out;
END fiHex2Num;
/
