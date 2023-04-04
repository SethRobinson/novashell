#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo "If I want to test something locally before committing to the real CL respository, this is how I copy from"
echo "my windows box.  Doesn't really belong here but hey."


sudo svn export --force /media/clanlib ~/dev/ClanLib-0.8

Pause;

