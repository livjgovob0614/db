@echo clrtemp.bat
del S\*.scp
del ..\obj\*.obj
del ..\obj\*.lib
del *.sym
del *.swp
del /S \papyrus\src\rsrc\dlg\*.bkp
del /S \papyrus\*.tmp
del /S \papyrus\*.temp
rem 
rem 3/01/2005
rem ��������� ������, ������ ����������������� ������ ������� 
rem ������ ���������� �� �������� papyrus\src\lib
rem
rem del ..\lib\*.lib
