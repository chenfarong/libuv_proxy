rem @echo off
title ����VS2010�����ļ�
cls
color 0A

ECHO ɾ����ʱĿ¼
dir /A:D /s  Debug /B >Debug.txt
dir /A:D /s  x64 /B >x64.txt


rem for /F?%i? in (x64.txt) do 
for /f %%i in (x64.txt) do rmdir /S /Q %%i 
for /f %%i in (Debug.txt) do rmdir /S /Q %%i 


rmdir /S /Q x64 
rmdir /S /Q Debug 

del /S *.aps
del /S *.ncb
del /S *.htm
del /S *.obj
del /S *.manifest
del /S *.pch
del /S *.pdb
del /S *.idb
del /S *.ilk
del /S *.exp
del /S *.res
del /S *.dep
del /S *.bsc
del /S *.sbr
del /S *.dsp
del /S *.dsw
del /S *.dev
del /S *.sdf
del /S *.cod
del /S *.ipch
del /S *.tlog
del /S *.log
del /S *.plg
del /S *.opt
del /S *.positions
del /S *.stc
del /S *.stt
del /S ���ྫƷԴ�����.*
del /S �����ؿ�.txt
del /S *.db
del /S *.o
del /S *.tlog
del /S .vs
del /S .VC.db


ECHO �������

pause