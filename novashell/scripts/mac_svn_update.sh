#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}


#sh mac_update_clanlib_manually.sh

echo Updating Clanlib from SVN
cd ~/dev/ClanLib-1.0
svn update

echo Building clanlib...
xcodebuild -project ClanLib_xcode_v2x.xcodeproj -configuration Deployment
if [ $? -ne 0 ]
then
echo -e "Error building clanlib. \a"
Pause
exit
else
echo "Success!"
fi 
cd ../../rtsvn
echo Updating project from SVN
svn update
cd clanlibstuff/novashell/scripts