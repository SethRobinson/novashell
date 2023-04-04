REM FTP our API reference files to the server, create dirs if need be

call win_make_docs.bat
REM get our ftp logon info
call ../../../SetFTPLogonInfo.bat

ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell/docs api

REM Let's also put the history.txt on the website so it can be easily viewed before download

call ../../../SetFTPLogonInfo.bat
ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell ../bin/history.txt
call ../../../KillFTPLogonInfo.bat

call ../../../SetFTPLogonInfo.bat
ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell ../license.txt
call ../../../KillFTPLogonInfo.bat

call ../../../SetFTPLogonInfo.bat
ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell ../readme.txt
call ../../../KillFTPLogonInfo.bat

call ../../../SetFTPLogonInfo.bat
ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell ../bin/credits.txt
call ../../../KillFTPLogonInfo.bat

REM remove environmental vars we used
set C_FILENAME=
call ../../../KillFTPLogonInfo.bat
