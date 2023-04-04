#!/bin/bash
Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Running from $PWD
#just in case the network drive wasn't up when we booted...
sudo mount -a
sh ./linux_build_clanlib.sh
cd ../../..
svn update
cd clanlibstuff/novashell/scripts
sh ./linux_build_release.sh
sh ./linux_media_update.sh
sh ./linux_package_release.sh

Pause
