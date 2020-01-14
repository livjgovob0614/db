CREATE OR REPLACE FUNCTION sqlfunc(id INTEGER)
RETURNS varchar AS $$
declare ret varchar;
BEGIN
  if id > 10 then
    ret = 'C';
  else if id > 0 then
    ret = 'D';
  else
    ret = 'E';
  end if;
END
