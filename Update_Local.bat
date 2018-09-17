@echo off
::Mega128A
Set C_FILES=*.c
Set H_FILES=*.h
Set BAT_FILES=*.bat
Set DOC_FILES=*.docx
Set XLS_FILES=*.xlsx
if exist E:\mega128\sw_mega128\*.c (
    Set SRC_FOLDER1=E:\mega128\sw_mega128) else (
    Set SRC_FOLDER1=D:\mega128\sw_mega128
    )
echo %SRC_FOLDER1%
::Set OBJ_FOLDER1=C:\Myfolder\Project_sz\AVR_prj\mega128\sw_mega128


echo Update Local Files

@echo off
pause

::copy sw_mega128 to U_disk

echo d|xcopy "%SRC_FOLDER1%\%C_FILES%" %cd%"\%C_FILES%" /r/y/D
echo d|xcopy "%SRC_FOLDER1%\%H_FILES%" %cd%"\%H_FILES%" /r/y/D
echo d|xcopy "%SRC_FOLDER1%\%BAT_FILES%" %cd%"\%BAT_FILES%" /r/y/D
echo d|xcopy "%SRC_FOLDER1%\%DOC_FILES%" %cd%"\%DOC_FILES%" /r/y/D
echo d|xcopy "%SRC_FOLDER1%\%XLS_FILES%" %cd%"\%XLS_FILES%" /r/y/D




pause