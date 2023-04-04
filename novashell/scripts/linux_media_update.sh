#!/bin/bash

#erase the old stuff
echo Erasing local media directory
sudo rm -R ../bin/base
sudo rm -R ../bin/worlds

echo Coping media tree from master server
cp -r /media/projects/clanlibstuff/novashell/bin/base ../bin
cp -r /media/projects/clanlibstuff/novashell/bin/worlds ../bin

cp -r /media/projects/clanlibstuff/novashell/bin/packaging ../bin
