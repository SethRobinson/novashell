
REM This will FTP the latest build into the correct dir at RTSOFT

if "%C_FILENAME%" == "" ( 
   echo C_FILENAME not set.
   beeper.exe /p
   exit
  )

REM get our ftp logon info
call ../../../SetFTPLogonInfo.bat

REM create script for the FTP process

if exist ftp.tmp del ftp.tmp

echo user %_FTP_USER_% %_FTP_PASS_% >> ftp.tmp
echo cd www >> ftp.tmp
echo cd novashell >> ftp.tmp
echo binary >> ftp.tmp
echo put ..\%C_FILENAME% >> ftp.tmp
echo quit >> ftp.tmp

ftp -n -i -d -s:ftp.tmp %_FTP_SITE_%

del ftp.tmp

REM remove environmental vars we used
set C_FILENAME=
call ../../../KillFTPLogonInfo.bat
