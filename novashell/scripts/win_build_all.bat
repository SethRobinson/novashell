REM ** Make sure american code page is used, otherwise the %DATE environmental var might be wrong
CHCP 437
cd ..
call vnet.bat
set C_TARGET_EXE=bin\game.exe

REM erase it so we know it got built right
del %C_TARGET_EXE% > NUL

REM Compile everything 
devenv source\novashell.sln /build "Static MT Release"

REM make it smaller
REM upx.exe %C_TARGET_EXE%

REM Make sure the file compiled ok
if not exist %C_TARGET_EXE% beeper.exe /p

REM Copy to the packing dir, not used for windows, but OSX/Linux want the exe here
copy %C_TARGET_EXE% bin\packaging\win
cd scripts
call DeleteGarbageFiles.bat