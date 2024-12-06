
#include "config.h"
#include "cdr.h"
#include "gcf.h"
#include "socketTools.h"
#include "zlib\zlib.h"
#include "direct.h"
#include "Rijndael.h"
#include "CSClient.h"
#include "debug.h"
#include "steamClient.h"
#ifdef CF_TOOLBOX
#include "Bandwidth.h"
#endif

#define GCF_FILE_BLOCK_ZIPPED_AND_CRYPTED	0x0002
#define GCF_FILE_BLOCK_CRYPTED				0x0004

void getCFFileName(char * root, char * gcfName, int type, char * out)
{
	char extension[30];
	
	switch (type)
	{
	case 1 : strcpy(extension,"gcf");break;
	case 2 : strcpy(extension,"ncf");break;
	default : sprintf(extension,"%dcf",type);break;
	}
	
	
	sprintf(out,"%s\\%s.%s",root,gcfName,extension);
	strlwr(out);
}



int createGcfFile(GCF * gcf, char * root, char * gcfName, char * commonName, char *appName, int desiredLevel)
{
	getCFFileName(root,gcfName,gcf->cfType,gcf->_fileName); 
	
	
	gcf->isArchive=0;
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Creating content file",gcf->_fileName,desiredLevel);
	BUFFEREDHANDLE f=fopen_64(gcf->_fileName,"wb");
	
	if (!f) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File creation error",gcf->_fileName,desiredLevel);
		return -1; // can't create file
	}
	int size=serializeGCFHeaders(gcf,0);
	
	char * buffer=(char*)malloc(size);
	
	size=serializeGCFHeaders(gcf,buffer);
	
	fwrite_64(buffer,1,size,f);
	free(buffer);
	fclose_64(f);
	if (gcf->cfType==1)
	{
		// gcf
		if (setFileSize64(gcf->_fileName,gcf->fileSize))
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Can't allocate GCF file space",gcf->fileSize,desiredLevel);
			return -2;
		}
	}
	else
	{
		// ncf, create container folders
		if (!commonName) return -3; 
		return buildNcfCommon(gcf,root, commonName,appName,desiredLevel);
	}
	
	return 0;
}
DWORD findFirstFreeDataBlock(GCF * gcf)
{ // same as get, but doesn't update firstUnusedblock
	
	unsigned int ind=0;
	for (ind=0;ind<gcf->dataBlocks->blockCount;ind++)
	{
		if (!gcf->dataBlocks->blockUsage[ind]) return ind;
	}
	return ind;
}

int updateGcfFileBlock(GCF * gcf,DWORD dataBlockId, char * block,DWORD size, BUFFEREDHANDLE f)
{
	
	
	_int64 realPos=(_int64)gcf->dataBlocks->firstBlockOffset+(_int64)gcf->blockSize*(_int64)dataBlockId;
	
	
	if (!f) return -1; // can't create file
	
	fseek_64(f,realPos,SEEK_SET);
	
	fwrite_64(block,1,size,f);
	
	
	return 0;
}

DWORD getFirstFreeDataBlock(GCF * gcf)
{
	DWORD nb=gcf->dataBlocks->blockCount;
	DWORD res=gcf->fragmentationMap->firstUnusedEntry;
	
	while (res<nb && gcf->dataBlocks->blockUsage[res]) res++;
	
	if (res>=nb) 
	{
		//	printf("wow ! res >=nb\n");
		gcf->fragmentationMap->firstUnusedEntry=0;
		return nb; // dirty, but shouldn't occur
	}
	
	gcf->dataBlocks->blockUsage[res]=1;
	gcf->fragmentationMap->nextDataBlockIndex[res]=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	gcf->dataBlocks->blocksUsed++;
	//printf("nb usedDataBlocks : %d\n",gcf->dataBlocks->blocksUsed);
	
	unsigned int ind=res+1;
	
	while (ind<nb && gcf->dataBlocks->blockUsage[ind]) ind++;
	
	gcf->fragmentationMap->firstUnusedEntry=(ind >= nb ? 0 : ind);
	return res;
}

int isUpdated(GCF * gcf, DWORD fileId, DWORD * updatedFiles, DWORD nbUpdated);
 

int getBanner(SOCKET socket, char * url)
{
	sendSocket(socket,"00 00 00 05 00 00 00 00 00");
	int ack=readSocket(socket,1);
	if (!ack) { return -1;}
	
	int len= readSocket(socket,2) ;
	recvSafe(socket,url,len,0);
	url[len]=0;
	return len;
}

DWORD openAppAndGetDirectoryChecksum(SOCKET socket,DWORD * connectionId, DWORD * messageId, int app,int version,char * ticket,int ticketLen)
{
	int len=reverseBytes(0x11+ticketLen);
	sendSafe(socket,(char*)&len,4,0);
	
	sendSocket(socket,"0A");
	
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	app=reverseBytes(app);
	version=reverseBytes(version);
	
	sendSafe(socket,(char*)&app,4,0);
	sendSafe(socket,(char*)&version,4,0);
	
	/* // deprecated steam2ticket
	int ticketLenR=reverseBytes(ticketLen,2);
	sendSafe(socket,(char*)&ticketLenR,2,0);
	sendSafe(socket,ticket,ticketLen,0);
	*/

	sendSafe(socket,ticket,ticketLen,0); // steam2ticket + cmsession + appOwnershipTicket given by cmclient


	if (recvSafe(socket,(char*)connectionId,4,0)<=0) return 0;
	if (recvSafe(socket,(char*)messageId,4,0)<=0) return 0;
	
	int ack=readSocket(socket,1);
	
	if (ack) { return 0;}
	
	if (recvSafe(socket,(char*)connectionId,4,0)<=0) return 0;
	
	DWORD directoryChecksum=0;
	if (recvSafe(socket,(char*)&directoryChecksum,4,0)<=0) return 0;
	(*messageId)++;
	return directoryChecksum;
	
}

DWORD readContentServerData(SOCKET socket,DWORD * connectionId, DWORD * messageId, char * data, DWORD totalDataLength)
{
	DWORD totalRead=0;
	
	while (totalRead<totalDataLength)
	{
		recvSafe(socket,(char*)connectionId,4,0);
		recvSafe(socket,(char*)messageId,4,0);
		
		DWORD dataLength=readSocket(socket,4);
		
		/*DWORD read=0;
		while (read<dataLength)
		{
			DWORD r=recvSafe(socket,((data)+totalRead+read),dataLength-read,0);
			if (r<=0) return r;
			read+=r;
		}*/
		int read=recvSafe(socket,data+totalRead,dataLength,0);
		if (read<=0) return read;

		totalRead+=dataLength;
	}
	return totalRead;
}

DWORD getDirectory(SOCKET socket, DWORD * connectionId, DWORD * messageId, char ** directory)
{
	sendSocket(socket,"00 00 00 09 04");
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	int ack=readSocket(socket,1);
	if (ack) {return 0;}
	DWORD totalDataLength=readSocket(socket,4);
	
	*directory=(char*)malloc(totalDataLength);
	
	totalDataLength=readContentServerData(socket,connectionId,messageId,*directory,totalDataLength);
	if (totalDataLength<0) totalDataLength=0;
	
	(*messageId)++;
	return totalDataLength;
}

DWORD getUpdatedFiles(SOCKET socket, DWORD * connectionId, DWORD * messageId, DWORD sinceVersion, DWORD ** updated)
{
	sinceVersion=reverseBytes(sinceVersion);
	
	sendSocket(socket,"00 00 00 0D 05");
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	sendSafe(socket,(char*)&sinceVersion,4,0);
	
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	int ack=readSocket(socket,1);
	
	if (ack==1) return 0; // no file updated update
	if (ack!=2) return -1; // error
	
	DWORD nbDifferences=readSocket(socket,4);
	
	DWORD totalDataLength=nbDifferences*4;
	*updated=(DWORD*)malloc(totalDataLength);
	
	totalDataLength=readContentServerData(socket,connectionId,messageId,(char*)*updated,totalDataLength);
	if (totalDataLength>=0) nbDifferences=totalDataLength/4;
	else	
		nbDifferences=totalDataLength;
	
	(*messageId)++;
	return nbDifferences;
}


DWORD getChecksums(SOCKET socket, DWORD * connectionId, DWORD * messageId, char ** checksums)
{
	sendSocket(socket,"00 00 00 09 06");
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	int ack=readSocket(socket,1);
	if (ack) {  return 0;}
	
	DWORD totalDataLength=readSocket(socket,4);
	*checksums=(char*)malloc(totalDataLength);
	
	totalDataLength=readContentServerData(socket,connectionId,messageId,*checksums,totalDataLength);
	if (totalDataLength<0) totalDataLength=0;
	
	(*messageId)++;
	return totalDataLength;
}

int closeApp(SOCKET socket,DWORD * connectionId,DWORD * messageId)
{
	sendSocket(socket,"00 00 00 09 03");
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	int ack=readSocket(socket,1);
	if (ack) {  return 0;}
	sendSocket(socket,"00 00 00 01 01");
	(*messageId)++;
	return 1;
}



int readFileChunk(DWORD fileId,bool showRealBandwidth,bool validate,SOCKET socket,DWORD * connectionId,DWORD * messageId,int chunk, char * data, int mode, DWORD checksum,DWORD toRead, char * appName, int desiredLevel)
{
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	DWORD totalDataLength=readSocket(socket,4);
	// printf("size : %d ",totalDataLength); 
	
	char * temp=(char*)malloc(totalDataLength);
	
	totalDataLength=readContentServerData(socket,connectionId,messageId,temp,totalDataLength);
	
	if (totalDataLength<=0) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"No data retrieved / connection lost",desiredLevel);
		totalDataLength=0; 
	}
	else
	{
#ifdef CF_TOOLBOX
		if (showRealBandwidth) notifyBandwidthMonitor(fileId, totalDataLength);	//zipped size
#endif
		//	hexaPrintf(temp,totalDataLength);
		switch (mode)
		{
		case 0 :	// raw data 
			memcpy(data,temp,totalDataLength);break;
		case 1 : {
			// zipped data
			DWORD realLength=4*0x2000;
		//	printDebug(DEBUG_LEVEL_TODO,"csclient","recieved",temp,totalDataLength);
			int result=uncompress((unsigned char*)data,&realLength,(unsigned char*)temp,totalDataLength);
			if (result) {
				printDebug(DEBUG_LEVEL_ERROR,appName,"Zlib error",result,desiredLevel);
				free(temp);
				return 0;
			} // unzip error
			totalDataLength=realLength;
			break;
				 }
		case 2 : {
			// crypted zipped data. format: DWORD len, DWORD unzipped len, crypted data, 00 padding
			DWORD cryptLength=*(DWORD*)temp;
			DWORD realLength=*(DWORD*)(temp+4);
			
			
			memset(data,0,realLength);
			memcpy(data,temp,totalDataLength);
			totalDataLength=realLength;
			
			break;
				 }
		case 3 : {
			// crypted data. 
			memcpy(data,temp,totalDataLength);break;
				 }
		}
	 	if (validate && mode<2)
		{
			DWORD computedChecksum= computeGcfChunkChecksum(data,totalDataLength);
			
			if (computedChecksum!=checksum) 
			{
				printDebug(DEBUG_LEVEL_ERROR,appName,"Checksum error",desiredLevel);
				totalDataLength=0; // error
			}
		}
		
	}
	
	free(temp);
#ifdef CF_TOOLBOX
	if (!showRealBandwidth) notifyBandwidthMonitor(fileId, totalDataLength); // unzipped size
#endif
	return totalDataLength;
}

unsigned int getFileChunk(DWORD fileId,bool showRealBandwidth,bool validate,SOCKET socket,DWORD * connectionId,DWORD * messageId, int checksumIndex, int chunkStart, int nbChuncksRequested, char * data, DWORD * checksums,DWORD toRead,char * appName, int desiredLevel,DWORD * transferMode=0)
{
	
	sendSocket(socket,"00 00 00 16 07");
	sendSafe(socket,(char*)connectionId,4,0);
	sendSafe(socket,(char*)messageId,4,0);
	
	int unknown=2; // priority
	checksumIndex=reverseBytes(checksumIndex,4);
	chunkStart=reverseBytes(chunkStart,4);
	nbChuncksRequested=reverseBytes(nbChuncksRequested,4);
	
	sendSafe(socket,(char*)&checksumIndex,4,0);
	sendSafe(socket,(char*)&chunkStart,4,0);
	sendSafe(socket,(char*)&nbChuncksRequested,4,0);
	sendSafe(socket,(char*)&unknown,1,0);	
	
	recvSafe(socket,(char*)connectionId,4,0);
	recvSafe(socket,(char*)messageId,4,0);
	
	int ack=readSocket(socket,1);
	if (ack) {  
		printDebug(DEBUG_LEVEL_ERROR,appName,"Download error",desiredLevel);
		return 0;
	}
	//printf("[ack=%d]",ack);
	int nbChunksSent=0;
	int mode=0;
	nbChunksSent=readSocket(socket,4);
	mode=readSocket(socket,4);
	
	if (transferMode) *transferMode=mode;
	
	if (mode<0 || mode>3) 
	{// not supported mode ?
		printDebug(DEBUG_LEVEL_TODO,appName,"Unknown file transfert mode",mode,desiredLevel);
		printf("unknown mode : %d\n",mode);
		return 0;
	} 
	
	//printf(" reception de %d chunks   mode : %d\n",nbChunksSent,mode);
	
	for (int ind=0;ind<nbChunksSent;ind++)
	{
		DWORD len=readFileChunk(fileId,showRealBandwidth,validate,socket,connectionId,messageId,chunkStart+ind, data+(ind*4*0x2000),mode,checksums[ind],toRead,appName,desiredLevel);
	
		toRead-=len;
		if (!len) {
			return 0;
		} // error 
		//printf(" read chunk %d of %d bytes\n",ind,len);
	}
	
	(*messageId)++;
	return nbChunksSent;
}



void setBlockMode(GCF * gcf, DWORD blockId,DWORD transferMode)
{
	if (gcf->blocks->block[blockId].entryType&(1|2|4)) return; // already setted
	
	switch (transferMode)
	{
	case 0 :
	case 1: gcf->blocks->block[blockId].entryType |= 0x0001;break;
	case 2: gcf->blocks->block[blockId].entryType |= 0x0002;break; // zipped & crypted
	case 3: gcf->blocks->block[blockId].entryType |= 0x0004;break; // crypted
	}
}

DWORD updateGcfDataBlocks(GCF * gcf,DWORD blockIndex,DWORD currentDataBlock,DWORD nbDataBlocks,BUFFEREDHANDLE f, DWORD size,char * data, int securedUpdates)
{
	char * dataBlock=data;
	//	FILE * f=fopen(gcf->fileName,"r+b");
	
	for (unsigned int ind=0;ind<nbDataBlocks;ind++)
	{
		if (gcf->blocks->block[blockIndex].fileDataSize==0)
		{
			currentDataBlock=getFirstFreeDataBlock(gcf);
			gcf->blocks->block[blockIndex].firstDataBlockIndex=currentDataBlock;
			//printf("registring first data block %d of block\n",currentDataBlock);
		}
		else
		{
			gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock]=getFirstFreeDataBlock(gcf);
			//			printf(" setting frag map : %d -> %d\n",currentDataBlock,gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock]);
			currentDataBlock=gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock];
		}
		
		DWORD dataSize=(size>gcf->blockSize ? gcf->blockSize : size);
		
		
		updateGcfFileBlock(gcf,currentDataBlock,data, dataSize,f);
		
		gcf->blocks->block[blockIndex].fileDataSize+=dataSize; 
		//		printf("  block size : %d\n",gcf->blocks->block[blockIndex].fileDataSize);
		size-=dataSize;
		data+=gcf->blockSize;
	}
	//	fclose_64(f);
	if (securedUpdates) updateGcfFileHeaders(gcf,f); 
	
	return currentDataBlock;
}

int downloadGcfFile(SOCKET socket,DWORD * connectionId,DWORD * messageId,GCF * gcf,GCFDirectoryEntry entry, DWORD blockIndex,BUFFEREDHANDLE f, char * appName, int desiredLevel,DWORD * taskProgress, DWORD * remainingTime,time_t startTime,DWORD startDownloaded,int * pleaseStop,int securedUpdates,bool showRealBandwidth, bool validate)
{
/*
	// cft 1.0.7b ignore filesize=0
	if (!entry.itemSize) return 0;
*/
	DWORD len=16*4*0x2000;
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	char * data=(char*) malloc(len);
	char dbg[1000];
	if (entry.directoryType&ENCRYPTED_FLAG) 
		sprintf(dbg,"Pre-loading %s (size:%d)",gcf->directory->directoryNames+entry.nameOffset,entry.itemSize);
	else
		sprintf(dbg,"Downloading %s (size:%d)",gcf->directory->directoryNames+entry.nameOffset,entry.itemSize);
	
	printDebug(DEBUG_LEVEL_DEBUG,appName,dbg,desiredLevel);
	
	
	GCFBlock block=gcf->blocks->block[blockIndex];
	DWORD currentDataBlock=block.firstDataBlockIndex;
	if (currentDataBlock!=terminator &&  currentDataBlock<gcf->dataBlocks->blockCount)
	{
		while (gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock]!=(gcf->fragmentationMap->terminator ? 0xFFFFFFFF : 0xFFFF) &&
		       gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock]<gcf->dataBlocks->blockCount) 
			currentDataBlock=gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock];
		//sprintf(path,"d:\\cg\\%s",gcf->directory->directoryNames+entry.nameOffset);
	}
	
	DWORD currentSize=block.fileDataOffset+block.fileDataSize;
	
	//printf("currentSize : %d   supposed : %d\n",currentSize,entry.itemSize);
	if (currentSize>=entry.itemSize) 
	{
		//	printf("was finished\n");
		// download was finished
		return 0;
	}
	
	DWORD chunkSize=gcf->blockSize*4;
	
	DWORD nbDownloadedBlocks=currentSize/chunkSize;
	
	DWORD rest=currentSize%gcf->blockSize;
	if (rest) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Half-downloaded file size error",desiredLevel);
		//printf("file not full downloaded and downloaded size not a multiple of block size!\n"); 
		return -1;
	}
	
	DWORD toRead=entry.itemSize-nbDownloadedBlocks*gcf->blockSize*4;
	GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
	//printf("\n");
	DWORD transferMode=0;
	while (nbDownloadedBlocks<checksumEntry.checksumCount && !(*pleaseStop))
	{
		int nbChunks=(checksumEntry.checksumCount-nbDownloadedBlocks >16 ? 16 : checksumEntry.checksumCount-nbDownloadedBlocks);
		
		//printf("  downloaded chunks : %d / %d\n",nbDownloadedBlocks,checksumEntry.checksumCount);
//		printBar(nbDownloadedBlocks,checksumEntry.checksumCount);
//		enterSection();
		unsigned int done=getFileChunk(gcf->appId, showRealBandwidth,validate,socket,connectionId,messageId,entry.checksumIndex,nbDownloadedBlocks,nbChunks,data,gcf->checksums->checksum+(checksumEntry.firstChecksumIndex+nbDownloadedBlocks),toRead,appName,desiredLevel,&transferMode);
	//	exitSection();
		if (done)
		{
			DWORD readSize=(toRead < done*4*0x2000 ? toRead : done*4*0x2000);
			toRead-=readSize; 
			//	printf("done: %d    readSize:%d   toRead:%d\n",done,readSize,toRead);
			
			int nbDataBlocks=readSize/0x2000+ (readSize%0x2000 ? 1 : 0) ;
			setBlockMode(gcf,blockIndex,transferMode);
			currentDataBlock=updateGcfDataBlocks(gcf,blockIndex,currentDataBlock,nbDataBlocks,f,readSize,data,securedUpdates);
		}
		// SAVE the data blocks, update fragmentation map and maybe block first data block
		//fwrite_64(data,1,readSize,f);
		
		if (!done) {
			printDebug(DEBUG_LEVEL_ERROR,appName,"Download error",desiredLevel);
			free(data); 
			return -1;
		}
		if (taskProgress)
		{
			*taskProgress=(100*gcf->dataBlocks->blocksUsed)/gcf->dataBlocks->blockCount;
		}
		if (remainingTime)
		{
			time_t now;
			time(&now);
			now-=startTime;
			if (now)
			{
				*remainingTime=(gcf->dataBlocks->blockCount-gcf->dataBlocks->blocksUsed)*now/(gcf->dataBlocks->blocksUsed-startDownloaded);
			}
		}

		//	printf(" read %d chunks\n",readed);
		nbDownloadedBlocks+=done;
	}
	
//	printBar(checksumEntry.checksumCount,checksumEntry.checksumCount);
	//	printf("\r\n");
	
	free(data);
	
	return 0;
}


int downloadNcfFile(SOCKET socket,DWORD * connectionId,DWORD * messageId,GCF * gcf,GCFDirectoryEntry entry, char * path,ULONGLONG downloadedsize,ULONGLONG totalSize, char * appName, int desiredLevel,DWORD * taskProgress, DWORD*remainingTime,time_t startTime,ULONGLONG startSize,int * pleaseStop,bool showRealBandwidth, bool validate)
{
	
	DWORD len=16*4*0x2000;
	char * data=(char*) malloc(len);
	char dbg[1000];
	char * fName=gcf->directory->directoryNames+entry.nameOffset;
	if (entry.directoryType&ENCRYPTED_FLAG) 
		sprintf(dbg,"Pre-loading %s (size:%d)",fName,entry.itemSize);
	else
		sprintf(dbg,"Downloading %s (size:%d)",fName,entry.itemSize);
	
	printDebug(DEBUG_LEVEL_DEBUG,appName,dbg,desiredLevel);
	
	BUFFEREDHANDLE f=fopen_64(path,"r+b");
	
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",path,desiredLevel);
		return -1;
	}
	
	fseek_64(f,0,SEEK_END);
	
	DWORD currentSize=ftell_64(f);
	
	if (currentSize==entry.itemSize) 
	{
		fclose_64(f);
		// download was finished
		return 0;
	}
	
	DWORD chunkSize=GCF_BLOCK_SIZE*4;
	
	DWORD nbDownloadedBlocks=currentSize/chunkSize;
	
	DWORD rest=currentSize%GCF_BLOCK_SIZE;
	
	// still data to be downloaded, but size is not a multiple of block size => some "garbage" maybe caused by disk full error
	fseek_64(f,rest,SEEK_END); // rewinding the resting bytes
	
	DWORD toRead=entry.itemSize-nbDownloadedBlocks*GCF_BLOCK_SIZE*4;
	GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
	while (nbDownloadedBlocks<checksumEntry.checksumCount && !(*pleaseStop))
	{
		int nbChunks=(checksumEntry.checksumCount-nbDownloadedBlocks >16 ? 16 : checksumEntry.checksumCount-nbDownloadedBlocks);
		
		//printf("  downloaded chunks : %d / %d\n",nbDownloadedBlocks,checksumEntry.checksumCount);
//		printBar(nbDownloadedBlocks,checksumEntry.checksumCount);
	//	enterSection();
		unsigned int done=getFileChunk(gcf->appId, showRealBandwidth,validate,socket,connectionId,messageId,entry.checksumIndex,nbDownloadedBlocks,nbChunks,data,gcf->checksums->checksum+(checksumEntry.firstChecksumIndex+nbDownloadedBlocks),toRead,appName,desiredLevel);
	//	exitSection();
		DWORD readSize=(toRead < done*4*0x2000 ? toRead : done*4*0x2000);
		toRead-=readSize; 
		//	printf("done: %d    readSize:%d   toRead:%d\n",done,readSize,toRead);
		
		fwrite_64(data,1,readSize,f);
		
		if (!done) {
			printDebug(DEBUG_LEVEL_ERROR,appName,"Download error",desiredLevel);
			free(data); 
			fclose_64(f); 
			setFileSize64(path,0);
			return -1;
		}
		downloadedsize+=readSize;
		if (taskProgress)
		{
 			*taskProgress=(downloadedsize>totalSize ? 100 : (100*downloadedsize)/totalSize);
		}
		if (remainingTime)
		{
			time_t now;
			time(&now);
			now-=startTime;
			if (now)
			{
				*remainingTime=(totalSize-downloadedsize)*now/(downloadedsize-startSize);
			}
		}
		//	printf(" read %d chunks\n",readed);
		nbDownloadedBlocks+=done;
	}
	
	
//	printBar(checksumEntry.checksumCount,checksumEntry.checksumCount);
	//	printf("\r\n");
	free(data);
	fclose_64(f);
	return 0;
}

DWORD createBlock(GCF * gcf, DWORD fileId)
{
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	DWORD start=0;//(gcf->blocks->lastUsedBlock==gcf->blocks->blockCount ? 0 : gcf->blocks->lastUsedBlock);
	for (DWORD ind=start;ind<gcf->blocks->blockCount;ind++)
	{
		if (!(gcf->blocks->block[ind].entryType & 0x8000)) //// WAS & 0xFFFF before but can be remaining bits
		{
			// first free block;
			gcf->blocks->block[ind].entryType=0x8000 ; // used bloc, no compression or crypto
			
			if (isToBeExtracted(gcf,fileId)) gcf->blocks->block[ind].entryType+=0x4000; // extracted
			gcf->blocks->block[ind].fileDataOffset=0;
			gcf->blocks->block[ind].fileDataSize=0;
			gcf->blocks->block[ind].firstDataBlockIndex=terminator;
			gcf->blocks->block[ind].nextBlockEntryIndex= gcf->blocks->blockCount;
			gcf->blocks->block[ind].previousBlockEntryIndex= gcf->blocks->blockCount;
			gcf->blocks->block[ind].directoryIndex=fileId;
			//printf("Created block at %d\n",ind);
			gcf->blocks->blockUsed++;
			gcf->blocks->lastUsedBlock=ind;
			return ind;
		}
	}
	return gcf->blocks->blockCount;
}

int downloadGcfFiles(GCF * gcf, SOCKET socket,DWORD * connectionId,DWORD * messageId, char * appName, int desiredLevel, char * taskName, DWORD * taskProgress,DWORD * remainingTime, int * pleaseStop,int securedUpdates,bool showRealBandwidth, bool validate)
{
	BUFFEREDHANDLE f=fopen_64(gcf->_fileName,"r+b");
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",gcf->_fileName,desiredLevel);
		return -1;
	}
	buildDataBlockUsageMap(gcf,0,0); // 0,0 => no updated files now (was processed before)
	gcf->fragmentationMap->firstUnusedEntry=0;
	DWORD startDownloaded=gcf->dataBlocks->blocksUsed;
	time_t startTime;
	time(&startTime);

//	startExecAnalyse();

	for (unsigned int ind=0;!(*pleaseStop) && ind<gcf->directory->itemCount;ind++)
	{
		GCFDirectoryEntry entry=gcf->directory->entry[ind];
		
			if (entry.itemSize && taskName && taskProgress)
			{
				sprintf(taskName,"Downloading : %s",gcf->directory->directoryNames+entry.nameOffset);
			}


		if (entry.directoryType)
		{
			//printDebug(DEBUG_LEVEL_TODO,appName,">> ",taskName,desiredLevel);
			// 1.0.7b ignore size==0
			if (!entry.itemSize) 
			{
				updateGcfFileHeaders(gcf,f);
				gcf->directoryMap->firstBlockIndex[ind]=gcf->blockCount;
				//fclose_64(f);
				continue;			
			}

			DWORD blockIndex=gcf->blockCount;
			if (gcf->directoryMap->firstBlockIndex[ind]==gcf->blockCount)
			{
				// no block yet
				blockIndex=createBlock(gcf,ind);
				gcf->directoryMap->firstBlockIndex[ind]=blockIndex;
				if (securedUpdates) updateGcfFileHeaders(gcf,f);
			}
			else
				blockIndex=gcf->directoryMap->firstBlockIndex[ind];
			
			//GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
			DWORD downloadedSize=0;
			int anotherBlock=0;
			do 
			{
				anotherBlock=0;
				downloadedSize+=gcf->blocks->block[blockIndex].fileDataSize;
				if (gcf->blocks->block[blockIndex].nextBlockEntryIndex!=gcf->blocks->blockCount)
				{
					blockIndex=gcf->blocks->block[blockIndex].nextBlockEntryIndex;
					anotherBlock=1;
				}
			} while (anotherBlock || gcf->blocks->block[blockIndex].nextBlockEntryIndex!=gcf->blocks->blockCount);
			
			
			if (downloadedSize<entry.itemSize)
			{
				//	printf("downloaded : %d   supposed : %d\n",downloadedSize,entry.itemSize);
				// not full downloaded
				if (downloadGcfFile(socket,connectionId,messageId,gcf,entry, blockIndex,f,appName,desiredLevel,taskProgress,remainingTime,startTime,startDownloaded,pleaseStop,securedUpdates,showRealBandwidth,validate)<0)
				{
					updateGcfFileHeaders(gcf,f);
					fclose_64(f);
					return -1;
				}
			}
			
		}
		else
		{
			// a folder
			
			gcf->directoryMap->firstBlockIndex[ind]=gcf->blockCount; // no content
			//updateGcfFileHeaders(gcf); // let's make it at the end
		}
		
	}

//	stopExecAnalyse();

	updateGcfFileHeaders(gcf,f);
	fclose_64(f);
	return 0;
}

DWORD FileSize(const char * path);

int downloadNcfFiles(GCF * gcf , char * root, SOCKET socket,DWORD * connectionId,DWORD * messageId, char * appName, int desiredLevel , char * taskName, DWORD * taskProgress,DWORD * remainingTime, int * pleaseStop,int securedUpdates,bool showRealBandwidth, bool validate)
{
	ULONGLONG downloadedSize=getNCFDownloadedSize(gcf,root ,false);
	ULONGLONG totalSize=getGCFRealSize(gcf);
	ULONGLONG startSize=downloadedSize;
	time_t startTime;
	time(&startTime);
	char path[2001];
//	_getcwd(cwd,1000);
	strcpy(path,root);
	char * fpath=path+strlen(path);
	//_chdir(root); 
	BUFFEREDHANDLE f=fopen_64(gcf->_fileName,"r+b");
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",gcf->_fileName,desiredLevel);
		return -1;
	}
//	startExecAnalyse();
	for (unsigned int ind=0;!(*pleaseStop) && ind<gcf->directory->itemCount;ind++)
	{
		int flag=gcf->directoryMap->firstBlockIndex[ind]; // file status flag;
		if (flag==1 || flag==2) // not downloaded || downloading
		{
			GCFDirectoryEntry entry=gcf->directory->entry[ind];
			if (taskName && taskProgress)
			{
				sprintf(taskName,"Downloading : %s",gcf->directory->directoryNames+entry.nameOffset);
			}
			if (entry.directoryType)
			{
				// file
				gcf->directoryMap->firstBlockIndex[ind]=2; // downloading
				if (securedUpdates) updateGcfFileHeaders(gcf,f); 
				
				//GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
				//fpath[0]='.';
				getFilePath(gcf,ind,fpath);
				ULONGLONG sizeBefore=FileSize(path);
			
				if (sizeBefore>(ULONGLONG)entry.itemSize) sizeBefore=(ULONGLONG)entry.itemSize;
				if (downloadNcfFile(socket,connectionId,messageId,gcf,entry, path,downloadedSize/*+sizeBefore*/,totalSize,appName,desiredLevel,taskProgress,remainingTime,startTime,startSize, pleaseStop,showRealBandwidth,validate)<0)
				{
					updateGcfFileHeaders(gcf,f);
					fclose_64(f);
					return -1;
				}
				downloadedSize-=sizeBefore;
				downloadedSize+=(ULONGLONG)entry.itemSize;
			 
				if (!*pleaseStop) gcf->directoryMap->firstBlockIndex[ind]=3; // downloaded
				if (securedUpdates) updateGcfFileHeaders(gcf,f); 
			}
			else
			{
				// a folder
				gcf->directoryMap->firstBlockIndex[ind]=0; // no content
				//				updateGcfFileHeaders(gcf); // not needed
			}
		}
	}
//	stopExecAnalyse();
	updateGcfFileHeaders(gcf,f); 
	fclose_64(f);
//	_chdir(cwd);
	return 0;
}

int downloadAppFiles(char * root,char * gcfName,char * commonName, SOCKET socket,DWORD * connectionId,DWORD * messageId, char * appName, int desiredLevel, char * taskName, DWORD * taskProgress,DWORD * remainingTime, int * pleaseStop,int securedUpdates,bool showRealBandwidth,bool validate)
{
 
	GCF * gcf=parseGCF(gcfName);
	if ((int)gcf<=1) return 0;
	switch (gcf->cfType)
	{
	case 1 : return downloadGcfFiles(gcf,socket,connectionId,messageId,appName,desiredLevel,taskName,taskProgress,remainingTime,pleaseStop,securedUpdates,showRealBandwidth,validate);
	case 2 : {
		char extPath[1000];
		sprintf(extPath,"%s\\common\\%s",root,commonName);
		return downloadNcfFiles(gcf,extPath,socket,connectionId,messageId,appName,desiredLevel,taskName,taskProgress,remainingTime,pleaseStop,securedUpdates,showRealBandwidth,validate);
			 }
	}
 
	return 0;
}


DWORD getUpdatedFileId(DWORD id, GCF * original, GCF * updated)
{
	if (original->directory->entry[id].parentIndex!=-1)
	{
		DWORD updatedRoot=getUpdatedFileId(original->directory->entry[id].parentIndex,original,updated);		
		if (updatedRoot==-1) return -1;
		DWORD possibleIndex=updated->directory->entry[updatedRoot].firstIndex;
		char * name=original->directory->directoryNames+original->directory->entry[id].nameOffset;
		DWORD directoryType=original->directory->entry[id].directoryType;
		while (possibleIndex)
		{
			DWORD updatedDirectoryType=updated->directory->entry[possibleIndex].directoryType;
			int sameDirectoryType=(directoryType==0 && updatedDirectoryType==0) || (directoryType!=0 && updatedDirectoryType!=0);
			if (!strcmp(name,updated->directory->directoryNames+updated->directory->entry[possibleIndex].nameOffset) && sameDirectoryType) return possibleIndex;
			possibleIndex=updated->directory->entry[possibleIndex].nextIndex;
		}
		
		return -1;
	}
	else
		return 0; // should be the root always...
}

DWORD recopyGcfFragmentationMap(GCF * from,GCF * to,DWORD dataBlockId)
{
	//	printf(" has frag map, recopying frag map \n");
	DWORD fromTerminator=(!from->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	DWORD toTerminator=(!to->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	
	if (dataBlockId==fromTerminator) return toTerminator;
	if (dataBlockId==from->dataBlocks->blockCount) return to->dataBlocks->blockCount;
	DWORD dataBlock=dataBlockId;
	
	while (dataBlock!=toTerminator && dataBlock<from->dataBlocks->blockCount)
	{
		DWORD next=from->fragmentationMap->nextDataBlockIndex[dataBlock];
		if (next==fromTerminator) next=toTerminator;
		if (next==from->dataBlocks->blockCount) next=to->dataBlocks->blockCount;
		to->fragmentationMap->nextDataBlockIndex[dataBlock]=next;
		dataBlock=next;
	}
	return dataBlockId;
}

DWORD recopyGcfBlock(GCF * from,GCF * to,DWORD blockId,DWORD updatedFileId,DWORD previousBlock/*gcf->blocks->blockCount*/)
{
	if (blockId==from->blocks->blockCount) return to->blocks->blockCount;
	
	DWORD updatedBlockId=createBlock(to,updatedFileId);
	
	to->blocks->block[updatedBlockId].entryType=from->blocks->block[blockId].entryType;
	to->blocks->block[updatedBlockId].fileDataOffset=from->blocks->block[blockId].fileDataOffset;
	to->blocks->block[updatedBlockId].fileDataSize=from->blocks->block[blockId].fileDataSize;
	if (to->blocks->block[updatedBlockId].fileDataSize)
		to->blocks->block[updatedBlockId].firstDataBlockIndex=recopyGcfFragmentationMap(from,to,from->blocks->block[blockId].firstDataBlockIndex);
	else
		to->blocks->block[updatedBlockId].firstDataBlockIndex=(!to->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	to->blocks->block[updatedBlockId].nextBlockEntryIndex=recopyGcfBlock(from,to,from->blocks->block[blockId].nextBlockEntryIndex,updatedFileId,updatedBlockId);
	to->blocks->block[updatedBlockId].previousBlockEntryIndex= previousBlock;
	
	return updatedBlockId;
}

int copyBlock(GCF * gcf,BUFFEREDHANDLE f, DWORD from, DWORD to, char * buffer, char * appName, int desiredLevel)
{
	
	_int64 from2=(_int64)gcf->dataBlocks->firstBlockOffset+(_int64)from*(_int64)gcf->dataBlocks->blockSize;
	_int64 to2=(_int64)gcf->dataBlocks->firstBlockOffset+(_int64)to*(_int64)gcf->dataBlocks->blockSize;
	
	
				fseek_64(f,from2,SEEK_SET);
				// read data
				DWORD read=0;
				while (read<gcf->dataBlocks->blockSize) 
				{
					DWORD r=fread_64(buffer+read,1,gcf->dataBlocks->blockSize-read,f);
					if (r<=0) {printDebug(DEBUG_LEVEL_ERROR,appName,"File read error",desiredLevel);return -1;}
					read+=r;
				}
				fflush_64(f);
				
				fseek_64(f,to2,SEEK_SET);
				DWORD written=0;fflush_64(f);
				while (written<gcf->dataBlocks->blockSize) 
				{
					DWORD w=fwrite_64(buffer+written,1,gcf->dataBlocks->blockSize-written,f);
					if (w<=0) {printDebug(DEBUG_LEVEL_ERROR,appName,"File write error",desiredLevel);return -1;}
					written+=w;
				}
				fflush_64(f);
				
				
				return 1;
}

DWORD collapseGaps(GCF * localVersion,DWORD * updatedFiles,DWORD nbUpdated,DWORD targetBlockCount/*, SOCKET socket, time_t * lastSentContentServerPacket*/, char * appName, int desiredLevel)
{
	if (!updatedFiles) return localVersion->dataBlocks->blockCount;
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Disk usage optimisation",desiredLevel);
	BUFFEREDHANDLE f=fopen_64(localVersion->_fileName,"r+b");
	//DWORD nbMoved=0;
	if (!f){printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",localVersion->_fileName,desiredLevel);return 0;}
	
	char * buffer;
	buffer=(char*)malloc(localVersion->dataBlocks->blockSize);
	
	DWORD terminator=(!localVersion->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	
	
	buildDataBlockUsageMap(localVersion,updatedFiles,nbUpdated);
	//printf("BEFORE :\n");
	DWORD nbUsedBlocksForUpdate=0;
	for (unsigned int ind=0;ind<localVersion->dataBlocks->blockCount;ind++)
	{
		if (localVersion->dataBlocks->blockUsage[ind]) nbUsedBlocksForUpdate++;
		//	printf("%d;",localVersion->dataBlocks->blockUsage[ind]);
	}
	//	printf("\nused blocks:%d\n",nbUsedBlocksForUpdate);	
	DWORD firstFree=findFirstFreeDataBlock(localVersion);
	
	if (firstFree==localVersion->dataBlocks->blockCount) {printDebug(DEBUG_LEVEL_DEBUG,appName,"No more free blocks",desiredLevel); fclose_64(f);return nbUsedBlocksForUpdate;}
	
	for (unsigned int localFileId=0;localFileId<localVersion->directory->itemCount;localFileId++)
	{
		//	printf("\r done %d/%d.",localFileId,localVersion->directory->itemCount);
	//	printBar(localFileId,localVersion->directory->itemCount);
		if (!isUpdated(localVersion,localFileId,updatedFiles,nbUpdated))
		{// and is not updated
			
			DWORD blockId=localVersion->directoryMap->firstBlockIndex[localFileId];
			while (blockId!=localVersion->blocks->blockCount)
			{
				DWORD dataBlockId=localVersion->blocks->block[blockId].firstDataBlockIndex;
				int isFirst=1;	
				while (dataBlockId!=terminator && dataBlockId<localVersion->dataBlocks->blockCount)
				{
					if (dataBlockId>=nbUsedBlocksForUpdate)
					{
						copyBlock(localVersion,f,dataBlockId,firstFree,buffer,appName,desiredLevel);
						
						//printf(" copy %d -> %d ",dataBlockId,firstFree);
						localVersion->dataBlocks->blockUsage[firstFree]=2;
						localVersion->dataBlocks->blockUsage[dataBlockId]=0;
						// correct datamaps and first blockIndex
						
						if (isFirst)
						{
							localVersion->blocks->block[blockId].firstDataBlockIndex=firstFree;
							localVersion->fragmentationMap->nextDataBlockIndex[firstFree]=localVersion->fragmentationMap->nextDataBlockIndex[dataBlockId];
							localVersion->fragmentationMap->nextDataBlockIndex[dataBlockId]=terminator;
						}
						else
						{
							for (unsigned int ind=0;ind<localVersion->dataBlocks->blockCount;ind++)
							{
								if (localVersion->fragmentationMap->nextDataBlockIndex[ind]==dataBlockId)
									localVersion->fragmentationMap->nextDataBlockIndex[ind]=firstFree;
							}
							localVersion->fragmentationMap->nextDataBlockIndex[firstFree]=localVersion->fragmentationMap->nextDataBlockIndex[dataBlockId];
							localVersion->fragmentationMap->nextDataBlockIndex[dataBlockId]=terminator;
						}
						dataBlockId=firstFree;
						firstFree=findFirstFreeDataBlock(localVersion);
						if (firstFree==localVersion->dataBlocks->blockCount) 
						{
							printf("no more free blocks"); 
							// fixed in 1.0.9 , 							
							free(buffer);
							fclose_64(f);
							return nbUsedBlocksForUpdate;
						}
					}
					dataBlockId=localVersion->fragmentationMap->nextDataBlockIndex[dataBlockId];
					isFirst=0;
				}
				blockId=localVersion->blocks->block[blockId].nextBlockEntryIndex;	
			}
		}
	}
	
	//	printf("\n");
	
	
	free(buffer);
	fclose_64(f);
	//	printf(" blocks used : %d\n",nbUsedBlocksForUpdate);
	//	sendHeartBeat(socket,lastSentContentServerPacket);
	return nbUsedBlocksForUpdate;
}




DWORD updateGcfChunk(GCF * gcf,BUFFEREDHANDLE f, DWORD dataBlockIndex,char * buffer)
{// write 0x8000 = 4 data blocks maxi , initialiser with zeroes
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	int written=0;
	while (written<4 && dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount)
	{
		//printf("-write chunk %d\n",dataBlockIndex);
		//printf("On datablk %d\n",dataBlockIndex);
		_int64 offset=(_int64)gcf->dataBlocks->firstBlockOffset+(_int64)gcf->dataBlocks->blockSize*(_int64)dataBlockIndex;
		fseek_64(f,offset,SEEK_SET);
		
		fwrite_64(buffer+gcf->dataBlocks->blockSize*written,gcf->dataBlocks->blockSize,1,f);
		
		written++;
		dataBlockIndex=gcf->fragmentationMap->nextDataBlockIndex[dataBlockIndex];
	}
	//printf("finished update datablk. next : %d\n",dataBlockIndex);
	return dataBlockIndex;
}



DWORD decryptChunk(char * iv,unsigned char * key,int mode,char * buffer, DWORD read, char * decrypted , char * appName, int desiredLevel)
{
	DWORD encryptedLen=*(DWORD*)buffer;
	DWORD decryptedLen=*(DWORD*)(buffer+4);
	
	CRijndael aes;
	aes.MakeKey((char*)key, iv, 16, 16);
	aes.ResetChain();
	
	if (mode == 3) 
	{ // decrypt
		DWORD encrSize=read;
		if (encrSize%16) encrSize=((encrSize/16)+1)*16;
		aes.Decrypt(buffer,decrypted,encrSize,CRijndael::CFB);	
		decryptedLen=read;
	}
	else
	{ // decrypt && dezip
		char tmp[0x8000];
		memset(tmp,0,0x8000);
		if (encryptedLen>0x8000) encryptedLen=0x8000;
		
		memset(buffer+8+encryptedLen,0,0x8010-8-encryptedLen);	
		DWORD encrSize=encryptedLen;
		if (encrSize%16) encrSize=((encrSize/16)+1)*16;
		
		aes.Decrypt(buffer+8,tmp,encrSize,CRijndael::CFB);
		
		
		DWORD uncompressedSize=0x8000;
		int result=uncompress((unsigned char*)decrypted,&uncompressedSize,(unsigned char*)tmp,(DWORD)encrSize);
		
		if (result) 
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Zlib error",desiredLevel);
			return 0;
		}
		if (uncompressedSize != decryptedLen) 
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Decrypted data length don't match",desiredLevel);
			
			return 0;
		}
	}
	//printf("decypted len : %x\n",decryptedLen);
	return decryptedLen;
}

int decryptGcfFile(GCF * gcf,DWORD fileId,unsigned char * key,DWORD * checksums/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char * appName,int desiredLevel)
{
	char buffer[0x8010];
	char decrypted[0x8000];
	char iv[16]; memset(iv,0,16);
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	BUFFEREDHANDLE f = fopen_64(gcf->_fileName,"r+b");
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",gcf->_fileName,desiredLevel);
		return 0;

	}
	GCFDirectoryEntry entry=gcf->directory->entry[fileId];
	printDebug(DEBUG_LEVEL_INFO,appName,"Decrypting",gcf->directory->directoryNames+entry.nameOffset,desiredLevel);
	//printf("Decrypting : %s (size: %d)\n",gcf->directory->directoryNames+entry.nameOffset,entry.itemSize);
	
	DWORD blockIndex=gcf->directoryMap->firstBlockIndex[fileId];
	DWORD chunk=0;
	DWORD rest=entry.itemSize;
	while (blockIndex!=gcf->blocks->blockCount)
	{// for each block
		//		 printf("--------------\nblock %d\n",blockIndex);
		GCFBlock block=gcf->blocks->block[blockIndex];
		
		// added in 1.0.11 : check if already decrypted
		if (!(block.entryType&GCF_FILE_BLOCK_ZIPPED_AND_CRYPTED) && !(block.entryType&GCF_FILE_BLOCK_CRYPTED))
		{
			printDebug(DEBUG_LEVEL_DEBUG,appName,"Block already decrypted",gcf->directory->directoryNames+entry.nameOffset,desiredLevel);
			continue;
		}
		
		if (block.fileDataSize%0x8000 && block.nextBlockEntryIndex!=gcf->blocks->blockCount)
		{
			fclose_64(f);
			return 0;
		}
		
		
		DWORD mode=1;
		if (block.entryType&2) mode=2;
		if (block.entryType&4) mode=3;				
		
		if (block.fileDataSize)
		{ // if block has content
			DWORD dataBlockIndex=block.firstDataBlockIndex;
			while (dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount)
			{ // while data in block
				//printf("chunk %d / %d of len : %x\n",chunk,gcf->checksums->checksumEntry[entry.checksumIndex].checksumCount,(rest>0x8000 ? 0x8000 : rest));
				DWORD read=getGcfChunk(gcf,f,dataBlockIndex,buffer);
				DWORD originalChecksum=computeGcfChunkChecksum(buffer,(rest>0x8000 ? 0x8000 : rest));
				char * output;
				DWORD decryptedLen;
				if (originalChecksum==checksums[chunk])
				{
					// seems it was already decrypted
					decryptedLen=(rest>0x8000 ? 0x8000 : rest);
					output=buffer;
					//printf("-");
				}
				else
				{
					decryptedLen=decryptChunk(iv,key,mode,buffer,read,decrypted,appName,desiredLevel);
					
					memset(decrypted+(rest>0x8000 ? 0x8000 : rest),0,0x8000-(rest>0x8000 ? 0x8000 : rest));
					
					DWORD computedChecksum=computeGcfChunkChecksum(decrypted,(rest>0x8000 ? 0x8000 : rest));
					
					output=decrypted;
					
					if (computedChecksum!=checksums[chunk]) 
					{
						printDebug(DEBUG_LEVEL_ERROR,appName,"Checksum error",desiredLevel);
						fclose_64(f);
						return 0;
					}
					//	else printf("*");
				}
				
				//else printf("checksum ok\n");
				rest-=decryptedLen;
				//hexaPrintf(decrypted,160);
				dataBlockIndex=updateGcfChunk(gcf,f,dataBlockIndex,output);				
				chunk++;
				//	if (!(chunk%20)) sendHeartBeat(socket,lastSentContentServerPacket); // every megs
			}
		}
		blockIndex=block.nextBlockEntryIndex;
	}
	//	sendHeartBeat(socket,lastSentContentServerPacket);
	fclose_64(f);
	//	printf("\n");
	return 1;
}

int decryptNcfFile(char * path,unsigned char * key,DWORD size, DWORD * checksums/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char * appName,int desiredLevel)
{
	BUFFEREDHANDLE f=fopen_64(path,"r+b");
	if (!f) {printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",path,desiredLevel);return 0;}
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Decrypting",path,desiredLevel);
	
	DWORD toDecrypt=size;
	char buffer[0x8010];
	char decrypted[0x8000];
	char iv[16]; memset(iv,0,16);
	int mode=0;
	DWORD chunk=0;
	
	while (toDecrypt)
	{	
		fseek_64(f,(_int64)0x8000*(_int64)chunk,SEEK_SET);
		memset(buffer,0,0x8010);
		memset(decrypted,0,0x8010);
		int read=fread_64(buffer,1,0x8000,f);
		DWORD encryptedLen=*(DWORD*)buffer;
		DWORD decryptedLen=*(DWORD*)(buffer+4);
		
		if (!mode)
		{
			mode=3;
			if (encryptedLen && decryptedLen && decryptedLen<=toDecrypt && encryptedLen<=decryptedLen) // not a very good check ... but nothing better 2nd part especialy can be false
				mode=2; // zipped then encrypted
		}
		
		DWORD originalChecksum= computeGcfChunkChecksum(buffer,read);
		char * output;
		if (originalChecksum==checksums[chunk])
		{
			// seems it was already decrypted
			output=buffer;
			//	printf("-");
		}
		else
		{
			decryptedLen=decryptChunk(iv,key,mode,buffer,read,decrypted,appName,desiredLevel);
			
			DWORD computedChecksum= computeGcfChunkChecksum(decrypted,decryptedLen);
			output=decrypted;
			if (computedChecksum!=checksums[chunk]) 
			{
				printDebug(DEBUG_LEVEL_ERROR,appName,"Checksum error",desiredLevel);
				fclose_64(f);
				return 0;
			}
			else printf("*");
		}
		fflush_64(f);
		fseek_64(f,(_int64)0x8000*(_int64)chunk,SEEK_SET);
		fwrite_64(output,1,decryptedLen,f);
		fflush_64(f);
		chunk++;
		//	if (!(chunk%20)) sendHeartBeat(socket,lastSentContentServerPacket); // every megs
		toDecrypt-=read;
	}
	//	printf("\n");
	fclose_64(f);
	setFileSize64(path,size);
	//	sendHeartBeat(socket,lastSentContentServerPacket);
	return 1;
}

void decryptFile(char * root, char * commonName,GCF * gcf,DWORD fileId,unsigned char * key/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char* appName,int desiredLevel)
{
	DWORD cIndex=gcf->directory->entry[fileId].checksumIndex;
	cIndex=gcf->checksums->checksumEntry[cIndex].firstChecksumIndex;
	
	switch (gcf->cfType)
	{
	case 1 : {
		if (!decryptGcfFile(gcf,fileId,key,gcf->checksums->checksum+cIndex/*,socket,lastSentContentServerPacket*/,appName,desiredLevel))
		{
			// error, reset file
			gcf->directoryMap->firstBlockIndex[fileId]=gcf->blocks->blockCount; // not downloaded
			
			// not needed to free blocks, they will not be "recopied" when finishing update
		}
		break;
			 }
	case 2 : {
		char path[2000];
		sprintf(path,"%s\\common\\%s",root,commonName);
		getFilePath(gcf,fileId,path+strlen(path));
		
		// /new encrypted test in 1.0.11
		bool encrypted=gcf->directoryMap->firstBlockIndex[fileId]&0x80000000;
		if (encrypted)
		{
			if (!decryptNcfFile(path,key,gcf->directory->entry[fileId].itemSize,gcf->checksums->checksum+cIndex/*,socket,lastSentContentServerPacket*/,appName,desiredLevel))
			{
				// error, delete file
				setFileSize64(path,0);
				gcf->directoryMap->firstBlockIndex[fileId]=1; // not downloaded
			}
		}
		else
		{
			printDebug(DEBUG_LEVEL_DEBUG,appName,"File already decrypted",gcf->directory->directoryNames+gcf->directory->entry[fileId].nameOffset,desiredLevel);
		}
		break;
			 }
	}
	
}

int hasDecryptableFiles(GCF * localVersion,GCF * newVersion,DWORD * updatedFiles,DWORD nbUpdated)
{
	
	
	for (int ind=0;ind<localVersion->directory->itemCount;ind++)
	{
		if (isUpdated(localVersion,ind,updatedFiles,nbUpdated)) continue; // not needed to decrypt if updated...
		if (localVersion->directory->entry[ind].directoryType&ENCRYPTED_FLAG) // encrypted
		{
			DWORD updatedFileId=getUpdatedFileId(ind,localVersion,newVersion);
			if (updatedFileId!=-1)
			{
				// exists in new version
			/*	if (!(newVersion->directory->entry[updatedFileId].directoryType&ENCRYPTED_FLAG)) 
				{
					// file is not marked as encrypted anymore
					// let's decrypt
					return 1;
					
				}
				*/
				return 1; // fix in cft 1.0.11 : steam decrypts all files even if marked as encrypted

			}
		}
	}
	return 0;
}

void decrypt(char * root, char * commonName,GCF * localVersion,GCF * newVersion, char * keyStr,DWORD * updatedFiles,DWORD nbUpdated/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char * appName,int desiredLevel, char * taskName, DWORD * taskProgress)
{
	
	unsigned char key[100];
	int len=hexa_2Buffer(key,keyStr);
	
	int first=1;
	
	if (len!=16) {printDebug(DEBUG_LEVEL_ERROR,appName,"Invalid decryption key length",keyStr,desiredLevel);return;}

	for (int ind=0;ind<localVersion->directory->itemCount;ind++)
	{
		if (isUpdated(localVersion,ind,updatedFiles,nbUpdated)) continue; // not needed to decrypt if updated...
		if (localVersion->directory->entry[ind].directoryType&ENCRYPTED_FLAG) // encrypted
		{
			DWORD updatedFileId=getUpdatedFileId(ind,localVersion,newVersion);
			if (updatedFileId!=-1)
			{
				// exists in new version
			/*	if (!(newVersion->directory->entry[updatedFileId].directoryType&ENCRYPTED_FLAG))  fix in 1.0.11 : // steam decrypts even if still marked as encrypted 
				{
					// file is not marked as encrypted anymore
					// let's decrypt*/
					//	if (first) printf("Decrypting gcf/ncf with key : %s\n",keyStr);
					first=0;
					if (taskName) 
					{
						sprintf(taskName,"Decrypting : %s",newVersion->directory->directoryNames+newVersion->directory->entry[updatedFileId].nameOffset);
					}
					decryptFile(root,commonName,localVersion,ind,key/*,socket,lastSentContentServerPacket*/,appName,desiredLevel);
					if (taskProgress) *taskProgress=((100*ind)/localVersion->directory->itemCount)+1;
			/*	}*/
			}
		}
	}
}

DWORD getShiftedIndex(DWORD index, int shift, DWORD max)
{
	if (!max) return 0;
	LONGLONG newIndex=index;
	newIndex-=shift;
	while (newIndex<0) newIndex+=max;
	return newIndex%max;
}

int updateVersionGcf(char * root, char * commonName,GCF * localVersion,GCF * newVersion,DWORD * updatedFiles,DWORD nbUpdated/*, SOCKET socket, time_t * lastSentContentServerPacket*/,char* appName,int desiredLevel, char * taskName, DWORD * taskProgress)
{
	// glueing all blocks together
	DWORD reusedBlocks=collapseGaps(localVersion,updatedFiles,nbUpdated,newVersion->dataBlocks->blockCount/*,socket, lastSentContentServerPacket*/,appName,desiredLevel);
	
	//	printf("%d / %d data blocks kept from local version\n",reusedBlocks,localVersion->dataBlocks->blockCount);
	
	// computing how many blocks to move for being able to store the headers
	int blockShift=0;
	while (newVersion->dataBlocks->firstBlockOffset <= localVersion->dataBlocks->firstBlockOffset+localVersion->blockSize*(1+blockShift)) blockShift--;
	while (newVersion->dataBlocks->firstBlockOffset > localVersion->dataBlocks->firstBlockOffset+localVersion->blockSize*(blockShift)) blockShift++;
	newVersion->dataBlocks->firstBlockOffset=localVersion->dataBlocks->firstBlockOffset+localVersion->blockSize*(blockShift);
	newVersion->fileSize=newVersion->dataBlocks->firstBlockOffset+newVersion->blockCount*localVersion->blockSize;
	
	//	printf("block shift : %d\n",blockShift);
	
	if (localVersion->fileSize<newVersion->fileSize) 	
	{
		if (taskName) strcpy(taskName,"Allocating file space");
		if (taskProgress) *taskProgress=0;
		printDebug(DEBUG_LEVEL_DEBUG,appName,"Allocating file space",desiredLevel);
		if (setFileSize64(localVersion->_fileName,newVersion->fileSize)){printDebug(DEBUG_LEVEL_ERROR,appName,"File space allocation error",desiredLevel);return -1;}
	}
	
	// move blocks, frag map, firstDatablockIndex (usage map is not changing )
	if (blockShift)
	{
		if (taskName) strcpy(taskName,"Defragmenting file");
		
		BUFFEREDHANDLE f=fopen_64(localVersion->_fileName,"r+b");
		
		if (!f){printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",localVersion->_fileName,desiredLevel); return 0;}
		
		char * buffer;
		buffer=(char*)malloc(localVersion->dataBlocks->blockSize);
		
		DWORD * oldFragMap=(DWORD*)malloc(4*reusedBlocks);
		for (int ind=0;ind<reusedBlocks;ind++)
		{
			oldFragMap[ind]=localVersion->fragmentationMap->nextDataBlockIndex[ind];
		}
		printf("\n");
		if (blockShift<0)
		{
			// take blocks at the end, put at begining
			for (int ind=blockShift;ind<0;ind++)
			{
				//	printf("\r moving block %d/%d -> %d",reusedBlocks+ind,reusedBlocks,ind);
				copyBlock(localVersion,f,reusedBlocks+ind,ind,buffer,appName,desiredLevel);
				//	if (!(ind%80)) sendHeartBeat(socket,lastSentContentServerPacket); // every megas
				if (taskProgress) *taskProgress=(100*ind)/blockShift+1;
			}
		}
		
		if (blockShift>0)
		{
			// take blocks at the begining, put at the end
			for (int ind=0;ind<blockShift;ind++)
			{
				//	printf("\r moving block %d/%d -> %d",ind,reusedBlocks,reusedBlocks+ind);
				copyBlock(localVersion,f,ind,reusedBlocks+ind,buffer,appName,desiredLevel);
				if (taskProgress) *taskProgress=(100*ind)/blockShift+1;
				//	if (!(ind%80)) sendHeartBeat(socket,lastSentContentServerPacket); // every megas
			}
		}	
		
		free(buffer);
		fclose_64(f);
		//		printf("\n");
		//	sendHeartBeat(socket,lastSentContentServerPacket);
		
		DWORD terminator=(!localVersion->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
		
		if (reusedBlocks)
		{// shift first data blocks
			for (int ind=0;ind<localVersion->blocks->blockCount;ind++)
			{
				DWORD newIndex=localVersion->blocks->block[ind].firstDataBlockIndex;
				
				//	if (newIndex)
				newIndex=getShiftedIndex(newIndex,blockShift,reusedBlocks);
				char *blk="";
				char * name=blk;
				if (localVersion->blocks->block[ind].entryType) name=localVersion->directory->directoryNames+localVersion->directory->entry[localVersion->blocks->block[ind].directoryIndex].nameOffset;
				// 	printf(" %d - moving block first data block index (%d-%d)mod %d -> %d (%s)\n",ind,localVersion->blocks->block[ind].firstDataBlockIndex,blockShift,reusedBlocks,newIndex,name);
				
				localVersion->blocks->block[ind].firstDataBlockIndex=newIndex;
			}
			// shift frag map 
			for (int ind=0;ind<reusedBlocks;ind++)
			{
				DWORD newFrom=ind;
				DWORD newTo=oldFragMap[ind];
				
				newFrom=getShiftedIndex(newFrom,blockShift,reusedBlocks);
				if (newTo!=terminator) newTo=getShiftedIndex(newTo,blockShift,reusedBlocks);
				
				//printf(" moving frag map %d->%d  =>  %d->%d\n",ind, localVersion->fragmentationMap->nextDataBlockIndex[ind],newFrom,newTo);
				
				localVersion->fragmentationMap->nextDataBlockIndex[newFrom]=newTo;
			}
		}
		free(oldFragMap);
	}
	
	if (localVersion->fileSize>newVersion->fileSize) 
	{
		//	printf("Optimizing file space\n");
		if (taskName) strcpy(taskName,"Allocating file space");
		if (taskProgress) *taskProgress=0;
		if (setFileSize64(localVersion->_fileName,newVersion->fileSize)){printDebug(DEBUG_LEVEL_ERROR,appName,"File space allocation error",desiredLevel);return -1;}
	}
	
	//	printf("Updating allocation tables\n");
	if (taskName) strcpy(taskName,"Updating allocation tables");
	newVersion->dataBlocks->blocksUsed=reusedBlocks;
	for (int localFileId=0;localFileId<localVersion->directory->itemCount;localFileId++)
	{
		//	printf("processing %d / %d\n",localFileId,localVersion->directory->itemCount);
		DWORD updatedFileId=getUpdatedFileId(localFileId,localVersion,newVersion);
		if (updatedFileId!=-1)
		{// file existing in new version
			if (!isUpdated(localVersion,localFileId,updatedFiles,nbUpdated))
			{// and is not updated
				newVersion->directoryMap->firstBlockIndex[updatedFileId]=recopyGcfBlock(localVersion,newVersion,localVersion->directoryMap->firstBlockIndex[localFileId],updatedFileId,newVersion->blocks->blockCount);
			}
			else
			{						
			/*	char path[1000];getFilePath(localVersion,localFileId,path);		
				printf("Reseted updated file : %s\n",path);*/
			}
		}
		
	}
	//	printf("Allocation tables updated\n");
	return 0;
}
int updateVersionNcf(char * root, char * commonName,GCF * localVersion,GCF * newVersion,DWORD * updatedFiles,DWORD nbUpdated,char* appName,int desiredLevel)
{
	if (setFileSize64(localVersion->_fileName,newVersion->headersSize)){printDebug(DEBUG_LEVEL_ERROR,appName,"File space allocation error",desiredLevel); return -1;}
	char path[4001];
	char tmp[256];
	strcpy(tmp,commonName);
	strlwr(tmp);
		
	sprintf(path,"%s\\common",root);

	/*_getcwd(cwd,1000);
	_chdir(root); 
	mkdir("common");
	chdir("common");
	mkdir(commonName);
	chdir(commonName);
	*/
	mkdir(path);
	strcat(path,"\\");
	strcat(path,tmp);
	mkdir(path);
	//FILE * log=fopen("d:\\log.txt","wt");
	// delete files & updated headers
	for (int localFileId=0;localFileId<localVersion->directory->itemCount;localFileId++)
	{
	//	fprintf(log,"------- localGCF:%d \n",(int)localVersion);
		DWORD updatedFileId=getUpdatedFileId(localFileId,localVersion,newVersion);
	//	fprintf(log,"localFileId:%d  |  updatedFileId:%d\n",localFileId,updatedFileId);
		char toDelete=1;
		if (updatedFileId!=-1)
		{// file existing in new version
			if (!isUpdated(localVersion,localFileId,updatedFiles,nbUpdated))
			{// and is not updated
				newVersion->directoryMap->firstBlockIndex[updatedFileId]=localVersion->directoryMap->firstBlockIndex[localFileId];
				toDelete=0;
			}
		}
	//	fprintf(log,"     toDelete:%d\n",toDelete);
		if (toDelete)
		{
			char * fpath=path+strlen(path);
			//path[0]='.';
			getFilePath(localVersion,localFileId,fpath);
			//	printf("Reseting updated file : %s\n",path);
	//		fprintf(log,"Deleting : %s\n",path);
			unlink(path);
			BUFFEREDHANDLE tmp=fopen_64(path,"wb");
			if (tmp) fclose_64(tmp);
			*fpath=0;
		}
	}
	// create new empty files
	GCFDirectoryEntry rootE=newVersion->directory->entry[0];
	while (rootE.parentIndex!=-1) rootE=newVersion->directory->entry[rootE.parentIndex];
	
	int res=createNcfFolders(newVersion,path,rootE,appName,desiredLevel);
	
	
//	_chdir(cwd);
	return res;
}
int updateVersion(char * root, char * commonName,GCF * localVersion,GCF * newVersion,DWORD * updatedFiles,DWORD nbUpdated/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char* appName,int desiredLevel, char * taskName, DWORD * taskProgress)
{
	if (localVersion->cfType==1)
	{ //gcf
		updateVersionGcf(root,commonName,localVersion,newVersion,updatedFiles,nbUpdated/*,socket,lastSentContentServerPacket*/,appName,desiredLevel,taskName,taskProgress);
	}
	else
	{	// ncf
		updateVersionNcf(root,commonName,localVersion,newVersion,updatedFiles,nbUpdated,appName,desiredLevel); // no heartbeat/task update ... fast operation 
	}
	
	// build newVersion from localVersion
	//	printf("Finishing gcf/ncf update\n");
	strcpy(newVersion->_fileName,localVersion->_fileName);
	updateGcfFileHeaders(newVersion,0);	
	return 0;
}


int downloadApp(sockaddr_in contentServer, CDRApp manifest, CDRApp app, char * ticket,int ticketLen, char * root, char*appLogName,int desiredLogLevel,char * contentServerURL,char * taskName,DWORD * taskProgress, DWORD*remainingTime,int * pleaseStop,int securedUpdates,bool showRealBandwidth,bool validate)
{
	if (*pleaseStop) return 0;
	if (remainingTime) *remainingTime=0;	
	
	DWORD version=app.currentVersionId;
	char gcfFileName[1000];
	//	time_t lastSentContentServerPacket=0;
	int cfType=(app.manifestOnlyApp || app.appOfManifestOnlyCache ? 2 : 1);
	int cfVersion=(cfType==1 ? 6 : 1);
	char * commonDirName=0;
	int decryptedApp=0;
	/*
	RegistryVector * vector=getCDRAppUserDefinedData(manifest);  
 	
	if (vector)
	{
		RegistryNode * gamedir=vector->getNode("gamedir");
		if (gamedir) commonDirName=gamedir->getStringValue();
	}
	*/
	commonDirName=manifest.installDirName;

	getCFFileName(root,app.installDirName,cfType,gcfFileName); 
	
	GCF * localVersion=parseGCF(gcfFileName);
	
	if (int(localVersion)>1)
	{
		char dbg[1000];
		sprintf(dbg,"%s (version : %d)",localVersion->_fileName,localVersion->appVersion);
		printDebug(DEBUG_LEVEL_DEBUG,appLogName,"Resuming GCF/NCF",dbg,desiredLogLevel);
	}
	else
		if (int(localVersion)==1)
		{
			printDebug(DEBUG_LEVEL_ERROR,appLogName,"Deleting corrupted GCF/NCF",gcfFileName,desiredLogLevel);
			//printf("corrupted %s found. deleting\n",gcfFileName);
		}

		char csip[50];
		itoip(contentServer.sin_addr.S_un.S_addr,csip);
		printDebug(DEBUG_LEVEL_DEBUG,appLogName,"Connecting to content server",csip,desiredLogLevel);
		SOCKET socket=connectSocket(contentServer);
		if (socket==0) {closesocket(socket); return -1;}
		sendSocket(socket,"00 00 00 07");
		
		int ack=readSocket(socket,1);
		if (!ack) {closesocket(socket); return -1;}
		
		 
		if (getBanner(socket,contentServerURL)<=0)  {closesocket(socket); return -2;}
		printDebug(DEBUG_LEVEL_DEBUG,appLogName,"Content server banner",contentServerURL,desiredLogLevel);
		
		DWORD connectionId=0;
		DWORD messageId=0;
		
		printDebug(DEBUG_LEVEL_DEBUG,appLogName,"Requesting latest version",version,desiredLogLevel);
		
		DWORD directoryChecksum=openAppAndGetDirectoryChecksum(socket,&connectionId,&messageId,app.appId,version,ticket,ticketLen);
		if (!directoryChecksum) {printDebug(DEBUG_LEVEL_ERROR,appLogName,"Not owning app or app not available",desiredLogLevel);closesocket(socket); return -3;}
		if (*pleaseStop) 
		{
			closeApp(socket,&connectionId,&messageId);
			closesocket(socket);
			return 0;
		}
		
			if (taskName && taskProgress)
			{
				strcpy(taskName,"Retrieving directory");
				*taskProgress=0;
			}		
		//	printf("Downloading directory\n");
		char * directory;
		int directoryLen=getDirectory(socket,&connectionId,&messageId,&directory);
		if (!directoryLen) {printDebug(DEBUG_LEVEL_ERROR,appLogName,"Directory download error",desiredLogLevel);	closeApp(socket,&connectionId,&messageId);closesocket(socket); return -1;}
		
			if (taskName && taskProgress)
			{
				strcpy(taskName,"Retrieving checksums");
				*taskProgress=0;
			}		
		
		//printf("Downloading checksums\n");
		char * checksums;
		int checksumsLen=getChecksums(socket,&connectionId,&messageId,&checksums);
		if (!checksumsLen) {printDebug(DEBUG_LEVEL_ERROR,appLogName,"Checksums download error",desiredLogLevel);	closeApp(socket,&connectionId,&messageId);free(directory);closesocket(socket); return -1;}
		
		//printf("Building gcf/ncf headers\n");
		GCF * gcf=parseGcf(directory,checksums,cfType,cfVersion);
		free(directory);
		free(checksums);
		if (*pleaseStop) 
		{
			closeApp(socket,&connectionId,&messageId);
			closesocket(socket);
			return 0;
		}		
		if ((int)localVersion<=1)
		{
	

			if (taskName && taskProgress)
			{
				strcpy(taskName,"Creating cache");
				*taskProgress=0;
			}				//printf("Creating empty gcf/ncf\n");
			createGcfFile(gcf,root,app.installDirName,commonDirName,appLogName,desiredLogLevel);
		}
		else
		{
			if (localVersion->appId!=gcf->appId)
			{
				printDebug(DEBUG_LEVEL_ERROR,appLogName,"GCF/NCF id error for",localVersion->_fileName,desiredLogLevel);
				//printf("Existing gcf/ncf file is not matching appId\n");
				return -1;
			}
			if (localVersion->appVersion>gcf->appVersion)
			{
				printDebug(DEBUG_LEVEL_ERROR,appLogName,"Local GCF/NCF version is higher than server side version",localVersion->_fileName,desiredLogLevel);
				//printf("Existing gcf/ncf file version is higher than server side version\n");
				return -2;
			}
			if (localVersion->appVersion<gcf->appVersion) 
			{
				
				//	printf("Requesting updated files ID from version %d to %d\n",localVersion->appVersion, version);
				DWORD * updated=0;
				int nbUpdated=getUpdatedFiles(socket,&connectionId,&messageId,localVersion->appVersion,&updated);		
				if (nbUpdated==-1) 
				{
					printDebug(DEBUG_LEVEL_ERROR,appLogName,"GCF/NCF update not available from version",localVersion->appVersion,desiredLogLevel);
					return -3;
				}
				
				CDRAppVersion version=getCDRAppVersion(app,localVersion->appVersion);
				if (*pleaseStop)
				{
					closeApp(socket,&connectionId,&messageId);
					closesocket(socket);
					return 0;	
				}
				if (version.isEncryptionKeyAvailable && hasDecryptableFiles(localVersion,gcf,updated,nbUpdated))
				{
					// this can be long, closing connection 
					closeApp(socket,&connectionId,&messageId);
					closesocket(socket);
					//	time(&lastSentContentServerPacket);
					CDRAppVersion version=getCDRAppVersion(app,localVersion->appVersion);
					if (version.isEncryptionKeyAvailable) 
					{
						char * keyStr=version.depotEncryptionKey;
						if (taskName && taskProgress)
						{			
							strcpy(taskName,"Decrypting");
							*taskProgress=0;
						}
						decrypt(root,commonDirName,localVersion,gcf,keyStr,updated,nbUpdated/*,socket,&lastSentContentServerPacket*/,appLogName,desiredLogLevel,taskName,taskProgress);
					}
					decryptedApp=1;
					
					// return 1000; /// stopping there for tests
				}
				//printf("%d files updated\n",nbUpdated); 
				//	printf("Updating local gcf/ncf version : %d -> %d\n",localVersion->appVersion,gcf->appVersion);
						if (taskName && taskProgress)
						{			
							strcpy(taskName,"Updating");
							*taskProgress=0;
						}
				if (updateVersion(root,commonDirName,localVersion,gcf,updated,nbUpdated/*,socket,&lastSentContentServerPacket*/,appLogName,desiredLogLevel,taskName,taskProgress)) {if (updated) free(updated);return -3;}
				if (updated) free(updated);
			}
			else
			{
				//	printf("Resuming gcf/ncf download\n");
			}
			
		}
		
		
		deleteGCF(gcf);
		int res=1; // must reconnect to download
		
		if (!decryptedApp)	
		{
			res=0;
			if (taskName && taskProgress)
			{
				strcpy(taskName,"Downloading");
				*taskProgress=0;
			}

			//	printf("Downloading content\n");
			if (*pleaseStop)
			{
				closeApp(socket,&connectionId,&messageId);
				closesocket(socket);
				return 0;
			}
			res=downloadAppFiles(root,gcfFileName,commonDirName,socket,&connectionId,&messageId,appLogName,desiredLogLevel,taskName,taskProgress,remainingTime,pleaseStop,securedUpdates,showRealBandwidth,validate);
			closeApp(socket,&connectionId,&messageId);
			closesocket(socket);
		}
		//printGCFXML(gcf);
		//
		
		
		
		return res;
}



DWORD getNonCompletedFiles(GCF * gcf, DWORD * buffer)
{
	DWORD nb=0;
	
	for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
	{
		if (gcf->directory->entry[ind].directoryType)
		{
			int add=false;
			switch (gcf->cfType)
			{
			case 1 : {
				DWORD downloadedSize=0;
				DWORD blockIndex=gcf->directoryMap->firstBlockIndex[ind];
				int anotherBlock=0;
				if (blockIndex!=gcf->blocks->blockCount)
					do 
					{
						anotherBlock=0;
						downloadedSize+=gcf->blocks->block[blockIndex].fileDataSize;
						if (gcf->blocks->block[blockIndex].nextBlockEntryIndex!=gcf->blocks->blockCount)
						{
							blockIndex=gcf->blocks->block[blockIndex].nextBlockEntryIndex;
							anotherBlock=1;
						}
					} while (anotherBlock || gcf->blocks->block[blockIndex].nextBlockEntryIndex!=gcf->blocks->blockCount);
					add = (downloadedSize<gcf->directory->entry[ind].itemSize);
					break;
					 }
			case 2 : 
				{
					add=(gcf->directoryMap->firstBlockIndex[ind]!=3); 
					break;
				}
			}
			if (add) buffer[nb++]=gcf->directory->entry[ind].checksumIndex;
		}
	}
	return nb;
}

int copyGcfFile(GCF * source,GCF * gcf,char * root,char * commonName,DWORD fileId, BUFFEREDHANDLE file, char * appName, int desiredLevel,int securedUpdates)
{

// 1.0.7b copy only if file has size>0
	GCFDirectoryEntry entry=source->directory->entry[fileId];

	if (!entry.itemSize) 
	{
		gcf->directoryMap->firstBlockIndex[fileId]=gcf->blocks->blockCount;
		return 0;
	}

	// warning is working only on empty cleaned gcf files
	DWORD blockIndex=createBlock(gcf,fileId);
	gcf->directoryMap->firstBlockIndex[fileId]=blockIndex;
	if (securedUpdates) updateGcfFileHeaders(gcf,file);	
	
	 

	DWORD len=16*4*0x2000;
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	char * data=(char*) malloc(len);
	char dbg[1000];
	//	if (entry.directoryType&ENCRYPTED_FLAG) 
	
	

	sprintf(dbg,"Copying %s (size:%d)",source->directory->directoryNames+entry.nameOffset,entry.itemSize);
	
	printDebug(DEBUG_LEVEL_DEBUG,appName,dbg,desiredLevel);
	

	
	GCFBlock block=gcf->blocks->block[blockIndex];
	DWORD currentDataBlock=block.firstDataBlockIndex;
	/*	if (currentDataBlock!=terminator)
	{
	while (gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock]!=(gcf->fragmentationMap->terminator ? 0xFFFFFFFF : 0xFFFF)) 
	currentDataBlock=gcf->fragmentationMap->nextDataBlockIndex[currentDataBlock];
	//sprintf(path,"d:\\cg\\%s",gcf->directory->directoryNames+entry.nameOffset);
	}
	
	  DWORD currentSize=block.fileDataOffset+block.fileDataSize;
	  
		//printf("currentSize : %d   supposed : %d\n",currentSize,entry.itemSize);
		if (currentSize>=entry.itemSize) 
		{
		//	printf("was finished\n");
		// download was finished
		return 0;
		}
		
		  DWORD chunkSize=gcf->blockSize*4;
		  
			DWORD nbDownloadedBlocks=currentSize/chunkSize;
			
			  DWORD rest=currentSize%gcf->blockSize;
			  if (rest) 
			  {
			  printDebug(DEBUG_LEVEL_ERROR,appName,"Half-downloaded file size error",desiredLevel);
			  //printf("file not full downloaded and downloaded size not a multiple of block size!\n"); 
			  return -1;
}*/
	DWORD nbDownloadedBlocks=0;
	DWORD toRead=entry.itemSize-nbDownloadedBlocks*gcf->blockSize*4;
	GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
	//printf("\n");
	DWORD transferMode=0;
	while (nbDownloadedBlocks<checksumEntry.checksumCount)
	{
		//printf("startLoop\n"); 
		int nbChunks=(checksumEntry.checksumCount-nbDownloadedBlocks >16 ? 16 : checksumEntry.checksumCount-nbDownloadedBlocks);
		
		//printf("  downloaded chunks : %d / %d\n",nbDownloadedBlocks,checksumEntry.checksumCount);
		
		//	unsigned int done=getFileChunk(socket,connectionId,messageId,entry.checksumIndex,nbDownloadedBlocks,nbChunks,data,gcf->checksums->checksum+(checksumEntry.firstChecksumIndex+nbDownloadedBlocks),toRead,appName,desiredLevel,&transferMode);
		DWORD readSize=getGCFBlocks(source,root,commonName,fileId, nbDownloadedBlocks*4,4*nbChunks, data,appName,desiredLevel,&transferMode);
		//printf("read : %d  / excepted : %d\n",readSize,nbChunks*0x8000);
		if (readSize)
		{
			//	DWORD readSize=(toRead < done*4*0x2000 ? toRead : done*4*0x2000);
			toRead-=readSize; 
			//	printf("done: %d    readSize:%d   toRead:%d\n",done,readSize,toRead);
			
			int nbDataBlocks=readSize/0x2000+ (readSize%0x2000 ? 1 : 0) ;
			setBlockMode(gcf,blockIndex,transferMode);
			//printf("updateGcf (%d blocks)\n",nbDataBlocks); 
			currentDataBlock=updateGcfDataBlocks(gcf,blockIndex,currentDataBlock,nbDataBlocks,file,readSize,data,securedUpdates);
			//printf("updated\n");
			//	printf("copied %d blocks\n",nbDataBlocks);
		}
		// SAVE the data blocks, update fragmentation map and maybe block first data block
		//fwrite_64(data,1,readSize,f);
		int done=readSize/0x8000+(readSize%0x8000 ? 1 : 0);
		
		if (!done) {
			printDebug(DEBUG_LEVEL_ERROR,appName,"Copy error",desiredLevel);
			free(data); 
			return -1;
		}
		//	printf(" read %d chunks\n",readed);
		
		nbDownloadedBlocks+=(done);
	}
	
	//	printBar(checksumEntry.checksumCount,checksumEntry.checksumCount);
	//	printf("\r\n");
	
	free(data);
	return 0;
}

int copyNcfFile(GCF * source,GCF * gcf,char * root,char * commonName,DWORD fileId,char * appName, int desiredLevel)
{
	DWORD len=16*4*0x2000;
	char path[1001];
	char path2[1001];
	
	getFilePath(gcf,fileId,path);
	sprintf(path2,"%s\\common\\%s%s",root,commonName,path);
	
	char * data=(char*) malloc(len);
	GCFDirectoryEntry entry=source->directory->entry[fileId];
	char dbg[1000];
 
	
	
	
	BUFFEREDHANDLE f=fopen_64(path2,"r+b");
	
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",path,desiredLevel);
		return -1;
	}
	
	fseek_64(f,0,SEEK_END);
	 	
	// check if some data inside (file has been manually copied)

	DWORD currentSize=ftell_64(f);
	
	if (currentSize==entry.itemSize) 
	{
	  sprintf(dbg,"File already existing, not copying %s (size:%d)",source->directory->directoryNames+entry.nameOffset,entry.itemSize);
	  printDebug(DEBUG_LEVEL_DEBUG,appName,dbg,desiredLevel);
	  fclose_64(f);
	  gcf->directoryMap->firstBlockIndex[fileId]=3; // download was finished, file was manualy copied	
	  return 0;
	}
	else if (currentSize)
	{
		// there is some unknown size data in the file which should be empty ... reseting it.
		sprintf(dbg,"Reseting and copying %s (size:%d)",source->directory->directoryNames+entry.nameOffset,entry.itemSize);
		fclose_64(f);
		unlink(path2);
		

		f=fopen_64(path2,"wb");
		fclose_64(f);


		f=fopen_64(path2,"r+b");
	}
	else
		sprintf(dbg,"Copying %s (size:%d)",source->directory->directoryNames+entry.nameOffset,entry.itemSize);

	printDebug(DEBUG_LEVEL_DEBUG,appName,dbg,desiredLevel);

	DWORD chunkSize=GCF_BLOCK_SIZE*4;
	
	DWORD nbDownloadedBlocks=0;
	
	// still data to be downloaded, but size is not a multiple of block size => some "garbage" maybe caused by disk full error
	//fseek(f,rest,SEEK_END); // rewinding the resting bytes
	
	DWORD toRead=entry.itemSize;
	GCFChecksumEntry checksumEntry=gcf->checksums->checksumEntry[entry.checksumIndex];
	while (nbDownloadedBlocks<checksumEntry.checksumCount)
	{
		int nbChunks=(checksumEntry.checksumCount-nbDownloadedBlocks >16 ? 16 : checksumEntry.checksumCount-nbDownloadedBlocks);
		
		//printf("  downloaded chunks : %d / %d\n",nbDownloadedBlocks,checksumEntry.checksumCount);
		//printBar(nbDownloadedBlocks,checksumEntry.checksumCount);
		//	unsigned int done=getFileChunk(socket,connectionId,messageId,entry.checksumIndex,nbDownloadedBlocks,nbChunks,data,gcf->checksums->checksum+(checksumEntry.firstChecksumIndex+nbDownloadedBlocks),toRead,appName,desiredLevel);
		//	DWORD readSize=(toRead < done*4*0x2000 ? toRead : done*4*0x2000);
		DWORD readSize=getGCFBlocks(source,root,commonName,fileId, nbDownloadedBlocks*4,4*nbChunks, data,appName,desiredLevel,0);
		
		toRead-=readSize; 
		//	printf("done: %d    readSize:%d   toRead:%d\n",done,readSize,toRead);
		
		fwrite_64(data,1,readSize,f);
		int done=readSize/0x8000+(readSize%0x8000 ? 1 : 0);
		if (!done) {
			printDebug(DEBUG_LEVEL_ERROR,appName,"Copy error",desiredLevel);
			free(data); 
			fclose_64(f); 
			setFileSize64(path2,0);
			gcf->directoryMap->firstBlockIndex[fileId]=1; // not downloaded, file reset
			return -1;
		}
		//	printf(" read %d chunks\n",readed);
		nbDownloadedBlocks+=done;
	}
	
	
	//	printBar(checksumEntry.checksumCount,checksumEntry.checksumCount);
	//	printf("\r\n");
	free(data);
	fclose_64(f);
	gcf->directoryMap->firstBlockIndex[fileId]=3; // downloaded
	return 0;
}

int copyFiles(GCF * source,GCF * dest,char * root, char * commonName,DWORD * files,DWORD nbFiles, char * appName, int desiredLevel, DWORD * taskProgress, int * pleaseStop,int securedUpdates)
{

	//	char extPath[1000];
	//	sprintf(extPath,"%s\\common\\%s",root,commonName);
	BUFFEREDHANDLE f=fopen_64(dest->_fileName,"r+b");
	if (!f)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",dest->_fileName,desiredLevel);
		return 0;
	}
	for (unsigned int ind=0;ind<source->directory->itemCount;ind++)
	{
	 
			if (pleaseStop && *pleaseStop) break;
			if (isUpdated(source,ind,files,nbFiles))
			{
				switch (dest->cfType)
				{
				case 1 : copyGcfFile(source,dest,root,commonName,ind,f,appName,desiredLevel,securedUpdates);break;
				case 2 : copyNcfFile(source,dest,root,commonName,ind,appName,desiredLevel);break;
				}
			}
		
		if (taskProgress) *taskProgress=((100*ind)/source->directory->itemCount)+1;

	}
	updateGcfFileHeaders(dest,f);
	fclose_64(f);

	return 0;
}


int createArchive(char * gcfPath,char * archivePath, char * appName, int desiredLevel,char * taskName,DWORD * taskProgress)
{
	GCF * gcf=parseGCF(gcfPath);
	if (((int)gcf)<=1)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file not found or corrupted",gcfPath,desiredLevel);
		return 1;
	}
	int size=serializeArchive(gcf,0);
	char  * buffer=(char*)malloc(size);
	serializeArchive(gcf,buffer,1);
	char name[1000];
	buildArchiveName(gcf,name,archivePath);
	deleteGCF(gcf);
	BUFFEREDHANDLE f=fopen_64(name,"wb");
	if (f) 
	{
		printDebug(DEBUG_LEVEL_INFO,appName,"Created archive",name,desiredLevel);
		fwrite_64(buffer,1,size,f);
		fclose_64(f);
		free(buffer);
		return 0;
	}
	printDebug(DEBUG_LEVEL_ERROR,appName,"Archive creation error",name,desiredLevel);
	free(buffer);
	return 1;
}

int createUpdateFile(char * archivePath, char * gcfPath,char * commonName, char * patchPath, char * appName, int desiredLevel, char * taskName, DWORD * taskProgress, int * pleaseStop,int securedUpdates)
{
	GCF * archive=parseGCF(archivePath);
	
	if (((int)archive)<=1)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Archive not found or corrupted",archivePath,desiredLevel);
		return 1;
	}
	GCF * gcf=parseGCF(gcfPath);
	if (((int)gcf)<=1)
	{
		deleteGCF(archive);
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file not found or corrupted",gcfPath,desiredLevel);
		return 1;
	}
	
	if (gcf->appId!=archive->appId)
	{
		deleteGCF(gcf);
		deleteGCF(archive);
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file ID doesn't match archive ID",gcfPath,desiredLevel);
		return 1;
	}
		// 1.0.7 ensure array size 
	DWORD size=gcf->directory->itemCount;
	if (size<archive->directory->itemCount) size=archive->directory->itemCount;

	DWORD * updated=(DWORD*)malloc(4*size);
	
	DirectoryEntry * archiveDirectory=buildDirectory(archive,appName,desiredLevel,0);  
	DirectoryEntry * gcfDirectory=buildDirectory(gcf,appName,desiredLevel,0);  
	
	int nbUpdated=searchDifferences(gcfDirectory,archiveDirectory,gcf,archive,updated);
	
	deleteDirectory(archiveDirectory);
	deleteDirectory(gcfDirectory);	
	
	// add incomplete files (first block index == 0 in archive)
	for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
	{	
		GCFDirectoryEntry entry=gcf->directory->entry[ind];
		if (entry.directoryType && entry.itemSize && !isUpdated(gcf,ind,updated,nbUpdated))
		{
			int archiveInd=getUpdatedFileId(ind,gcf,archive);
			if (archiveInd>-1 && !archive->directoryMap->firstBlockIndex[archiveInd])
			{
				updated[nbUpdated++]=entry.checksumIndex;
			}
		}
	}
 
	DWORD dataSize=0;
	DWORD clustersCount=0;
	for (int ind=0;ind<gcf->directory->itemCount;ind++)
	{	
		if (isUpdated(gcf,ind,updated,nbUpdated))
		{
			DWORD more=(gcf->directory->entry[ind].itemSize/GCF_BLOCK_SIZE)+(gcf->directory->entry[ind].itemSize%GCF_BLOCK_SIZE ? 1 : 0);
			//	printf("adding %d blocks for :%s\n",more,gcf->directory->directoryNames+gcf->directory->entry[ind].nameOffset);
			clustersCount+=more;
			more*=GCF_BLOCK_SIZE;
			dataSize+=more;
		}
	}
	
	int dirLen=serializeGCFDirectory(0,gcf);
	char*directory=(char*)malloc(dirLen);
	serializeGCFDirectory(directory,gcf);
	
	int checkLen=serializeGCFChecksums(0,gcf);
	char*checksums=(char*)malloc(checkLen);
	serializeGCFChecksums(checksums,gcf);
	
	GCF * patch=parseGcf(directory,checksums, 1,6,clustersCount); // patch is a gcf ...
	//printf("setSize : head:%d  data:%d  total:%d\n",serializeGCFHeaders(gcf,0),dataSize,dataSize+serializeGCFHeaders(gcf,0));
	
	patch->fileSize=dataSize+serializeGCFHeaders(patch,0);
	/*
	added in V1.0.6 for V2 compatibility
	*/

 

	// end of 1.0.6

	free(checksums);
	free(directory);
	
	char root[1000];
	char name[1000];
	strcpy(root,gcf->_fileName);
	if (!strchr(root,'\\'))
	{
		strcpy(root,".");
	//	strcpy(root,patchPath);
		strcpy(name,gcf->_fileName);
	}
	else
	{
		*strrchr(root,'\\')=0;
		strcpy(name,gcf->_fileName+strlen(root)+1);
	//	strcpy(root,patchPath);
	}
	
	*strchr(name,'.')=0;
	
	char versions[1000];
	
	sprintf(versions,".%d_to_%d.update",archive->appVersion,gcf->appVersion);
	
	strcat(name,versions);
	
	createGcfFile(patch,patchPath,name, "", appName,desiredLevel);
	
	patch=parseGCF(patch->_fileName);
	buildDataBlockUsageMap(patch,0,0); // 0,0 => no updated files now (was processed before)
	if (taskName) strcpy(taskName,"Building patch");
	copyFiles(gcf,patch,root,commonName,updated,nbUpdated, appName,desiredLevel, taskProgress,pleaseStop,securedUpdates);
	free(updated);

	// check size...
	DWORD realPatchSize=patch->headersSize+patch->dataBlocks->blocksUsed*GCF_BLOCK_SIZE;

	if (realPatchSize<patch->fileSize)
	{
		// correct the file size;
		patch->fileSize=realPatchSize;
		patch->checksum=sumBytes(patch->unknown1)+sumBytes(patch->cfType)+sumBytes(gcf->cfVersion)+sumBytes(patch->appId)+sumBytes(patch->appVersion)+sumBytes(patch->updating)+sumBytes(patch->unknown3)+sumBytes(patch->fileSize)+sumBytes(patch->blockSize)+sumBytes(patch->blockCount);
		updateGcfFileHeaders(patch,0);
		setFileSize64(patch->_fileName,realPatchSize);
		char err[1000];sprintf(err,"At least one file was not included in %s.",name);
		printDebug(DEBUG_LEVEL_ERROR,appName,err,desiredLevel);
		MessageBox(NULL,err,"CF Toolbox",MB_ICONERROR|MB_OK);
	}

	printDebug(DEBUG_LEVEL_INFO,appName,"Update created",patch->_fileName,desiredLevel);
	deleteGCF(gcf);
	deleteGCF(archive);
	deleteGCF(patch);
	return 0;
}

int applyUpdate(char * updatePath, char * gcfPath, char * commonName, char * decryptionKey, char  * appName, int desiredLevel,char * taskName,DWORD * taskProgress, int * pleaseStop,int securedUpdates)
{

	GCF * update=parseGCF(updatePath);
	
	
	if (((int)update)<=1)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Update not found or corrupted",updatePath,desiredLevel);
		return 1;
	}
	GCF * gcf=parseGCF(gcfPath);
	if (((int)gcf)<=1)
	{
		deleteGCF(update);
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file not found or corrupted",gcfPath,desiredLevel);
		return 1;
	}
	
	if (gcf->appId!=update->appId)
	{
		deleteGCF(gcf);
		deleteGCF(update);
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file ID doesn't match update ID",gcfPath,desiredLevel);
		return 1;
	}
	
	if (gcf->appVersion>update->appVersion)
	{
		deleteGCF(gcf);
		deleteGCF(update);
		
		printDebug(DEBUG_LEVEL_ERROR,appName,"Content file is older than update file",gcfPath,desiredLevel);
		return 1;
	}
	
	int dirLen=serializeGCFDirectory(0,update);
	char*directory=(char*)malloc(dirLen);
	serializeGCFDirectory(directory,update);
	
	int checkLen=serializeGCFChecksums(0,update);
	char*checksums=(char*)malloc(checkLen);
	serializeGCFChecksums(checksums,update);
	
	GCF * newGcf=parseGcf(directory,checksums, gcf->cfType,gcf->cfVersion);  
	
	free(checksums);
	free(directory);
	
	char root[1000];
	char name[1000];
	strcpy(root,gcf->_fileName);
	if (!strchr(root,'\\'))
	{
		strcpy(root,".");
		strcpy(name,gcf->_fileName);
	}
	else
	{
		*strrchr(root,'\\')=0;
		strcpy(name,gcf->_fileName+strlen(root)+1);
	}
	
	*strchr(name,'.')=0;
	
	// 1.0.7 ensure array size 
	DWORD size=gcf->directory->itemCount;
	if (size<update->directory->itemCount) size=update->directory->itemCount;

	DWORD * updated=(DWORD*)malloc(4*size);
	
	DirectoryEntry * updateDirectory=buildDirectory(update,appName,desiredLevel,0);  
	DirectoryEntry * gcfDirectory=buildDirectory(gcf,appName,desiredLevel,0);  
	
	int nbUpdated=searchDifferences(gcfDirectory,updateDirectory,gcf,update,updated);
	
	deleteDirectory(updateDirectory);
	deleteDirectory(gcfDirectory);	
	
	if ( decryptionKey && hasDecryptableFiles(gcf,newGcf,updated,nbUpdated))
	{
		if (taskName) strcpy(taskName,"Decrypting");
		decrypt(root,commonName,gcf,newGcf,decryptionKey,updated,nbUpdated ,appName,desiredLevel,taskName,taskProgress);
	}
	
	updateVersion(root,commonName,gcf,newGcf,updated,nbUpdated,appName,desiredLevel,taskName,taskProgress);
	
	gcf=parseGCF(gcf->_fileName);
	
	// build non complete file list
	nbUpdated=getNonCompletedFiles(gcf,updated);
	
	buildDataBlockUsageMap(gcf,0,0); // 0,0 => no updated files now (was processed before)
	if (taskName) strcpy(taskName,"Applying patch");
	copyFiles(update,gcf,root,commonName,updated,nbUpdated, appName,desiredLevel,taskProgress,pleaseStop,securedUpdates);
	free(updated);
	printDebug(DEBUG_LEVEL_INFO,appName,"Content file updated",gcf->_fileName,desiredLevel);	
	deleteGCF(gcf);
	deleteGCF(update);
	return 0;	
}

int downloadApp(char * host, int port, RegistryVector * cdrVector, DWORD requestedAppId, char * root, char * ticket,int ticketLen, char*appName,int desiredLevel,char * contentServerURL,char * taskName,DWORD * taskProgress,DWORD * remainingTime,int * pleaseStop,int securedUpdates,bool showRealBandwidth,bool validate)
{
	if (*pleaseStop) return 2;
	if (taskName && taskProgress)
	{
		strcpy(taskName,"Initiating download");
		*taskProgress=0;
		*remainingTime=0;
	}
	char dbg[500];

	int nbContentServersMax=20;
  	 
 
	ContentDescriptionRecord cdr=getCDR(cdrVector);
	
	CDRApp app=getCDRApp(cdr,requestedAppId);
	if (!app.node) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Unknown GCF/NCF id",requestedAppId,desiredLevel);	
		return 0;
	}
	CDRApp appManifest=app;
	
	if (app.appOfManifestOnlyCache)
	{
		appManifest=getCDRApp(cdr,app.appOfManifestOnlyCache);
	}
	
	if (app.manifestOnlyApp)
	{
		RegistryVector * vector=getCDRAppUserDefinedData(app);
		DWORD primaryCache=atoi(vector->getNode("primarycache")->getStringValue());
		app=getCDRApp(cdr,primaryCache);
	}
//// for tests
//	app.currentVersionId=0;
/// end for tests
	sprintf(dbg,"%s (id : %d  version : %d)",app.installDirName,app.appId,app.currentVersionId);
	printDebug(DEBUG_LEVEL_INFO,appName,"Downloading GCF/NCF",dbg,desiredLevel);
	
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Retrieving Content servers for GCF/NCF",dbg,desiredLevel);		
 	sockaddr_in * contentServers=(sockaddr_in * )malloc(sizeof(sockaddr_in)*nbContentServersMax*2);
 
	int nbContent=getContentServers(  host,   port,contentServers, nbContentServersMax*2, app.appId, app.currentVersionId,appName,desiredLevel);
	
	if (!nbContent) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"No Content server available for GCF/NCF",dbg,desiredLevel);
		free(contentServers);
		return 0;
	}
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Number of content server retrieved",nbContent/2,desiredLevel);
	int res=-3;

	time_t now;
	time(&now);
	srand(now);

	//int retries=nbContent/2;
	int retries=1;
	while (res==-3 && retries)
	{
		retries--;
		
		//do
		{
			now=rand();
			now&=(nbContent/2-1);
			printDebug(DEBUG_LEVEL_DEBUG,appName,"Selecting content server number",now+1,desiredLevel);
		}
		//while (!ping(contentServer[2*now+1]))

		while ((res=downloadApp(contentServers[2*now+1],appManifest,app,ticket,ticketLen,root,appName,desiredLevel,contentServerURL,taskName,taskProgress,remainingTime,pleaseStop,securedUpdates,showRealBandwidth,validate))==1);
	}

//		while ((res=downloadApp(contentServers[1],appManifest,app,ticket,ticketLen,root,appName,desiredLevel,contentServerURL,taskName,taskProgress,pleaseStop,securedUpdates))==1);
	free(contentServers);

	if (res)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Download interrupted",dbg,desiredLevel);
 		return 0;
	}

	if (*pleaseStop)
	{
		printDebug(DEBUG_LEVEL_INFO,appName,"Download paused",dbg,desiredLevel);
 		return 2;
	}

	printDebug(DEBUG_LEVEL_INFO,appName,"Download completed",dbg,desiredLevel);
 	return 1;
  
} 