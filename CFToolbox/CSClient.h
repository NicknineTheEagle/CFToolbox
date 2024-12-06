// Content Server Client
#ifndef _CSCLIENT_H_
#define _CSCLIENT_H_
#include "cdr.h"
#include "gcf.h"

int downloadApp(char * host, int port, RegistryVector * cdrVector, DWORD requestedAppId, DWORD requestedAppVer, char * root, char * ticket,int ticketLen, char*appName,int desiredLogLevel,char * contentServerURL,char * taskName,DWORD * taskProgress,DWORD * remainingTime,int * pleaseStop,int securedUpdates,bool showRealBandwidth,bool validate);
int createGcfFile(GCF * gcf, char * root, char * gcfName, char * extractionName, char *appName, int desiredLevel);

int createArchive(char * gcfPath,char * archivePath, char * appName, int desiredLevel,char * taskName,DWORD * taskProgress);
int createUpdateFile(char * archivePath, char * gcfPath,char * commonName, char * patchPath, char * appName, int desiredLevel,char * taskName,DWORD * taskProgress, int * pleaseStop,int securedUpdates);
int applyUpdate(char * updatePath, char * gcfPath,char * commonName,char * decryptionKey,char * appName, int desiredLevel,char * taskName,DWORD * taskProgress, int * pleaseStop,int securedUpdates);

#endif