#!/bin/bash

# Example "ex72.sh" modified to use encrypted password.

#  Note that this is still rather insecure,
#+ since the decrypted password is sent in the clear.
#  Use something like "ssh" if this is a concern.

E_BADARGS=65

if [ -z "$1" ]
then
  echo "Usage: `basename $0` filename"
  exit $E_BADARGS
fi  

Username=rtsoft           # Change to suit.
Password=`cat ~/ftp_pass.file`
# File containing password in plain text.
Filename=`basename $1`  # Strips pathname out of file name.

Server="rtsoft.com"
Directory="www/novashell"         # Change above to actual server name & directory.

echo FTPing $Filename

ftp -nv $Server <<End-Of-Session
user $Username $Password
binary
bell
cd $Directory
put $Filename
bye
End-Of-Session
# -n option to "ftp" disables auto-logon.
# Note that "bell" rings 'bell' after each file transfer.

exit 0
