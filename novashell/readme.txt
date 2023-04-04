(these notes are from 2011 or something and badly outdated. Uh, ignore them and read the ones on the github!)

** Novashell notes **

* LINUX: Doesn't work with Syngergy mouse/keyboard controller very well...


** To build Novashell from source **

** WINDOWS **

Install or checkout Clanlib 1.0.x (not 2.0) from the latest svn  Also download the related precompiled
binaries you will need from here:  http://clanlib.org/download-binaries-win32-vc80.html

Make sure the examples work ok and everything compiles.

Check out the Novashell SVN tree somewhere from svn://rtsoft.com/rtsvn.

Load clanlibstuff/novashell/source/novashell.sln with VC8.

You may get errors about missing projects like "clanCore" or "clanDisplay", you can just ignore and remove these projects, 
sometimes I have those projects added for when I debug clanLib and Novashell together.

You may need to add the path to Clanlib's libs (or put them in SharedLib/lib and SharedLib/include where it expects
them), but otherwise it should compile ok, as everything else uses relative paths and is included.
(LinearParticle, Lua, etc)

Oops:  One more thing, you will need to grab the boost libraries.  Novashell (well, luabind, really) requires a few of its headers,
it doesn't need any libraries compiled so this is easy.

Boost download:  http://www.boost.org/users/download/#releases

(Currently it's at Version 1.38.0, don't download the Jam one, download the one under it)

After you download it, drag the "boost" directory out of the zip and into your novashell source directory, or someplace else
and just add its parent directory as an include path in the vs project settings.

You should now be able to click build from inside VS 2005 and have everything build, then copy the .exe over
to a novashell install to try it.

Note:  Build Static MT Release or Static MT Debug.  Ignore the other ones...

--- TO GET WINDOWS BUILD SCRIPTS/ETC WORKING (not required!)

The .bat file novashell/scripts/win_build.bat is a script that will build the game.
The .bat file novashell/scripts/win_package.bat is a script that will create the installer, build the docs and
ftp everything to my site.

 To use these, you would need to do the following:
 
  * Add <checkoutdir>util to your system path
  * Install NSIS 2.35+ in <checkoutdir>util/NSIS
  * Install the "AccessControl" NSIS plugin (the installer sets some permissions to make it easier
    for users to edit files)
  * install naturaldocs in <checkoutdir>util/naturaldocs  (http://naturaldocs.org/ )
  
Create a batch file in <checkoutdir> called SetFTPLogonInfo.bat with the following:

SET _FTP_SITE_= rtsoft.com
SET _FTP_USER_= username
SET _FTP_PASS_= password


** OSX **

Checkout Clanlib 1.0.x from SVN from www.clanlib.org.  Also download the related precompiled
binaries you will need from here: http://clanlib.org/download-binaries-osx-gcc40-universal.html

After checking out the Novashell SVN tree, open clanlibstuff/novashell/mac/novashell.xcodeproj, you may need to
remove the Clanlib packages and re-add them from wherever you installed and built clanlib.

-- Updating from SVN/building/packing from the command line (Not required!) 
The script scripts/mac_build_and_package_release.sh will SVN update everything, build Clanlib, build Novashell, package
it up into a dmg and upload it.  You'd have to edit the scripts involved to get this working...

** LINUX **

Checkout Clanlib 1.0.0 from SVN from www.clanlib.org.  You'll also need to grab the additional libs clanlib needs, but
hey, you're a linux guy, you can probably figure out the dependencies.

Run the its automake stuff and do "make install".

Next, check out all of rtsvn.  (svn checkout svn://rtsoft.com/rtsvn)

From its root, do:
sh linux_make_novashell.sh


Linux troubleshooting:

Ok, for anyone getting this error with novashell in linux:

"./novashell: error while loading shared libraries: libclanApp-0.8.so.1: cannot open shared object file: No such file or directory"

It might be because your clanlib stuff is in usr/local/lib instead of usr/lib.

To fix this, from the shell prompt, right before running novashell, do this:

export LD_LIBRARY_PATH=/usr/local/lib

(to check that it 'took', you can view it like this)

echo $LD_LIBRARY_PATH

(you should see it say: /user/local/lib )

Novashell should now be able to find the clanlib library stuff.

That's it!

Good luck!

-Seth