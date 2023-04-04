Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

#sh ./linux_update_clanlib_manually.sh

#first update clanlib
cd ../../../../ClanLib-1.0
echo Updating ClanLib from SVN
sudo svn update


./autogen.sh
./configure
sudo make install
if [ $? -ne 0 ]
then
echo -e "Error building ClanLib, check it out. \a"
Pause
exit
else
echo "Success!"
fi 

cd ../rtsvn/clanlibstuff/novashell/scripts
