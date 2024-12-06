#include "steamClient.h"
#include <time.h>
#include "SHA1.h"
#include "Rijndael.h"
#include "crypto.h"
#include <stdio.h>
#include "socketTools.h"
#include "debug.h"
#include "iniparser.h"

#ifdef VIRTUAL_STEAM_SERVER
#include "steamPatcher.h"
#include "HttpClient.h"
#include "HttpServer.h"
#endif

#ifdef CF_TOOLBOX
#include "ProgressPopup.h"
#endif

#include "Registry.h"
#include "ServerRegistry.h"
#include <direct.h>
#include "zlib\zlib.h"

int getCSERServers(char * host, int port, sockaddr_in * addrs, int maxi)
{
	
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = getHostIp(host);
	clientService.sin_port = htons( port );
	
	SOCKET socket=connectSocket(clientService);
	if (socket==0) {closesocket(socket); return 0;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return 0;}
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"14");
	
	int howMuch=readSocket(socket,4);
	int nbAddr=readSocket(socket,2);
	
	int toRead=(nbAddr>maxi ? maxi : nbAddr);
	
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind<toRead)
		{
			addrs[ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&(addrs[ind].sin_addr.s_addr),4,0);
			addrs[ind].sin_port = (unsigned short) readSocket(socket,2);
		}
		else readSocket(socket,6);
	}
	closesocket(socket);
	
	return toRead;
}

int getGeneralDirectoryServers(char * host, int port, sockaddr_in * addrs, int maxi)
{
	
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = getHostIp(host);
	clientService.sin_port = htons( port );
	
	SOCKET socket=connectSocket(clientService);
	if (socket==0) {closesocket(socket); return 0;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return 0;}
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"03");
	
	int howMuch=readSocket(socket,4);
	int nbAddr=readSocket(socket,2);
	
	int toRead=(nbAddr>maxi ? maxi : nbAddr);
	
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind<toRead)
		{
			addrs[ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&(addrs[ind].sin_addr.s_addr),4,0);
			addrs[ind].sin_port = (unsigned short) readSocket(socket,2);
		}
		else readSocket(socket,6);
	}
	closesocket(socket);
	
	return toRead;
}

int getContentServersIps(sockaddr_in addr, sockaddr_in * addrs, int maxi)
{
		SOCKET socket=connectSocket(addr);
		if (socket==0) {closesocket(socket); return -1;}
		sendSocket(socket,"00 00 00 02");
		
		int ack=readSocket(socket,1);
		if (ack!=1) {closesocket(socket); return -1;}
		
		int maxi2=maxi/=2;
		sendSocket(socket,"00 00 00 15 00 00 00 00 00 00 00 00 00 00 00 00");
		sendSafe(socket,(char*)&maxi2,1,0);
		sendSocket(socket,"00 00 00 05 FF FF FF FF");
		
		int howMuch=readSocket(socket,4);
		int nbAddr=readSocket(socket,2);
		
		int toRead=(nbAddr>maxi ? maxi: nbAddr);
		int readed=0;
		for (int ind=0;ind<nbAddr;ind++)
		{
			if (ind*2<toRead)
			{
				DWORD serverId=readSocket(socket,4);
				addrs[2*ind].sin_family = AF_INET;
				recvSafe(socket,(char*)&(addrs[2*ind].sin_addr.s_addr),4,0);
				addrs[2*ind].sin_port = (unsigned short) readSocket(socket,2);
				addrs[2*ind+1].sin_family = AF_INET;
				recvSafe(socket,(char*)&(addrs[2*ind+1].sin_addr.s_addr),4,0);
				addrs[2*ind+1].sin_port = (unsigned short) readSocket(socket,2);	
				readed=2*(ind+1);
				
			}
			else readSocket(socket,0x10);
		}
		closesocket(socket);
		
		return readed;
}

int getContentServers(char * host, int port, sockaddr_in * addrs, int maxi)
{
	
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = getHostIp(host);
	clientService.sin_port = htons( port );
	
	// requesting content servers registry ips
	
	SOCKET socket=connectSocket(clientService);
	if (socket==0) {closesocket(socket); return 0;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return 0;}
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"06");
	
	int howMuch=readSocket(socket,4);
	int nbAddr=readSocket(socket,2);
	
	int toRead=(nbAddr>10 ? 10 : nbAddr);
	
	sockaddr_in raddrs[10];
	
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind<toRead)
		{
			raddrs[ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&(raddrs[ind].sin_addr.s_addr),4,0);
			raddrs[ind].sin_port = (unsigned short) readSocket(socket,2);
		}
		else readSocket(socket,6);
	}
	closesocket(socket);
	
	// connect to server and get real adresses
	int nbRead=0;
	for (int ind=0;ind<toRead;ind++)
	{
		nbRead+=getContentServersIps(raddrs[ind],addrs+nbRead,maxi-nbRead);
	 
		if (nbRead>=maxi) return nbRead;
	}
	
	
	return nbRead;
}


int updateClientKeys(sockaddr_in addr)
{
	
	SOCKET socket=connectSocket(addr);
	if (socket==0) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't connect to config server",DEBUG_LEVEL_INFO); return -1;}
	sendSocket(socket,"00 00 00 03");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Config server protocol error",DEBUG_LEVEL_INFO); return -1;}
	
	int externalIp=readSocket(socket,4);
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"04");
	
	char key160[160];
	char key256[256];
	
	int howMuch=readSocket(socket,2);
	recvSafe(socket,key160,160,0);
	
	//sendSocket(socket,"01 00");
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Updating Steam instance public key...",DEBUG_LEVEL_INFO);
	howMuch=readSocket(socket,2);
	recvSafe(socket,key256,256,0);
	
	
	closesocket(socket);
	
	
	RegistryKey * registry=getServerRegistry(1);
	RegistryKey * serverData=registry->setKey("/TopKey/SynchronizedData");
	
	RegistryValue * record=new RegistryValue();
	record->setName("SteamInstancePublicKey");
	record->setData(key160,160);
	serverData->addValue(record);
	
	record=new RegistryValue();
	record->setName("SteamInstancePublicKeySignature");
	record->setData(key256,256);
	serverData->addValue(record);
	
	saveServerRegistry();
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Steam instance public key updated",DEBUG_LEVEL_INFO);
	return 0;
}

int updateClientsBlob(sockaddr_in addr)
{
	
	SOCKET socket=connectSocket(addr);
	if (socket==0) {closesocket(socket); printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't connect to config server",DEBUG_LEVEL_INFO);return -1;}
	sendSocket(socket,"00 00 00 03");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Config server protocol error",DEBUG_LEVEL_INFO); return -1;}
	
	int externalIp=readSocket(socket,4);
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"01");
	
	int howMuch=readSocket(socket,4);
	
	char * buffer=(char*)malloc(howMuch);
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Updating Clients description record...",DEBUG_LEVEL_INFO);
	recvSafe(socket,buffer,howMuch,0);
	
	closesocket(socket);
	
	RegistryKey * registry=getServerRegistry(1);
	RegistryKey * serverData=registry->setKey("/TopKey/SynchronizedData");
	
	RegistryValue * record=new RegistryValue();
	record->setName("ClientsDescriptionRecord");
	record->setData(buffer,howMuch);
	serverData->addValue(record);
	
	saveServerRegistry();
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Clients description record updated",DEBUG_LEVEL_INFO);
	
/*	Registry r;
	r.parseVector(buffer)->printXML();
*/
	free(buffer);
	
	return 0;
}

 
char * downloadCDR(sockaddr_in configServer, char * appName, int expectedLevel, int * size)
{
	if (size) *size=0;
	time_t currentTime;
	time(&currentTime);
	
	SOCKET socket=connectSocket(configServer);
	if (socket==0) {
		closesocket(socket); 
		printDebug(DEBUG_LEVEL_ERROR,appName,"Can't connect to config server",expectedLevel); 
		return 0;
	}
	sendSocket(socket,"00 00 00 03");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {
		closesocket(socket); 
		printDebug(DEBUG_LEVEL_ERROR,appName,"Invalid Config server protocol",expectedLevel); 
		return 0;
	}
	
	int externalIp=readSocket(socket,4);
	
	sendSocket(socket,"00 00 00 15");
	sendSocket(socket,"09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	
	char dummy[11];
	recvSafe(socket,dummy,11,0);
	
	long howMuch=readSocket(socket,4);
	//	dlg->m_progress.SetRange32(0,howMuch);
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Downloading Content description record...",expectedLevel); 
	
	char * buffer;
	buffer = (char*) malloc (howMuch);
	if (buffer == NULL) return 0;

#ifdef CF_TOOLBOX
	CProgressPopup progress;
	progress.m_message="Updating ContentDescriptionRecord";
	progress.Create(IDD_PROGRESS);
	progress.m_progress.SetRange32(0,howMuch);
	progress.m_progress.SetPos(0);
	
	
#endif	

	int readed=0;
	while (readed<howMuch)
	{
		readed+=recv(socket,buffer+readed,howMuch-readed,0);
#ifdef CF_TOOLBOX
	progress.m_progress.SetPos(readed);
#endif
		//	dlg->m_progress.SetPos(readed);
	}
#ifdef CF_TOOLBOX
	progress.DestroyWindow(); 
#endif	
	// unzip
	
	WORD header=*(WORD*)buffer;
	DWORD compressedSize=*(DWORD*)(buffer+0x02)-0x14;
	DWORD uncompressedSize=*(DWORD*)(buffer+0x0A);
	WORD unknown2=*(WORD*)(buffer+0x12);
	char * zData=buffer+0x14;
	
	char * unzipBuffer=(char*)malloc((DWORD)uncompressedSize);
	
	//	printf("Recieved zipped CDR (%d bytes) (%d bytes unzipped)\n",compressedSize,uncompressedSize);
	
	int result=uncompress((unsigned char*)unzipBuffer,&uncompressedSize,(unsigned char*)zData,(DWORD)compressedSize);
	free(buffer);
	
	if (result) 
	{
		free(unzipBuffer);
		printf("Unzip error : %d\n",result);
		return 0;
	}
	if (size) *size=uncompressedSize;
	return unzipBuffer;
}
 
#ifdef VIRTUAL_STEAM_SERVER
int updateApplicationBlob(sockaddr_in addr,CUpdate * dlg)
{
	time_t currentTime;
	time(&currentTime);
	
	SOCKET socket=connectSocket(addr);
	if (socket==0) {
		closesocket(socket); 
		printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't connect to config server",DEBUG_LEVEL_INFO); 
		return -1;
	}
	sendSocket(socket,"00 00 00 03");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {
		closesocket(socket); 
		printDebug(DEBUG_LEVEL_ERROR,"Steam update","Invalid Config server protocol",DEBUG_LEVEL_INFO); 
		return -1;
	}
	
	int externalIp=readSocket(socket,4);
	
	sendSocket(socket,"00 00 00 15");
	sendSocket(socket,"09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	
	char dummy[11];
	recvSafe(socket,dummy,11,0);

	long howMuch=readSocket(socket,4);
	dlg->m_progress.SetRange32(0,howMuch);
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Updating Content description record...",DEBUG_LEVEL_INFO); 

	char * buffer;
	buffer = (char*) malloc (howMuch);
	if (buffer == NULL) return -1;
	
	int readed=0;
	while (readed<howMuch)
	{
		readed+=recv(socket,buffer+readed,howMuch-readed,0);
		dlg->m_progress.SetPos(readed);
	}
	

	// unzip

/*	WORD header=*(WORD*)buffer;
	DWORD compressedSize=*(DWORD*)(buffer+0x02)-0x14;
	DWORD uncompressedSize=*(DWORD*)(buffer+0x0A);
	WORD unknown2=*(WORD*)(buffer+0x12);
	char * zData=buffer+0x14;
	
	char * unzipBuffer=(char*)malloc((DWORD)uncompressedSize);
	
	printf("Recieved zipped CDR (%d bytes) (%d bytes unzipped)\n",compressedSize,uncompressedSize);
 
	int result=uncompress((unsigned char*)unzipBuffer,&uncompressedSize,(unsigned char*)zData,(DWORD)compressedSize);
	free(buffer);
	
	if (result) 
	{
		free(unzipBuffer);
		printf("Unzip error : %d\n",result);
		return 1;
	}
	// end unzip
*/
	//rebindSteam(buffer,howMuch,"VSTEAMSRV");
	
	RegistryKey * registry=getServerRegistry(1);
	RegistryKey * serverData=registry->setKey("/TopKey/SynchronizedData");
	RegistryValue * date=new RegistryValue();
	date->setName("ContentDescriptionRecordDate");
	//date->setType(TYPE_DWORD);
	char buff[30];
	date->setString(getDateTime(buff));
	serverData->addValue(date);
	
	RegistryValue * record=new RegistryValue();
	record->setName("OriginalContentDescriptionRecord");
	//record->setType(TYPE_DATA);
	record->setData(buffer,readed);
	serverData->addValue(record);
	
	saveServerRegistry();
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Content description record updated",DEBUG_LEVEL_INFO); 
	//fwrite(buffer,1,howMuch,out);
//	free(unzipBuffer);
	closesocket(socket);
	dlg->m_progress.SetPos(0);
	return 0;
}

DWORD getAppsDescriptions(char * host,CUpdate * dlg)
{
	RegistryKey * registry=getServerRegistry(1);
	RegistryValue * serverData=registry->getKey("/TopKey/SynchronizedData")->getValue("OriginalContentDescriptionRecord");
	
	deleteHttpServerPath("/apps");
	
	Registry reg;
	//hexaPrintf(serverData->getData(),serverData->getDataSize());

	char * unzippedBlob;
	reg.unzipBlob(serverData->getData(),&unzippedBlob);

	RegistryVector * root=reg.parseVector(unzippedBlob);
	//	root->printXML();
	RegistryVector * apps=root->getNode(1)->getVectorValue();
	char buffer[200000];
	char contentType[1000];
	char path[100];
	char file[100];
	
	WORD count=0;
	WORD downloaded=0;
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Updating Steam apps meta data...",DEBUG_LEVEL_INFO); 
	dlg->m_progress.SetRange(0,apps->getNbNodes());
	for (unsigned int ind=0;ind<apps->getNbNodes();ind++)
	{
		RegistryVector * app=apps->getNodeAt(ind)->getVectorValue();
		dlg->m_progress.SetPos(ind);
		RegistryNode * node14=app->getNode(14);
		if (node14!=0 && node14->getValueType()==NODE_VALUE_TYPE_VECTOR && node14->getVectorValue()->getNbNodes()>0) 
		{
			
			sprintf(path,"apps/AppID_%d",app->getNode(1)->getDWordValue());
			sprintf(file,"AppID_%d",app->getNode(1)->getDWordValue());
			
		//	printf("- %d %s\n",app->getNode(1)->getDWordValue(),app->getNode(2)->getStringValue());
			long len=httpDownload(host,80,path,0,contentType,buffer);
			count++;
			//	printf("        : %d\n", len);
			
			if (len>0) 
			{
				
				addHttpServerFile(path,buffer,len,contentType);
				downloaded++;
			}
		}
	}
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Steam apps meta data updated",DEBUG_LEVEL_INFO); 
	free(unzippedBlob);
	saveServerRegistry();
	delete root ;
	dlg->m_progress.SetPos(0);
	return (((DWORD)count)*0x10000)+(DWORD)downloaded;
}
#endif

int saveBuffer(char * fileName, char * data, DWORD len)
{
	unlink(fileName); 
	FILE * file=fopen(fileName,"wb");
	if (file==NULL)
	{
		return -1;
	}
	
	fwrite (data,1,len,file);
	fclose(file);
	return 0;
}

#ifdef VIRTUAL_STEAM_SERVER
DWORD getSteamClientUpdateFile(SOCKET socket, char * name,CUpdate * dlg)
{
	int nameLen=strlen(name);
	int totalLen=4+4+4+nameLen+4;
	int reversed=reverseBytes(totalLen,4);
	sendSafe(socket,(char*)&reversed,4,0);
	sendSocket(socket,"00 00 00 00");
	sendSocket(socket,"00 00 00 00");
	reversed=reverseBytes(nameLen,4);
	sendSafe(socket,(char*)&reversed,4,0);
	sendSafe(socket,name,nameLen,0);
	sendSocket(socket,"00 00 00 00");
	
	
	DWORD dataLen=readSocket(socket,4);
	if (!dataLen) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Content server protocol error",DEBUG_LEVEL_INFO); return -1;}
	DWORD dataLen2=readSocket(socket,4);
	if (dataLen!=dataLen2) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Content server protocol error",DEBUG_LEVEL_INFO); return -1;}
	
	char * data=(char*)malloc(dataLen);
//	printf("%s (%d bytes):",name,dataLen);
	char dbg[1000];
	sprintf(dbg,"Downloading %s (%d bytes) ...",name,dataLen);
	printDebug(DEBUG_LEVEL_INFO,"Steam update",dbg,DEBUG_LEVEL_INFO);
	dlg->m_progress.SetRange32(0,dataLen);
	int start=0;
	while (dataLen>0)
	{
		int read=recv(socket,data+start,dataLen,0);
		if (read>0)
		{
	//		printf(".");
			start+=read;
			dlg->m_progress.SetPos(start);
			dataLen-=read;
		}
		else
		{
			Sleep(100);
		}
	}
//	printf("\n\n");

	/*RegistryValue * record=new RegistryValue();
	record->setName(name);
	record->setData(data,dataLen2);
	serverData->addValue(record);*/
	char * root=getServerRegistry(1)->getKey("/TopKey/ServerConfig")->getValue("PKGPath")->getString();
	char path[1000];
//	_mkdir(root);
	sprintf(path,"%s\\%s",root,name);
	if (saveBuffer(path,data,dataLen2)<0) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't create file",path,DEBUG_LEVEL_INFO); return -2;}	

	free(data);
	dlg->m_progress.SetPos(0);
	return 0;
}

DWORD getSteamClientUpdatePkg(sockaddr_in addr,CUpdate * dlg)
{
	
	SOCKET socket=connectSocket(addr);
    if (socket==0) {closesocket(socket);printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't connect to content server",DEBUG_LEVEL_INFO); return -1;}
	sendSocket(socket,"00 00 00 03");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); printDebug(DEBUG_LEVEL_ERROR,"Steam update","Content server protocol error",DEBUG_LEVEL_INFO);return -1;}

	
	//RegistryKey * registry=getServerRegistry();
	//RegistryKey * serverData=registry->setKey("/TopKey/SynchronizedData/ClientUpdate");
	
	Registry r;
	RegistryKey * synchronizedData=getServerRegistry(1)->getKey("/TopKey/SynchronizedData");

	RegistryValue * clientsDataV=synchronizedData->getValue("ClientsDescriptionRecord");

	RegistryVector * clientsData=r.parseVector(clientsDataV->getData());
	
	DWORD steamVersion=clientsData->getNode(1)->getDWordValue();
	DWORD steamUIVersion=clientsData->getNode(2)->getDWordValue();


	char name[1000];
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Updating client packages...",DEBUG_LEVEL_INFO);
	sprintf(name,"Steam_%d.pkg",steamVersion);
	int res=0;
	if (res=getSteamClientUpdateFile(socket,name,dlg)) return res;
	sprintf(name,"Steam_%d.pkg_rsa_signature",steamVersion);
	if (res=getSteamClientUpdateFile(socket,name,dlg)) return res;
	sprintf(name,"SteamUI_%d.pkg",steamUIVersion);
	if (res=getSteamClientUpdateFile(socket,name,dlg)) return res;
	sprintf(name,"SteamUI_%d.pkg_rsa_signature",steamUIVersion);
	if (res=getSteamClientUpdateFile(socket,name,dlg)) return res;

	
	closesocket(socket);

	saveServerRegistry();
	delete clientsData;
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Client packages updated",DEBUG_LEVEL_INFO);
	return 0;
}
#endif
int patchContentDescriptionRecord()
{
			RegistryKey * registry=getServerRegistry(1);
			if (!registry) 
		return 1;			
			RegistryKey * serverData=registry->getKey("/TopKey/SynchronizedData");
			RegistryKey * conf=registry->getKey("/TopKey/ServerConfig");
			if (!serverData) 
		return 1;
			RegistryValue * record=serverData->getValue("OriginalContentDescriptionRecord");
			RegistryValue * httpPort=conf->getValue("HttpServerPort");
			if (!record) 
			return 1;

	    	Registry r;
			char * unzippedBlob;
			DWORD unzippedLen=r.unzipBlob(record->getData(),&unzippedBlob);
			RegistryVector * root=r.parseVector(unzippedBlob);
			// winui

			RegistryVector * gcfsList=root->getNode(1)->getVectorValue();

			RegistryNode * winui=gcfsList->getNode(7);

			RegistryVector * winuiConfig=winui->getVectorValue()->getNode(14)->getVectorValue();

			RegistryNode * storeFrontURL =winuiConfig->getNode("StorefrontURL");

			char url[100]="http://VSTEAMSRV/v/?client=1";
			if (strcmp(httpPort->getString(),"80")) sprintf(url,"http://VSTEAMSRV:%s/v/?client=1",httpPort->getString());
//printf(url);

			storeFrontURL->setStringValue(url);

			int msgIndex=1;
			RegistryNode * msgName=0;
			char key[100];
			
			// remove advertisements
			while (1)
			{
				sprintf(key,"Msg%dName",msgIndex);
				msgName=winuiConfig->getNode(key);
				if (!msgName) break;

				winuiConfig->removeNode(key);
				sprintf(key,"Msg%dUrl",msgIndex);winuiConfig->removeNode(key);
				sprintf(key,"Msg%dHaveAppId",msgIndex);winuiConfig->removeNode(key);
				sprintf(key,"Msg%dHavePlayedAppID",msgIndex);winuiConfig->removeNode(key);
				sprintf(key,"Msg%dHaveSubId",msgIndex);winuiConfig->removeNode(key);
				sprintf(key,"Msg%dNotHaveAppId",msgIndex);winuiConfig->removeNode(key);
				sprintf(key,"Msg%dNotHaveSubId",msgIndex);winuiConfig->removeNode(key);

				msgIndex++;
			}
			
			// add advertisement

			RegistryNode * node=0;
			
			node=new RegistryNode();
			node->setStringDescription("Msg1Url");
			node->setStringValue("http://VSTEAMSRV/advertisement/");
			winuiConfig->addNode(node);

			node=new RegistryNode();
			node->setStringDescription("Msg1Name");
			node->setStringValue("VirtualSteamServer_advertisement");
			winuiConfig->addNode(node);

			node=new RegistryNode();
			node->setStringDescription("Msg1HaveAppId");
			node->setDWordValue(48);
			winuiConfig->addNode(node);

			node=new RegistryNode();
			node->setStringDescription("Msg1HaveSubId");
			node->setDWordValue(48);
			winuiConfig->addNode(node);

			node=new RegistryNode();
			node->setStringDescription("Msg1NotHaveAppId");
			node->setDWordValue(48);
			winuiConfig->addNode(node);

			node=new RegistryNode();
			node->setStringDescription("Msg1NotHaveSubId");
			node->setDWordValue(48);
			winuiConfig->addNode(node);


			// packs
		 	RegistryVector * packs=root->getNode(2)->getVectorValue();
			RegistryNode * steamPack=packs->getNode((DWORD)0);
			RegistryVector * allowedApps=steamPack->getVectorValue()->getNode(6)->getVectorValue();
			allowedApps->clear();
				
			RegistryVector * gcfs=root->getNode(1)->getVectorValue();
			
			for (unsigned int ind=0;ind<gcfs->getNbNodes();ind++)
			{
				RegistryNode * allowedApp=new RegistryNode();
				allowedApp->setDwordDescription(gcfs->getNodeAt(ind)->getDwordDescription());
			//	printf("allowing : %d\n",gcfs->getNodeAt(ind)->getDwordDescription());
				allowedApps->addNode(allowedApp);
			}
		//	steamPack->printXML();
		
			steamPack->setNextNode(0);
		//	steamPack->printXML();

   
			char * buffer=(char*)malloc(unzippedLen);
			int len=r.serializeVector(root,buffer);
			free(unzippedBlob);
 
			char * zippedBlob=0;

			int zippedBlobLen=	r.zipBlob(buffer,len, &zippedBlob);
			free(buffer);

			RegistryValue * value=new RegistryValue();
			value->setName("ContentDescriptionRecord");
		//	value->setData(buffer,len);
			value->setData(zippedBlob,zippedBlobLen);
			serverData->addValue(value);

			saveServerRegistry();
			delete(root);

			free(zippedBlob);
			return 0;
}

#ifdef VIRTUAL_STEAM_SERVER
int updateCDRFromSteamServers(char * host,int port,CUpdate * dlg)
{
	sockaddr_in addrs[10];
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Retrieving config servers from",host,DEBUG_LEVEL_INFO);
	int nb=getGeneralDirectoryServers(host,port,addrs,10);
	
	if (nb==0) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't retrieve config servers from",host,DEBUG_LEVEL_INFO);
		return -1;
	}
	for (int ind=0;ind<nb;ind++)
	{	
		if (!updateApplicationBlob(addrs[ind],dlg)) return 0;
	}
	return -1;
}
#endif
int updateClientsBlobAndKeys(char * host,int port)
{
	sockaddr_in addrs[10];
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Retrieving config servers from",host,DEBUG_LEVEL_INFO);
	int nb=getGeneralDirectoryServers(host,port,addrs,10);
	
	if (nb==0) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't retrieve config servers from",host,DEBUG_LEVEL_INFO);
		return -1;
	}
	for (int ind=0;ind<nb;ind++)
	{	
		if (!updateClientsBlob(addrs[ind])) 
		{
			for (int ind2=ind;ind2<nb;ind2++)
			{	
				if (!updateClientKeys(addrs[ind2])) 
				{
					return 0;
				}
				
			}
			return 0;
		}
	}
	return -1;
}

#ifdef VIRTUAL_STEAM_SERVER
int updateClientFromSteamServers(char * host,int port,CUpdate * dlg)
{
	sockaddr_in addrs[10];
	printDebug(DEBUG_LEVEL_INFO,"Steam update","Retrieving content servers from",host,DEBUG_LEVEL_INFO);
	int nb=getContentServers(host,port,addrs,10);
	
	if (nb==0) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"Steam update","Can't retrieve content servers from",host,DEBUG_LEVEL_INFO);
		return -1;
	}
	for (int ind=0;ind<nb;ind++)
	{	
		if (!getSteamClientUpdatePkg(addrs[ind],dlg)) return 0;
	}
	
	
	return -1;
	
}

void updateSteam(int updateCDR, int updateClient, int updateAppDescriptions,CUpdate * dlg)
{
 
		RegistryKey * config=getServerRegistry(1)->setKey("/TopKey/ServerConfig");
		
		if (updateCDR)
		{
			updateCDRFromSteamServers(config->getValue("ValveConfigServerAddress")->getString(),27030,dlg);
			patchContentDescriptionRecord();
		}

		if (updateClient)
		{
			updateClientsBlobAndKeys(config->getValue("ValveConfigServerAddress")->getString(),27030);
			updateClientFromSteamServers(config->getValue("ValveConfigServerAddress")->getString(),27030,dlg);
		}

		if (updateAppDescriptions)
		{
 			getAppsDescriptions(config->getValue("ValveStorefrontAddress")->getString(),dlg);
		}
		
		return;
}
#endif
DWORD addSteamClientUpdateFile(RegistryKey * key, char * name)
{
 	char * root=getServerRegistry(1)->getKey("/TopKey/ServerConfig")->getValue("PKGPath")->getString();
	char path[1000];
//	_mkdir(root);
	sprintf(path,"%s\\%s",root,name);

	FILE * file=fopen(path,"rb");
	if (file==NULL)
	{
		return 0;
	}
	
	// obtain file size.
	fseek (file , 0 , SEEK_END);
	long lSize = ftell (file);
	rewind (file);
	
	char * data;
	// allocate memory to contain the whole file.
	data = (char*) malloc (lSize);
	if (!data)  
	{
		return 0;//throw exception("File error");
	}
	
	// copy the file into the buffer.
	fread (data,1,lSize,file);
	fclose(file);

	RegistryValue * value=new RegistryValue();
	value->setName(name);
	value->setData(data,lSize);

	key->addValue(value);

	free(data);
	return lSize;
}

int backupCurrentConfig(char * fileName)
{
	unlink(fileName); 
	FILE * file=fopen(fileName,"wb");
	if (file==NULL)
	{
		return 0;
	}

	int size=3000;

	RegistryKey * registry=getServerRegistry(1);
	RegistryKey * serverData=registry->getKey("/TopKey/SynchronizedData");


	Registry reg;
	RegistryKey * root=new RegistryKey();
	root->setName("Version");
	
	RegistryValue * value=new RegistryValue();
	value->setName("ContentDescriptionRecordDate");
	value->setString(serverData->getValue("ContentDescriptionRecordDate")->getString());
	root->addValue(value);

	value=new RegistryValue();
	value->setName("ContentDescriptionRecord");
	value->setData(serverData->getValue("ContentDescriptionRecord")->getData(),serverData->getValue("ContentDescriptionRecord")->getDataSize());
	root->addValue(value);
	size+=serverData->getValue("ContentDescriptionRecord")->getDataSize();

	value=new RegistryValue();
	value->setName("ClientsDescriptionRecord");
	value->setData(serverData->getValue("ClientsDescriptionRecord")->getData(),serverData->getValue("ClientsDescriptionRecord")->getDataSize());
	root->addValue(value);
	size+=serverData->getValue("ClientsDescriptionRecord")->getDataSize();

	value=new RegistryValue();
	value->setName("SteamInstancePublicKey");
	value->setData(serverData->getValue("SteamInstancePublicKey")->getData(),serverData->getValue("SteamInstancePublicKey")->getDataSize());
	root->addValue(value);

	value=new RegistryValue();
	value->setName("SteamInstancePublicKeySignature");
	value->setData(serverData->getValue("SteamInstancePublicKeySignature")->getData(),serverData->getValue("SteamInstancePublicKeySignature")->getDataSize());
	root->addValue(value);

	RegistryKey * pkg=new RegistryKey();
	pkg->setName("pkg");
	root->addChild(pkg);

	RegistryValue * clientsDataV=serverData->getValue("ClientsDescriptionRecord");

	RegistryVector * clientsData=reg.parseVector(clientsDataV->getData());
	
	DWORD steamVersion=clientsData->getNode(1)->getDWordValue();
	DWORD steamUIVersion=clientsData->getNode(2)->getDWordValue();

	char name[1000];

	sprintf(name,"Steam_%d.pkg",steamVersion);
	size+=addSteamClientUpdateFile(pkg,name);
	sprintf(name,"Steam_%d.pkg_rsa_signature",steamVersion);
	size+=addSteamClientUpdateFile(pkg,name);
	sprintf(name,"SteamUI_%d.pkg",steamUIVersion);
	size+=addSteamClientUpdateFile(pkg,name);
	sprintf(name,"SteamUI_%d.pkg_rsa_signature",steamUIVersion);
	size+=addSteamClientUpdateFile(pkg,name);
	
	char * blob=(char *) malloc(size);
	unsigned long blobSize=reg.serializeBlob(root,blob);
	char * zip=0;
	unsigned long zipSize=reg.zipBlob(blob,blobSize,&zip);
	
//	fwrite (blob,1,blobSize,file);
	free(blob);


 	
	// copy the file into the buffer.
	fwrite (zip,1,zipSize,file);

	fclose(file);
	 
	free(zip);
	delete clientsData;
	return 1;
}

void restoreSteamClientUpdateFile(RegistryKey * key, char * name)
{
	RegistryValue * file=key->getValue(name);
	if (!file) return;

	char * root=getServerRegistry(1)->getKey("/TopKey/ServerConfig")->getValue("PKGPath")->getString();
	char path[1000];
//	_mkdir(root);
	sprintf(path,"%s\\%s",root,name);
	saveBuffer(path,file->getData(),file->getDataSize());
}


#ifdef VIRTUAL_STEAM_SERVER

int restoreConfig(char * fileName, ServerManager * serverManager)
{
	FILE * file=fopen(fileName,"rb");
	if (file==NULL)
	{
		return 0;
	}
	
	// obtain file size.
	fseek (file , 0 , SEEK_END);
	long lSize = ftell (file);
	rewind (file);
	
	char * zip;
	// allocate memory to contain the whole file.
	zip = (char*) malloc (lSize);
	if (!zip)  
	{
		return 0;//throw exception("File error");
	}
	
	// copy the file into the buffer.
	fread (zip,1,lSize,file);
	fclose(file);

	char * blob=0;
	Registry reg;
	unsigned long size=reg.unzipBlob(zip,&blob);
	free(zip);
	if (!size) return 0;

	RegistryKey * root=reg.parseBlob(blob);
	free(blob);
	if (!root) return 0;

	if (strcmp("Version",root->getName()))
	{
		delete root;
		return 0;
	}
	
	RegistryKey * pkg=root->getKey("/Version/pkg");
	if (pkg)
	{
		RegistryValue * v=pkg->getFirstValue();
		while (v)
		{
			restoreSteamClientUpdateFile(pkg,v->getName());
			v=v->getNextValue();
		}
	}
	
	RegistryKey * registry=getServerRegistry(1);
	RegistryKey * serverData=registry->getKey("/TopKey/SynchronizedData");
	serverManager->setUpdating();
	setUpdating(1);
	
		RegistryValue * v=root->getFirstValue();
		while (v)
		{
			RegistryValue * copy=new RegistryValue();
			copy->setName(v->getName());
			copy->setData(v->getData(),v->getDataSize());
			copy->setType(v->getType());
			serverData->addValue(copy);
			v=v->getNextValue();
		}
	

	saveServerRegistry();
	delete root;
	return 1;
}
#endif
int getContentServersIps(sockaddr_in addr, sockaddr_in * addrs, int maxi, int appId, int version)
{
	
	SOCKET socket=connectSocket(addr);
	if (socket==0) {closesocket(socket); return -1;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return -1;}
	
	//int maxi2=maxi/=2;
	sendSocket(socket,"00 00 00 19 00 00 01");
	appId=reverseBytes(appId);
	version=reverseBytes(version);
	maxi=reverseBytes(maxi,2);
	sendSafe(socket,(char*)&appId,4,0);
	sendSafe(socket,(char*)&version,4,0);
	sendSafe(socket,(char*)&maxi,2,0);
	sendSocket(socket,"00 00 00 05 FF FF FF FF FF FF FF FF"); // not sure
	
	int howMuch=readSocket(socket,4);
	int nbAddr=readSocket(socket,2);
	
	int toRead=(nbAddr>maxi ? maxi: nbAddr);
	int readed=0;
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind*2<toRead)
		{
			DWORD serverId=readSocket(socket,4);
			addrs[2*ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&(addrs[2*ind].sin_addr.s_addr),4,0);
			addrs[2*ind].sin_port = (unsigned short) readSocket(socket,2);
			addrs[2*ind+1].sin_family = AF_INET;
			recvSafe(socket,(char*)&(addrs[2*ind+1].sin_addr.s_addr),4,0);
			addrs[2*ind+1].sin_port = (unsigned short) readSocket(socket,2);	
			readed=2*(ind+1);
		}
		else readSocket(socket,0x10);
	}
	closesocket(socket);
	
	return readed;
	
}

int getContentServers(char * host, int port, sockaddr_in * addrs, int maxi, int appId, int version,char*appName,int desiredLevel)
{
	// forced mode added in 1.0.11
	dictionary * d=0;
    char * cshost=0;
	char * csport=0;

	d = iniparser_load("ContentServer.ini");
	
	if (d)
	{
		cshost=iniparser_getstr(d, "contentserver:host");
		csport=iniparser_getstr(d, "contentserver:port");
	}

	if (cshost && csport)
	{
		// forcing Content server
		printDebug(DEBUG_LEVEL_DEBUG,appName,"Forcing content server selection from ContentServer.ini",desiredLevel);    

		addrs[0].sin_family = AF_INET;
		addrs[0].sin_addr.s_addr=getHostIp(cshost);
		addrs[0].sin_port = htons(atoi(csport));
		
		addrs[1].sin_family = AF_INET;
		addrs[1].sin_addr.s_addr=getHostIp(cshost);
		addrs[1].sin_port = htons(atoi(csport));

		if (d) iniparser_freedict(d);

		return 2;
	}
	else
	{
		if (d) iniparser_freedict(d);

		sockaddr_in clientService; 
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = getHostIp(host);
		clientService.sin_port = htons( port );
		
		// requesting content servers registry ips
		
		SOCKET socket=connectSocket(clientService);
		if (socket==0) {closesocket(socket); return 0;}
		sendSocket(socket,"00 00 00 02");
		
		int ack=readSocket(socket,1);
		if (ack!=1) {closesocket(socket); return 0;}
		
		sendSocket(socket,"00 00 00 01");
		sendSocket(socket,"06");
		
		int howMuch=readSocket(socket,4);
		int nbAddr=readSocket(socket,2);
		
		int toRead=(nbAddr>10 ? 10 : nbAddr);
		
		sockaddr_in raddrs[10];
		
		for (int ind=0;ind<nbAddr;ind++)
		{
			if (ind<toRead)
			{
				raddrs[ind].sin_family = AF_INET;
				recvSafe(socket,(char*)&(raddrs[ind].sin_addr.s_addr),4,0);
				raddrs[ind].sin_port = (unsigned short) readSocket(socket,2);
			}
			else readSocket(socket,6);
		}
		closesocket(socket);
		
		// connect to server and get real adresses
		int nbRead=0;
		for (int ind=0;ind<toRead;ind++)
		{
			nbRead+=getContentServersIps(raddrs[ind],addrs+nbRead,maxi-nbRead,appId,version);
			if (nbRead>=maxi) return nbRead;
		}
		
		
		return nbRead;
	}
}

int getConfigServers(char * host, int port, sockaddr_in * addrs, int maxi)
{
	
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = getHostIp(host);
	clientService.sin_port = htons( port );
	
	// requesting auth servers registry ips
	
	SOCKET socket=connectSocket(clientService);
	if (socket==0) {closesocket(socket); return 0;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return 0;}
	
	sendSocket(socket,"00 00 00 01");
	sendSocket(socket,"03");
	
	
	int howMuch=readSocket(socket,4);
	int nbAddr=readSocket(socket,2);
	
	int toRead=(nbAddr>10 ? 10 : nbAddr);
	
	
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind<toRead)
		{
			addrs[ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&( addrs[ind].sin_addr.s_addr),4,0);
			addrs[ind].sin_port = (unsigned short) readSocket(socket,2);
		}
		else readSocket(socket,6);
	}
	closesocket(socket);
 	
	
	return toRead;
}

int getAuthServers(/*char * host, int port,*/sockaddr_in clientService, sockaddr_in * addrs, int maxi,char * login)
{
	
/*	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = getHostIp(host);
	clientService.sin_port = htons( port );
*/	
	// requesting auth servers registry ips
	
	SOCKET socket=connectSocket(clientService);
	if (socket==0) {closesocket(socket); return 0;}
	sendSocket(socket,"00 00 00 02");
	
	int ack=readSocket(socket,1);
	if (ack!=1) {closesocket(socket); return 0;}
	

	char data[9];
	*(DWORD*)(data)=reverseBytes(0x05,4);
	*(data+0x04)=0x00;
	*(DWORD*)(data+0x05)=jenkinsHash((unsigned char*)login,strlen(login),0);
	sendSafe(socket,data,9,0);
/*
	sendSocket(socket,"00 00 00 05");
	sendSocket(socket,"00");
	
	

	//DWORD loginhash=getUserHash(login);
	DWORD loginhash=
	//printf(" login hash : %u\n",loginhash);
	sendSafe(socket,(char*)&loginhash,4,0);
*/	
	int error=0;
	int howMuch=readSocket(socket,4,&error);
	if (error) return 0;
	int nbAddr=readSocket(socket,2,&error);
	if (error) return 0;	
	int toRead=(nbAddr>10 ? 10 : nbAddr);
	
	
	for (int ind=0;ind<nbAddr;ind++)
	{
		if (ind<toRead)
		{
			addrs[ind].sin_family = AF_INET;
			recvSafe(socket,(char*)&( addrs[ind].sin_addr.s_addr),4,0);
			addrs[ind].sin_port = (unsigned short) readSocket(socket,2);
		}
		else readSocket(socket,6);
	}
	closesocket(socket);
 	
	
	return toRead;
}

int authenticateAndGetTicket(sockaddr_in authServer,char * login, char * password,char * ticket, CProgressCtrl * progress)
{
	SOCKET socket=connectSocket(authServer);
	if (socket==0) {closesocket(socket); return -10;}
	
	sendSocket(socket,"00 00 00 00 04");
	if (progress) progress->SetPos(1);
	DWORD data[2];
	
	DWORD localIp=reverseBytes(getCurrentIp());
	DWORD loginhash=reverseBytes(jenkinsHash((unsigned char*)login,strlen(login),0));
	
	data[0]=localIp;
	data[1]=loginhash;
	
	sendSafe(socket,(char*)data,8,0);
	localIp=reverseBytes(localIp);
	
	
	int ack=readSocket(socket,1);
	if (progress) progress->SetPos(2);
	if (ack) {closesocket(socket); return -10;}
	
	DWORD externalIp=readSocket(socket,4);
	
	int loginLen=strlen(login);
	
	DWORD packetLen=reverseBytes(5+2*loginLen);
	
	WORD loginLenR=(WORD)reverseBytes(loginLen,2);
	
	sendSafe(socket,(char*)&packetLen,4,0);
	sendSocket(socket,"02");
	sendSafe(socket,(char*)&loginLenR,2,0);
	sendSafe(socket,login,loginLen,0);
	sendSafe(socket,(char*)&loginLenR,2,0);
	sendSafe(socket,login,loginLen,0);
	if (progress) progress->SetPos(3);
	char salt[8];
	recvSafe(socket,salt,8,0);
	if (progress) progress->SetPos(4);
	char aesKey1[16];
	getAesKey1(password, (char*)&salt,aesKey1);
	
	char aesKey2[16];
	getAesKey2(localIp, externalIp, aesKey2);
	
	ULONGLONG timestamp=getNanosecondsSinceTime0();
	
	char plain[16];
	memcpy(plain,(char*)&timestamp,8);
	XOR(plain,aesKey2,8);
	memcpy(plain+8,(char*)&localIp,4);
	memset(plain+12,0x04,4);
	
	char iv[40];
	CSHA1 sha1;
	sha1.Reset();
	sha1.Update((UINT_8 *)&timestamp,8);
	sha1.Final();
	sha1.GetHash((UINT_8 *)iv);
	
	char cypher[16];
	
	CRijndael aes;
	aes.MakeKey(aesKey1, iv, 16, 16);
	aes.ResetChain();
	aes.Encrypt(plain,cypher,16,CRijndael::CBC);	
	
	sendSocket(socket,"00 00 00 24");
	sendSafe(socket,iv,16,0);
	sendSocket(socket,"00 0C 00 10"); // decr / encr sizez
	sendSafe(socket,cypher,16,0);
	if (progress) progress->SetPos(5);
	char ok=1;
	if (recvSafe(socket,&ok,1,0)!=1)
	{
 		closesocket(socket);
		return -5; // server closed connection
	}
	
	
	ULONGLONG serverTimestamp=0;
	ULONGLONG clockSkewTolerance=0;
	
	if (recvSafe(socket,(char*)&serverTimestamp,8,0)!=8)
	{
 		closesocket(socket);
		return -5; // server closed connection
	}
	if (recvSafe(socket,(char*)&clockSkewTolerance,8,0)!=8)
	{
 		closesocket(socket);
		return -5; // server closed connection
	}

	if (progress) progress->SetPos(6);
	
	if (ok) {
		closesocket(socket);  
		
		if (ok!=3 && serverTimestamp && clockSkewTolerance)
		{ // not officialy a timestamp problem
			// check timestamps
			ULONGLONG delta=0;

			if (serverTimestamp>timestamp)
				delta=serverTimestamp-timestamp;
			else
				delta=timestamp-serverTimestamp;
		
			if (delta >= clockSkewTolerance)
			{
				ok=3;
			}

			// 1.0.10
			if (serverTimestamp) adjustTime(serverTimestamp);
		}
		return -ok;
	}

	// 1.0.10	
	if (serverTimestamp) adjustTime(serverTimestamp);
	/*
	case 4: 
	//Response: The account is disabled. 
	case 3: 
	//Response: Your clock differs too much from the servers time of UTC. 
	case 2: 
	//Response: The account does not exist, or the password that you entered was not correct. 
	case 1: 
	//Response: The account does not exist. 
	case 0: 
	//Response: Login OK 
	*/

	
	DWORD cryptedLen=readSocket(socket,4);
	if (cryptedLen==0) {
 		closesocket(socket);
		return 0;// 0 means no ticket 
	}
	if (progress) progress->SetPos(7);
	char * accountData=(char*)malloc(cryptedLen);
	
	recvSafe(socket,accountData,cryptedLen,0);
	if (progress) progress->SetPos(8);
	closesocket(socket);
	
	//	debugAccountInformation(password,salt,localIp,externalIp,cryptedTicket,cryptedLen);
	
	char * iv2=(char*)((int)accountData+2);
	char * cypher2=(char*)((int)accountData+2+16+4);
	WORD * ticketLen=(WORD *)((int)cypher2+64);
	int tL=reverseBytes(*ticketLen,2);
	char * ticketData=(char*)((int)ticketLen+2);
	
	memcpy(ticket,ticketData,tL);
	
	free(accountData);
	
	
	return tL;
}
