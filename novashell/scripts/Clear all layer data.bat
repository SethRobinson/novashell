rem move to the correct drive letter
%~d0%
rem move to the correct directory
cd %~d0%~p0
cd ..\bin
del layer.dat /s
pause