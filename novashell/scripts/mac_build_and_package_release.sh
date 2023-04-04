#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

cd /Volumes/projects/clanlibstuff/novashell/scripts
sh ./mac_build_release.sh
sh ./mac_package_release.sh
Pause