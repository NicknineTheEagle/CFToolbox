// Account.cpp: implementation of the CAccount class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "Account.h"
#include "ServerRegistry.h"
#include "steamClient.h"
#include "Debug.h"
#include "SteamNetwork.h"
#include "EnterPasswordDlg.h"
#include "ProgressPopup.h"
#include "socketTools.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "CFToolbox_dll.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAccount::CAccount()
{
	oldId=0;
	id=0;
	login=0;
	password=0;
	steamguard=0;
	savePassword=false;
/*
	noResetTicket=false;
	ticket=0;
	ticketSize=0;
*/
	node=0; 
}

CAccount::CAccount(char *  _id)
{
	setId(id);
	load();
}

CAccount::~CAccount()
{
	if (oldId) free(oldId);
	if (id) free(id);
	if (login) free(login);
	if (password) free(password);
	if (steamguard) free(steamguard);
//	if (ticket) free(ticket);
}

char *  CAccount::getId()
{
	return id;
}

void CAccount::setId(char *  newId)
{
	if (id) 
	{
		if (oldId) free(oldId);
		oldId=id;
	}
	id=(char*)malloc(strlen(newId)+1);
	strcpy(id,newId);
}

char *  CAccount::getLogin()
{
	return login;
}

void CAccount::setLogin(char *  newLogin)
{
	if (login) free(login);
	login=0;
	if (newLogin && strlen(newLogin))
	{
		login=(char*)malloc(strlen(newLogin)+1);
		strcpy(login,newLogin);
	}
}

char * CAccount::getSteamguard()
{
	return steamguard;
}


char * CAccount::getPassword()
{
	return password;
}

void CAccount::setSteamguard(char *  newSteamguard)
{
	if (steamguard) free(steamguard);
	steamguard=0;
	if (newSteamguard && strlen(newSteamguard))
	{
		steamguard=(char*)malloc(strlen(newSteamguard)+1);
		strcpy(steamguard,newSteamguard);
	}
} 

void CAccount::setPassword(char *  newPassword)
{
	if (password) free(password);
	password=0;
	if (newPassword && strlen(newPassword))
	{
		password=(char*)malloc(strlen(newPassword)+1);
		strcpy(password,newPassword);
	}
} 


void CAccount::setSavePassword(bool isSavePassword)
{
	savePassword=isSavePassword;
} 

bool CAccount::isSavePassword()
{
	return savePassword;
} 
/*
void CAccount::setNoResetTicket(bool _isNoResetTicket)
{
	noResetTicket=_isNoResetTicket;
} 

bool CAccount::isNoResetTicket()
{
	return noResetTicket;
} 

void CAccount::setTicket(char * newTicket, int size)
{
	if (ticket) free(ticket);
	ticket=0;
	if (size)
	{
		ticket=(char*)malloc(size);
		memcpy(ticket,newTicket,size);
	}
	ticketSize=size;
} 

int CAccount::getTicket(char * _ticket)
{
	if (ticket && _ticket)
	{
		memcpy(_ticket,ticket,ticketSize);
	}
	return ticketSize;
} 
*/ 
 
bool CAccount::commit()
{
	RegistryKey * reg=getServerRegistry();
	char path[1000];
	if (oldId && strcmp(oldId,id))
	{
		sprintf(path,"/TopKey/Accounts/%s",oldId);
		delete reg->setKey(path)->remove();
	}
	
	if (!id) return false;
	
	sprintf(path,"/TopKey/Accounts/%s",id);
	RegistryKey * acc=reg->setKey(path);
	
	RegistryValue * val;
	
	
	acc->removeValue("login");
	if (login && strlen(login))
	{
		val=new RegistryValue();
		val->setName("login");
		val->setString(login);
		acc->addValue(val);
	}
	acc->removeValue("password");
	acc->removeValue("steamguard");
	if (password && savePassword && strlen(password))
	{
		val=new RegistryValue();
		val->setName("password");
		val->setString(password);
		acc->addValue(val);
	}
	if (steamguard && savePassword && strlen(steamguard))
	{
		val=new RegistryValue();
		val->setName("steamguard");
		val->setString(steamguard);
		acc->addValue(val);
	}
	acc->removeValue("savePassword");
	{
		val=new RegistryValue();
		val->setName("savePassword");
		val->setDWORD(savePassword && password);
		acc->addValue(val);
	}
/*	acc->removeValue("noResetTicket");
	{
		val=new RegistryValue();
		val->setName("noResetTicket");
		val->setDWORD(noResetTicket);
		acc->addValue(val);
	}
	acc->removeValue("ticket");
	if (ticket && ticketSize)
	{
		val=new RegistryValue();
		val->setName("ticket");
		val->setData(ticket,ticketSize);
		acc->addValue(val);
	}
*/ 
	saveServerRegistry();
	return true;
}


bool CAccount::load()
{
	RegistryKey * reg=getServerRegistry();
	char path[1000];
	
	sprintf(path,"/TopKey/Accounts/%s",id);
	RegistryKey * acc=reg->getKey(path);
	
	if (!acc) return false;
	
	node=acc;

	RegistryValue * val;
	
	login=0;
	val=acc->getValue("login");
	if (val)
	{
		setLogin(val->getString());
	}
	
	password=0;
	val=acc->getValue("password");
	if (val)
	{
		setPassword(val->getString());
	}

	steamguard=0;
	val=acc->getValue("steamguard");
	if (val)
	{
		setSteamguard(val->getString());
	}
	
	savePassword=false;
	val=acc->getValue("savePassword");
	if (val)
	{
		setSavePassword((bool)val->getDWORD());
	}

/*	noResetTicket=false;
	val=acc->getValue("noResetTicket");
	if (val)
	{
		setNoResetTicket((bool)val->getDWORD());
	}
	
	ticket=0;
	ticketSize=0;
	val=acc->getValue("ticket");
	if (val)
	{
		setTicket(val->getData(),val->getDataSize());
	}
	*/
 
	return true;
}

bool CAccount::remove()
{
	RegistryKey * reg=getServerRegistry();
	char path[1000];
	if (id)
	{
		sprintf(path,"/TopKey/Accounts/%s",id);
		delete reg->setKey(path)->remove();
	}
	saveServerRegistry();
	return true;
}

/*
// CFT 1.1.14 : old auth method using Master server
int CAccount::authenticate()
{
	if (login && !password && !savePassword)
	{
		password=askForPassword();
	}
	
	if (!login || !password) return 0;

	SteamNetwork network;	
 
	char log2[1000];
	strcpy(log2,login);
	strlwr(log2);
	int tries=1;
	do
	{
		CProgressPopup progress;
		progress.m_message.Format("Authenticating %s ...",id);
		progress.Create(IDD_PROGRESS);
		progress.m_progress.SetRange32(0,10);
		progress.m_progress.SetPos(0);
		sockaddr_in authServers[20];

		int nbAuth=0;

		nbAuth=getAuthServers(  getService( network.getConfigServer(), network.getConfigServerPort()),authServers, 2,log2);
		progress.m_progress.SetPos(1);	
		if (!nbAuth) 
		{
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"No Authentication server available",ACCOUNTS_LOGLEVEL);
			if (tries==1) 
			{
				if (MessageBox(NULL,"No Authentication server available. Automatically retry ?","Authentication error",MB_RETRYCANCEL|MB_ICONERROR)==IDRETRY)
					tries=10;
			}
			progress.DestroyWindow(); 
			if (tries==1) return 0;
			else continue;
			
		}
		if (ticket) free(ticket);
		ticket=(char*)malloc(2000);
		printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Authenticating",login,ACCOUNTS_LOGLEVEL);
		ticketSize=authenticateAndGetTicket(authServers[0],log2, password,ticket,	&(progress.m_progress));
			progress.m_progress.SetPos(9);
		switch (ticketSize)
		{
		case -5: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Server closed connection",login,ACCOUNTS_LOGLEVEL);
			if (tries==1) 
			{
				if (MessageBox(NULL,"Server closed connection. Automatically retry ?","Authentication error",MB_RETRYCANCEL|MB_ICONERROR)==IDRETRY)
					tries=10;
			}
			break;
		case -4: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Account has been disabled",login,ACCOUNTS_LOGLEVEL);
			MessageBox(NULL,"Account has been disabled","Authentication error",MB_OK|MB_ICONERROR);break;
		case -3: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"System date / time / time zone is not properly configured (maximum clockskew exceeded)",login,ACCOUNTS_LOGLEVEL);
			MessageBox(NULL,"System date / time / time zone is not properly configured. Time adjusted, retry to login","Authentication error",MB_OK|MB_ICONERROR);break;
		case -2: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Invalid password",login,ACCOUNTS_LOGLEVEL);
			MessageBox(NULL,"Invalid password","Authentication error",MB_OK|MB_ICONERROR);break;
		case -1: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Invalid login",login,ACCOUNTS_LOGLEVEL);
			MessageBox(NULL,"Invalid login","Authentication error",MB_OK|MB_ICONERROR);break;
		case 0: 
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"No ticket retrieved",login,ACCOUNTS_LOGLEVEL);
			if (tries==1) 
			{
				if (MessageBox(NULL,"No ticket retrieved. Automatically retry ?","Authentication error",MB_RETRYCANCEL|MB_ICONERROR)==IDRETRY)
					tries=20;
			}
 			break;
		default: 
			if (ticketSize<0)
			{
				printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Unknown authentication error code",-ticketSize,ACCOUNTS_LOGLEVEL);
				MessageBox(NULL,"Unknown authentication error","Authentication error",MB_OK|MB_ICONERROR);break;
			}
			else
			{
				progress.m_progress.SetPos(10);
				tries=1;
				printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Authentication succeded",login,ACCOUNTS_LOGLEVEL);break;

			}
		}
		tries--;
		if (tries) progress.m_progress.SetPos(0);
		progress.DestroyWindow(); 
	} while (tries>0);
	if (ticketSize<=0)
	{
		ticketSize=0;
		if (ticket) free(ticket);
		ticket=0; 
		return 0;
	}


	return 1;
}
*/

// CFT 1.1.14 : new auth method using CM client
int CAccount::authenticate()
{
	if (login && !password && !savePassword)
	{
		Credentials cred=askForPassword();
		password=cred.password;
		steamguard=cred.steamguard;
	}
	
	if (!login || !password) return 0;

	SteamNetwork network;	
 
	char log2[1000];
	strcpy(log2,login);
	strlwr(log2);
	int tries=1;
	bool ok=false;
	do
	{
		CProgressPopup progress;
		if (steamguard)
			progress.m_message.Format("Authenticating %s (steam guard)...",id);
		else
			progress.m_message.Format("Authenticating %s ...",id);
		progress.Create(IDD_PROGRESS);
		progress.m_progress.SetRange32(0,10);
		progress.m_progress.SetPos(0);
		sockaddr_in authServers[20];
	
		
		int nbAuth=0;

		progress.m_progress.SetPos(1);	
	
/*
		if (ticket) free(ticket);
		ticket=(char*)malloc(2000);
*/
		if (steamguard)
			printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Authenticating (with steam guard)",login,ACCOUNTS_LOGLEVEL);
		else
			printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Authenticating",login,ACCOUNTS_LOGLEVEL);
		//ticketSize=authenticateAndGetTicket(authServers[0],log2, password,ticket,	&(progress.m_progress));
		
		CFT_CMClient client=::connect(log2, password,steamguard,true);
		if (client) printDebug(DEBUG_LEVEL_DEBUG,ACCOUNTS_NAME,"Client connected",log2,ACCOUNTS_LOGLEVEL);
		progress.m_progress.SetPos(6);
		ok=client!=0;
//		if (client) disconnect(client);

		progress.m_progress.SetPos(9);

		if (!ok)
		{
			printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Authentication error",ACCOUNTS_LOGLEVEL);
			MessageBox(NULL,"Authentication error","Authentication error",MB_OK|MB_ICONERROR);break;
		}
		else
		{
			progress.m_progress.SetPos(10);
			tries=1;
			printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Authentication succeded",login,ACCOUNTS_LOGLEVEL);break;

		}

		tries--;
		if (tries) progress.m_progress.SetPos(0);
		progress.DestroyWindow(); 
	} while (tries>0);
/*	if (ticketSize<=0)
	{
		ticketSize=0;
		if (ticket) free(ticket);
		ticket=0; 
		return 0;
	}

*/
	return ok ? 1 : 0;
}

Credentials CAccount::askForPassword()
{
	Credentials cred;
	cred.password=0;
	cred.steamguard=0;

	CEnterPasswordDlg enterPassword;
	enterPassword.m_account=login;
	enterPassword.m_password="";
	enterPassword.m_steamguard="";
	int nResponse = enterPassword.DoModal();
	if (nResponse == IDOK)
	{
		cred.password=(char*)malloc(strlen(enterPassword.m_password)+1);
		strcpy(cred.password,enterPassword.m_password);
		cred.steamguard=(char*)malloc(strlen(enterPassword.m_steamguard)+1);
		strcpy(cred.steamguard,enterPassword.m_steamguard);
		setSavePassword(enterPassword.m_savePassword);
		return cred;
	}
	else if (nResponse == IDCANCEL)
	{
		return cred;
	}
	return cred;
}

void CAccount::disconnect()
{
	CFT_CMClient client=::getClient(login);
	if (login) ::disconnect(client);
	printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Client disconnected",login,ACCOUNTS_LOGLEVEL);
}

int CAccount::getContentServerAuthTickets(unsigned int appId, char  * ticket)
{
	CFT_CMClient client=getClient(login);
	if (!client) authenticate();
	client=getClient(login);
	if (!client) return 0;
	int length=::getContentServerAuthTickets(client,appId,(unsigned char*)ticket);
	if (length) printDebug(DEBUG_LEVEL_DEBUG,ACCOUNTS_NAME,"Tickets retrieved for",appId,ACCOUNTS_LOGLEVEL);
	else printDebug(DEBUG_LEVEL_ERROR,ACCOUNTS_NAME,"Error retrieving tickets for",appId,ACCOUNTS_LOGLEVEL);
	return length;
}
