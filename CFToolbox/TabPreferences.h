#if !defined(AFX_TABPREFERENCES_H__3956373E_D22F_48EF_857B_1BAF48CF4F71__INCLUDED_)
#define AFX_TABPREFERENCES_H__3956373E_D22F_48EF_857B_1BAF48CF4F71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabPreferences.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabPreferences dialog

class CTabPreferences : public CDialog
{
// Construction
public:

	CDialog * parent;

	void fillAccForm();
	void setAccForm(bool enabled);
	void init();
	CTabPreferences(CWnd* pParent = NULL);   // standard constructor
	BOOL PreTranslateMessage(MSG* pMsg) ;	
 
// Dialog Data
	//{{AFX_DATA(CTabPreferences)
	enum { IDD = IDD_TAB_PREFERENCES };
	CEdit	m_steamguardCtrl;
	CButton	m_doDisconnectCtrl;
	CComboBox	m_filesDoubleClick;
	CComboBox	m_appsDoubleClick;
	CButton	m_noResetCtrl;
	CComboBox	m_language;
	CButton	m_deleteButton;
	CComboBox	m_accountsCtrl;
	CButton	m_doLoginCtrl;
	CButton	m_rememberCtrl;
	CEdit	m_passwordCtrl;
	CEdit	m_loginCtrl;
	CString	m_host;
	int		m_port;
	BOOL	m_remember;
	CString	m_login;
	CString	m_password;
	int		m_accounts;
	CString	m_gcfPath;
	BOOL	m_automaticUpdate;
	int		m_logsLevelVal;
	CComboBox	m_logsLevel;
	int		m_maxTasks;
	CString	m_outputPath;
	BOOL	m_validateBeforeArchiveUpdate;
	CString	m_commandLine;
	CString	m_fileCommandLine;
	BOOL	m_waitApp;
	BOOL	m_waitFile;
	BOOL	m_sharing;
	BOOL	m_secured;
	BOOL	m_noReset;
	BOOL	m_bandwidth;
	BOOL	m_resetMiniGCF;
	BOOL	m_validateDownloads;
	BOOL	m_accurateNcfCompletion;
	BOOL	m_disableInstall;
	CString	m_steamguard;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabPreferences)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabPreferences)
	afx_msg void OnSetConfigServer();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual BOOL OnInitDialog();
	afx_msg void OnNewAccount();
	afx_msg void OnDeleteAccount();
	afx_msg void OnDoLogin();
	afx_msg void OnSelchangeAccounts();
	afx_msg void OnGcfBrowse();
	afx_msg void OnUpdateCDR();
	afx_msg void OnAutomaticCDRUpdate();
	afx_msg void OnSelChangeLevel();
	afx_msg void OnSetMaxTask();
	afx_msg void OnPatchBrowse();
	afx_msg void OnValidatePatchs();
	afx_msg void OnSelchangeLanguage();
	afx_msg void OnSetCommandLine();
	afx_msg void OnSetFileCommandLine();
	afx_msg void OnWaitFile();
	afx_msg void OnWaitApp();
	afx_msg void OnSharing();
	afx_msg void OnStop();
	afx_msg void OnKill();
	afx_msg void OnSecured();
	afx_msg void OnRealBandwidthUsage();
	afx_msg void OnResetMiniGCF();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeFileDblclick();
	afx_msg void OnSelchangeAppDblclick();
	afx_msg void OnValidateDownloads();
	afx_msg void OnAccurate();
	afx_msg void OnCheckDisableInstall();
	afx_msg void OnDoDisconnect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
CTabPreferences * getTabPreferences();
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABPREFERENCES_H__3956373E_D22F_48EF_857B_1BAF48CF4F71__INCLUDED_)
