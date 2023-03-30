;--------------------------------
; Headers
;--------------------------------
!include "MUI.nsh"
;--------------------------------

;--------------------------------
; Settings
;--------------------------------
Name "Heroes of Newerth"
OutFile "HoNInstall.exe"

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
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Heroes of Newerth"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

InstallDir "$PROGRAMFILES\Heroes of Newerth\"

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
Section "Heroes of Newerth" main_install
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

	; DirectX
	SetOutPath "$INSTDIR\directxredist"
	File directxredist\*
	ExecWait '$INSTDIR\directxredist\dxsetup.exe /silent'
	;Delete "$INSTDIR\directxredist\*"
	SetOutPath "$INSTDIR"
	;RMDir "$INSTDIR\directxredist"

	;/*
	; Core files
	File hon.exe
	File k2.dll
	File vid_d3d9.dll
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
	File libraries.txt
	File license.txt
	File ca-bundle.crt
	
	; Base files
	SetOutPath "$INSTDIR\base"
	File base\*.s2z
	
	SetOutPath "$INSTDIR\base\core"
	File /r /x "CVS" /x ".#*" /x ".cvs*" /x "*_debug.*" base\core\shaders
	
	; Game Files
	SetOutPath "$INSTDIR\game"
	File game\game_shared.dll
	File game\game.dll
	File game\cgame.dll
	File game\resources0.s2z
	File game\textures.s2z
	
	SetOutPath "$INSTDIR\game\maps"
	File game\maps\*.s2z
	
	SetOutPath "$INSTDIR\game\core"
	File /r /x "CVS" /x ".#*" /x ".cvs*" /x "*_debug.*" game\core\shaders
	
	; Editor files
	/*
	SetOutPath "$INSTDIR\editor"
	File editor\cgame.dll
	File editor\*.s2z
	*/
	
	; Model viewer files
	/*
	SetOutPath "$INSTDIR\modelviewer"
	File modelviewer\cgame.dll
	File modelviewer\*.s2z
	SetOutPath "$INSTDIR\modelviewer\maps"
	File modelviewer\maps\*.s2z
	*/
	
	/**/
	
	SetOutPath "$INSTDIR"
	
	
	; Default settings
	CreateDirectory "$DOCUMENTS\Heroes of Newerth\game\"
	FileOpen $9 "$DOCUMENTS\Heroes of Newerth\game\startup.cfg" w
	HwInfo::GetVideoCardMemory
	IntCmp $0 128 graphicslow graphicslow graphicsnotlow
	graphicslow:
		FileWrite $9 "SetSave vid_textureDownsize 1$\r$\n"
		FileWrite $9 "SetSave vid_shadows false$\r$\n"
		FileWrite $9 "SetSave gfx_foliage true$\r$\n"
		FileWrite $9 "SetSave vid_shadowmapSize 512$\r$\n"
		Goto graphicsdone
	graphicsnotlow:
		FileWrite $9 "SetSave vid_shadows true$\r$\n"
		FileWrite $9 "SetSave gfx_foliage true$\r$\n"
		IntCmp $0 256 graphicsmed graphicsmed graphicshigh
	graphicsmed:
		FileWrite $9 "SetSave vid_textureDownsize 0$\r$\n"
		FileWrite $9 "SetSave vid_shadowmapSize 1024$\r$\n"
		Goto graphicsdone
	graphicshigh:
		FileWrite $9 "SetSave vid_textureDownsize 0$\r$\n"
		FileWrite $9 "SetSave vid_shadowmapSize 2048$\r$\n"
		Goto graphicsdone
	graphicsdone:
	
	HwInfo::GetVideoCardPixelShaderModel
	IntCmp $0 2 shaderlow shaderlow shaderhigh
	shaderlow:
		FileWrite $9 "SetSave vid_shaderLightingQuality 1$\r$\n"
		FileWrite $9 "SetSave vid_shaderFalloffQuality 1$\r$\n"
		FileWrite $9 "SetSave vid_shaderFogQuality 0$\r$\n"
		FileWrite $9 "SetSave vid_dynamicLights 0$\r$\n"
		FileWrite $9 "SetSave vid_postEffects 0$\r$\n"
		Goto shaderend
	shaderhigh:
		FileWrite $9 "SetSave vid_shaderLightingQuality 0$\r$\n"
		FileWrite $9 "SetSave vid_shaderFogQuality 0$\r$\n"
		FileWrite $9 "SetSave vid_shaderFalloffQuality 0$\r$\n"
		FileWrite $9 "SetSave vid_dynamicLights 0$\r$\n"
		FileWrite $9 "SetSave vid_postEffects 1$\r$\n"
	Goto shaderend
	shaderend:

	FileClose $9
	
	; Create start menu shortcuts
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
 	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Heroes of Newerth.lnk" "$INSTDIR\hon.exe" "" "$INSTDIR\hon.exe"
	;CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Map Editor.lnk" "$INSTDIR\hon.exe" "-mod game;editor" "$INSTDIR\hon.exe"
	;CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Model Viewer.lnk" "$INSTDIR\hon.exe" "-mod game;modelviewer" "$INSTDIR\hon.exe"
	WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Match Replays.url" "InternetShortcut" "URL" "http://www.honreplays.com"
	WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Create Account.url" "InternetShortcut" "URL" "https://hon.s2games.com/create_account.php"
	WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Player Rankings.url" "InternetShortcut" "URL" "http://www.heroesofnewerth.com/player_ladder.php"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$SYSDIR\shell32.dll" 145
 	!insertmacro MUI_STARTMENU_WRITE_END

	; Create URL association
	WriteRegStr HKCR "hon" "" "URL:Heroes of Newerth Server"
	WriteRegStr HKCR "hon" "URL Protocol" ""
	WriteRegStr HKCR "hon\DefaultIcon" "" "$INSTDIR\hon.exe"
	WriteRegStr HKCR "hon\shell\open\command" "" '$INSTDIR\hon.exe -connect %1'

	; Replay file association
	WriteRegStr HKLM "Software\Classes\.honreplay" "" "hon.Replay"
	WriteRegStr HKLM "Software\Classes\hon.Replay\shell" "" "WatchReplay"
	WriteRegStr HKLM "Software\Classes\hon.Replay\shell\WatchReplay" "" "Watch replay"
	WriteRegStr HKLM "Software\Classes\hon.Replay\shell\WatchReplay\command" "" '"$INSTDIR\hon.exe" set host_autoexec StartReplay #SystemPath(%1)#'
	WriteRegStr HKLM "Software\Classes\hon.Replay\DefaultIcon" "" "$INSTDIR\hon.exe"
	
	; Uninstaller
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "DisplayName" "Heroes of Newerth"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "DisplayIcon" "$INSTDIR\hon.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "Publisher" "S2 Games"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "HelpLink" "http://forums.s2games.com/forumdisplay.php?f=8"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "URLUpdateInfo" "http://www.heroesofnewerth.com"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "DisplayVersion" "1.0.0"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon" "NoRepair" 1
	
	WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop shortcut"
	CreateShortCut "$DESKTOP\Heroes of Newerth.lnk" "$INSTDIR\hon.exe" "" "$INSTDIR\hon.exe"
SectionEnd

Section "Quick Launch shortcut"
	CreateShortCut "$QUICKLAUNCH\Heroes of Newerth.lnk" "$INSTDIR\hon.exe" "" "$INSTDIR\hon.exe"
SectionEnd

Section "un.Heroes of Newerth"
	; Game files
	;RMDir /r "$INSTDIR\modelviewer"
	;RMDir /r "$INSTDIR\editor"
	RMDir /r "$INSTDIR\game"
	RMDir /r "$INSTDIR\base"
	RMDir /r "$INSTDIR\Update"
	
	Delete "$INSTDIR\vcredist_x86.exe"
	RMDir /r "$INSTDIR\directxredist"

	Delete "$INSTDIR\hon.exe"
	Delete "$INSTDIR\k2.dll"
	Delete "$INSTDIR\vid_d3d9.dll"
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
	Delete "$INSTDIR\ca-bundle.crt"
	;Delete "$INSTDIR\Editor.bat"
	;Delete "$INSTDIR\ModelViewer.bat"
	Delete "$INSTDIR\DedicatedServer.bat"

	; Shortcuts
	Delete "$DESKTOP\Heroes of Newerth.lnk"
	Delete "$QUICKLAUNCH\Heroes of Newerth.lnk"

	ReadRegStr $0 HKCU "Software\Heroes of Newerth" "Start Menu Folder"
	Delete "$SMPROGRAMS\$0\Heroes of Newerth.lnk"
	;Delete "$SMPROGRAMS\$StartMenuFolder\Map Editor.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Match Replays.url"
	Delete "$SMPROGRAMS\$StartMenuFolder\Create Account.url"
	Delete "$SMPROGRAMS\$StartMenuFolder\Player Rankings.url"
	Delete "$SMPROGRAMS\$0\Uninstall.lnk"
	RMDir "$SMPROGRAMS\$0"


	; Registry keys
	DeleteRegKey HKCR "hon"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\hon"
	DeleteRegKey HKCU "Software\Heroes of Newerth"
	DeleteRegKey HKLM "Software\Classes\.s2r"
	DeleteRegKey HKLM "Software\Classes\hon.Replay"

	; Uninstaller
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"
SectionEnd

Section /o "un.User Data (Configs, screen shots, replays...)"
	RMDir /r "$DOCUMENTS\Heroes of Newerth"
SectionEnd
;--------------------------------
