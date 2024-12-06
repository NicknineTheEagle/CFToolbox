// SteamNetwork.h: interface for the SteamNetwork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STEAMNETWORK_H__311D2736_A290_4818_8185_96CC72FA2094__INCLUDED_)
#define AFX_STEAMNETWORK_H__311D2736_A290_4818_8185_96CC72FA2094__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RegistryVector.h"

class SteamNetwork  
{
public:
	int updateContentDescriptionRecord();
	char * getContentDescriptionRecordSerialized();
	RegistryVector * getContentDescriptionRecord(); // cached
	char * getConfigServer();
	unsigned short getConfigServerPort();
	void setConfigServer(char * configServer, unsigned short port);
	char * getUpdatesHistoryURL();


	SteamNetwork();
	virtual ~SteamNetwork();
 
};

#endif // !defined(AFX_STEAMNETWORK_H__311D2736_A290_4818_8185_96CC72FA2094__INCLUDED_)
