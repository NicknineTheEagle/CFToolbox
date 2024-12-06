#ifndef _CFTOOLBOX_DLL_H_
#define _CFTOOLBOX_DLL_H_

#ifdef CFTOOLBOX_DLL_EXPORTS
#define CFTOOLBOX_DLL_API __declspec(dllexport)
#else
#define CFTOOLBOX_DLL_API __declspec(dllimport)
#endif

typedef void * CFT_CMClient;

CFTOOLBOX_DLL_API CFT_CMClient connect(const char * accountName, const char * password, const char * steamGuardKey, bool cacheClient);
CFTOOLBOX_DLL_API CFT_CMClient getClient(const char * accountName);
CFTOOLBOX_DLL_API unsigned int getContentServerAuthTickets(CFT_CMClient client, unsigned long appId, unsigned char * ticket);
// returns all the tickets required for content server (steam2ticket,cmSessionToken,appOwnershipTicket)
CFTOOLBOX_DLL_API void disconnect(CFT_CMClient client);
CFTOOLBOX_DLL_API const char * getConnectedAccountsList();

#endif
