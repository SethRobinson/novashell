;NSIS Modern User Interface version 1.70
;Header Bitmap Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General
  SetCompressor lzma
  !define _TITLE_ "Novashell Game Creation System"
  !define _VERSION_ "$%C_TEXT_VERSION%"
  !define _COMPILE_DATE_ "${__DATE__}"

  ;Name and file
  Name "${_TITLE_}"
 ; Icon
!define MUI_ICON "novashell.ico" 
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

RequestExecutionLevel admin

OutFile "..\$%C_FILENAME%"

!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome_side.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome_side.bmp"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Novashell"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\RTSOFT\NOVASHELL" "path"

BrandingText " "
;InitPluginsDir
;  File /oname=$PLUGINSDIR\splash.bmp "path\to\your\bitmap.bmp"
;  advsplash::show 1000 600 400 -1 $PLUGINSDIR\splash
;  Pop $0

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "header.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "header.bmp"
  !define MUI_ABORTWARNING
 
;--------------------------------
;Pages

  
  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${_TITLE_}. (${_VERSION_} released on ${_COMPILE_DATE_})\r\n\r\nClick Next to continue."
  !define MUI_WELCOMEPAGE_TITLE "${_TITLE_} ${_VERSION_} Installer"
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\bin\readme.txt"
  
  
  ;Customize component texts
  
  !define MUI_COMPONENTSPAGE_TEXT_COMPLIST "Select extra components to install."
  !define MUI_COMPONENTSPAGE_TEXT_TOP "Here, you can choose whether or not you want a desktop icon in addition to the normal start menu options."
  !insertmacro MUI_PAGE_COMPONENTS
  
  
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
 
!define MUI_FINISHPAGE_LINK "Click here to visit the RTsoft website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.rtsoft.com/"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
 
 !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
 
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

;Remove the - part in from of Main Game if you don't want it to be hidden...
Section "-Main Game" SecMain
SectionIn RO
  SetOutPath "$INSTDIR"
  File "..\bin\game.exe"
  File "..\bin\readme.txt"
  File "..\bin\license.txt"
  File "..\bin\history.txt"
  File "..\bin\credits.txt"
;  File "..\bin\libjpeg.dll"
;  File "..\bin\libpng13.dll"
;  File "..\bin\zlib1.dll"
;  File "..\bin\fmod.dll"
 
  SetOutPath "$INSTDIR"
  File /r "..\bin\base"
  File /r "..\bin\worlds"
SetOutPath "$INSTDIR\packaging"
  File /r "..\bin\packaging\mac"
SetOutPath "$INSTDIR"

AccessControl::GrantOnFile "$INSTDIR" "(S-1-5-32-545)" "GenericRead + GenericWrite + DeleteChild"

CreateDirectory "$SMPROGRAMS\${_TITLE_}"
;ok, this will create a Folder in your Start menue

  ;Store installation folder
  WriteRegStr HKLM "Software\RTSOFT\NOVASHELL" "path" $INSTDIR

  ;write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}" "DisplayName" "${_TITLE_} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\${_TITLE_}\${_TITLE_}.lnk" "$INSTDIR\game.exe" "" "$INSTDIR\game.exe"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Report a bug or make a comment.url" "InternetShortcut" "URL" "http://www.rtsoft.com/pages/feedback_app.htm?game=${_TITLE_}&version=${_VERSION_}"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Robinson Technologies Website.url" "InternetShortcut" "URL" "http://www.rtsoft.com"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Getting started - Tutorials and help.url" "InternetShortcut" "URL" "http://www.rtsoft.com/novashell/docs"
  ;CreateShortCut "$SMPROGRAMS\${_TITLE_}\Quick Help.lnk" "$INSTDIR\help\documentation.htm" ; use defaults for parameters, icon, etc.
  CreateShortCut "$SMPROGRAMS\${_TITLE_}\Uninstall ${_TITLE_}.lnk" "$INSTDIR\Uninstall.exe" ; use defaults for parameters, icon, etc.

; file associations

; back up old value of .opt
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".novashell" ""
  StrCmp $1 "" "${Index}-NoBackup"
    StrCmp $1 "NovashellFile" "${Index}-NoBackup"
    WriteRegStr HKCR ".novashell" "backup_val" $1
"${Index}-NoBackup:"
  WriteRegStr HKCR ".novashell" "" "NovashellFile"
  ReadRegStr $0 HKCR "NovashellFile" ""
  StrCmp $0 "" 0 "${Index}-Skip"
	WriteRegStr HKCR "NovashellFile" "" "Novashell World"
	WriteRegStr HKCR "NovashellFile\shell" "" "open"
	WriteRegStr HKCR "NovashellFile\DefaultIcon" "" "$INSTDIR\game.exe,0"
"${Index}-Skip:"
  WriteRegStr HKCR "NovashellFile\shell\open\command" "" \
    '"$INSTDIR\game.exe" "%1"'
  ;WriteRegStr HKCR "OptionsFile\shell\edit" "" "Edit Options File"
  ;WriteRegStr HKCR "OptionsFile\shell\edit\command" "" \
   ;'$INSTDIR\execute.exe "%1"'
 
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
!undef Index


;novazip

; back up old value of .opt
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".novazip" ""
  StrCmp $1 "" "${Index}-NoBackup"
    StrCmp $1 "NovashellZipFile" "${Index}-NoBackup"
    WriteRegStr HKCR ".novazip" "backup_val" $1
"${Index}-NoBackup:"
  WriteRegStr HKCR ".novazip" "" "NovashellZipFile"
  ReadRegStr $0 HKCR "NovashellZipFile" ""
  StrCmp $0 "" 0 "${Index}-Skip"
	WriteRegStr HKCR "NovashellZipFile" "" "Novashell Zipped World"
	WriteRegStr HKCR "NovashellZipFile\shell" "" "open"
	WriteRegStr HKCR "NovashellZipFile\DefaultIcon" "" "$INSTDIR\game.exe,0"
"${Index}-Skip:"
  WriteRegStr HKCR "NovashellZipFile\shell\open\command" "" \
    '"$INSTDIR\game.exe" "%1"'
  ;WriteRegStr HKCR "OptionsFile\shell\edit" "" "Edit Options File"
  ;WriteRegStr HKCR "OptionsFile\shell\edit\command" "" \
   ;'$INSTDIR\execute.exe "%1"'
 
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
!undef Index


SectionEnd


;Installer Sections

Section "Desktop Icon" SecDesktopIcon
  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\${_TITLE_}.lnk" "$INSTDIR\game.exe" 

SectionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\game.exe"
FunctionEnd



;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "The main game files, these are required to play the game."
  LangString DESC_SecDesktopIcon ${LANG_ENGLISH} "This option will throw one of those handy desktop icons on the main menu for easy access to the program."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopIcon} $(DESC_SecDesktopIcon)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

;default delete

  Delete "$INSTDIR\game.exe"
  Delete "$INSTDIR\history.txt"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\credits.txt"
  Delete "$INSTDIR\Uninstall.exe"
  
;  Delete "$INSTDIR\libjpeg.dll"
;  Delete "$INSTDIR\libpng13.dll"
;  Delete "$INSTDIR\zlib1.dll"
;  Delete "$INSTDIR\fmod.dll"
  Delete "$INSTDIR\log.txt"
  RMDir /r "$INSTDIR\base"
  
  DeleteRegKey HKLM "Software\RTSOFT\NOVASHELL\path"

  DeleteRegKey /ifempty HKLM "Software\RTSOFT\NOVASHELL"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}"
  Delete "$SMPROGRAMS\${_TITLE_}\*.*"
  RMDir "$SMPROGRAMS\${_TITLE_}"
  Delete "$DESKTOP\${_TITLE_}.lnk"

;delete user stuff too?

MessageBox MB_YESNO "Would you like to also delete pres, all world data and saved games? (warning: if you made changes or added worlds, they will be deleted)" IDNO skip_it 
RMDir /r "$INSTDIR\worlds"
RMDir /r "$INSTDIR\profiles"
Delete "$INSTDIR\prefs.dat"

RMDir "$INSTDIR"
skip_it:

  ;start of restore script
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".novashell" ""
  StrCmp $1 "NovashellFile" 0 "${Index}-NoOwn" ; only do this if we own it
    ReadRegStr $1 HKCR ".novashell" "backup_val"
    StrCmp $1 "" 0 "${Index}-Restore" ; if backup="" then delete the whole key
      DeleteRegKey HKCR ".novashell"
    Goto "${Index}-NoOwn"
"${Index}-Restore:"
      WriteRegStr HKCR ".novashell" "" $1
      DeleteRegValue HKCR ".novashell" "backup_val"
   
    DeleteRegKey HKCR "NovashellFile" ;Delete key with association settings
 
    System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
"${Index}-NoOwn:"
!undef Index
  ;rest of script 
  
  
  
   ;start of restore script
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".novazip" ""
  StrCmp $1 "NovashellZipFile" 0 "${Index}-NoOwn" ; only do this if we own it
    ReadRegStr $1 HKCR ".novazip" "backup_val"
    StrCmp $1 "" 0 "${Index}-Restore" ; if backup="" then delete the whole key
      DeleteRegKey HKCR ".novazip"
    Goto "${Index}-NoOwn"
"${Index}-Restore:"
      WriteRegStr HKCR ".novazip" "" $1
      DeleteRegValue HKCR ".novazip" "backup_val"
   
    DeleteRegKey HKCR "NovashellZipFile" ;Delete key with association settings
 
    System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
"${Index}-NoOwn:"
!undef Index


SectionEnd