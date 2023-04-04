#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Running from $PWD - Building shared libaries

echo Building Box2D...
cd Box2D/Source
make
cd ../..

echo Building linearparticle...
cd linearparticle
aclocal
automake
autoconf
./configure
make
cd ..

echo Building lua
cd lua
aclocal
automake
autoconf
./configure
make
cd ..

echo Building luabind
cd luabind
aclocal
automake
autoconf
./configure
make
cd ..


