#!/bin/bash

packedfilename=novashell.tar.gz

echo "Packing final release... $packedfilename"

cd ..
cd bin
rm log.txt
cd ..

unlink $packedfilenames

mv bin novashell
tar cvfz $packedfilename novashell/novashell novashell/worlds novashell/base novashell/packaging novashell/license.txt novashell/credits.txt novashell/history.txt
mv novashell bin

if [ -f $packedfilename ] 
then

 echo FTPing file
	
   sh ./scripts/linux_upload.sh $packedfilename

 else
  echo -e "Error with gzipping or something. \a"
  Pause
 exit 1;
fi

cd scripts
