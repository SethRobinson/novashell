#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Packing release
cd /Volumes/projects/clanlibstuff/novashell/mac

#update our mac app with mac distribution stuff and the latest windows binaries
mkdir ~/build/Default/novashell.app/Contents/Resources/packaging
mkdir ~/build/Default/novashell.app/Contents/Resources/packaging/mac
cp -R /Volumes/projects/clanlibstuff/novashell/bin/packaging/mac/Resources ~/build/Default/novashell.app/Contents/Resources/packaging/mac
cp /Volumes/projects/clanlibstuff/novashell/bin/packaging/mac/Info.plist ~/build/Default/novashell.app/Contents/Resources/packaging/mac
cp /Volumes/projects/clanlibstuff/novashell/bin/packaging/mac/Pkginfo ~/build/Default/novashell.app/Contents/Resources/packaging/mac

cp -R /Volumes/projects/clanlibstuff/novashell/bin/packaging/win ~/build/Default/novashell.app/Contents/Resources/packaging


mkdir dist

#clean out the old one
rm -R ./dist/novashell.app
#copy in the new one
echo Copying files to dist folder
cp -R ~/build/Default/novashell.app ./dist


rm ./novashell.dmg
#hdiutil internet-enable -yes novashell.dmg
#return to the script directory

echo Creating dmg
make
echo Done
sh ../scripts/linux_upload.sh novashell.dmg
cd ../scripts
Pause