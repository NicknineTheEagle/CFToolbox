#ifndef _STEAM_CLIENT_H_
#define _STEAM_CLIENT_H_
#include "config.h"
#include <stdio.h>
#include "afxsock.h"


#include "stdafx.h"
#include "resource.h"
#include "afxsock.h"

#ifdef VIRTUAL_STEAM_SERVER
#include "Update.h"
#include "ServerManager.h"
#endif
int getCSERServers(char * host, int port, sockaddr_in * addrs, int maxi);
int getGeneralDirectoryServers(char * ip, int port, sockaddr_in * addrs, int maxi);
int getContentServers(char * host, int port, sockaddr_in * addrs, int maxi);


#ifdef VIRTUAL_STEAM_SERVER
 int backupCurrentConfig(char * fileName);
 int restoreConfig(char * fileName, ServerManager * serverManager);
 void updateSteam(int updateCDR, int updateClient, int updateAppDescriptions, CUpdate * dlg);
#endif

char * downloadCDR(sockaddr_in configServer, char * appName, int expectedLevel, int * cdrSize=0);

int getContentServers(char * host, int port, sockaddr_in * addrs, int maxi);
int getContentServers(char * host, int port, sockaddr_in * addrs, int maxi, int appId, int version, char*appName,int desiredLevel);
int getConfigServers(char * host, int port, sockaddr_in * addrs, int maxi);
//int getAuthServers(char * host, int port, sockaddr_in * addrs, int maxi,char * login);
int getAuthServers(/*char * host, int port,*/sockaddr_in clientService, sockaddr_in * addrs, int maxi,char * login);
int authenticateAndGetTicket(sockaddr_in authServer,char * login, char * password,char * ticket, CProgressCtrl * progress);

#endif