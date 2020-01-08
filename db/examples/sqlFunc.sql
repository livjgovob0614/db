CREATE OR REPLACE FUNCTION sqlFunc(clientId INTEGER)
RETURNS varchar AS $$
declare result varchar;
BEGIN
  if clientId > 1000 then
    result = "Z_ID";
  else
    result = "X_ID";
  end if;
  return result;
END;
