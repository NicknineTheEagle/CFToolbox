// Registry.h: interface for the Registry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRY_H__9B110B33_0ACC_42A7_BA1A_F7352E4E5AA0__INCLUDED_)
#define AFX_REGISTRY_H__9B110B33_0ACC_42A7_BA1A_F7352E4E5AA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "RegistryVector.h"
#include "RegistryKey.h"
#include "RegistryValue.h"

class Registry  
{
public:
	RegistryKey * createNewRegistry();
	
	Registry();
	~Registry();
	
	RegistryKey * load(char * fileName);
	
	RegistryKey * parseBlob(char * blob);
	
	RegistryVector * parseVector(char * blob);
	
	unsigned long save(char * fileName, RegistryKey * root);
	
	unsigned long serializeBlob(RegistryKey * root,char * blob);

	unsigned long serializeVector(RegistryVector * vector,char * blob);

	unsigned long zipBlob(char * blob,unsigned long blobSize, char ** zippedBlob);

	unsigned long unzipBlob(char * zippedBlob,char ** blob);
private:
	typedef struct {
		unsigned short magic;
		unsigned long zipLength;
		unsigned long zipPadding;
		unsigned long blobLength;
		unsigned long blobPadding;
		unsigned short unknown;
		char * zData;
	} ZipHeader;
	
	typedef struct {
		unsigned short magic;
		unsigned long dataLength;
		unsigned long nullPadding;
	} NodeHeader;
	
	typedef struct {
		unsigned short descriptorLength;
		unsigned long dataLength;
	} Node;
	unsigned long parseVectorHeader(char * blob, RegistryVector *);
	unsigned long parseValue(char * valueBlob,RegistryValue *);
	unsigned long parseValues(char * values,RegistryKey *);
	unsigned long parseKeyContent(char * keyContent,RegistryKey *);
	unsigned long parseKey(char * keyBlob,RegistryKey *);
	unsigned long parseNodeHeader(NodeHeader * nodeHeader, const char * blob);
	unsigned long parseNode(Node * node, const char * blob);
	unsigned long parseZipHeader(ZipHeader * zipHeader, char * blob);

	unsigned long serializeKey(RegistryKey * key, char * keyBlob);
	unsigned long serializeNodeHeader(NodeHeader * nodeHeader, char * blob);
	unsigned long serializeNode(Node *node, char * blob);
	unsigned long serializeKeyContent(RegistryKey * key,char * keyContent);
	unsigned long serializeValues(RegistryKey * key,char * valuesBlob);
	unsigned long serializeValue(RegistryValue * value,char * valueBlob);
	unsigned long serializeZipHeader(ZipHeader * zipHeader,char * blob);
};

#endif // !defined(AFX_REGISTRY_H__9B110B33_0ACC_42A7_BA1A_F7352E4E5AA0__INCLUDED_)
