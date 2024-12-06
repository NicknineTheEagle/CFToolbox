// Accounts.cpp: implementation of the CAccounts class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "Accounts.h"
#include "ServerRegistry.h"
#include "Debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAccounts::CAccounts()
{

}

CAccounts::~CAccounts()
{

}

int CAccounts::getNbAccounts()
{
	RegistryKey * acc=getServerRegistry()->setKey("/TopKey/Accounts")->getFirstChild();
	int nb=0;
	while (acc) {
		nb++;
		acc=acc->getNextSibiling();
	}
	return nb;
}

char * CAccounts::getAccountIdAt(int index)
{
	RegistryKey * acc=getServerRegistry()->setKey("/TopKey/Accounts")->getFirstChild();
	int nb=0;
	while (acc) {
		if (index==nb) return acc->getName();
		nb++;
		acc=acc->getNextSibiling();
	}
	return 0;
}

CAccount * CAccounts::getAccount(char *id)
{
	CAccount * acc=new CAccount;
	acc->setId(id);
	if (!acc->load())
	{
		delete acc;
		return 0;
	}
	return acc;
}

/*
void CAccounts::reconnectAllAccounts(bool onlyExpired)
{
	int nbAccs=getNbAccounts();
	printDebug(DEBUG_LEVEL_INFO,ACCOUNTS_NAME,"Reconnecting accounts...",ACCOUNTS_LOGLEVEL);
	for (int ind=0;ind<nbAccs;ind++)
	{
		CAccount * acc=getAccount(getAccountIdAt(ind));
		//if (!acc->isDisabled())
		{
			if (!onlyExpired || !acc->getTicket(0))
			{
				acc->authenticate();
				acc->commit();
			}
		}
		delete acc;
	}
}
*/
/*
void CAccounts::resetAllTickets()
{
	int nbAccs=getNbAccounts();
	printDebug(DEBUG_LEVEL_DEBUG,ACCOUNTS_NAME,"Reseting tickets...",ACCOUNTS_LOGLEVEL);
	for (int ind=0;ind<nbAccs;ind++)
	{
		CAccount * acc=getAccount(getAccountIdAt(ind));
		if (!acc->isNoResetTicket())
		{
			acc->setTicket(0,0);
			acc->commit();
		}
		delete acc;
	}
}
*/
char * CAccounts::getAccountIdForFileId(DWORD fileId)
{
	char id[20];
	sprintf(id,"%d",fileId);
	int nbAccs=getNbAccounts();
	for (int ind=0;ind<nbAccs;ind++)
	{
		char * accId=getAccountIdAt(ind);
		CAccount * acc=getAccount(accId);
		RegistryKey * apps=acc->node->setKey("_Apps");
		if (apps->getKey(id)) return acc->node->getName();
	}
	return 0;
}

void CAccounts::setAccountIdForFileId(DWORD fileId,char * accountId)
{
	char id[20];
	sprintf(id,"%d",fileId);
	int nbAccs=getNbAccounts();
	for (int ind=0;ind<nbAccs;ind++)
	{
		char * accId=getAccountIdAt(ind);
		CAccount * acc=getAccount(accId);
		RegistryKey * apps=acc->node->setKey("_Apps");
		RegistryKey * app=apps->getKey(id);
		if (app) delete app->remove();
	}
	CAccount * acc=getAccount(accountId);
	if (acc)
	{
		RegistryKey * apps=acc->node->setKey("_Apps");
		RegistryKey * app=apps->setKey(id);
	}
	saveServerRegistry();
}

