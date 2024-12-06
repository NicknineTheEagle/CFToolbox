; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CEnterPasswordDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "CFToolbox.h"

ClassCount=15
Class1=CCFToolboxApp
Class2=CCFToolboxDlg
Class3=CAboutDlg

ResourceCount=24
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_CFTOOLBOX_DIALOG
Resource4=IDD_TAB_FILES (English (U.S.))
Resource5=IDD_TAB_LOGS (English (U.S.))
Class4=CEnterPasswordDlg
Resource6=IDR_FILES_MENU (English (U.S.))
Class5=CTabPreferences
Resource7=IDD_CFTOOLBOX_DIALOG (English (U.S.))
Class6=CQuestionPopup
Resource8=IDD_PROPERTIES (English (U.S.))
Class7=CTabAdd
Resource9=IDD_QUESTION (English (U.S.))
Resource10=IDD_ABOUTBOX (English (U.S.))
Class8=CProgressPopup
Resource11=IDD_TAB_BANDWIDTH (English (U.S.))
Class9=CTabFiles
Resource12=IDD_TAB_HELP (English (U.S.))
Resource13=IDR_POPUP_MENU (English (U.S.))
Resource14=IDD_TAB_ADD (English (U.S.))
Class10=CPropertiesPopup
Resource15=IDD_TAB_PREFERENCES (English (U.S.))
Class11=CTabApps
Resource16=IDD_TAB_APPS (English (U.S.))
Class12=CAppPropertiesPopup
Resource17=IDD_ENTER_PASSWORD (English (U.S.))
Resource18=IDD_APP_PROPERTIES (English (U.S.))
Class13=CChooseAccount
Resource19=IDD_PROGRESS (English (U.S.))
Class14=CTabBandwidth
Resource20=IDR_APPS_MENU (English (U.S.))
Class15=CTabHelp
Resource21=IDR_ADDFILES_MENU (English (U.S.))
Resource22=IDR_TASKS_MENU (English (U.S.))
Resource23=IDD_CHOOSE_ACCOUNT (English (U.S.))
Resource24=IDR_ADDAPPS_MENU (English (U.S.))

[CLS:CCFToolboxApp]
Type=0
HeaderFile=CFToolbox.h
ImplementationFile=CFToolbox.cpp
Filter=N

[CLS:CCFToolboxDlg]
Type=0
HeaderFile=CFToolboxDlg.h
ImplementationFile=CFToolboxDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_ACCOUNTS

[CLS:CAboutDlg]
Type=0
HeaderFile=CFToolboxDlg.h
ImplementationFile=CFToolboxDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg


[DLG:IDD_CFTOOLBOX_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CCFToolboxDlg

[DLG:IDD_CFTOOLBOX_DIALOG (English (U.S.))]
Type=1
Class=CCFToolboxDlg
ControlCount=3
Control1=IDC_TAB,SysTabControl32,1342193664
Control2=IDC_TASKS,SysListView32,1350631425
Control3=IDC_ACCOUNTS,edit,1350633600

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342406657

[CLS:CEnterPasswordDlg]
Type=0
HeaderFile=EnterPasswordDlg.h
ImplementationFile=EnterPasswordDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CEnterPasswordDlg

[CLS:CTabPreferences]
Type=0
HeaderFile=TabPreferences.h
ImplementationFile=TabPreferences.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_STEAMGUARD
VirtualFilter=dWC

[DLG:IDD_TAB_PREFERENCES (English (U.S.))]
Type=1
Class=CTabPreferences
ControlCount=60
Control1=IDC_HOST,edit,1350631552
Control2=IDC_PORT,edit,1350639744
Control3=IDC_SET,button,1342275584
Control4=IDC_UPDATECDR,button,1342275584
Control5=IDC_AUTOMATICUPDATE,button,1342275587
Control6=IDC_ACCOUNTS,combobox,1344340227
Control7=IDC_BUTTON6,button,1342275584
Control8=IDC_DELETE,button,1476493312
Control9=IDC_LOGIN,edit,1484849280
Control10=IDC_PASSWORD,edit,1484849312
Control11=IDC_REMEMBER,button,1476493315
Control12=IDC_NORESET,button,1208057859
Control13=IDC_DO_LOGIN,button,1476493312
Control14=IDC_GCF_PATH,edit,1350568064
Control15=IDC_GCF_BROWSE,button,1342275584
Control16=IDC_PATCH_PATH,edit,1350568064
Control17=IDC_PATCH_BROWSE,button,1342275584
Control18=IDC_FILECOMMAND,edit,1350631568
Control19=IDC_SET4,button,1342275584
Control20=IDC_WAIT_FILE,button,1342275587
Control21=IDC_COMMAND,edit,1350631568
Control22=IDC_SET3,button,1342275584
Control23=IDC_WAIT_APP,button,1342275587
Control24=IDC_LANGUAGE,combobox,1344340227
Control25=IDC_LEVEL,combobox,1344339971
Control26=IDC_MAX_TASKS,edit,1350639744
Control27=IDC_SET2,button,1342275584
Control28=IDC_STOP,button,1342275584
Control29=IDC_VALIDATE_PATCHS,button,1342275587
Control30=IDC_SECURED,button,1342275587
Control31=IDC_SHARING,button,1342275587
Control32=IDC_BANDWIDTH,button,1342275587
Control33=IDC_STATIC,button,1342177287
Control34=IDC_STATIC,static,1342308866
Control35=IDC_STATIC,button,1342177287
Control36=IDC_STATIC,static,1342308866
Control37=IDC_STATIC,static,1342308866
Control38=IDC_STATIC,button,1342177287
Control39=IDC_STATIC,button,1342177287
Control40=IDC_STATIC,button,1342177287
Control41=IDC_STATIC,static,1342308866
Control42=IDC_STATIC,static,1342308866
Control43=IDC_STATIC,button,1342177287
Control44=IDC_STATIC,button,1342177287
Control45=IDC_STATIC,button,1342177287
Control46=IDC_STATIC,static,1342308866
Control47=IDC_STATIC,static,1342308866
Control48=IDC_KILL,button,1073840128
Control49=IDC_MINI,button,1342275587
Control50=IDC_STATIC,button,1342177287
Control51=IDC_STATIC,static,1342308866
Control52=IDC_STATIC,static,1342308866
Control53=IDC_FILE_DBLCLICK,combobox,1344340227
Control54=IDC_APP_DBLCLICK,combobox,1344340227
Control55=IDC_VALIDATE_DOWNLOADS,button,1342275587
Control56=IDC_SECURED2,button,1342275587
Control57=IDC_CHECK1,button,1342275587
Control58=IDC_DO_DISCONNECT,button,1476493312
Control59=IDC_STEAMGUARD,edit,1484849312
Control60=IDC_STATIC,static,1342308866

[DLG:IDD_ENTER_PASSWORD (English (U.S.))]
Type=1
Class=CEnterPasswordDlg
ControlCount=9
Control1=IDC_PASSWORD,edit,1350631584
Control2=IDOK,button,1342275585
Control3=IDCANCEL,button,1342275584
Control4=IDC_STATIC,static,1342308866
Control5=IDC_STATIC,static,1342308866
Control6=IDC_ACCOUNT,edit,1350568064
Control7=IDC_SAVE_PASSWORD,button,1342275587
Control8=IDC_STEAMGUARD2,edit,1350631584
Control9=IDC_STATIC,static,1342308866

[CLS:CQuestionPopup]
Type=0
HeaderFile=QuestionPopup.h
ImplementationFile=QuestionPopup.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CQuestionPopup

[CLS:CTabAdd]
Type=0
HeaderFile=TabAdd.h
ImplementationFile=TabAdd.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_APPS
VirtualFilter=dWC

[DLG:IDD_TAB_ADD (English (U.S.))]
Type=1
Class=CTabAdd
ControlCount=7
Control1=IDC_APPS,SysListView32,1350631513
Control2=IDC_FILES,SysListView32,1350664217
Control3=IDC_ACCOUNT_LIST,combobox,1344340227
Control4=IDC_DOWNLOAD,button,1342275584
Control5=IDC_GMTFRM,button,1342177287
Control6=IDC_FILESFRM,button,1342177287
Control7=IDC_ACCLBL,static,1342308866

[DLG:IDD_TAB_LOGS (English (U.S.))]
Type=1
Class=?
ControlCount=2
Control1=IDC_LOGS,edit,1353713668
Control2=IDC_CLEAR,button,1342275584

[DLG:IDD_QUESTION (English (U.S.))]
Type=1
Class=CQuestionPopup
ControlCount=4
Control1=IDC_VALUE,edit,1350631552
Control2=IDOK,button,1342275585
Control3=IDCANCEL,button,1342275584
Control4=IDC_Question,static,1342308864

[CLS:CProgressPopup]
Type=0
HeaderFile=ProgressPopup.h
ImplementationFile=ProgressPopup.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CProgressPopup

[DLG:IDD_PROGRESS (English (U.S.))]
Type=1
Class=CProgressPopup
ControlCount=2
Control1=IDC_MESSAGE,static,1342308352
Control2=IDC_PROGRESS,msctls_progress32,1342177280

[DLG:IDD_TAB_FILES (English (U.S.))]
Type=1
Class=CTabFiles
ControlCount=1
Control1=IDC_FILES,SysListView32,1350631697

[CLS:CTabFiles]
Type=0
HeaderFile=TabFiles.h
ImplementationFile=TabFiles.cpp
BaseClass=CDialog
Filter=D
LastObject=CTabFiles
VirtualFilter=dWC

[DLG:IDR_POPUP_MENU (English (U.S.))]
Type=1
Class=?
ControlCount=0

[DLG:IDD_PROPERTIES (English (U.S.))]
Type=1
Class=CPropertiesPopup
ControlCount=25
Control1=IDC_PATH,edit,1350633600
Control2=IDC_DESCRIPTION,edit,1342244992
Control3=IDC_COMMONPATH,edit,1342244992
Control4=IDC_TYPE,edit,1342244992
Control5=IDC_CSIZE,edit,1342244992
Control6=IDC_ID,edit,1342244992
Control7=IDC_VERSION,edit,1342244992
Control8=IDC_COMPLETION,edit,1342244992
Control9=IDC_FILES,edit,1342244992
Control10=IDC_BLOCKS,edit,1342244992
Control11=IDC_FOLDERD,edit,1342244992
Control12=IDC_KEY,edit,1342244992
Control13=IDC_STATIC,static,1342308354
Control14=IDC_STATIC,static,1342308354
Control15=IDC_STATIC,static,1342308354
Control16=IDC_STATIC,static,1342308354
Control17=IDC_STATIC,static,1342308354
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308354
Control20=IDC_STATIC,static,1342308354
Control21=IDC_STATIC,static,1342308354
Control22=IDC_STATIC,static,1342308354
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308354
Control25=IDC_STATIC,static,1342308354

[CLS:CPropertiesPopup]
Type=0
HeaderFile=PropertiesPopup.h
ImplementationFile=PropertiesPopup.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_DESCRIPTION

[DLG:IDD_TAB_APPS (English (U.S.))]
Type=1
Class=CTabApps
ControlCount=1
Control1=IDC_APPS,SysListView32,1350631761

[CLS:CTabApps]
Type=0
HeaderFile=TabApps.h
ImplementationFile=TabApps.cpp
BaseClass=CDialog
Filter=D
LastObject=CTabApps
VirtualFilter=dWC

[CLS:CAppPropertiesPopup]
Type=0
HeaderFile=AppPropertiesPopup.h
ImplementationFile=AppPropertiesPopup.cpp
BaseClass=CDialog
Filter=D
LastObject=CAppPropertiesPopup
VirtualFilter=dWC

[DLG:IDD_CHOOSE_ACCOUNT (English (U.S.))]
Type=1
Class=CChooseAccount
ControlCount=7
Control1=IDC_ACCOUNT_LIST,combobox,1344340227
Control2=IDC_REMEMBER,button,1342275587
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_FILE,edit,1350633600
Control6=IDC_STATIC,static,1342308866
Control7=IDC_STATIC,static,1342308866

[DLG:IDD_APP_PROPERTIES (English (U.S.))]
Type=1
Class=CAppPropertiesPopup
ControlCount=17
Control1=IDC_NAME,edit,1350633600
Control2=IDC_DEVELOPPER,edit,1342244992
Control3=IDC_MANUAL,edit,1342244992
Control4=IDC_ID,edit,1342244992
Control5=IDC_CSIZE,edit,1342244992
Control6=IDC_GCFS,SysListView32,1350664469
Control7=IDC_PROPERTIES,SysListView32,1350664469
Control8=IDC_STATIC,static,1342308354
Control9=IDC_MANUALS,static,1342308354
Control10=IDC_STATIC,static,1342308354
Control11=IDC_STATIC,static,1342308354
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_EXPLORER1,{8856F961-340A-11D0-A96B-00C04FD705A2},1342242816
Control15=IDC_STATIC,static,1342308352
Control16=IDC_COMMON,edit,1342244992
Control17=IDC_STATIC,static,1342308354

[CLS:CChooseAccount]
Type=0
HeaderFile=ChooseAccount.h
ImplementationFile=ChooseAccount.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

[DLG:IDD_TAB_BANDWIDTH (English (U.S.))]
Type=1
Class=CTabBandwidth
ControlCount=14
Control1=IDC_COMBO1,combobox,1344340227
Control2=IDC_MAX,edit,1350633600
Control3=IDC_CURRENT,edit,1350633600
Control4=IDC_MINIMUM,edit,1350633600
Control5=IDC_BANDWIDTH_GRAPH,static,1342177284
Control6=IDC_STATIC,button,1342177287
Control7=IDC_STATIC,static,1342308866
Control8=IDC_STATIC,static,1342308866
Control9=IDC_STATIC,static,1342308866
Control10=IDC_EXPLORER2,{8856F961-340A-11D0-A96B-00C04FD705A2},1342177280
Control11=IDC_HIDER,static,1342177280
Control12=IDC_FROM,button,1342177287
Control13=IDC_REMAINING,edit,1350633600
Control14=IDC_STATIC,static,1342308866

[CLS:CTabBandwidth]
Type=0
HeaderFile=TabBandwidth.h
ImplementationFile=TabBandwidth.cpp
BaseClass=CDialog
Filter=C
LastObject=CTabBandwidth
VirtualFilter=dWC

[DLG:IDD_TAB_HELP (English (U.S.))]
Type=1
Class=CTabHelp
ControlCount=2
Control1=IDC_EXPLORER1,{8856F961-340A-11D0-A96B-00C04FD705A2},1342242816
Control2=IDC_NOTFOUND,static,1342308352

[CLS:CTabHelp]
Type=0
HeaderFile=TabHelp.h
ImplementationFile=TabHelp.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CTabHelp

[MNU:IDR_TASKS_MENU (English (U.S.))]
Type=1
Class=?
Command1=ID_ROOT_CANCEL
CommandCount=1

[MNU:IDR_APPS_MENU (English (U.S.))]
Type=1
Class=?
Command1=ID_ROOT_RUN
Command2=ID_ROOT_COPY
Command3=ID_ROOT_ACCOUNT
Command4=ID_ROOT_DOWNLOAD
Command5=ID_ROOT_PAUSE_
Command6=ID_ROOT_PATCHS_MAKEARCHIVE
Command7=ID_ROOT_PATCHS_MAKEUPDATE
Command8=ID_ROOT_PATCHS_APPLYUPDATE
Command9=ID_ROOT_CANCEL
Command10=ID_ROOT_VALIDATE
Command11=ID_ROOT_CORRECT
Command12=ID_ROOT_MINI
Command13=ID_ROOT_PROPERTIES
CommandCount=13

[MNU:IDR_FILES_MENU (English (U.S.))]
Type=1
Class=?
Command1=ID_ROOT_OPEN
Command2=ID_ROOT_RUN
Command3=ID_ROOT_COPY
Command4=ID_ROOT_ACCOUNT
Command5=ID_ROOT_DOWNLOAD
Command6=ID_ROOT_PAUSE_
Command7=ID_ROOT_PATCHS_MAKEARCHIVE
Command8=ID_ROOT_PATCHS_MAKEUPDATE
Command9=ID_ROOT_PATCHS_APPLYUPDATE
Command10=ID_ROOT_CANCEL
Command11=ID_ROOT_VALIDATE
Command12=ID_ROOT_CORRECT
Command13=ID_ROOT_MINI
Command14=ID_ROOT_DELETE
Command15=ID_ROOT_PROPERTIES
CommandCount=15

[MNU:IDR_ADDFILES_MENU (English (U.S.))]
Type=1
Class=?
Command1=ID_ROOT_RUN
Command2=ID_ROOT_SELECT
Command3=ID_ROOT_PROPERTIES
CommandCount=3

[MNU:IDR_ADDAPPS_MENU (English (U.S.))]
Type=1
Class=?
Command1=ID_ROOT_RUN
Command2=ID_ROOT_PROPERTIES
CommandCount=2

