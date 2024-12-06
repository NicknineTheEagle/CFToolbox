#ifndef _GCF_H_
#define _GCF_H_

#include "afxsock.h"
#include "LargeFile.h"

#define GCF_BLOCK_SIZE	0x2000 // may change ...
#define ENCRYPTED_FLAG	0x0100
//////////////////////////

typedef struct
{
	DWORD appVersion;
	DWORD blockCount;
	DWORD blockSize;
	DWORD firstBlockOffset;
	DWORD blocksUsed;
	DWORD checksum;
	char *blockUsage; // not in gcf / ncf, buit on demand
} GCFDataBlocks;

//////////////////////////

typedef struct
{
	DWORD checksumCount;
	DWORD firstChecksumIndex;
} GCFChecksumEntry;

//////////////////////////

typedef struct
{
	DWORD unknown1;// Always 0x00000001
	DWORD checksumSize;
	DWORD unknown2;// Always 0x14893721
	DWORD unknown3;// Always 0x00000001
	DWORD itemCount;
	DWORD checksumCount;
	
	GCFChecksumEntry * checksumEntry;
	
	DWORD * checksum;
	char rsaSignature[128];
} GCFChecksums;

//////////////////////////

typedef struct
{
	DWORD unknown1;// Always 0x00000001
	DWORD unknown2;// Always 0x00000000
	
	DWORD * firstBlockIndex; // for ncf , is a status : 0 = folder,1=not downloaded,2=not full downloaded, 3=up to date
	
} GCFDirectoryMap;

//////////////////////////

typedef struct
{
	DWORD nameOffset;
	DWORD itemSize;
	DWORD checksumIndex;
	DWORD directoryType; 
	// 0x4000 = Overwrite local copy
	// 0x000A = copy localy
	// 0x0001 = local copy most important
	// 0x0040 = backup local copy
	// 0x0800 = secondary exe ?
	// 0x0100 = encrypted
	DWORD parentIndex;
	DWORD nextIndex;
	DWORD firstIndex;
} GCFDirectoryEntry;

//////////////////////////

typedef struct
{
	DWORD unknown1;// Always 0x00000004
	DWORD appId;
	DWORD appVersion;
	DWORD itemCount;
	DWORD fileCount;
	DWORD chunkSize;
	DWORD directorySize;
	DWORD nameSize;
	DWORD hashedFilesIndexSize;
	DWORD copyCount;
	DWORD localCount;
	DWORD unknown2;
	DWORD globalChecksum;
	DWORD checksum;
	
	GCFDirectoryEntry * entry;
	
	char * directoryNames;
	DWORD * hashedFilesIndex; // hashedFilesIndexSize indexes of first hashedFiles element 
	// if -1, no files 
	// index must be decremented by hashedFilesIndexSize;
	// path doesn't start with an \ and is lowercased 
    //
	// DWORD hash=jenkinsHash((unsigned char*)path,strlen(path),1); 
	// DWORD index=hash&(hashedFilesIndexSizet-1);
	DWORD * hashedFiles;// directoryEntry indexes, last element for this hash if &80000000h
	DWORD * copyEntry;
	DWORD * localEntry;		
} GCFDirectory;

//////////////////////////

typedef struct
{
	DWORD blockCount;
	DWORD firstUnusedEntry;	// The index of the first unused fragmentation map entry.
	DWORD terminator;		// The block entry terminator; 0 = 0x0000ffff or 1 = 0xffffffff.
	DWORD checksum;
	
	DWORD * nextDataBlockIndex;
	
} GCFFragmentationMap;

//////////////////////////

typedef struct
{
	DWORD entryType;// Flags for the block entry.  &0xBFFF==0 => not used
	/*
		8000 : used
		4000 : extracted file
		   1 : normal data
		   2 : zip then crypted 
		   4 : crypted
	*/
	DWORD fileDataOffset;
	DWORD fileDataSize;	// The length of the data in this block entry.
	DWORD firstDataBlockIndex;
	DWORD nextBlockEntryIndex;
	DWORD previousBlockEntryIndex;
	DWORD directoryIndex;
} GCFBlock;

//////////////////////////

typedef struct
{
	DWORD blockCount;
	DWORD blockUsed;
	DWORD lastUsedBlock;  // index of next free block -1
	DWORD unknown2; // 0 
	DWORD unknown3; // 0 
	DWORD unknown4; // 0
	DWORD unknown5; // 0 
	DWORD checksum;
	
	GCFBlock * block;
	
} GCFBlocks;

//////////////////////////

typedef struct
{

	char _fileName[1024];
	int isArchive;
	DWORD headersSize;

	/////

	DWORD unknown1; // always 1
	DWORD cfType; // 1=gcf 2=ncf
	DWORD cfVersion; // last gcf : 6   last ncf : 2
	DWORD appId;
	DWORD appVersion;

	DWORD updating; 
	DWORD unknown3; // 00 
	DWORD fileSize;
	DWORD blockSize;
	DWORD blockCount;
	DWORD checksum;
	GCFBlocks * blocks;
	GCFFragmentationMap * fragmentationMap;

	GCFDirectory * directory;

	GCFDirectoryMap * directoryMap;

	GCFChecksums * checksums;

	GCFDataBlocks * dataBlocks;
} GCF;

//////////////////////////


typedef struct DirectoryEntry 
{
	char * name;
	GCFDirectoryEntry * gcfEntry;
	DirectoryEntry * sibling;
	DirectoryEntry * child;
} DirectoryEntry;

DirectoryEntry * buildDirectory(GCF * gcf,char * appName, DWORD desiredDebugLevel,DWORD root=0);
DirectoryEntry * findInDirectory(DirectoryEntry * folder, char * name);
DWORD sumBytes(DWORD data);

GCF * parseGCF(char * fileName);

void deleteGCF(GCF * gcf);

void printGCFXML(GCF * gcf,int full=0);

long serializeGCFDirectory(char* buffer,GCF * gcf);

long serializeGCFChecksums(char* buffer,GCF * gcf);

DWORD getGCFBlocks(GCF * gcf,char * gcfPath, char * appCommonName, DWORD fileId, DWORD blockStart,DWORD nbBlockRequested, char * buffer,char * appName,int desiredDebugLevel, DWORD * transferMode=0);

void buildArchiveName(GCF * gcf, char * name, char * root=0);

void deleteDirectory(DirectoryEntry * root);

long serializeArchive(GCF * gcf, char * buffer,int withFirstBlockIndex=0);

void getFilePath(GCF * gcf,DWORD fileId,char * path);
DWORD getFileId(GCF * gcf, char * path);

DWORD computeGcfChunkChecksum(char * chunk, DWORD len);

long serializeGCFHeaders(GCF * gcf, char * buffer);

int updateGcfFileHeaders(GCF * gcf, BUFFEREDHANDLE file);

GCF * parseGcf(char * directory, char * checksums, int cfType,int cfVersion,DWORD dataBlocksCount=-1L);

int isToBeExtracted(GCF * gcf, DWORD fileId);

int validate(char * root, char * extractionName,GCF * gcf,int correctFile, char * appName,int desiredLevel,char * taskName, DWORD * taskProgress, int * pleaseStop);

DWORD getGcfChunk(GCF * gcf,BUFFEREDHANDLE f, DWORD dataBlockIndex,char * buffer);

int searchDifferences(DirectoryEntry * oldRoot,DirectoryEntry * newRoot,GCF * oldGcf,GCF * newGcf, DWORD * buffer);
DWORD getGCFRealSize(GCF * gcf);
float getGCFCompletion(GCF * gcf,char* commonPath, bool accurate);
float getGCFCompletion(GCF * gcf,char * root,char* common, bool accurate);
void buildDataBlockUsageMap(GCF * gcf, DWORD * updatedFiles, DWORD nbUpdated);
DWORD copyGcfFileContentToBuffer(GCF * gcf, DWORD fileId, char * buffer, DWORD max, char * appName, int desiredLevel);
int buildNcfCommon(GCF * gcf, char * root, char * commonName,char * appName, int desiredLevel);
int createNcfFolders(GCF * gcf,char * root, GCFDirectoryEntry entry, char * appName, int desiredLevel);
DWORD getNCFDownloadedSize(GCF * gcf,char * common, bool onlyFinished=false);
#endif