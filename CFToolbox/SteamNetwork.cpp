// SteamNetwork.cpp: implementation of the SteamNetwork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "ServerRegistry.h"
#include "SteamNetwork.h"
#include "Debug.h"
#include "SteamClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define STEAMCLIENT "Steam client"

char * defaultConfigServer="gds1.steampowered.com";
char * defaultUpdatesHistoryUrl="http://storefront.steampowered.com/platform/update_history/index.php?skin=2&id=%d";
unsigned short defaultConfigServerPort=27030;
static RegistryVector * cdrCache=0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SteamNetwork::SteamNetwork()
{
 
}

SteamNetwork::~SteamNetwork()
{
	 
}
char * SteamNetwork::getUpdatesHistoryURL()
{
	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return defaultUpdatesHistoryUrl;
	RegistryValue * url=network->getValue("UpdatesHistoryUrl");
	if (!url) 
	{
		url=new RegistryValue();
		url->setName("UpdatesHistoryUrl");
		url->setString(defaultUpdatesHistoryUrl);
		network->addValue(url);
		return defaultUpdatesHistoryUrl;
	}

	return url->getString();
}
char * SteamNetwork::getConfigServer()
{
	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return defaultConfigServer;
	RegistryValue * server=network->getValue("ConfigServer");
	if (!server) return defaultConfigServer;
	return server->getString();
}

unsigned short SteamNetwork::getConfigServerPort()
{
	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return defaultConfigServerPort;
	RegistryValue * port=network->getValue("ConfigServerPort");
	if (!port) return defaultConfigServerPort;
	return (unsigned short)port->getDWORD();
}

void SteamNetwork::setConfigServer(char * configServer, unsigned short port)
{
	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return;
	network->removeValue("ConfigServer");
	network->removeValue("ConfigServerPort");
	RegistryValue * val;
	
	val=new RegistryValue();
	val->setName("ConfigServer");
	val->setString(configServer);
	network->addValue(val);

	val=new RegistryValue();
	val->setName("ConfigServerPort");
	val->setDWORD(port);
	network->addValue(val);
	saveServerRegistry();

	char dbg[1000];
	sprintf(dbg,"%s:%d",getConfigServer(),getConfigServerPort());
	printDebug(DEBUG_LEVEL_DEBUG,STEAMCLIENT,"Config directory server modified",dbg,0);
}


RegistryVector * SteamNetwork::getContentDescriptionRecord()
{
	if (cdrCache) return cdrCache;
	char * cdrBlob=getContentDescriptionRecordSerialized();
 
	if (!cdrBlob) 
	{
	
		return 0;
	}

	Registry reg;
	
	cdrCache=reg.parseVector(cdrBlob);

	return cdrCache;
}

char * SteamNetwork::getContentDescriptionRecordSerialized()
{
	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return 0;
	RegistryValue * cdr=network->getValue("ContentDescriptionRecord");
	if (!cdr)
	{
		updateContentDescriptionRecord();
		cdr=network->getValue("ContentDescriptionRecord");
		if (!cdr)
		{
			return 0;
		}
	}
	return cdr->getData();
}

int SteamNetwork::updateContentDescriptionRecord()
{
	
	sockaddr_in genDirServers[20];
	
	printDebug(DEBUG_LEVEL_DEBUG,STEAMCLIENT,"Retrieving General directory servers...",0);	
	int nbGenDir=getGeneralDirectoryServers(getConfigServer(),getConfigServerPort(),genDirServers, 20);	

	if (!nbGenDir) {
		MessageBox(NULL,"Unable to retrieve the Content Description Record","CF Toolbox",MB_OK|MB_ICONERROR);
		printDebug(DEBUG_LEVEL_ERROR,STEAMCLIENT,"Can't retrieve General directory servers",0);	
		return 0;
	}
	int cdrSize=0;
	char * cdrBlob=downloadCDR(genDirServers[0],STEAMCLIENT,0,&cdrSize);
	if (!cdrSize || !cdrBlob)
	{
		MessageBox(NULL,"Unable to retrieve the Content Description Record","CF Toolbox",MB_OK|MB_ICONERROR);
		printDebug(DEBUG_LEVEL_ERROR,STEAMCLIENT,"ContentDescriptionRecord download error",0);	
		return 0;
	}

	RegistryKey * network=getServerRegistry()->setKey("/TopKey/SteamNetwork");
	if (!network) return 0;
	network->removeValue("ContentDescriptionRecord");
 
	RegistryValue * val;

	val=new RegistryValue();
	val->setName("ContentDescriptionRecord");
	val->setData(cdrBlob,cdrSize);
	network->addValue(val);
	saveServerRegistry();

	free(cdrBlob);
	if (cdrCache) 
	{
		delete cdrCache;
		cdrCache=0;
	}
	printDebug(DEBUG_LEVEL_DEBUG,STEAMCLIENT,"ContentDescriptionRecord updated",0);
	return 1;
}
