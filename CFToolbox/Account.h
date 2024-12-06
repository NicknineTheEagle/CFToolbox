// Account.h: interface for the CAccount class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCOUNT_H__3B4C3177_F5BE_47AE_8537_80F007BB06E9__INCLUDED_)
#define AFX_ACCOUNT_H__3B4C3177_F5BE_47AE_8537_80F007BB06E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RegistryKey.h"

typedef struct
{
	char * password; 
	char * steamguard;
} Credentials;


class CAccount  
{
public:
	void setId(char *  newId);
	char *  getId();
	void setLogin(char *  newLogin);
	char *  getLogin();
	void setPassword(char *  newPassword);
	char *  getPassword();
	void setSteamguard(char *  newSteamguard);
	char *  getSteamguard();
	void setSavePassword(bool isSavePassword);
	bool isSavePassword();

	int getContentServerAuthTickets(unsigned int appId, char  * ticket);

	void disconnect();
/*
	void setNoResetTicket(bool isNoReset);
	bool isNoResetTicket();

	void setTicket(char * ticket, int size);
	int getTicket(char * ticket);	
*/	
	bool commit();
	bool load();
	bool remove();
	
	int authenticate();
	
	CAccount();
	CAccount(char * id);
	virtual ~CAccount();
	RegistryKey * node;
protected:
	char * oldId;
	char * id;
	char *  login;
	char *  password;
	char * steamguard;
	bool savePassword;
/*
	bool noResetTicket;

	char * ticket;
	int ticketSize;
*/
private:
	Credentials askForPassword();
};

#endif // !defined(AFX_ACCOUNT_H__3B4C3177_F5BE_47AE_8537_80F007BB06E9__INCLUDED_)
