CREATE OR REPLACE FUNCTION sqlFunc(clientId INTEGER)
RETURNS varchar AS $$
declare result varchar;
BEGIN
  if clientId > 1000 then
    if (clientId > 1500) then
      result = "TT_ID";
    else
      result = "TF_ID";
    end if;
  else
    result = "F_ID";
  end if;
  return result;
END;
