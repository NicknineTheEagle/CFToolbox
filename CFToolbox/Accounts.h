// Accounts.h: interface for the CAccounts class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCOUNTS_H__1E0AFFDB_31F1_46CE_9C0A_FA4ACA201431__INCLUDED_)
#define AFX_ACCOUNTS_H__1E0AFFDB_31F1_46CE_9C0A_FA4ACA201431__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Account.h"

class CAccounts  
{
public:
	char * getAccountIdForFileId(DWORD fileId);
	void setAccountIdForFileId(DWORD fileId,char * accountId);
//	void resetAllTickets();
	CAccount * getAccount(char * id);
	char * getAccountIdAt(int index);
	int getNbAccounts();
	CAccounts();
	virtual ~CAccounts();

};

#endif // !defined(AFX_ACCOUNTS_H__1E0AFFDB_31F1_46CE_9C0A_FA4ACA201431__INCLUDED_)
