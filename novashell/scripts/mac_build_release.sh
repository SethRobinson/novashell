Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

#sh mac_svn_update.sh

cd /Volumes/projects/clanlibstuff/novashell/mac

echo Building universal release game binaries

xcodebuild -target novashell_release
if [ $? -ne 0 ]
then
echo -e "Error building game, check it out. \a"
Pause
exit
else
echo "Success!"
fi 

#First check to see if our network drive is available

if [ -d /Volumes/projects/clanlibstuff/novashell/bin/base ] 
then

 echo Located network drive.
 else
  echo -e "Network drive not available!!  Turn it on now and hit a key! \a"
  Pause
fi

#refresh media data from the main server (first deleting the old stuff)
rm ~/build/Default/novashell.app/Contents/Resources/log.txt
rm ~/build/Default/novashell.app/Contents/Resources/prefs.dat
rm ~/build/Default/novashell.app/Contents/Resources/temp.tmp
rm -R ~/build/Default/novashell.app/Contents/Resources/base
rm -R ~/build/Default/novashell.app/Contents/Resources/worlds

rm -R ~/build/Default/novashell.app/Contents/Resources/profiles

#copy the new one
cp -R /Volumes/projects/clanlibstuff/novashell/bin/base ~/build/Default/novashell.app/Contents/Resources

#copy the worlds we want over
cp -R /Volumes/projects/clanlibstuff/novashell/bin/worlds ~/build/Default/novashell.app/Contents/Resources

#copy our binaries to the windows server
cp -R ~/build/Default/novashell.app/Contents/MacOS /Volumes/projects/clanlibstuff/novashell/bin/packaging/mac

cp -R ~/build/Default/novashell.app/Contents/Frameworks /Volumes/projects/clanlibstuff/novashell/bin/packaging/mac

cd ../scripts

