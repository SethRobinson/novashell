del novashell\bin\game.exe

SET VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\

IF NOT EXIST "%VSPATH%" (
  echo Could not find Visual Studio 2022. Please make sure it is installed and the path is correct.
  pause
  exit /b 1
)

echo Compiling the solution using Visual Studio 2022...
call "%VSPATH%vcvars64.bat"
MSBuild "novashell.sln" /t:Build /p:Configuration=Release;Platform=Win32

if %errorlevel% neq 0 (
  echo There was an error during compilation.
   pause
  exit /b 1

)

echo Compilation was successful.



:First, sign the exe if needed (naturally this will only work on Seth's computer)
call %RT_PROJECTS%\Signing\sign.bat novashell\bin\game.exe "Novashell Game Creation System"

rmdir tempbuild /S /Q

copy novashell\bin\game.exe novashell\bin\packaging\win /Y


mkdir tempbuild
xcopy novashell\bin\base tempbuild\base\ /E /F /Y /I
xcopy novashell\bin\packaging tempbuild\packaging\ /E /F /Y /I
xcopy novashell\bin\worlds tempbuild\worlds\ /E /F /Y /I
copy novashell\bin\game.exe tempbuild
copy novashell\bin\credits.txt tempbuild
copy novashell\bin\history.txt tempbuild
copy novashell\bin\readme.txt tempbuild
copy novashell\bin\Play_at_* tempbuild


:create the archive
set FNAME=Novashell_Windows.zip
del %FNAME%
%RT_PROTON_UTIL%\7za.exe a -r -tzip %FNAME% tempbuild

:Rename the root folder
%RT_PROTON_UTIL%\7z.exe rn %FNAME% tempbuild\ Novashell\

pause
