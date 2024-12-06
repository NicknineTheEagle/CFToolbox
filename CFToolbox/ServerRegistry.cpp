#include "ServerRegistry.h"
#include "Registry.h"
#include "stdio.h"
#include "socketTools.h"

RegistryKey * registry=0;

//int _s_lock__=0;
int isUpdating=0;
void setUpdating(int updating)
{
	isUpdating=updating;
}
/*
void sCCLock()
{
	while (1)
	{
		if (!_s_lock__)
		{
			_s_lock__++;
			return;
		}

		Sleep(100);
	}
}

void sCCUnlock()
{
	_s_lock__--;
}
*/

CRITICAL_SECTION srCs; 
void sCCLock()
{

	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&srCs);
}
 
void sCCUnlock()
{
	/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&srCs);
}
int initServerRegistry()
{
		/* Initialize the critical section -- This must be done before locking */
	InitializeCriticalSection(&srCs);
	sCCLock();
	Registry reg;
	registry=reg.load(SERVER_REGISTRY_FILE);
	sCCUnlock();
	if (!registry)
	{
		registry=reg.createNewRegistry();
	 
		return (reg.save(SERVER_REGISTRY_FILE,registry)>0);
	}
	return 1;
}

int saveServerRegistry()
{
	sCCLock();
	Registry reg;
	int res=(reg.save(SERVER_REGISTRY_FILE,registry)>0);
	sCCUnlock();
	return res;
	
}

RegistryKey * getServerRegistry(int updateMode)
{
	if (!registry) initServerRegistry();
	// block acces while updating
	if (!updateMode && isUpdating) while (1) Sleep(1000);
	return registry;
}

RegistryKey * getUserData(ULONGLONG accountId)
{
		RegistryKey * users=getServerRegistry()->getKey("/TopKey/_Users");
		RegistryKey * user=users->getFirstChild();
accountId&=0xFFFF;
		while (user!=0)
		{
		//	user->printXML();
//printf("$$checking %x  /  %x\n",accountId,(user->getValue("accountId")->getULONGLONG()&0xFFFF));
			if (accountId==(user->getValue("accountId")->getULONGLONG()&0xFFFF))
				return user;
			user=user->getNextSibiling();
		}
		return 0;
		
}

RegistryKey * getUserDataByChatId(DWORD id)
{
		RegistryKey * users=getServerRegistry()->getKey("/TopKey/_Users");
		RegistryKey * user=users->getFirstChild();
		RegistryKey * friends;
		while (user!=0)
		{
			if (friends=user->getChild("friends"))
			{
			if (id==friends->getValue("id")->getDWORD())
				return user;
			}
			user=user->getNextSibiling();
		}
		return 0;
		
}

