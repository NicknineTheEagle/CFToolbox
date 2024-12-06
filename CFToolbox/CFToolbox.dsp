# Microsoft Developer Studio Project File - Name="CFToolbox" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CFToolbox - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CFToolbox.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CFToolbox.mak" CFG="CFToolbox - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CFToolbox - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CFToolbox - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CFToolbox - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /w /W0 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 CFToolbox.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CFToolbox - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 CFToolbox.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CFToolbox - Win32 Release"
# Name "CFToolbox - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Account.cpp
# End Source File
# Begin Source File

SOURCE=.\Accounts.cpp
# End Source File
# Begin Source File

SOURCE=.\AppDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\AppIcons.cpp
# End Source File
# Begin Source File

SOURCE=.\AppPropertiesPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\Bandwidth.cpp
# End Source File
# Begin Source File

SOURCE=.\cdr.cpp
# End Source File
# Begin Source File

SOURCE=.\CFDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\CFManager.cpp
# End Source File
# Begin Source File

SOURCE=.\CFTBTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CFToolbox.cpp
# End Source File
# Begin Source File

SOURCE=.\CFToolbox.rc
# End Source File
# Begin Source File

SOURCE=.\CFToolboxDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChooseAccount.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigManager.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto.cpp
# End Source File
# Begin Source File

SOURCE=.\CSClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\dictionary.cpp
# End Source File
# Begin Source File

SOURCE=.\Download.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\EnterPasswordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExecutionAnalyser.cpp
# End Source File
# Begin Source File

SOURCE=.\gcf.cpp
# End Source File
# Begin Source File

SOURCE=.\HashMap.cpp
# End Source File
# Begin Source File

SOURCE=.\HashMapStr.cpp
# End Source File
# Begin Source File

SOURCE=.\HistogramCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\iniparser.cpp
# End Source File
# Begin Source File

SOURCE=.\LargeFile.cpp
# End Source File
# Begin Source File

SOURCE=.\launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\Plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertiesPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\QuestionPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryKey.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryNode.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryValue.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryVector.cpp
# End Source File
# Begin Source File

SOURCE=.\Rijndael.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\SHA1.cpp
# End Source File
# Begin Source File

SOURCE=.\socketTools.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitterBar.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\steamClient.cpp
# End Source File
# Begin Source File

SOURCE=.\SteamNetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\strlib.cpp
# End Source File
# Begin Source File

SOURCE=.\TabAdd.cpp
# End Source File
# Begin Source File

SOURCE=.\TabApps.cpp
# End Source File
# Begin Source File

SOURCE=.\TabBandwidth.cpp
# End Source File
# Begin Source File

SOURCE=.\TabFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\TabHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\TabLogs.cpp
# End Source File
# Begin Source File

SOURCE=.\TabPreferences.cpp
# End Source File
# Begin Source File

SOURCE=.\Task.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskManager.cpp
# End Source File
# Begin Source File

SOURCE=.\tga.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools.cpp
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Account.h
# End Source File
# Begin Source File

SOURCE=.\Accounts.h
# End Source File
# Begin Source File

SOURCE=.\AppDescriptor.h
# End Source File
# Begin Source File

SOURCE=.\AppIcons.h
# End Source File
# Begin Source File

SOURCE=.\AppPropertiesPopup.h
# End Source File
# Begin Source File

SOURCE=.\Bandwidth.h
# End Source File
# Begin Source File

SOURCE=.\cdr.h
# End Source File
# Begin Source File

SOURCE=.\CFDescriptor.h
# End Source File
# Begin Source File

SOURCE=.\CFManager.h
# End Source File
# Begin Source File

SOURCE=.\CFTBTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CFToolbox.h
# End Source File
# Begin Source File

SOURCE=.\CFToolbox_dll.h
# End Source File
# Begin Source File

SOURCE=.\CFToolboxDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChooseAccount.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\ConfigManager.h
# End Source File
# Begin Source File

SOURCE=.\crypto.h
# End Source File
# Begin Source File

SOURCE=.\CSClient.h
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\dictionary.h
# End Source File
# Begin Source File

SOURCE=.\Download.h
# End Source File
# Begin Source File

SOURCE=.\DownloadManager.h
# End Source File
# Begin Source File

SOURCE=.\EnterPasswordDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExecutionAnalyser.h
# End Source File
# Begin Source File

SOURCE=.\gcf.h
# End Source File
# Begin Source File

SOURCE=.\HashMap.h
# End Source File
# Begin Source File

SOURCE=.\HashMapStr.h
# End Source File
# Begin Source File

SOURCE=.\HistogramCtrl.h
# End Source File
# Begin Source File

SOURCE=.\iniparser.h
# End Source File
# Begin Source File

SOURCE=.\LargeFile.h
# End Source File
# Begin Source File

SOURCE=.\launcher.h
# End Source File
# Begin Source File

SOURCE=.\Plugins.h
# End Source File
# Begin Source File

SOURCE=.\ProgressPopup.h
# End Source File
# Begin Source File

SOURCE=.\PropertiesPopup.h
# End Source File
# Begin Source File

SOURCE=.\QuestionPopup.h
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\RegistryKey.h
# End Source File
# Begin Source File

SOURCE=.\RegistryNode.h
# End Source File
# Begin Source File

SOURCE=.\RegistryValue.h
# End Source File
# Begin Source File

SOURCE=.\RegistryVector.h
# End Source File
# Begin Source File

SOURCE=.\RegistryZip.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Rijndael.h
# End Source File
# Begin Source File

SOURCE=.\ServerRegistry.h
# End Source File
# Begin Source File

SOURCE=.\SHA1.h
# End Source File
# Begin Source File

SOURCE=.\socketTools.h
# End Source File
# Begin Source File

SOURCE=.\SplitterBar.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\steamClient.h
# End Source File
# Begin Source File

SOURCE=.\SteamNetwork.h
# End Source File
# Begin Source File

SOURCE=.\strlib.h
# End Source File
# Begin Source File

SOURCE=.\TabAdd.h
# End Source File
# Begin Source File

SOURCE=.\TabApps.h
# End Source File
# Begin Source File

SOURCE=.\TabBandwidth.h
# End Source File
# Begin Source File

SOURCE=.\TabFiles.h
# End Source File
# Begin Source File

SOURCE=.\TabHelp.h
# End Source File
# Begin Source File

SOURCE=.\TabLogs.h
# End Source File
# Begin Source File

SOURCE=.\TabPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Task.h
# End Source File
# Begin Source File

SOURCE=.\TaskManager.h
# End Source File
# Begin Source File

SOURCE=.\tga.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\Tools.h
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CFToolbox.ico
# End Source File
# Begin Source File

SOURCE=.\res\CFToolbox.rc2
# End Source File
# Begin Source File

SOURCE=.\res\empty.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon4.ico
# End Source File
# Begin Source File

SOURCE=.\res\SteamAppRebinder.ico
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zconf.in.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section CFToolbox : {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
# 	2:5:Class:CWebBrowser2
# 	2:10:HeaderFile:webbrowser2.h
# 	2:8:ImplFile:webbrowser2.cpp
# End Section
# Section CFToolbox : {8856F961-340A-11D0-A96B-00C04FD705A2}
# 	2:21:DefaultSinkHeaderFile:webbrowser2.h
# 	2:16:DefaultSinkClass:CWebBrowser2
# End Section
