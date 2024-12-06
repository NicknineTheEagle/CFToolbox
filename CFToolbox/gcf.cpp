 #include <stdio.h>
#include "gcf.h"
#include "socketTools.h"
#ifndef __JUST_PARSING__
#include "zlib\zlib.h"
#include "ServerRegistry.h"
#endif
#include "Debug.h"
#include <sys\types.h> 
#include <sys\stat.h> 
#include "direct.h"

#define GCF_FILE_BLOCK_ZIPPED_AND_CRYPTED	0x0002
#define GCF_FILE_BLOCK_CRYPTED				0x0004
 
int createNcfFolders(GCF * gcf,char * root, GCFDirectoryEntry entry, char * appName, int desiredLevel)
{
	if (*(gcf->directory->directoryNames+entry.nameOffset)==0) printDebug(DEBUG_LEVEL_DEBUG,appName,"Creating NCF common files...",desiredLevel);
	int index=entry.firstIndex;
	
	while (index && index!=-1)
	{
		GCFDirectoryEntry content=gcf->directory->entry[index];
		
		char * name=gcf->directory->directoryNames+content.nameOffset;
			strcat(root,"\\");
			strcat(root,name);		
		if (content.directoryType==0)
		{

			mkdir(root);
			// is a folder
/*			mkdir(name);
			_chdir(name);*/
			int res=createNcfFolders(gcf,root,content,appName,desiredLevel);
			*strrchr(root,'\\')=0;	
			//_chdir("..");
			if (res) return res;
		}
		// files shouln't be preallocated
		else
		{
			// is a file
			
			BUFFEREDHANDLE f=fopen_64(root,"rb"); // create only if not existing (for update)
			if (!f) f=fopen_64(root,"wb");
			*strrchr(root,'\\')=0;				
			if (!f) return -1; // can't create file
			
			fclose_64(f);
			
		}
				
		index=content.nextIndex;
	} 
	return 0;
	
}

int buildNcfCommon(GCF * gcf, char * root, char * commonName,char * appName, int desiredLevel)
{
		
		char path[2001];
		char tmp[256];
		strcpy(tmp,commonName);
		strlwr(tmp);
		
		sprintf(path,"%s\\common",root);
		
		/*
		_getcwd(cwd,1000);
		_chdir(root); 
		
		mkdir("common");
		chdir("common");
		mkdir(tmp);
		chdir(tmp);
		*/
		mkdir(path);
		strcat(path,"\\");
		strcat(path,tmp);
		mkdir(path);
		
		GCFDirectoryEntry rootE=gcf->directory->entry[0];
		while (rootE.parentIndex!=-1) rootE=gcf->directory->entry[rootE.parentIndex];
		
		int res=createNcfFolders(gcf,path,rootE,appName,desiredLevel);
		
		//_chdir(cwd);
		return res;

}

 
DWORD FileSize( const char * szFileName ) 
{ 
/*  struct stat fileStat; 
  int err = stat( szFileName, &fileStat ); 
  if (0 != err) 
  {
 	  return 0; 
  }
  return fileStat.st_size; 
  */
	/// large file support added in 1.0.7
  struct _stati64 fileStat; 
  int err = _stati64( szFileName, &fileStat ); 
  if (0 != err) 
  {
 	  return 0; 
  }
  return (DWORD)fileStat.st_size; 
}

long serializeGCFChecksums(char* buffer,GCF * gcf)
{
	GCFChecksums * checksums=gcf->checksums;
	if (!checksums) return 0;
	DWORD pos=0;
	if (buffer) memcpy(buffer+pos,(char*)&checksums->unknown2,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&checksums->unknown3,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&checksums->itemCount,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&checksums->checksumCount,4);pos+=4;
	
	if (checksums->checksumEntry)
		for (unsigned int ind=0;ind<checksums->itemCount;ind++)
		{
			GCFChecksumEntry entry=checksums->checksumEntry[ind];
			if (buffer) memcpy(buffer+pos,(char*)&entry.checksumCount,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.firstChecksumIndex,4);pos+=4;
		}
		if (checksums->checksum)
		{
			if (buffer) memcpy(buffer+pos,checksums->checksum,4*(checksums->checksumCount ));pos+=4*(checksums->checksumCount);
		}
		if (buffer) memcpy(buffer+pos,checksums->rsaSignature,128);pos+=128;
		return pos;
		
}
long serializeGCFDirectory(char* buffer,GCF * gcf)
{
	GCFDirectory * dir=gcf->directory;
	if (!dir) return 0;
	DWORD pos=0;
	if (buffer) memcpy(buffer+pos,(char*)&dir->unknown1,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->appId,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->appVersion,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->itemCount,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->fileCount,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->chunkSize,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->directorySize,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->nameSize,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->hashedFilesIndexSize,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->copyCount,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->localCount,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->unknown2,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->globalChecksum,4);pos+=4;
	if (buffer) memcpy(buffer+pos,(char*)&dir->checksum,4);pos+=4;
	
	if (dir->entry)
	{
		for (unsigned int ind=0;ind<dir->itemCount;ind++)
		{
			GCFDirectoryEntry entry=dir->entry[ind];
			if (buffer) memcpy(buffer+pos,(char*)&entry.nameOffset,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.itemSize,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.checksumIndex,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.directoryType,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.parentIndex,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.nextIndex,4);pos+=4;
			if (buffer) memcpy(buffer+pos,(char*)&entry.firstIndex,4);pos+=4;
		}
	}
		
	if (dir->directoryNames)
	{
		if (buffer) memcpy(buffer+pos,dir->directoryNames,dir->nameSize);pos+=dir->nameSize;
	}

	if (dir->hashedFilesIndex)
	{
		if (buffer) memcpy(buffer+pos,dir->hashedFilesIndex,4*dir->hashedFilesIndexSize);pos+=(4*dir->hashedFilesIndexSize);
	}

	if (dir->hashedFiles)
	{
		if (buffer) memcpy(buffer+pos,dir->hashedFiles,4*dir->itemCount);pos+=(4*dir->itemCount);
	}
	
	if (dir->copyEntry)
	{
		if (buffer) memcpy(buffer+pos,dir->copyEntry,4*dir->copyCount);pos+=(4*dir->copyCount);
	}
	
	if (dir->localEntry)
	{
		if (buffer) memcpy(buffer+pos,dir->localEntry,4*dir->localCount);pos+=(4*dir->localCount);
	}	
	
	return pos;
}

int parseGCFDataBlocks(GCFDataBlocks * dataBlocks, DWORD cfType,BUFFEREDHANDLE file)
{
	dataBlocks->blockUsage=0;
	fread_64(&(dataBlocks->appVersion),4,1,file);
	if (cfType==1)
	{
		fread_64(&(dataBlocks->blockCount),4,1,file);
		fread_64(&(dataBlocks->blockSize),4,1,file);
		fread_64(&(dataBlocks->firstBlockOffset),4,1,file);
		fread_64(&(dataBlocks->blocksUsed),4,1,file);
		fread_64(&(dataBlocks->checksum),4,1,file);
		DWORD computed=dataBlocks->blockCount+dataBlocks->blockSize+dataBlocks->firstBlockOffset+dataBlocks->blocksUsed;
		//printf("1 %d %d\n",dataBlocks->checksum,computed);
		if (dataBlocks->checksum!=computed) return 1;
	}
	else
	{
		dataBlocks->blockCount=0;
		dataBlocks->blockSize=0;
		dataBlocks->firstBlockOffset=0;
		dataBlocks->blocksUsed=0;
		dataBlocks->checksum=0;
	}
	return 0;
}

void printGCFDatablocks(GCFDataBlocks * dataBlocks,int full=0)
{
	printf("  <dataBlocks\n");
	printf("    appVersion=\"%d\"\n",dataBlocks->appVersion);
	printf("    blockCount=\"%d\"\n",dataBlocks->blockCount);
	printf("    blockSize=\"%d\"\n",dataBlocks->blockSize);
	printf("    firstBlockOffset=\"%d\"\n",dataBlocks->firstBlockOffset);
	printf("    blocksUsed=\"%d\"\n",dataBlocks->blocksUsed);
	printf("    checksum=\"%d\"/>\n",dataBlocks->checksum);
}

int parseGCFChecksumEntry(GCFChecksumEntry * entry, BUFFEREDHANDLE file)
{
	fread_64(&(entry->checksumCount),4,1,file);
	fread_64(&(entry->firstChecksumIndex),4,1,file);
	return 0;
}

int parseGCFChecksums(GCFChecksums * checksums, BUFFEREDHANDLE file)
{
	fread_64(&(checksums->unknown1),4,1,file);
	fread_64(&(checksums->checksumSize),4,1,file);
	fread_64(&(checksums->unknown2),4,1,file);
	fread_64(&(checksums->unknown3),4,1,file);
	fread_64(&(checksums->itemCount),4,1,file);
	fread_64(&(checksums->checksumCount),4,1,file);
	checksums->checksumEntry=0;
	
	if (checksums->itemCount)
	{
		checksums->checksumEntry=(GCFChecksumEntry*)malloc(checksums->itemCount*sizeof(GCFChecksumEntry));
		for (unsigned int ind=0;ind<checksums->itemCount;ind++)
		{
			if (parseGCFChecksumEntry(&(checksums->checksumEntry[ind]),file))
				return 1;
		}
	}
	
	
	checksums->checksum=(DWORD *)malloc((checksums->checksumCount)*sizeof(DWORD));
	fread_64(checksums->checksum,4,checksums->checksumCount,file);
	fread_64(checksums->rsaSignature,1,128,file);
	return 0;
}

void printGCFChecksums(GCFChecksums * checksums,int full=0)
{
	printf("  <checksums\n");
	printf("    unknown1=\"#%x\"\n",checksums->unknown1);
	printf("    checksumSize=\"%d\"\n",checksums->checksumSize);
	printf("    unknown2=\"#%x\"\n",checksums->unknown2);
	printf("    unknown3=\"#%x\"\n",checksums->unknown3);
	printf("    itemCount=\"%d\"\n",checksums->itemCount);
	printf("    checksumCount=\"%d\">\n",checksums->checksumCount);
	
	unsigned int nb=checksums->itemCount;
	if (!full && nb>3) nb=3;
	for (unsigned int ind=0;ind<nb;ind++)
	{
		printf("      <checksumEntry pos=\"%d\" checksumCount=\"%d\">%d</checksumEntry>\n",ind,checksums->checksumEntry[ind].checksumCount,checksums->checksumEntry[ind].firstChecksumIndex);
	}
	if (nb!=checksums->itemCount)
		printf("      <!-- ... -->\n");
	
	nb=checksums->checksumCount;
	if (!full && nb>3) nb=3;
	for (int ind=0;ind<nb;ind++)
	{
		printf("      <checksum pos=\"%d\">#%x</checksum>\n",ind,checksums->checksum[ind]);
	}
	if (nb!=checksums->checksumCount)
		printf("      <!-- ... -->\n");
		printf("      <rsaSignature><![CDATA[\n");
		hexaPrintf(checksums->rsaSignature,128,0);
		printf("      ]]></rsaSignature>\n");
	printf("  </checksums>\n");
}

int parseGCFDirectoryMap(GCFDirectoryMap * directoryMap,DWORD itemCount, BUFFEREDHANDLE file)
{
	fread_64(&(directoryMap->unknown1),4,1,file);
	fread_64(&(directoryMap->unknown2),4,1,file);
	directoryMap->firstBlockIndex=0;
	
	if (itemCount)
	{
		directoryMap->firstBlockIndex=(DWORD *)malloc(itemCount*sizeof(DWORD));
		fread_64(directoryMap->firstBlockIndex,4,itemCount,file);
	}
	return 0;
}

void printGCFDirectoryMap(GCFDirectory * directory,GCFDirectoryMap * map,int full=0)
{
	printf("  <directoryMap\n");
	printf("    unknown1=\"#%x\"\n",map->unknown1);
	printf("    unknown2=\"#%x\">\n",map->unknown2);
	
	unsigned int nb=directory->itemCount;
	if (!full && nb>3) nb=3;
	for (unsigned int ind=0;ind<nb;ind++)
	{
		printf("      <firstBlockIndex pos=\"%d\">%d</firstBlockIndex>\n",ind,map->firstBlockIndex[ind]);
	}
	if (nb!=directory->itemCount)
		printf("      <!-- ... -->\n");
	
	printf("  </directoryMap>\n");
}


int parseGCFDirectoryEntry(GCFDirectoryEntry * entry,BUFFEREDHANDLE file)
{
	fread_64(&(entry->nameOffset),4,1,file);
	fread_64(&(entry->itemSize),4,1,file);
	fread_64(&(entry->checksumIndex),4,1,file);
	fread_64(&(entry->directoryType),4,1,file);
	fread_64(&(entry->parentIndex),4,1,file);
	fread_64(&(entry->nextIndex),4,1,file);
	fread_64(&(entry->firstIndex),4,1,file);
	
	return 0;
}

void printGCFDirectoryEntry(GCFDirectory * directory, GCFDirectoryEntry * entry,int pos, int full=0)
{
	printf("      <entry pos=\"%d\"\n",pos);
	printf("        nameOffset=\"%d\"\n",entry->nameOffset);
	printf("        itemSize=\"%d\"\n",entry->itemSize);
	printf("        checksumIndex=\"%d\"\n",entry->checksumIndex);
	printf("        directoryType=\"#%x\"\n",entry->directoryType);
	printf("        parentIndex=\"%d\"\n",entry->parentIndex);
	printf("        nextIndex=\"%d\"\n",entry->nextIndex);
	printf("        firstIndex=\"%d\">\n",entry->firstIndex);
	printf("        %s\n",directory->directoryNames+entry->nameOffset);
	printf("      </entry>\n");
	
}

int parseGCFDirectory(GCFDirectory * directory,BUFFEREDHANDLE file)
{
	fread_64(&(directory->unknown1),4,1,file);
	fread_64(&(directory->appId),4,1,file);
	fread_64(&(directory->appVersion),4,1,file);
	fread_64(&(directory->itemCount),4,1,file);
	fread_64(&(directory->fileCount),4,1,file);
	fread_64(&(directory->chunkSize),4,1,file);
	fread_64(&(directory->directorySize),4,1,file);
	fread_64(&(directory->nameSize),4,1,file);
	fread_64(&(directory->hashedFilesIndexSize),4,1,file);
	fread_64(&(directory->copyCount),4,1,file);
	fread_64(&(directory->localCount),4,1,file);
	fread_64(&(directory->unknown2),4,1,file);
	fread_64(&(directory->globalChecksum),4,1,file);
	fread_64(&(directory->checksum),4,1,file);
	
	/*	DWORD computed=directory->unknown1+directory->appId+directory->appVersion+directory->itemCount+directory->fileCount+directory->unknown2+directory->directorySize+
	directory->nameSize+directory->info1Count+directory->copyCount+directory->localCount+directory->unknown3+directory->unknown4;
	printf("checksum2 : %d\n",directory->checksum);
	printf("computed2 : %d\n",computed);
	*/
	directory->entry=0;
	directory->directoryNames=0;
	directory->hashedFilesIndex=0;
	directory->hashedFiles=0;
	directory->copyEntry=0;
	directory->localEntry=0;
	
	if (directory->itemCount)
	{
		directory->entry=(GCFDirectoryEntry*)malloc(directory->itemCount*sizeof(GCFDirectoryEntry));
		for (unsigned int ind=0;ind<directory->itemCount;ind++)
		{
			if (parseGCFDirectoryEntry(&(directory->entry[ind]),file))
				return 1;
		}
	}
	
	if (directory->nameSize)
	{
		directory->directoryNames=(char *)malloc(directory->nameSize);
		fread_64(directory->directoryNames,directory->nameSize,1,file);
	}
	if (directory->hashedFilesIndexSize)
	{
		directory->hashedFilesIndex =(DWORD *)malloc(directory->hashedFilesIndexSize*sizeof(DWORD));
		fread_64(directory->hashedFilesIndex,4,directory->hashedFilesIndexSize,file);
	}
	if (directory->itemCount)
	{
		directory->hashedFiles =(DWORD *)malloc(directory->itemCount*sizeof(DWORD));
		fread_64(directory->hashedFiles ,4,directory->itemCount,file);
	}
	if (directory->copyCount)
	{
		directory->copyEntry=(DWORD *)malloc(directory->copyCount*sizeof(DWORD));
		fread_64(directory->copyEntry,4,directory->copyCount,file);
	}
	if (directory->localCount)
	{
		directory->localEntry=(DWORD *)malloc(directory->localCount*sizeof(DWORD));
		fread_64(directory->localEntry,4,directory->localCount,file);
	}
	return 0;
}

void printGCFDirectory(GCFDirectory * map,int full=0)
{
	printf("  <directory\n");
	printf("    unknown1=\"#%x\"\n",map->unknown1);
	printf("    appId=\"%d\"\n",map->appId);
	printf("    appVersion=\"%d\"\n",map->appVersion);
	printf("    itemCount=\"%d\"\n",map->itemCount);
	printf("    fileCount=\"%d\"\n",map->fileCount);
	printf("    chunkSize=\"%d\"\n",map->chunkSize);
	printf("    directorySize=\"%d\"\n",map->directorySize);
	printf("    nameSize=\"%d\"\n",map->nameSize);
	printf("    hashedFilesIndexSize=\"%d\"\n",map->hashedFilesIndexSize);
	printf("    copyCount=\"%d\"\n",map->copyCount);
	printf("    localCount=\"%d\"\n",map->localCount);
	printf("    unknown2=\"#%x\"\n",map->unknown2);
	printf("    globalChecksum=\"#%x\"\n",map->globalChecksum);
	printf("    checksum=\"%d\">\n",map->checksum);
	unsigned int nb=map->itemCount;
	if (!full && nb>3) nb=3;
	for (unsigned int ind=0;ind<nb;ind++)
	{
		printGCFDirectoryEntry(map,&(map->entry[ind]),ind);
	}
	if (nb!=map->itemCount)
		printf("      <!-- ... -->\n");
	
	nb=map->hashedFilesIndexSize;
	if (!full && nb>3) nb=3;
	for (int ind=0;ind<nb;ind++)
	{
		printf("      <hashedFilesIndex pos=\"%d\">#%x</hashedFilesIndex>\n",ind,map->hashedFilesIndex[ind]);
	}
	if (nb!=map->hashedFilesIndexSize)
		printf("      <!-- ... -->\n");
	
	nb=map->itemCount;
	if (!full && nb>3) nb=3;
	for (int ind=0;ind<nb;ind++)
	{
		printf("      <hashedFiles  pos=\"%d\">#%x</hashedFiles>\n",ind,map->hashedFiles[ind]);
	}
	if (nb!=map->itemCount)
		printf("      <!-- ... -->\n");
	
	nb=map->copyCount;
	if (!full && nb>3) nb=3;
	for (int ind=0;ind<nb;ind++)
	{
		printf("      <copy pos=\"%d\">%d</copy>\n",ind,map->copyEntry[ind]);
	}
	if (nb!=map->copyCount)
		printf("      <!-- ... -->\n");
	
	nb=map->localCount;
	if (!full && nb>3) nb=3;
	for (int ind=0;ind<nb;ind++)
	{
		printf("      <local pos=\"%d\">%d</local>\n",ind,map->localEntry[ind]);
	}
	if (nb!=map->localCount)
		printf("      <!-- ... -->\n");
	
	printf("  </directory>\n");
}


int parseGCFFragmentationMap(GCFFragmentationMap * fragmentationMap,BUFFEREDHANDLE file)
{
	fread_64(&(fragmentationMap->blockCount),4,1,file);
	fread_64(&(fragmentationMap->firstUnusedEntry),4,1,file);
	fread_64(&(fragmentationMap->terminator),4,1,file);
	fread_64(&(fragmentationMap->checksum),4,1,file);
	
	DWORD computed=fragmentationMap->blockCount+fragmentationMap->firstUnusedEntry+fragmentationMap->terminator;
	//printf("2 %d %d\n",fragmentationMap->checksum,computed);
	fragmentationMap->nextDataBlockIndex=0;
	if (fragmentationMap->checksum!=computed) return 1;
	if (fragmentationMap->blockCount>0)
	{
		fragmentationMap->nextDataBlockIndex=(DWORD*)malloc(fragmentationMap->blockCount*sizeof(DWORD));
		fread_64(fragmentationMap->nextDataBlockIndex,4,fragmentationMap->blockCount,file);
	}
	return 0;
}

void printGCFFragmentationMap(GCFFragmentationMap * map,int full=0)
{
	printf("  <fragmentationMap\n");
	printf("    blockCount=\"%d\"\n",map->blockCount);
	printf("    firstUnusedEntry=\"%d\"\n",map->firstUnusedEntry);
	printf("    terminator=\"%d\"\n",map->terminator);
	printf("    checksum=\"%d\">\n",map->checksum);
	
	if (map->blockCount>0)
	{
		unsigned int nb=map->blockCount;
		if (!full && nb>3) nb=3;
		for (unsigned int ind=0;ind<nb;ind++)
		{
			printf("      <nextDataBlockIndex pos=\"%d\">%d</nextDataBlockIndex>\n",ind,map->nextDataBlockIndex[ind]);
		}
		if (nb!=map->blockCount)
			printf("      <!-- ... -->\n");
	}
	
	printf("  </fragmentationMap>\n");
}

int parseGCFBlock(GCFBlock * block,BUFFEREDHANDLE file)
{
	fread_64(&(block->entryType),4,1,file);
	fread_64(&(block->fileDataOffset),4,1,file);
	fread_64(&(block->fileDataSize),4,1,file);
	fread_64(&(block->firstDataBlockIndex),4,1,file);
	fread_64(&(block->nextBlockEntryIndex),4,1,file);
	fread_64(&(block->previousBlockEntryIndex),4,1,file);
	fread_64(&(block->directoryIndex),4,1,file);
	return 0;
}

int parseGCFBlocksHeader(GCFBlocks * blocks, BUFFEREDHANDLE file)
{
	fread_64(&(blocks->blockCount),4,1,file);
	fread_64(&(blocks->blockUsed),4,1,file); 
	fread_64(&(blocks->lastUsedBlock),4,1,file);
	fread_64(&(blocks->unknown2),4,1,file);
	fread_64(&(blocks->unknown3),4,1,file);
	fread_64(&(blocks->unknown4),4,1,file);
	fread_64(&(blocks->unknown5),4,1,file);
	fread_64(&(blocks->checksum),4,1,file);
	
	DWORD computed=blocks->blockCount+blocks->blockUsed+blocks->lastUsedBlock+blocks->unknown2+blocks->unknown3+blocks->unknown4+blocks->unknown5;
	
	blocks->block=0;
	//printf("3 %d %d\n",blocks->checksum,computed);	
	if (blocks->checksum!=computed) return 1;
	
	if (blocks->blockCount>0)
	{
		blocks->block=(GCFBlock*)malloc(blocks->blockCount*sizeof(GCFBlock));
		for (unsigned int ind=0;ind<blocks->blockCount;ind++)
		{
			if (parseGCFBlock(&(blocks->block[ind]),file))
				return 1;
		}
	}
	
	return 0;
}

void printGCFBlock(GCFBlock * block,int pos, int full=0)
{
	printf("      <block pos=\"%d\"\n",pos);
	printf("        entryType=\"#%x\"\n",block->entryType);
	printf("        fileDataOffset=\"%d\"\n",block->fileDataOffset);
	printf("        fileDataSize=\"%d\"\n",block->fileDataSize);
	printf("        firstDataBlockIndex=\"%d\"\n",block->firstDataBlockIndex);
	printf("        nextBlockEntryIndex=\"%d\"\n",block->nextBlockEntryIndex);
	printf("        previousBlockEntryIndex=\"%d\"\n",block->previousBlockEntryIndex);
	printf("        directoryIndex=\"%d\"/>\n",block->directoryIndex);
}

void printGCFBlocksHeader(GCFBlocks * blocks,int full=0)
{
	printf("  <blocks\n");
	printf("    blockCount=\"%d\"\n",blocks->blockCount);
	printf("    blockUsed=\"%d\"\n",blocks->blockUsed);
	printf("    lastUsedBlock=\"%d\"\n",blocks->lastUsedBlock);
	printf("    unknown2=\"#%x\"\n",blocks->unknown2);
	printf("    unknown3=\"#%x\"\n",blocks->unknown3);
	printf("    unknown4=\"#%x\"\n",blocks->unknown4);
	printf("    unknown5=\"#%x\"\n",blocks->unknown5);
	printf("    checksum=\"%d\">\n",blocks->checksum);
	
	if (blocks->block)
	{
		//for (unsigned int ind=0;ind<blocks->blockCount;ind++)
		unsigned int nb=blocks->blockCount;
		if (!full && nb>3) nb=3;
		for (unsigned int ind=0;ind<nb;ind++)
		{
			printGCFBlock(&(blocks->block[ind]),ind);
		}
		if (nb!=blocks->blockCount)
			printf("      <!-- ... -->\n");
	}
	
	printf("  </blocks>\n");
}

DWORD sumBytes(DWORD data)
{
	DWORD sum=0;
	for (int ind=0;ind<4;ind++)
	{
		DWORD add=data&0xFF;
		sum+=add;
		data>>=8;
	}
	return sum;
}

int parseGCF(GCF * gcf, BUFFEREDHANDLE file)
{
	// obtain file size.
/*	fseek (file , 0 , SEEK_END);
	DWORD fileSize= ftell (file);
	rewind (file);
*/
	/// 1.0.7 : large file support
	DWORD fileSize=FileSize(gcf->_fileName);
	
	fread_64(&(gcf->unknown1),4,1,file);
	fread_64(&(gcf->cfType),4,1,file);
	fread_64(&(gcf->cfVersion),4,1,file);
	fread_64(&(gcf->appId),4,1,file);
	fread_64(&(gcf->appVersion),4,1,file);
	fread_64(&(gcf->updating),4,1,file);
	fread_64(&(gcf->unknown3),4,1,file);
	fread_64(&(gcf->fileSize),4,1,file);
	fread_64(&(gcf->blockSize),4,1,file);
	fread_64(&(gcf->blockCount),4,1,file);
	fread_64(&(gcf->checksum),4,1,file);
	gcf->blocks=0;
	gcf->fragmentationMap=0;
	gcf->directory=0;
	gcf->directoryMap=0;
	gcf->checksums=0;
	gcf->dataBlocks=0;
	DWORD computed=sumBytes(gcf->unknown1)+sumBytes(gcf->cfType)+sumBytes(gcf->cfVersion)+sumBytes(gcf->appId)+sumBytes(gcf->appVersion)+
		sumBytes(gcf->updating)+sumBytes(gcf->unknown3)+sumBytes(gcf->fileSize)+sumBytes(gcf->blockSize)+sumBytes(gcf->blockCount);
	
	if (gcf->checksum!=computed) return 1;
		
		
	if(gcf->fileSize && fileSize!=gcf->fileSize) return 1;
	

	if (gcf->cfType==1)
	{
		gcf->blocks=new GCFBlocks;
		// only if gcf. ncf has no content
		if (parseGCFBlocksHeader(gcf->blocks,file))
		{
			return 1;	
		}
		gcf->fragmentationMap=new GCFFragmentationMap;
		if (parseGCFFragmentationMap(gcf->fragmentationMap,file))
		{
			return 1;	
		}
	}
	/*else
	{
		printf("ncf");
	}*/
	gcf->directory=new GCFDirectory;
	if (parseGCFDirectory(gcf->directory,file))
	{
		return 1;	
	}
	gcf->directoryMap=new GCFDirectoryMap;
	if (parseGCFDirectoryMap(gcf->directoryMap,gcf->directory->itemCount,file))
	{
		return 1;	
	}
	gcf->checksums=new GCFChecksums;
	if (parseGCFChecksums(gcf->checksums,file))
	{
		return 1;	
	}
	gcf->dataBlocks=new GCFDataBlocks;
	if (parseGCFDataBlocks(gcf->dataBlocks,gcf->cfType,file))
	{
		return 1;	
	}
	gcf->headersSize=serializeGCFHeaders(gcf,0);
	
	return 0;
}

void printGCFXML(GCF * gcf,int full)
{
	if (((int)gcf)<=1) return;
	if (gcf->cfType==1)
		 printf("<gcf\n");
		else
		printf("<ncf\n");
	printf("  unknown1=\"#%x\"\n",gcf->unknown1);
	printf("  cfType=\"%d\"\n",gcf->cfType);
	printf("  cfVersion=\"%d\"\n",gcf->cfVersion);
	printf("  appId=\"%d\"\n",gcf->appId);
	printf("  appVersion=\"%d\"\n",gcf->appVersion);
	printf("  updating=\"%d\"\n",gcf->updating );
	printf("  unknown3=\"#%x\"\n",gcf->unknown3);
	printf("  fileSize=\"%d\"\n",gcf->fileSize);
	printf("  blockSize=\"%d\"\n",gcf->blockSize);
	printf("  blockCount=\"%d\"\n",gcf->blockCount);
	printf("  checksum=\"#%x\">\n",gcf->checksum);
	
	if (gcf->blocks) printGCFBlocksHeader(gcf->blocks,full);
	if (gcf->fragmentationMap) printGCFFragmentationMap(gcf->fragmentationMap,full);
	if (gcf->directory) printGCFDirectory(gcf->directory,full);
	if (gcf->directoryMap) printGCFDirectoryMap(gcf->directory,gcf->directoryMap,full);
	if (gcf->checksums) printGCFChecksums(gcf->checksums,full);
	if (gcf->dataBlocks) printGCFDatablocks(gcf->dataBlocks,full);
	if (gcf->cfType==1)
		 	printf("</gcf>\n");
		else
			printf("</ncf>\n");
	
}


void deleteGCF(GCF * gcf)
{
	if (gcf->blocks)
	{
		if (gcf->blocks->block)
			free(gcf->blocks->block);
		delete gcf->blocks;
	}
	if (gcf->fragmentationMap)
	{
		if (gcf->fragmentationMap->nextDataBlockIndex)
			free(gcf->fragmentationMap->nextDataBlockIndex);
		delete gcf->fragmentationMap;
	}
	if (gcf->directory)
	{
		if (gcf->directory->entry)
			free(gcf->directory->entry);
		if (gcf->directory->directoryNames)
			free(gcf->directory->directoryNames);
		
		if (gcf->directory->hashedFilesIndex )
			free(gcf->directory->hashedFilesIndex);
		if (gcf->directory->hashedFiles )
			free(gcf->directory->hashedFiles );
		if (gcf->directory->copyEntry)
			free(gcf->directory->copyEntry);
		if (gcf->directory->localEntry)
			free(gcf->directory->localEntry);
		
		delete gcf->directory;
	}
	
	if (gcf->directoryMap)
	{
		if (gcf->directoryMap->firstBlockIndex)
			free(gcf->directoryMap->firstBlockIndex);
		delete gcf->directoryMap;
	}
	
	if (gcf->checksums)
	{
		if (gcf->checksums->checksumEntry)
			free(gcf->checksums->checksumEntry);
		
		if (gcf->checksums->checksum)
			free(gcf->checksums->checksum);
		delete gcf->checksums;
	}
	if (gcf->dataBlocks)
	{
		if (gcf->dataBlocks->blockUsage) free(gcf->dataBlocks->blockUsage);
		delete(gcf->dataBlocks);
	}	
	delete gcf;
}


GCF * parseGCF(char * fileName)
{
	BUFFEREDHANDLE file=fopen_64(fileName,"rb");
	if (!file) return 0;
	
	GCF * gcf=new GCF;
	strcpy(gcf->_fileName,fileName);
	gcf->isArchive=0;

	if (parseGCF(gcf,file)) 
	{
		deleteGCF(gcf);
		fclose_64(file);
		return (GCF * )1;	
	}
	fclose_64(file);
	//	printGCFDirectory(gcf->directory,1);
	return gcf;
}

DWORD getFirstDataBlockIndex(GCF * gcf, DWORD fileId,DWORD blockId,DWORD *nbBlockOffset, DWORD * transferMode)
{
	// get a block for this file
	DWORD block=gcf->blockCount;
	for (DWORD ind=0;ind<gcf->blockCount;ind++)
	{
		if (gcf->blocks->block[ind].directoryIndex==fileId && gcf->blocks->block[ind].entryType&0x8000 ) // &0x8000 to be sure the block is used
		{
			block=ind;
			break;
		}
	}
 	//printf("a block Id : %d\n",block);

	if (block==gcf->blockCount) return -1; // not found
	
	
	// find first block
	while (gcf->blocks->block[block].previousBlockEntryIndex!=gcf->blockCount) block=gcf->blocks->block[block].previousBlockEntryIndex;
	

	// transferMode can be used for contentServer
	if (transferMode)
	{
		*transferMode=1;
		if (gcf->blocks->block[block].entryType&2) *transferMode=2;
		if (gcf->blocks->block[block].entryType&4) *transferMode=3;
	}

	//printf("first block Id : %d\n",block);

	// go to needed block
	DWORD offset=blockId*gcf->blockSize;
 	while ((block!=gcf->blockCount) && (offset>=gcf->blocks->block[block].fileDataOffset+gcf->blocks->block[block].fileDataSize))
	{
		block=gcf->blocks->block[block].nextBlockEntryIndex;
		DWORD offset=blockId*gcf->blockSize;
	}
	if (block==gcf->blockCount) return -1; // not found
	 
 	DWORD dataBlock=-1;
	if (gcf->blocks->block[block].firstDataBlockIndex!=gcf->blockCount)
		dataBlock=gcf->blocks->block[block].firstDataBlockIndex;

 	 //printf(" returned block : %d\n",block);
	if (gcf->blocks->block[block].fileDataOffset%gcf->blockSize)
	{
		printDebug(DEBUG_LEVEL_ERROR,"GCF","Block offset is not a multiple of data block size",gcf->_fileName,DEBUG_LEVEL_ERROR);
	}
	*nbBlockOffset=gcf->blocks->block[block].fileDataOffset/gcf->blockSize;
	 //printf("offset blkid:%d   (%d / %d)\n",*nbBlockOffset,gcf->blocks->block[block].fileDataOffset,gcf->blockSize);
	//printf("       --- FIRST DATA BLOCK %d \n",dataBlock);
	return dataBlock;
}

_int64 getFragmentedBlockOffset(GCF * gcf, DWORD fileId, DWORD blockId, char * appName,int desiredDebugLevel,DWORD * transferMode=0)
{ 	
	DWORD offsetBlockId;
	//printf("     --- FRAGMENTED BLOCK %d fichier %d \n",blockId,fileId);
	DWORD block=getFirstDataBlockIndex(gcf,fileId,blockId,&offsetBlockId,transferMode);
	
	 //printf("offsetBlockId:%d\n",offsetBlockId);
	if (block==-1) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"First data block index not found for file", fileId, desiredDebugLevel);
		return 0;
	}

	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
  	for (DWORD ind=0;(ind<blockId-offsetBlockId) && (block!=terminator && block<gcf->fragmentationMap->blockCount);ind++)
	{

		block=gcf->fragmentationMap->nextDataBlockIndex[block];
	}
	//printf("       ---  REAL FRAGMENTED BLOCK %d\n",block);
	if (block==terminator || block>=gcf->dataBlocks->blockCount) 
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Next fragmented data block index not found for file", fileId, desiredDebugLevel);
		return 0;
	}
	if (block>=gcf->fragmentationMap->blockCount)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"Next fragmented data block index >= blockCount", fileId, desiredDebugLevel);
		return 0;
	}
	return (_int64)gcf->dataBlocks->firstBlockOffset+(_int64)block*(_int64)gcf->dataBlocks->blockSize;	
} 
void getFilePath(GCF * gcf,DWORD fileId,char * path)
{
	DWORD pos=0;
	GCFDirectoryEntry entry=gcf->directory->entry[fileId];
	if (entry.parentIndex==-1)
	{
		strcpy(path,gcf->directory->directoryNames+entry.nameOffset);
		return;
	}
	getFilePath(gcf,entry.parentIndex,path);
	strcat(path,"\\");
	strcat(path,gcf->directory->directoryNames+entry.nameOffset);
}

 
DWORD findFileId(GCF * gcf,DWORD rootIndex,char * path)
{
	char * nextPath=strchr(path,'/');
	if (nextPath)
	{
		// there is at least 1 subfolder
		*nextPath=0; // we cut
		nextPath++;
	}
	for (unsigned int ind=rootIndex;ind<gcf->directory->itemCount;ind++) // the content should be after the parent ...
	{
		if (gcf->directory->entry[ind].parentIndex==rootIndex)
		{
			if (!_stricmp(path,gcf->directory->directoryNames+gcf->directory->entry[ind].nameOffset))
			{
				if (!nextPath) return ind;
				return findFileId(gcf,ind,nextPath);
			}
		}
	}
	return 0; // not found , returning root
}

DWORD getFileId(GCF * gcf, char * path)
{
// old method without gcf files hash	
	char * p=(char*)malloc(strlen(path)+1);
	strcpy(p,path);
 
	DWORD id=0;

	id=findFileId(gcf,0,p);

	free(p);
	return id;
}

 

int searchFileIds(GCF * gcf, char * filename, DWORD * ids, int maxResults)
{
	if (!maxResults) return 0;

	int nbResults=0;
	char * fileName=(char*)malloc(strlen(filename)+1);
	strcpy(fileName,filename);
  	strlwr(filename);

	
	// compute hashedFilesIndex index
	DWORD hash=jenkinsHash((unsigned char *)fileName,strlen(fileName),1);
	DWORD hashIndex=hash&(gcf->directory->hashedFilesIndexSize-1);

	// get the hashedFiles index
	DWORD hashedFileIndex=gcf->directory->hashedFilesIndex[hashIndex];
	if (hashedFileIndex==-1)
	{
		// file not found
		free(fileName);
		return 0;
	}
	hashedFileIndex-=gcf->directory->hashedFilesIndexSize;

	// search for the file in these files subset.
 
	DWORD stop=0;
	do
	{
		DWORD hashedValue=gcf->directory->hashedFiles[hashedFileIndex];
		DWORD fileId=hashedValue&0x7FFFFFFF;
		stop=hashedValue&0x80000000;

		if (!stricmp(fileName,gcf->directory->directoryNames+gcf->directory->entry[fileId].nameOffset))
		{
			// file found 
			ids[nbResults++]=fileId;
		}
		hashedFileIndex++;
	} while (!stop && nbResults<maxResults);
	
 
	free(fileName);
	return nbResults;
}
/*
DWORD getFileId(GCF * gcf, char * path)
{
	// new method with gcf files hash
 
	// clean the path and get file name
	int correct=(*path!='\\' && *path!='/' ? 1 : 0);
	char * p=(char*)malloc(strlen(path)+1+correct);
	if (correct) *p='\\';
	strcpy(p+correct,path);

	char * slash=0;
	while (slash=strchr(p,'/'))
		*slash='\\';
 
	char * fileName=strrchr(p,'\\')+1;
	if ((int)fileName==1) fileName=p;
	
	DWORD results[200]; // should be enough
	int nbFiles=searchFileIds(gcf,fileName,results,200);
	
	char filePath[1000];

	for (int ind=0;ind<nbFiles;ind++)
	{
		getFilePath(gcf,results[ind],filePath);
		
		if (!stricmp(filePath,p))
		{
			// file found
			free(p);
			return results[ind];
		}
	}
	
	// file not found
	free(p);
	return 0;
}
 */
DWORD getGCFBlock(GCF * gcf, BUFFEREDHANDLE file, DWORD fileId, DWORD blockId, char * buffer,char * appName,int desiredDebugLevel,DWORD * transferMode=0)
{
	if (blockId*gcf->blockSize>=gcf->directory->entry[fileId].itemSize) return 0;
 	_int64 pos=getFragmentedBlockOffset(gcf,fileId,blockId,appName,desiredDebugLevel,transferMode);

	if (pos==0) 
	{
		char path2[1000];
		getFilePath(gcf,fileId,path2);
		//printf("requested block %d (%d > %d) of file size %d : %s (%s)\n",blockId,blockId*0x2000,(blockId+1)*0x2000,gcf->directory->entry[fileId].itemSize,path2,gcf->_fileName);
		char debug[1000];
		//sprintf(debug,"Data block index %d (%d > %d) of file %s not found in %s ",blockId ,blockId*0x2000,(blockId+1)*0x2000,path2,gcf->_fileName);
		sprintf(debug,"Data block index %d of file %s not found in %s ",blockId  ,path2,gcf->_fileName);
		printDebug(DEBUG_LEVEL_ERROR,appName,debug, desiredDebugLevel);
		return 0;
	}
	fseek_64(file,pos,SEEK_SET);
	DWORD read=fread_64(buffer,1,gcf->blockSize,file);



	if ((blockId+1)*gcf->blockSize>gcf->directory->entry[fileId].itemSize)
		return gcf->directory->entry[fileId].itemSize%gcf->blockSize;
	else
		return read;
}

DWORD getFileBlock(GCF * gcf, BUFFEREDHANDLE file, DWORD fileId, DWORD blockId, char * buffer)
{
	if (blockId*0x2000>=gcf->directory->entry[fileId].itemSize) return 0;
	_int64 pos=(_int64)0x2000*(_int64)blockId;
	memset(buffer,0,0x2000);
	
	
	DWORD size=0;
	
	if ((blockId+1)*0x2000>gcf->directory->entry[fileId].itemSize)
		size=gcf->directory->entry[fileId].itemSize%0x2000;
	else
		size=0x2000;
	
	
	fseek_64(file,pos,SEEK_SET);
	DWORD read=fread_64(buffer,1,size,file);
	
	return read;
}




//getServerRegistry()->getKey("/TopKey/ServerConfig")->getValue("GCFPath")->getString()
DWORD getGCFBlocks(GCF * gcf,char * gcfPath,char * appCommonName, DWORD fileId, DWORD blockStart,DWORD nbBlockRequested, char * buffer,char * appName, int desiredDebugLevel,DWORD * transferMode)
{
#ifndef __JUST_PARSING__

	DWORD totalRead=0;
	if (transferMode) *transferMode=0;
	if (gcf->cfType==1)
	{
		
		BUFFEREDHANDLE file=fopen_64(gcf->_fileName,"rb");
		if (!file)
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Can't open file", gcf->_fileName, desiredDebugLevel);
			return 0;
		}
 		for (DWORD ind=0;ind<nbBlockRequested;ind++)
		{
 			DWORD read=0;
			if (!(read=getGCFBlock(gcf,file,fileId,blockStart+ind,buffer+(gcf->dataBlocks->blockSize*ind),appName, desiredDebugLevel,transferMode)))
				break;

			totalRead+=read;
		}
		fclose_64(file);
	}
	else
	{

		if (!appCommonName) 
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"gamedir not found in registry for ncf file", gcf->_fileName, desiredDebugLevel);
			return 0; 
		}
		char path[1000];
		char path2[1000];
		getFilePath(gcf,fileId,path);
		sprintf(path2,"%s\\common\\%s%s",gcfPath,appCommonName,path);

		BUFFEREDHANDLE file=fopen_64(path2,"rb");
		if (!file) 
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Can't open file", path2, desiredDebugLevel);
			return 0;
		}
		printDebug(DEBUG_LEVEL_EVERYTHING,appName,"loading chunk from", path2, desiredDebugLevel);
		for (DWORD ind=0;ind<nbBlockRequested;ind++)
		{
			DWORD read=0;
			if (!(read=getFileBlock(gcf,file,fileId,blockStart+ind,buffer+0x2000*ind)))
				break;
			totalRead+=read;
		}
		fclose_64(file);
	}
	//printDebug(DEBUG_LEVEL_EVERYTHING,"ContentServer","loaded file chunk size", totalRead, desiredDebugLevel);
	return totalRead;
#endif
#ifdef __JUST_PARSING__
	return 0;
#endif
}

DirectoryEntry * buildDirectory(GCF * gcf,char * appName,DWORD desiredDebugLevel, DWORD root)
{
	DirectoryEntry * result=0;
	DirectoryEntry * current=0;
	for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
	{
		
		GCFDirectoryEntry entry=gcf->directory->entry[ind];
		if (entry.parentIndex!=root) continue;

		if (!current)
		{
			result=new DirectoryEntry;
			if (result==0)
			{
				printDebug(DEBUG_LEVEL_ERROR,appName,"memory allocation error", gcf->_fileName, desiredDebugLevel);
				return 0;
			}
			current=result;
		}
		else
		{
			current->sibling=new DirectoryEntry;
			if (current->sibling==0)
			{
				printDebug(DEBUG_LEVEL_ERROR,appName,"memory allocation error", gcf->_fileName, desiredDebugLevel);
				return 0;
			}
			current=current->sibling;
		}

		current->name=gcf->directory->directoryNames+gcf->directory->entry[ind].nameOffset;
		current->gcfEntry=&gcf->directory->entry[ind];
		current->sibling=0;
		current->child=0;

		if (entry.directoryType==0)
		{
			current->child=buildDirectory(gcf,appName,desiredDebugLevel,ind);
		}
	}
	return result;
}

void deleteDirectory(DirectoryEntry * root)
{
	while (root)
	{
		if (root->child) deleteDirectory(root->child);
		DirectoryEntry * next=root->sibling;
		delete root;
		root=next;
	}
}

DirectoryEntry * findInDirectory(DirectoryEntry * folder, char * name)
{
//	printDebug(DEBUG_LEVEL_TODO,"find in dir","null folder, return",DEBUG_LEVEL_TODO);
	if (!folder) return 0;
	DirectoryEntry * elt=folder;
	while (elt)
	{
	//	printDebug(DEBUG_LEVEL_TODO,"find in dir","comparing",DEBUG_LEVEL_TODO);
	//	printDebug(DEBUG_LEVEL_TODO,"find in dir"," (with ",elt->name,DEBUG_LEVEL_TODO);
		
		if (!strcmp(elt->name,name)) return elt;
	//	printDebug(DEBUG_LEVEL_TODO,"find in dir","  (diff , next",DEBUG_LEVEL_TODO);
		elt=elt->sibling;
	}
//	printDebug(DEBUG_LEVEL_TODO,"find in dir","not found",DEBUG_LEVEL_TODO);
	return 0;
}

void buildArchiveName(GCF * gcf, char * name, char * root)
{
	if (root) 
	{
		strcpy(name,root);
		strcat(name,"\\");
		char * n=strrchr(gcf->_fileName,'\\');
		if (n)
			strcat(name,n+1);
		else
			strcat(name,gcf->_fileName);
	}
	else
		strcpy(name,gcf->_fileName);
	char * extension=strrchr(name,'.');
	itoa(gcf->appVersion,extension+1,10);
	strcat(name,".archive");
}

long serializeArchive(GCF * gcf, char * buffer,int withFirstBlockIndex)
{
	DWORD * buffer2=(DWORD*)buffer;
	long pos=0;
	long fileSizePos;
	long checksumPos;
	if (buffer)
	{
		buffer2[pos++]=gcf->unknown1;
		//buffer2[pos++]=gcf->cfType;
		buffer2[pos++]=2; // like ncf : no content...
		//buffer2[pos++]=gcf->cfVersion;
		buffer2[pos++]=1; // like ncf
		buffer2[pos++]=gcf->appId;
		buffer2[pos++]=gcf->appVersion;
		buffer2[pos++]=gcf->updating;
		buffer2[pos++]=gcf->unknown3;
		fileSizePos=pos;
		buffer2[pos++]=gcf->fileSize; 
		buffer2[pos++]=gcf->blockSize;
		//buffer2[pos++]=gcf->blockCount;
		buffer2[pos++]=0; // no content
		checksumPos=pos;
		buffer2[pos++]=gcf->checksum;
		pos=pos*4+serializeGCFDirectory((char*)(buffer2+pos),gcf);
	}
	else
		pos=11*4+serializeGCFDirectory(0,gcf);
	
	if (buffer)
	{
		long pos2=0;

		DWORD * buffer3=(DWORD*)(buffer+pos);
		
		buffer3[pos2++]=gcf->directoryMap->unknown1;pos+=4;
		buffer3[pos2++]=gcf->directoryMap->unknown2;pos+=4;

		if (!withFirstBlockIndex)
		{
			memset((char*)(buffer3+pos2),0,gcf->directory->itemCount*4);pos2+=gcf->directory->itemCount;
		}
		else
		{
			//printDebug(DEBUG_LEVEL_TODO,"test","block count",gcf->blockCount,0);
			for (unsigned int i=0;i<gcf->directory->itemCount;i++)
			{
				if (gcf->cfType==1)
				{
					//gcf
					int isOk=1;
					if (gcf->directoryMap->firstBlockIndex[i]==gcf->blockCount)
					{
						// no block => no content
						isOk=0;
					}
					else
					{
						// has content, checking it
						DWORD block=gcf->directoryMap->firstBlockIndex[i];
						DWORD fSize=0;
						while (block!=gcf->blockCount)
						{
							GCFBlock blk=gcf->blocks->block[block];
							fSize+=blk.fileDataSize;
							block=blk.nextBlockEntryIndex;
						}
						if (fSize!=gcf->directory->entry[i].itemSize) isOk=0;
					}
					*(buffer3+pos2)=isOk;pos2++;
				}
				else
				{
					//ncf
					*(buffer3+pos2)=gcf->directoryMap->firstBlockIndex[i]==3;pos2++;
				}
			}
		}
	}
	else pos+=8;
	pos+=gcf->directory->itemCount*4;
	
	if (buffer)
	{
		// 2 firsts dwords not serialised as not sent in content server
		long pos2=0;
		DWORD * buffer3=(DWORD*)(buffer+pos);
		buffer3[pos2++]=gcf->checksums->unknown1;pos+=4;
		buffer3[pos2++]=gcf->checksums->checksumSize;pos+=4;

		pos+=serializeGCFChecksums(buffer+pos,gcf);
	}
	else
		pos+=8+serializeGCFChecksums(0,gcf);
	
	if (buffer)
	{
		long pos2=0;
		DWORD * buffer3=(DWORD*)(buffer+pos);
		buffer3[pos2++]=gcf->dataBlocks->appVersion;pos+=4;
		buffer2[fileSizePos]=pos;
		DWORD computed=sumBytes(gcf->unknown1)+sumBytes(2)+sumBytes(1)+sumBytes(gcf->appId)+sumBytes(gcf->appVersion)+
		sumBytes(gcf->updating)+sumBytes(gcf->unknown3)+sumBytes(pos)+sumBytes(gcf->blockSize)+sumBytes(0);
		buffer2[checksumPos]=computed;		
		
	}
	else
		pos+=4;

	return pos;
}

DWORD computeGcfChunkChecksum(char * chunk, DWORD len)
{
	return adler32(0,(unsigned char*)chunk,len) ^ crc32(0,(unsigned char*)chunk,len);
}

long serializeGCFHeaders(GCF * gcf, char * buffer)
{
	
	
	long pos=0;
	//long fileSizePos;
	
	// header
	if (buffer)
	{
		DWORD * buffer2=(DWORD*)buffer;
		buffer2[pos++]=gcf->unknown1;
		buffer2[pos++]=gcf->cfType;
		buffer2[pos++]=gcf->cfVersion;
		buffer2[pos++]=gcf->appId;
		buffer2[pos++]=gcf->appVersion;
		buffer2[pos++]=gcf->updating;
		buffer2[pos++]=gcf->unknown3;
		//fileSizePos=pos;
		buffer2[pos++]=gcf->fileSize;
		buffer2[pos++]=gcf->blockSize;
		buffer2[pos++]=gcf->blockCount;
		gcf->checksum=sumBytes(gcf->unknown1)+sumBytes(gcf->cfType)+sumBytes(gcf->cfVersion)+sumBytes(gcf->appId)+sumBytes(gcf->appVersion)+sumBytes(gcf->updating)+sumBytes(gcf->unknown3)+sumBytes(gcf->fileSize)+sumBytes(gcf->blockSize)+sumBytes(gcf->blockCount);
		buffer2[pos++]=gcf->checksum;
		pos=pos*4;
	}
	else
		pos=11*4;
	
	if (gcf->cfType==1)
	{
		// only if gcf. ncf has no content
		// blocks
		if (buffer)
		{
			long pos2=0;
			DWORD * buffer3=(DWORD*)(buffer+pos);
			buffer3[pos2++]=gcf->blocks->blockCount;pos+=4;	
			buffer3[pos2++]=gcf->blocks->blockUsed;pos+=4;	
			buffer3[pos2++]=gcf->blocks->lastUsedBlock;pos+=4;	
			buffer3[pos2++]=gcf->blocks->unknown2;pos+=4;	
			buffer3[pos2++]=gcf->blocks->unknown3;pos+=4;	
			buffer3[pos2++]=gcf->blocks->unknown4;pos+=4;	
			buffer3[pos2++]=gcf->blocks->unknown5;pos+=4;
			gcf->blocks->checksum=gcf->blocks->blockCount+gcf->blocks->blockUsed+gcf->blocks->lastUsedBlock+gcf->blocks->unknown2+gcf->blocks->unknown3+gcf->blocks->unknown4+gcf->blocks->unknown5;
			buffer3[pos2++]=gcf->blocks->checksum;pos+=4;	
			
			
			for (unsigned int ind=0;ind<gcf->blocks->blockCount;ind++)
			{
				GCFBlock block=gcf->blocks->block[ind];
				buffer3[pos2++]=block.entryType;pos+=4;	
				buffer3[pos2++]=block.fileDataOffset;pos+=4;	
				buffer3[pos2++]=block.fileDataSize;pos+=4;	
				buffer3[pos2++]=block.firstDataBlockIndex;pos+=4;	
				buffer3[pos2++]=block.nextBlockEntryIndex;pos+=4;	
				buffer3[pos2++]=block.previousBlockEntryIndex;pos+=4;	
				buffer3[pos2++]=block.directoryIndex;pos+=4;	
			}
		}
		else
		{
			pos+=4*(8+gcf->blocks->blockCount*(7));
		} 
		
		// fragmentation map
		if (buffer)
		{
			long pos2=0;
			DWORD * buffer3=(DWORD*)(buffer+pos);
			buffer3[pos2++]=gcf->fragmentationMap->blockCount;pos+=4;	
			buffer3[pos2++]=gcf->fragmentationMap->firstUnusedEntry;pos+=4;	
			
			buffer3[pos2++]=gcf->fragmentationMap->terminator;pos+=4;	
			gcf->fragmentationMap->checksum=gcf->fragmentationMap->blockCount+gcf->fragmentationMap->firstUnusedEntry+gcf->fragmentationMap->terminator;
			buffer3[pos2++]=gcf->fragmentationMap->checksum;pos+=4;	
			
			if (gcf->fragmentationMap->blockCount>0)
			{
				memcpy(&(buffer3[pos2]),gcf->fragmentationMap->nextDataBlockIndex,4*gcf->fragmentationMap->blockCount);pos+=4*gcf->fragmentationMap->blockCount;
			}
		}
		else
		{
			pos+=4*(4+gcf->fragmentationMap->blockCount);
		}
	}
	
	// directory
	if (buffer)
	{
		pos+=serializeGCFDirectory((char*)(buffer+pos),gcf);
	}
	else
		pos+=serializeGCFDirectory(0,gcf);
	
	// directory map
	if (buffer)
	{
		long pos2=0;
		
		DWORD * buffer3=(DWORD*)(buffer+pos);
		
		buffer3[pos2++]=gcf->directoryMap->unknown1;pos+=4;
		buffer3[pos2++]=gcf->directoryMap->unknown2;pos+=4;
		
		for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
		{
			buffer3[pos2++]=gcf->directoryMap->firstBlockIndex[ind];
		}
	}
	else pos+=8;
	pos+=gcf->directory->itemCount*4;
	
	
	// checksums
	if (buffer)
	{
		// 2 firsts dwords not serialised as not sent in content server
		long pos2=0;
		DWORD * buffer3=(DWORD*)(buffer+pos);
		buffer3[pos2++]=gcf->checksums->unknown1;pos+=4;
		buffer3[pos2++]=gcf->checksums->checksumSize;pos+=4;
		
		pos+=serializeGCFChecksums(buffer+pos,gcf);
	}
	else
		pos+=8+serializeGCFChecksums(0,gcf);
	
	
	// data blocks
	if (buffer)
	{
		long pos2=0;
		DWORD * buffer3=(DWORD*)(buffer+pos);
		buffer3[pos2++]=gcf->dataBlocks->appVersion;pos+=4;
		if (gcf->cfType==1)
		{
			buffer3[pos2++]=gcf->dataBlocks->blockCount;pos+=4;
			buffer3[pos2++]=gcf->dataBlocks->blockSize;pos+=4;
			buffer3[pos2++]=gcf->dataBlocks->firstBlockOffset;pos+=4;
			buffer3[pos2++]=gcf->dataBlocks->blocksUsed;pos+=4;
			gcf->dataBlocks->checksum=gcf->dataBlocks->blockCount+gcf->dataBlocks->blockSize+gcf->dataBlocks->firstBlockOffset+gcf->dataBlocks->blocksUsed;
			buffer3[pos2++]=gcf->dataBlocks->checksum;pos+=4;
		}
	}
	else	
		pos+=4*(gcf->cfType==1 ? 6 : 1);
	return pos;
}

int updateGcfFileHeaders(GCF * gcf, BUFFEREDHANDLE handle)
{
	// only if headers length hasn't changed
	BUFFEREDHANDLE f;
	if (!handle) 
		f=fopen_64(gcf->_fileName,"r+b");
	else
		f=handle;

	if (!f) return -1; // can't create file
	
	rewind_64(f);
	int size=serializeGCFHeaders(gcf,0);
	
	char * buffer=(char*)malloc(size);
	
	size=serializeGCFHeaders(gcf,buffer);
	
	fwrite_64(buffer,1,size,f);
	free(buffer);
	if (!handle) fclose_64(f);	
	
	return 0;
}

DWORD getFirstFreeBlock(GCF * gcf)
{
	
	for (unsigned int ind=0;ind<gcf->blocks->blockCount;ind++)
	{
		if (gcf->blocks->block[ind].entryType&0x8000) return ind; // was &0xBFFF before but can be remaining bits
	}
	return -1;
}

char * parseGCFChecksumEntry(GCFChecksumEntry * entry, char * data)
{
	entry->checksumCount=*(DWORD*)data;data+=4;
	entry->firstChecksumIndex=*(DWORD*)data;data+=4;
	return data;
}

char* parseGCFChecksums(GCFChecksums * checksums, char * data)
{
	char * start=data;
	checksums->unknown1=1;
	//hexaPrintf(data,256);
	checksums->unknown2=*(DWORD*)data;data+=4;
	checksums->unknown3=*(DWORD*)data;data+=4;
	checksums->itemCount=*(DWORD*)data;data+=4;
	checksums->checksumCount=*(DWORD*)data;data+=4;
	checksums->checksumEntry=0;
	
	if (checksums->itemCount)
	{
		checksums->checksumEntry=(GCFChecksumEntry*)malloc(checksums->itemCount*(unsigned int) sizeof(GCFChecksumEntry));
		for (unsigned int ind=0;ind<checksums->itemCount;ind++)
		{
			//printf("parsing Checksum item %d on %u\n",ind,checksums->itemCount);
			data=parseGCFChecksumEntry(&(checksums->checksumEntry[ind]),data);
		}
	}
	
	checksums->checksum=(DWORD *)malloc((checksums->checksumCount)*sizeof(DWORD));
	memcpy(checksums->checksum,data,4*(checksums->checksumCount));
	data+=(checksums->checksumCount)*4;
	memcpy(checksums->rsaSignature,data,128);
	data+=128;
	
	checksums->checksumSize=data-start;
	
	return data;
}

char* parseGCFDirectoryEntry(GCFDirectoryEntry * entry,char * data)
{
	entry->nameOffset=*(DWORD*)data;data+=4;
	entry->itemSize=*(DWORD*)data;data+=4;
	entry->checksumIndex=*(DWORD*)data;data+=4;
	entry->directoryType=*(DWORD*)data;data+=4;
	entry->parentIndex=*(DWORD*)data;data+=4;
	entry->nextIndex=*(DWORD*)data;data+=4;
	entry->firstIndex=*(DWORD*)data;data+=4;
	
	return data;
}



char* parseGCFDirectory(GCFDirectory * directory,char * data)
{
	directory->unknown1=*(DWORD*)data;data+=4;
	directory->appId=*(DWORD*)data;data+=4;
	directory->appVersion=*(DWORD*)data;data+=4;
	directory->itemCount=*(DWORD*)data;data+=4;
	directory->fileCount=*(DWORD*)data;data+=4;
	directory->chunkSize=*(DWORD*)data;data+=4;
	directory->directorySize=*(DWORD*)data;data+=4;
	directory->nameSize=*(DWORD*)data;data+=4;
	directory->hashedFilesIndexSize=*(DWORD*)data;data+=4;
	directory->copyCount=*(DWORD*)data;data+=4;
	directory->localCount=*(DWORD*)data;data+=4;
	directory->unknown2=*(DWORD*)data;data+=4;
	directory->globalChecksum=*(DWORD*)data;data+=4;
	directory->checksum=*(DWORD*)data;data+=4;
	
	/*
		DWORD computed=directory->unknown1+directory->appId+directory->appVersion+directory->itemCount+directory->fileCount+directory->unknown2+directory->directorySize+
		directory->nameSize+directory->info1Count+directory->copyCount+directory->localCount+directory->unknown3+directory->unknown4;
		printf("checksum2 : %d\n",directory->checksum);
		printf("computed2 : %d\n",computed);
	*/
	directory->entry=0;
	directory->directoryNames=0;
	directory->hashedFilesIndex =0;
	directory->hashedFiles =0;
	directory->copyEntry=0;
	directory->localEntry=0;
	
	if (directory->itemCount)
	{
		directory->entry=(GCFDirectoryEntry*)malloc(directory->itemCount*sizeof(GCFDirectoryEntry));
		for (unsigned int ind=0;ind<directory->itemCount;ind++)
		{
			data=parseGCFDirectoryEntry(&(directory->entry[ind]),data);
		}
	}
	if (directory->nameSize)
	{
		directory->directoryNames=(char *)malloc(directory->nameSize);
		memcpy(directory->directoryNames,data,directory->nameSize);data+=directory->nameSize;
	}
	if (directory->hashedFilesIndexSize)
	{
		directory->hashedFilesIndex =(DWORD *)malloc(directory->hashedFilesIndexSize*sizeof(DWORD));
		memcpy(directory->hashedFilesIndex ,data,directory->hashedFilesIndexSize*4);data+=directory->hashedFilesIndexSize*4;
	}
	if (directory->itemCount)
	{
		directory->hashedFiles =(DWORD *)malloc(directory->itemCount*sizeof(DWORD));
		memcpy(directory->hashedFiles ,data,directory->itemCount*4);data+=directory->itemCount*4;
	}
	if (directory->copyCount)
	{
		directory->copyEntry=(DWORD *)malloc(directory->copyCount*sizeof(DWORD));
		memcpy(directory->copyEntry,data,directory->copyCount*4);data+=directory->copyCount*4;
	}
	if (directory->localCount)
	{
		directory->localEntry=(DWORD *)malloc(directory->localCount*sizeof(DWORD));
		memcpy(directory->localEntry,data,directory->localCount*4);data+=directory->localCount*4;
	}
	return data;
}

DWORD getNbBlocks(GCFDirectory * directory)
{
	DWORD nb=0;
	for (unsigned int ind=0;ind<directory->itemCount;ind++)
	{
		GCFDirectoryEntry entry=directory->entry[ind];
		if (entry.directoryType)
		{ // is not a folder
			nb+=entry.itemSize/0x2000 + (entry.itemSize%0x2000 ? 1 : 0);
		}
	}
	return nb;
}

GCF * parseGcf(char * directory, char * checksums, int cfType,int cfVersion,DWORD dataBlocksCount)
{
	GCF * gcf=new GCF;
	
	gcf->blocks=0;
	gcf->fragmentationMap=0;
	gcf->directory=0;
	gcf->directoryMap=0;
	gcf->checksums=0;
	gcf->dataBlocks=0;
	
	if (directory)
	{
		gcf->directory=new GCFDirectory;
		parseGCFDirectory(gcf->directory,directory);
	}
	
	if (checksums)
	{
		gcf->checksums=new GCFChecksums;
		parseGCFChecksums(gcf->checksums,checksums);
	}
	
	// header
	
	gcf->unknown1=1;// always 1
	gcf->cfType=cfType;
	gcf->cfVersion=cfVersion;
	gcf->appId=gcf->directory->appId;
	gcf->appVersion=gcf->directory->appVersion;
	gcf->updating=0;
	gcf->unknown3=0; // always 0 
	
	gcf->blockSize=(gcf->cfType==1 ? GCF_BLOCK_SIZE  : 0);
	if (gcf->cfType==1) 
		if (dataBlocksCount==-1L)
			gcf->blockCount=getNbBlocks(gcf->directory);
		else
			gcf->blockCount=dataBlocksCount;
	else
		gcf->blockCount=0;
	
	if (gcf->cfType==1)
	{
		// blocks
		gcf->blocks=new GCFBlocks;
		gcf->blocks->blockCount=gcf->blockCount;
		gcf->blocks->blockUsed=0;
		gcf->blocks->lastUsedBlock=gcf->blockCount;
		gcf->blocks->unknown2=0;
		gcf->blocks->unknown3=0;
		gcf->blocks->unknown4=0;
		gcf->blocks->unknown5=0;
		gcf->blocks->block=(GCFBlock*)malloc(gcf->blockCount*sizeof(GCFBlock));
		for (unsigned int ind=0;ind<gcf->blockCount;ind++)
		{
			
			gcf->blocks->block[ind].entryType=0;
			gcf->blocks->block[ind].fileDataOffset=0;
			gcf->blocks->block[ind].fileDataSize=0;
			gcf->blocks->block[ind].firstDataBlockIndex=0;
			gcf->blocks->block[ind].nextBlockEntryIndex=gcf->blockCount;
			gcf->blocks->block[ind].previousBlockEntryIndex=gcf->blockCount;
			gcf->blocks->block[ind].directoryIndex=-1;
		}
		
		// fragmentation map
		gcf->fragmentationMap=new GCFFragmentationMap;
		gcf->fragmentationMap->blockCount=gcf->blockCount;
		gcf->fragmentationMap->firstUnusedEntry=0;
		gcf->fragmentationMap->terminator=(gcf->blockCount>=0xFFFF ? 1 : 0);
		
		gcf->fragmentationMap->nextDataBlockIndex=0;
		
		if (gcf->fragmentationMap->blockCount>0)
		{
			gcf->fragmentationMap->nextDataBlockIndex=(DWORD*)malloc(gcf->fragmentationMap->blockCount*sizeof(DWORD));
			for (unsigned int ind=0;ind<gcf->fragmentationMap->blockCount;ind++)
			{
				//gcf->fragmentationMap->nextDataBlockIndex[ind]=gcf->blockCount;
				if (!gcf->fragmentationMap->terminator) gcf->fragmentationMap->nextDataBlockIndex[ind]=0xFFFF;
				else
					gcf->fragmentationMap->nextDataBlockIndex[ind]=0xFFFFFFFF;
			}
		}
	}
	
	// directory map
	gcf->directoryMap=new GCFDirectoryMap;
	gcf->directoryMap->unknown1=1;
	gcf->directoryMap->unknown2=0;
	
	gcf->directoryMap->firstBlockIndex=0;
	
	if (gcf->directory->itemCount)
	{
		gcf->directoryMap->firstBlockIndex=(DWORD *)malloc(gcf->directory->itemCount*sizeof(DWORD));
		for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
		{
			gcf->directoryMap->firstBlockIndex[ind]=(gcf->cfType==1 ? gcf->blockCount : 1);
		}
	}
	
	// data blocks
	DWORD headersSize=serializeGCFHeaders(gcf,0);
	
	gcf->dataBlocks=new GCFDataBlocks;
	gcf->dataBlocks->appVersion=gcf->appVersion;
	gcf->dataBlocks->blockUsage=0;
	if (cfType==1)
	{
		gcf->dataBlocks->blockCount=gcf->blockCount;
		gcf->dataBlocks->blockSize=gcf->blockSize;
		gcf->dataBlocks->firstBlockOffset=headersSize;
		gcf->dataBlocks->blocksUsed=0;
	}
	else
	{
		gcf->dataBlocks->blockCount=0;
		gcf->dataBlocks->blockSize=0;
		gcf->dataBlocks->firstBlockOffset=0;
		gcf->dataBlocks->blocksUsed=0;
	}
	

	//gcf->fileSize=(gcf->cfType==1 ? headersSize+gcf->blockCount*gcf->blockSize : headersSize);
	gcf->headersSize=headersSize;
	gcf->fileSize=(gcf->cfType==1 ? headersSize+gcf->blockCount*gcf->blockSize : 0);
	return gcf;
}


int isToBeExtracted(GCF * gcf, DWORD fileId)
{
	for (unsigned int ind=0;ind<gcf->directory->copyCount;ind++)
	{
		if (gcf->directory->copyEntry[ind]==fileId) return 1;
	}
	return 0;
}



DWORD getNextGcfChunkDataBlockIndex(GCF * gcf, DWORD dataBlockIndex)
{
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	int written=0;
	while (written<4 && dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount)
	{
 	 		written++;
		dataBlockIndex=gcf->fragmentationMap->nextDataBlockIndex[dataBlockIndex];
	}
 
	return dataBlockIndex;
}

int validateGcfFile(GCF * gcf,DWORD fileId,DWORD * checksums/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char * appName,int desiredLevel,int * pleaseStop)
{
	char buffer[0x8010];

	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	BUFFEREDHANDLE f = fopen_64(gcf->_fileName,"rb");
	GCFDirectoryEntry entry=gcf->directory->entry[fileId];
	printDebug(DEBUG_LEVEL_DEBUG,appName,"Validating",gcf->directory->directoryNames+entry.nameOffset,desiredLevel);

	//printf("Decrypting : %s (size: %d)\n",gcf->directory->directoryNames+entry.nameOffset,entry.itemSize);
	
	DWORD blockIndex=gcf->directoryMap->firstBlockIndex[fileId];
	DWORD chunk=0;
	DWORD rest=entry.itemSize;
	while (blockIndex!=gcf->blocks->blockCount)
	{
		if (pleaseStop && *pleaseStop) 
		{
			fclose_64(f);
			return 1;
		}
 		GCFBlock block=gcf->blocks->block[blockIndex];
		if (block.fileDataSize%0x8000 && block.nextBlockEntryIndex!=gcf->blocks->blockCount)
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Block size error",desiredLevel);
	
			fclose_64(f);
			return 0;
		}
 		
		if (block.fileDataSize)
		{ // if block has content
			DWORD dataBlockIndex=block.firstDataBlockIndex;
 			while (dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount && rest)
			{ // while data in block
				if (pleaseStop && *pleaseStop) 
				{
					fclose_64(f);
					return 1;
				}
 				//printf("chunk %d / %d of len : %x\n",chunk,gcf->checksums->checksumEntry[entry.checksumIndex].checksumCount,(rest>0x8000 ? 0x8000 : rest));
				DWORD read=getGcfChunk(gcf,f,dataBlockIndex,buffer);
 				DWORD originalChecksum=computeGcfChunkChecksum(buffer,(rest>0x8000 ? 0x8000 : rest));
		 
			 	if (originalChecksum!=checksums[chunk])
				{
						printDebug(DEBUG_LEVEL_ERROR,appName,"Checksum error",gcf->directory->directoryNames+entry.nameOffset,desiredLevel);
						fclose_64(f);
						return 0;
				}
				
				rest-=(rest>0x8000 ? 0x8000 : rest);
				chunk++;
				dataBlockIndex=getNextGcfChunkDataBlockIndex(gcf,dataBlockIndex);
 			
			}
		}
		blockIndex=block.nextBlockEntryIndex;
	}
 
	fclose_64(f);
//	printf("OK\n");
	return 1;
}

int validateNcfFile(char * fileName, char * path, DWORD size, DWORD * checksums/*,SOCKET socket, time_t * lastSentContentServerPacket*/,char * appName,int desiredLevel,int * pleaseStop)
{
	
	DWORD realFileSize=FileSize(path);
	if (realFileSize<size)
	{ 
		printDebug(DEBUG_LEVEL_ERROR,appName,"Incomplete",path,desiredLevel);
		return 0;
	}

	BUFFEREDHANDLE f=fopen_64(path,"rb");
	if (!f) {printDebug(DEBUG_LEVEL_ERROR,appName,"File open error",path,desiredLevel);return 0;}

	printDebug(DEBUG_LEVEL_DEBUG,appName,"Validating",fileName,desiredLevel);
	
	DWORD toRead=size;
	char buffer[0x8010];
 	DWORD chunk=0;
	
	while (toRead)
	{
		if (pleaseStop && *pleaseStop) 
		{
			fclose_64(f);
			return 1;
		}		
		fseek_64(f,(_int64)0x8000*(_int64)chunk,SEEK_SET);
		memset(buffer,0,0x8000);

		int read=fread_64(buffer,1,0x8000,f);

		
		DWORD originalChecksum= computeGcfChunkChecksum(buffer,read);
	 
		if (originalChecksum==checksums[chunk])
		{
		//	printf(".");
		}
		else
		{
			fclose_64(f);
			printDebug(DEBUG_LEVEL_ERROR,appName,"Checksum error",path,desiredLevel);
			return 0;		 
		}
 
		chunk++;
 
		toRead-=read;
	}

	fclose_64(f);
//printf("OK\n");
	return 1;
}

int validateFile(char * root, char * extractionName,GCF * gcf,DWORD fileId/*,SOCKET socket, time_t * lastSentContentServerPacket*/,int correctFile, char* appName,int desiredLevel,int * pleaseStop)
{
	if (pleaseStop && *pleaseStop) return -1;
	int res=1;
	if (!gcf->directory->entry[fileId].directoryType) return 1;
	
	/*
	// disabled in 1.0.11 : checking local file below and not manifest
	if (gcf->directory->entry[fileId].directoryType&ENCRYPTED_FLAG)
	{
		char path[1000];
		getFilePath(gcf,fileId,path);
		printDebug(DEBUG_LEVEL_DEBUG,appName,"Encrypted",path,desiredLevel);
		return 1;
	}*/

	
	DWORD cIndex=gcf->directory->entry[fileId].checksumIndex;

	cIndex=gcf->checksums->checksumEntry[cIndex].firstChecksumIndex;
	
	switch (gcf->cfType)
	{
	case 1 : {
			DWORD downloadedSize=0;
		 	DWORD blockIndex=gcf->directoryMap->firstBlockIndex[fileId];

	
			int anotherBlock=0;
			if (blockIndex!=gcf->blocks->blockCount)
			{
				// 1.0.7b : 0 sized files should't have blocks
				if (!gcf->directory->entry[fileId].itemSize)
				{
					char path[1000];
					getFilePath(gcf,fileId,path);
					printDebug(DEBUG_LEVEL_ERROR,appName,"Block allocation error",path,desiredLevel);
					
			if (correctFile)
			{
				
				DWORD index=gcf->directoryMap->firstBlockIndex[fileId];
				while (index!=gcf->blocks->blockCount)
				{
					gcf->blocks->block[index].entryType=0;
					gcf->blocks->block[index].fileDataOffset=0;
					gcf->blocks->block[index].fileDataSize=0;
					gcf->blocks->block[index].firstDataBlockIndex=0;
					DWORD next=gcf->blocks->block[index].nextBlockEntryIndex;
					gcf->blocks->block[index].nextBlockEntryIndex=gcf->blocks->blockCount;
					gcf->blocks->block[index].previousBlockEntryIndex=gcf->blocks->blockCount;
					gcf->blocks->block[index].directoryIndex=-1;
					index=next;
				}
				// not needed to free blocks, they will not be "recopied" when finishing update
	
				gcf->directoryMap->firstBlockIndex[fileId]=gcf->blocks->blockCount; // not downloaded
			}

					return 0;
				}
				// fix : 1.0.11 : check if encrypted in blocks :
				bool encrypted=gcf->blocks->block[blockIndex].entryType&GCF_FILE_BLOCK_ZIPPED_AND_CRYPTED || gcf->blocks->block[blockIndex].entryType&GCF_FILE_BLOCK_CRYPTED;
				if (encrypted)
				{
					char path[1000];
					getFilePath(gcf,fileId,path);
					printDebug(DEBUG_LEVEL_DEBUG,appName,"Encrypted",path,desiredLevel);
					return 1;
				}

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
			}
			if (pleaseStop && *pleaseStop) return -1;
			if (downloadedSize<gcf->directory->entry[fileId].itemSize)
			{
				char path[1000];
				getFilePath(gcf,fileId,path);
				printDebug(DEBUG_LEVEL_ERROR,appName,"Incomplete",path,desiredLevel);
				return 0;
			}
		if (!validateGcfFile(gcf,fileId,gcf->checksums->checksum+cIndex/*,socket,lastSentContentServerPacket*/,appName,desiredLevel,pleaseStop))
		{
			if (pleaseStop && *pleaseStop) return -1;
			// error, reset file
			if (correctFile)
			{
				
				DWORD index=gcf->directoryMap->firstBlockIndex[fileId];
				while (index!=gcf->blocks->blockCount)
				{
					gcf->blocks->block[index].entryType=0;
					gcf->blocks->block[index].fileDataOffset=0;
					gcf->blocks->block[index].fileDataSize=0;
					gcf->blocks->block[index].firstDataBlockIndex=0;
					DWORD next=gcf->blocks->block[index].nextBlockEntryIndex;
					gcf->blocks->block[index].nextBlockEntryIndex=gcf->blocks->blockCount;
					gcf->blocks->block[index].previousBlockEntryIndex=gcf->blocks->blockCount;
					gcf->blocks->block[index].directoryIndex=-1;
					index=next;
				}
				// not needed to free blocks, they will not be "recopied" when finishing update
	
				gcf->directoryMap->firstBlockIndex[fileId]=gcf->blocks->blockCount; // not downloaded
			}
			res=0;
		}

		break;
			 }
	case 2 : {

		char path[2000];
		sprintf(path,"%s\\common\\%s",root,extractionName);
		char * fname=path+strlen(path);
		getFilePath(gcf,fileId,fname);


		// strlen(fname) : 1.0.8 : in new ncf directory, strange files ....
if (strlen(fname))
{
				// fix 1.0.11 : check if file status has encrypted flag
				bool encrypted=gcf->directoryMap->firstBlockIndex[fileId]&0x80000000;
				if (encrypted)
				{
					char path[1000];
					getFilePath(gcf,fileId,path);
					printDebug(DEBUG_LEVEL_DEBUG,appName,"Encrypted",path,desiredLevel);
					return 1;
				}


	if (gcf->directoryMap->firstBlockIndex[fileId]==1 && gcf->directory->entry[fileId].itemSize)
		{
			// file is marked as not downloaded... check for presence anyway (not checking for state 2 = downloading)
				BUFFEREDHANDLE f=fopen_64(path,"rb");
				
				if (!f || !FileSize(path)) 
				{
					if (f) fclose_64(f);
					// file is really not existing


					printDebug(DEBUG_LEVEL_ERROR,appName,"Incomplete",path,desiredLevel);
					return 0;
				}
				// file is existing
				fclose_64(f);
				printDebug(DEBUG_LEVEL_ERROR,appName,"File marked as imcomplete but existing",path,desiredLevel);
				if (correctFile)
				{
					gcf->directoryMap->firstBlockIndex[fileId]=3; // marking as downloaded ... will be validated.
				}
				res=0;

		}		
		if (pleaseStop && *pleaseStop) return -1;
		if (!validateNcfFile(gcf->directory->directoryNames+gcf->directory->entry[fileId].nameOffset,path,gcf->directory->entry[fileId].itemSize,gcf->checksums->checksum+cIndex/*,socket,lastSentContentServerPacket*/,appName,desiredLevel,pleaseStop))
		{
			if (pleaseStop && *pleaseStop) return -1;
			if (correctFile)
			{
				// error, delete file if it's not downloading now
				if (gcf->directoryMap->firstBlockIndex[fileId]==3) 
				{	
					setFileSize64(path,0);
					gcf->directoryMap->firstBlockIndex[fileId]=1; // not downloaded
				}
			}
			res=0;
		}
		break;
}
			 }
	}
	if (pleaseStop && *pleaseStop) return -1;
	return res;
	
}

int checkBlocksSize(GCF * gcf,int correctFile, char * appName,int desiredLevel, char* taskName, DWORD * taskProgress, int * pleaseStop)
{
	if (gcf->cfType!=1) return 1;

	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
 
	int res=1;
	char filePath[1000];
	for (unsigned int localFileId=0;localFileId<gcf->directory->itemCount;localFileId++)
	{

		if (pleaseStop && *pleaseStop) return res;
		LONGLONG remainingFileSize=gcf->directory->entry[localFileId].itemSize;
		
		
 		DWORD blockId=gcf->directoryMap->firstBlockIndex[localFileId];
		int goToNextFile=0;
		while (blockId!=gcf->blocks->blockCount && !goToNextFile)
		{
			goToNextFile=0;
			if (pleaseStop && *pleaseStop) return res;
			LONGLONG blockSize=gcf->blocks->block[blockId].fileDataSize;
			
			if (blockSize>remainingFileSize) 
			{
				// Block is bigger than actual file size, mark file as corrupted
				getFilePath(gcf,localFileId,filePath);
				printDebug(DEBUG_LEVEL_ERROR,appName,"Invalid block size",filePath,desiredLevel);
				if (correctFile)
				{
					gcf->directoryMap->firstBlockIndex[localFileId]=gcf->blocks->blockCount;
				}
				res=0;
				break;
			}
			
			DWORD dataBlockId=gcf->blocks->block[blockId].firstDataBlockIndex;
			DWORD prevBlock=-1;
			while (dataBlockId!=terminator && dataBlockId<gcf->dataBlocks->blockCount && !goToNextFile)
			{
				if (blockSize<=0)
				{
					// more data blocks than the block size, cut the fragmentation map
					getFilePath(gcf,localFileId,filePath);
					printDebug(DEBUG_LEVEL_ERROR,appName,"Invalid fragmentation map",filePath,desiredLevel);
					if (correctFile)
					{
						if (prevBlock==-1) 
							gcf->directoryMap->firstBlockIndex[localFileId]=gcf->blocks->blockCount;
						else
							gcf->fragmentationMap->nextDataBlockIndex[prevBlock]=terminator;

					}
					res=0;
					goToNextFile=1;
					break;
				}
				if (pleaseStop && *pleaseStop) return res;
				gcf->dataBlocks->blockUsage[dataBlockId]=1;  
				prevBlock=dataBlockId;
				dataBlockId=gcf->fragmentationMap->nextDataBlockIndex[dataBlockId];
				blockSize-=gcf->dataBlocks->blockSize;
			}
			if (blockSize>0)
			{
					// less data blocks than the block size, mark file as corrupted
					getFilePath(gcf,localFileId,filePath);
					printDebug(DEBUG_LEVEL_ERROR,appName,"Invalid block size",filePath,desiredLevel);
					if (correctFile)
					{
						gcf->directoryMap->firstBlockIndex[localFileId]=gcf->blocks->blockCount;
					}
					res=0;
					goToNextFile=1;
					break;
			}
			remainingFileSize-=blockSize;
			blockId=gcf->blocks->block[blockId].nextBlockEntryIndex;	
		}
	}
	//	printf("total : expected data : %d   data : %d   blocks : %d\n",gcf->dataBlocks->blocksUsed,nbDataBlocks,nbBlocks);
	if (!res && correctFile)
	{
		// some files marked as corrupted, recompute datablocks usage map
		buildDataBlockUsageMap(gcf,0,0);
	}
	return res;
}

int validate(char * root, char * extractionName,GCF * gcf/*,SOCKET socket, time_t * lastSentContentServerPacket*/,int correctFile, char * appName,int desiredLevel, char* taskName, DWORD * taskProgress, int * pleaseStop)
{
	if (((int)gcf)<=1)
	{
		printDebug(DEBUG_LEVEL_ERROR,appName,"File not found or corrupted",desiredLevel);
		return 0;
	}
	int res=1;
	// for gcf, check maps
	if (gcf->cfType==1)
	{
		buildDataBlockUsageMap(gcf,0,0);
		
		DWORD nbUsed=0;
		// compute real usage:
		for (unsigned int ind=0;ind<gcf->dataBlocks->blockCount;ind++)
			if (gcf->dataBlocks->blockUsage[ind]) nbUsed+=1;
		if (gcf->dataBlocks->blocksUsed!=nbUsed)
		{
			res=0;
			printDebug(DEBUG_LEVEL_ERROR,appName,"Incorrect used block count",desiredLevel);
			if (correctFile)
			{
				gcf->dataBlocks->blocksUsed=nbUsed;
			}
		}

		res&=checkBlocksSize(gcf,  correctFile,   appName, desiredLevel,  taskName,  taskProgress,  pleaseStop);

	}

	// for ncf, check if common folder is existing
	if (gcf->cfType==2)
	{
		char path[1000];
		sprintf(path,"%s\\common\\%s",root,extractionName);
		
		if (!folderExists(path))
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Common folder not found",path,desiredLevel);
			if (correctFile)
			{
				buildNcfCommon(gcf,root,extractionName,appName,desiredLevel);
			}
			return 0;
		}
		if (correctFile)
		{
			buildNcfCommon(gcf,root,extractionName,appName,desiredLevel);
		}
	}
	char taskNameRoot[100];
	if (taskName)
	{
		strcpy(taskNameRoot,taskName);
	}
	else
		*taskNameRoot=0;
	for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
	{
		if (pleaseStop && *pleaseStop) break;
		if (taskName)
		{
			sprintf(taskName,"%s : %s",taskNameRoot,gcf->directory->directoryNames+gcf->directory->entry[ind].nameOffset);
		}
		int r=validateFile(root,extractionName,gcf,ind/*,socket,lastSentContentServerPacket*/,correctFile,appName,desiredLevel,pleaseStop);
		if (r<0) break; // pleaseStop
		res&=r;
		if (taskProgress) *taskProgress=(100*ind)/gcf->directory->itemCount+1;
	}
	if (correctFile && !res)
	{
		printDebug(DEBUG_LEVEL_INFO,appName,"Marking corrupted files as incomplete",desiredLevel);
		updateGcfFileHeaders(gcf,(BUFFEREDHANDLE)0);
	}
	return res;
}


DWORD getGcfChunk(GCF * gcf,BUFFEREDHANDLE f, DWORD dataBlockIndex,char * buffer)
{// read 0x8000 = 4 data blocks maxi , initialiser with zeroes
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	memset(buffer,0,0x8010);
	int read=0;
	while (read<4 && dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount)
	{
		//printf("-read chunk %d\n",dataBlockIndex);
		_int64 offset=(_int64)gcf->dataBlocks->firstBlockOffset+(_int64)gcf->dataBlocks->blockSize*(_int64)dataBlockIndex;
		fseek_64(f,offset,SEEK_SET);
		
		fread_64(buffer+gcf->dataBlocks->blockSize*read,gcf->dataBlocks->blockSize,1,f);
		
		read++;
		dataBlockIndex=gcf->fragmentationMap->nextDataBlockIndex[dataBlockIndex];
	}
	return read*gcf->dataBlocks->blockSize;
}


int addAllFiles(DirectoryEntry * oldRoot, DWORD * buffer)
{
	int nb=0;
	while (oldRoot)
	{
		DWORD checksumIndex=oldRoot->gcfEntry->checksumIndex;
		if (checksumIndex!=-1)
		{
			buffer[nb++]=checksumIndex;
		}
		else
			if (oldRoot->child)
			{
				nb+=addAllFiles(oldRoot->child,buffer+nb);
			}
		oldRoot=oldRoot->sibling;
	}
	return nb;
}

int hasDifferentChecksums(GCFDirectoryEntry * oldEntry,GCFDirectoryEntry * newEntry,GCF * oldGcf,GCF * newGcf)
{

		//	printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums",DEBUG_LEVEL_TODO);
 	if (oldEntry->checksumIndex==-1 && newEntry->checksumIndex==-1) 
	{
		//printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums : no checksums",DEBUG_LEVEL_TODO);
		return 0;
	}
	if (oldEntry->checksumIndex==-1 || newEntry->checksumIndex==-1) 
	{
	//	printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums : one has no checksums",DEBUG_LEVEL_TODO);
		return 1;
	}
/*char dbg[1000];sprintf(dbg,"old index %d >= old count %d ",oldEntry->checksumIndex,oldGcf->checksums->checksumCount);
	if (oldEntry->checksumIndex>=oldGcf->checksums->checksumCount) printDebug(DEBUG_LEVEL_TODO,"debug",dbg,DEBUG_LEVEL_TODO);
	if (newEntry->checksumIndex>=newGcf->checksums->checksumCount) printDebug(DEBUG_LEVEL_TODO,"debug","new index >= new count",DEBUG_LEVEL_TODO);
*/
	if (oldGcf->checksums->checksumEntry[oldEntry->checksumIndex].checksumCount!=
		newGcf->checksums->checksumEntry[newEntry->checksumIndex].checksumCount) 
	{
		//printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums : different checksumCount",DEBUG_LEVEL_TODO);
		return 1;
	}
	DWORD oldFirstChecksumIndex=oldGcf->checksums->checksumEntry[oldEntry->checksumIndex].firstChecksumIndex;
	DWORD newFirstChecksumIndex=newGcf->checksums->checksumEntry[newEntry->checksumIndex].firstChecksumIndex;

	DWORD len=oldGcf->checksums->checksumEntry[oldEntry->checksumIndex].checksumCount;
	for (unsigned int ind=0;ind<len;ind++)
	{
		if (oldGcf->checksums->checksum[oldFirstChecksumIndex+ind]!=newGcf->checksums->checksum[newFirstChecksumIndex+ind])
		{
		//	printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums : difference",DEBUG_LEVEL_TODO);
			return 1;
		}
	}
//	printDebug(DEBUG_LEVEL_TODO,"debug","hasDifferentChecksums : equals",DEBUG_LEVEL_TODO);
	return 0;
}

int searchDifferences(DirectoryEntry * oldRoot,DirectoryEntry * newRoot,GCF * oldGcf,GCF * newGcf, DWORD * buffer)
{
	int nb=0;
	DirectoryEntry * oldElt=oldRoot;

	//		printDebug(DEBUG_LEVEL_TODO,"search","old GCF has checksums nb ",oldGcf->checksums->checksumCount,DEBUG_LEVEL_TODO);
	//		printDebug(DEBUG_LEVEL_TODO,"search","new GCF has checksums nb ",newGcf->checksums->checksumCount,DEBUG_LEVEL_TODO);
	while (oldElt)
	{
//			printDebug(DEBUG_LEVEL_TODO,"search","processing",oldElt->name,DEBUG_LEVEL_TODO);

		DirectoryEntry * newElt=findInDirectory(newRoot,oldElt->name);
//			printDebug(DEBUG_LEVEL_TODO,"search result"," ",(int)newElt,DEBUG_LEVEL_TODO);
//				printDebug(DEBUG_LEVEL_TODO,"search res entry"," ",(int)newElt->gcfEntry,DEBUG_LEVEL_TODO);
		
		if (!newElt || ((newElt->gcfEntry->directoryType && !oldElt->gcfEntry->directoryType) || (!newElt->gcfEntry->directoryType && oldElt->gcfEntry->directoryType))) 
		{
//			printDebug(DEBUG_LEVEL_TODO,"search","> no equivalent for",oldElt->name,DEBUG_LEVEL_TODO);
			// old file / folder was deleted / renamed
			
			// if has a checksum (is not a folder) add to list
			if (oldElt->gcfEntry->checksumIndex!=-1)
			{
//				printDebug(DEBUG_LEVEL_TODO,"search","   > adding",oldElt->gcfEntry->checksumIndex,DEBUG_LEVEL_TODO);
				buffer[nb++]=oldElt->gcfEntry->checksumIndex;
			}
			else
			{
				// if has children, add to list
				if (oldElt->child)
				{
//				printDebug(DEBUG_LEVEL_TODO,"search","   > adding all children of",oldElt->gcfEntry->checksumIndex,DEBUG_LEVEL_TODO);
					nb+=addAllFiles(oldElt->child,buffer+nb);
				}
			}
//			printDebug(DEBUG_LEVEL_TODO,"search","   ! added ",oldElt->gcfEntry->checksumIndex,DEBUG_LEVEL_TODO);
		}
		else
		if (oldElt->child)
		{
//			printDebug(DEBUG_LEVEL_TODO,"search","   > old has children processing contents of both : new=",(int)newElt->child,DEBUG_LEVEL_TODO);
				nb+=searchDifferences(oldElt->child, newElt->child,oldGcf,newGcf,buffer+nb);	
		}
		else
		if (oldElt->gcfEntry->itemSize!=newElt->gcfEntry->itemSize)
		{
//			printDebug(DEBUG_LEVEL_TODO,"search","   > different sizes, adding",DEBUG_LEVEL_TODO);
			if (oldElt->gcfEntry->checksumIndex!=-1)
				buffer[nb++]=oldElt->gcfEntry->checksumIndex;
		}
		else
		if (hasDifferentChecksums(oldElt->gcfEntry,newElt->gcfEntry,oldGcf,newGcf))
		{
//			printDebug(DEBUG_LEVEL_TODO,"search","   > different checksums, adding",DEBUG_LEVEL_TODO);
			if (oldElt->gcfEntry->checksumIndex!=-1)
				buffer[nb++]=oldElt->gcfEntry->checksumIndex;
		}
//		printDebug(DEBUG_LEVEL_TODO,"search","Going to next silbling, adding",DEBUG_LEVEL_TODO);
		oldElt=oldElt->sibling;
	}
//	printDebug(DEBUG_LEVEL_TODO,"search","No more sibling going up",DEBUG_LEVEL_TODO);
	return nb;
}

DWORD getGCFRealSize(GCF * gcf)
{
	// gcf
	if (gcf->cfType==1) return gcf->fileSize;

	DWORD size=0;
	//ncf
	for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
	{
		size+=gcf->directory->entry[ind].itemSize;
	}
	
	return size+gcf->headersSize;
}

float getGCFCompletion(GCF * gcf,char * root,char* common, bool accurate)
{
	if (!gcf) return 0;
	
	if (common)
	{
		char cP[1000];
		sprintf(cP,"%s\\common\\%s",root,common);
		return getGCFCompletion(gcf,cP,accurate);
	}
	return getGCFCompletion(gcf,root,accurate);
	
}
float getGCFCompletion(GCF * gcf,char * common,bool accurate)
{ 
	if (!gcf) return 0;

	float completion=0;
 
	if (gcf->cfType>1) 
	{
		//ncf
		char path[1000];
		char absolutePath[1000];
		LONGLONG downloadedSize=0; 
		LONGLONG totalSize=0;
 		for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
		{
			GCFDirectoryEntry entry=gcf->directory->entry[ind];
			if (entry.directoryType)
			{
				// is a file
				totalSize+=entry.itemSize;
				
				if (accurate) 
				{
					getFilePath(gcf,ind,path);
					sprintf(absolutePath,"%s%s",common,path);	
				
					DWORD size=FileSize( absolutePath);
					downloadedSize+=(size < entry.itemSize ? size : entry.itemSize); 
				}
				else
				{
					downloadedSize+=(gcf->directoryMap->firstBlockIndex[ind]==3 ? entry.itemSize : 0);	
				}
			}
		}
	 
		if (!totalSize) 
			completion=100.0f;
		else
			completion=((float)downloadedSize)/(float)(totalSize)*100.0f;
	
	}
	else
	{
		//gcf
		buildDataBlockUsageMap(gcf,0,0);
		for (unsigned int ind=0;ind<gcf->dataBlocks->blockCount;ind++)
		if (gcf->dataBlocks->blockUsage[ind]) completion+=1;
		if (!gcf->dataBlocks->blockCount) 
			completion=100.0f;
		else
			completion=((float)completion*100)/(float)gcf->dataBlocks->blockCount;
	} 
	return completion;
}
DWORD getNCFDownloadedSize(GCF * gcf,char * common,bool  onlyFinished)
{ 
	DWORD downloadedSize=0; 
 
	if (gcf->cfType>1) 
	{
		//ncf
		char path[1000];
		char absolutePath[1000];
		 
 		for (unsigned int ind=0;ind<gcf->directory->itemCount;ind++)
		{
			GCFDirectoryEntry entry=gcf->directory->entry[ind];
			if (!onlyFinished || gcf->directoryMap->firstBlockIndex[ind]==3)
				if (entry.directoryType)
				{
 					// is a file
					getFilePath(gcf,ind,path);
					sprintf(absolutePath,"%s%s",common,path);	
					
					DWORD size=FileSize( absolutePath);
					downloadedSize+=(size < entry.itemSize ? size : entry.itemSize); 
				}
		}
	}
 	return downloadedSize;
}

int isUpdated(GCF * gcf, DWORD fileId, DWORD * updatedFiles, DWORD nbUpdated)
{
	for (DWORD ind=0;ind<nbUpdated;ind++)
	{
		if (gcf->directory->entry[fileId].checksumIndex==updatedFiles[ind]) return 1;
	}
	return 0;
}

void buildDataBlockUsageMap(GCF * gcf, DWORD * updatedFiles, DWORD nbUpdated)
{

	if (gcf->cfType!=1) return;
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);
	if (gcf->dataBlocks->blockUsage)
	{
		free(gcf->dataBlocks->blockUsage);
	}
	gcf->dataBlocks->blockUsage=(char*)malloc(gcf->dataBlocks->blockCount); 
 
	memset(gcf->dataBlocks->blockUsage,0,gcf->dataBlocks->blockCount); // everything unused 
	
	for (unsigned int localFileId=0;localFileId<gcf->directory->itemCount;localFileId++)
	{
//		printDebug(DEBUG_LEVEL_TODO,"test","TERMINATOR : ",gcf->fragmentationMap->terminator,0);
//		printDebug(DEBUG_LEVEL_TODO,"test","File id : ",localFileId,0);
		if (!isUpdated(gcf,localFileId,updatedFiles,nbUpdated))
		{// and is not updated
			DWORD blockId=gcf->directoryMap->firstBlockIndex[localFileId];
//			printDebug(DEBUG_LEVEL_TODO,"test","  > first block ",blockId,0);
			while (blockId!=gcf->blocks->blockCount)
			{
				DWORD dataBlockId=gcf->blocks->block[blockId].firstDataBlockIndex;
//					printDebug(DEBUG_LEVEL_TODO,"test","      > first data block ",dataBlockId,0);
				while (dataBlockId!=terminator && dataBlockId<gcf->dataBlocks->blockCount)
				{
					gcf->dataBlocks->blockUsage[dataBlockId]=1;  
					dataBlockId=gcf->fragmentationMap->nextDataBlockIndex[dataBlockId];
//						printDebug(DEBUG_LEVEL_TODO,"test","      > next data block ",dataBlockId,0);
				// printDebug(DEBUG_LEVEL_TODO,"test","   (end data block is",terminator,0);
				}
				blockId=gcf->blocks->block[blockId].nextBlockEntryIndex;	
//				printDebug(DEBUG_LEVEL_TODO,"test","  > next block ",blockId,0);
//				printDebug(DEBUG_LEVEL_TODO,"test","   (end block is",gcf->blocks->blockCount,0);
			}
		}
	}
	//	printf("total : expected data : %d   data : %d   blocks : %d\n",gcf->dataBlocks->blocksUsed,nbDataBlocks,nbBlocks);
}


DWORD copyGcfFileContentToBuffer(GCF * gcf, DWORD fileId, char * buffer, DWORD max, char * appName, int desiredLevel)
{
	// DOESNT SUPPORT NCF , cause not needed yet
	memset(buffer,0,max);
	DWORD terminator=(!gcf->fragmentationMap->terminator ? 0xFFFF : 0xFFFFFFFF);

	GCFDirectoryEntry entry=gcf->directory->entry[fileId];
	if (max<entry.itemSize) return 0;
	if (!entry.itemSize) return 0;

	//printf("Decrypting : %s (size: %d)\n",gcf->directory->directoryNames+entry.nameOffset,entry.itemSize);
	BUFFEREDHANDLE f = fopen_64(gcf->_fileName,"rb");	
	DWORD blockIndex=gcf->directoryMap->firstBlockIndex[fileId];
	DWORD chunk=0;
	DWORD rest=entry.itemSize;
	DWORD readTotal=0;
	while (blockIndex!=gcf->blocks->blockCount)
	{
 
		GCFBlock block=gcf->blocks->block[blockIndex];
		if (block.fileDataSize%0x8000 && block.nextBlockEntryIndex!=gcf->blocks->blockCount)
		{
			printDebug(DEBUG_LEVEL_ERROR,appName,"Block size error",desiredLevel);

			fclose_64(f);
			return 0;
		}
			
		
		if (block.fileDataSize)
		{ // if block has content
			DWORD dataBlockIndex=block.firstDataBlockIndex;
			while (dataBlockIndex!=terminator && dataBlockIndex<gcf->dataBlocks->blockCount)
			{ // while data in block
				
				//printf("chunk %d / %d of len : %x\n",chunk,gcf->checksums->checksumEntry[entry.checksumIndex].checksumCount,(rest>0x8000 ? 0x8000 : rest));
				DWORD read=getGcfChunk(gcf,f,dataBlockIndex,buffer+readTotal);
				readTotal+=read; 
				rest-=(rest>0x8000 ? 0x8000 : rest);
				chunk++;
				dataBlockIndex=getNextGcfChunkDataBlockIndex(gcf,dataBlockIndex);
			}
		}
		blockIndex=block.nextBlockEntryIndex;
	}
 
	fclose_64(f);
	return entry.itemSize;
}

