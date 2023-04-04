#!/bin/bash

filename="novashell"

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Running from $PWD - building $filename
cd ..
echo Setting up for retail build

#erase file that was there
rm -f ../bin/$filename

#first go build/update the support libraries
cd ../../SharedLib
sh linux_build_libs.sh
cd ../clanlibstuff/novashell

#actually make it
aclocal
autoheader
automake -ac
autoconf
./configure
make

#success?

if [ -f source/$filename ] 
then
 echo Successfully built it.
 mv source/$filename bin/$filename
  cd bin  
  strip -s $filename
  cd ..
 else
  echo -e "Error building executable! \a"
  Pause
 exit 1;
fi
#return to the dir from whence we came
cd scripts
