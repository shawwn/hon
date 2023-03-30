;--------------------------------
; Headers
;--------------------------------
!include "MUI.nsh"
;--------------------------------

;--------------------------------
; Settings
;--------------------------------
Name "Heroes of Newerth Test (Garena) Server"
OutFile "HoNGarenaServer.exe"

!define MUI_ICON hon.ico
!define MUI_UNICON hon.ico

!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "header.bmp"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_BGCOLOR 000000
!define MUI_FONTCOLOR ffffff

!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKCU
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Heroes of Newerth Test (Garena) Server"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

InstallDir "$PROGRAMFILES\Heroes of Newerth Test (Garena) Server\"

Var StartMenuFolder
;--------------------------------

;-------------------------------
; Test if Visual Studio Redistributables 2005+ SP1 installed
; Returns -1 if there is no VC redistributables intstalled
;--------------------------------
Function CheckVCRedist
   Push $R0
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{7299052b-02a4-4627-81f2-1818da5d550d}" "Version"

   ; if VS 2005+ redist SP1 not installed, install it
   IfErrors 0 VSRedistInstalled
   StrCpy $R0 "-1"

VSRedistInstalled:
   Exch $R0
FunctionEnd
;--------------------------------

;--------------------------------
; Pages
;--------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "tos.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
;--------------------------------

;--------------------------------
; Languages
;--------------------------------
!insertmacro MUI_LANGUAGE "English"
;--------------------------------

;--------------------------------
; Sections
;--------------------------------
Section "Heroes of Newerth Test" main_install
	SetOutPath "$INSTDIR"

	; Visual studio redistributables
	File vcredist_x86.exe
	Call CheckVCRedist
	Pop $R0
	StrCmp $R0 "-1" installvcredist skipvcredist
	installvcredist:
	ExecWait 'vcredist_x86.exe /q:a /c:"vcredi~3.exe /q:a /c:""msiexec /i vcredist.msi /qb!"" "'
	;Delete "$INSTDIR\vcredist_x86.exe"
	skipvcredist:

	;/*
	; Core files
	File hon.exe
	File k2.dll
	File hon_update.exe
	File dbghelp.dll
	File fmodex.dll
	File libcurl.dll
	File libeay32.dll
	File libxml2.dll
	File ssleay32.dll
	File zlibwapi.dll
	File manifest.xml
	File change_log.txt
	File ca-bundle.crt
	File libraries.txt
	File license.txt
	File compat_ignore.txt
	
	; Base files
	SetOutPath "$INSTDIR\base"
	File base\*.s2z
	
	; Game Files
	SetOutPath "$INSTDIR\game"
	File game\game_shared.dll
	File game\game.dll
	File game\resources0.s2z
	
	SetOutPath "$INSTDIR\game\maps"
	File game\maps\caldavar.s2z
	File game\maps\darkwoodvale.s2z
	File game\maps\test.s2z
	File game\maps\test_simple.s2z
	File game\maps\watchtower.s2z
	
	/**/
	
	SetOutPath "$INSTDIR"
	
	; Default settings
	CreateDirectory "$DOCUMENTS\Heroes of Newerth\game\"
	FileOpen $9 "$DOCUMENTS\Heroes of Newerth\game\server.cfg" w
	FileWrite $9 "SetSave vid_textureDownsize 1$\r$\n"
	FileWrite $9 "SetSave vid_shadows false$\r$\n"
	FileWrite $9 "SetSave gfx_foliage true$\r$\n"
	FileWrite $9 "SetSave vid_shadowmapSize 512$\r$\n"
	FileClose $9
	
	; Create start menu shortcuts
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
 	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Dedicated Server.lnk" "$INSTDIR\hon.exe" "-dedicated" "$INSTDIR\hon.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Server Manager.lnk" "$INSTDIR\hon.exe" "-manager" "$INSTDIR\hon.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$SYSDIR\shell32.dll" 145
 	!insertmacro MUI_STARTMENU_WRITE_END
	
	; Uninstaller
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "DisplayName" "Heroes of Newerth Test (Garena) Server"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "DisplayIcon" "$INSTDIR\hon.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "Publisher" "S2 Games"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "HelpLink" "http://forums.heroesofnewerth.com/forumdisplay.php?f=4"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "URLUpdateInfo" "http://www.heroesofnewerth.com"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "DisplayVersion" "1.0.2"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver" "NoRepair" 1
	
	WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop shortcut"
	CreateShortCut "$DESKTOP\Server Manager.lnk" "$INSTDIR\hon.exe" "-manager" "$INSTDIR\hon.exe"
SectionEnd

Section "Quick Launch shortcut"
	CreateShortCut "$QUICKLAUNCH\Server Manager.lnk" "$INSTDIR\hon.exe" "-manager" "$INSTDIR\hon.exe"
SectionEnd

Section "un.Heroes of Newerth Test"
	; Game files
	RMDir /r "$INSTDIR\game"
	RMDir /r "$INSTDIR\base"
	RMDir /r "$INSTDIR\Update"
	
	Delete "$INSTDIR\vcredist_x86.exe"

	Delete "$INSTDIR\hon.exe"
	Delete "$INSTDIR\k2.dll"
	Delete "$INSTDIR\hon_update.exe"
	Delete "$INSTDIR\dbghelp.dll"
	Delete "$INSTDIR\fmodex.dll"
	Delete "$INSTDIR\libcurl.dll"
	Delete "$INSTDIR\libeay32.dll"
	Delete "$INSTDIR\libxml2.dll"
	Delete "$INSTDIR\ssleay32.dll"
	Delete "$INSTDIR\zlibwapi.dll"
	Delete "$INSTDIR\manifest.xml"
	Delete "$INSTDIR\change_log.txt"
	Delete "$INSTDIR\DedicatedServer.bat"
	Delete "$INSTDIR\ca-bundle.crt"
	Delete "$INSTDIR\compat_ignore.txt"

	; Shortcuts
	Delete "$DESKTOP\Server Manager.lnk"
	Delete "$QUICKLAUNCH\Server Manager.lnk"

	ReadRegStr $0 HKCU "Software\Heroes of Newerth Test" "Start Menu Folder"
	Delete "$SMPROGRAMS\$0\Dedicated Server.lnk"
	Delete "$SMPROGRAMS\$0\Server Manager.lnk"
	Delete "$SMPROGRAMS\$0\Uninstall.lnk"
	RMDir "$SMPROGRAMS\$0"

	; Registry keys
	DeleteRegKey HKCR "honserver"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\honserver"
	DeleteRegKey HKCU "Software\Heroes of Newerth Test Server"

	; Uninstaller
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"
SectionEnd

Section /o "un.User Data (Configs, screen shots, replays...)"
	RMDir /r "$DOCUMENTS\Heroes of Newerth"
SectionEnd
;--------------------------------
