REM ** Make sure american code page is used, otherwise the %DATE environmental var might be wrong
CHCP 437

REM Set full path to destination
set C_TARGET_DIR=c:\TEMPBACKUP

SET C_FILENAME=novashell_%DATE:~4,2%_%DATE:~7,2%.zip
del %RTBACKUP%\clanlibstuff\%C_FILENAME% > NUL

REM Erase everything in that dir (if it existed)
rd %C_TARGET_DIR% /S /Q

REM Recreate it
md %C_TARGET_DIR%

REM prepare things before copy

REM Move the required lib files over
xcopy %RTPROJECT%\SharedLib %C_TARGET_DIR%\SharedLib /I /E /D /EXCLUDE:SharedLibExcludeList.txt

REM Move the required utility files over
xcopy %RTPROJECT%\Util %C_TARGET_DIR%\Util /I /E /D /EXCLUDE:UtilExcludeList.txt

md %C_TARGET_DIR%/clanlibstuff
REM Move the Program files over
xcopy %RTPROJECT%\clanlibstuff\novashell %C_TARGET_DIR%\clanlibstuff\novashell /I /E /D /EXCLUDE:ProgramExcludeList.txt

REM additional setup for before zipping


REM finish up in the same dir we started in

//go zip it

c:
cd \TEMPBACKUP

wzzip %RTBACKUP%\ClanlibStuff\%C_FILENAME% -rp
pause
