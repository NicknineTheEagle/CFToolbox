// RegistryNode.h: interface for the RegistryNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRYNODE_H__3A649725_9F08_4387_82AE_80E8F4ECF659__INCLUDED_)
#define AFX_REGISTRYNODE_H__3A649725_9F08_4387_82AE_80E8F4ECF659__INCLUDED_
class RegistryNode;
#include "RegistryVector.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NODE_DESC_TYPE_DWORD 0
#define NODE_DESC_TYPE_STRING 1

#define NODE_VALUE_TYPE_BYTE 0
#define NODE_VALUE_TYPE_WORD 1
#define NODE_VALUE_TYPE_DWORD 2
#define NODE_VALUE_TYPE_ULONGLONG 3
#define NODE_VALUE_TYPE_DATA 4
#define NODE_VALUE_TYPE_STRING 5
#define NODE_VALUE_TYPE_VECTOR 6

class RegistryNode  
{
public:
	int getValueType();
	void printXML(char * indent=0);

	unsigned int getDataValueSize();
	char * getDataValue();
	void setDataValue(char * value, unsigned int size);
	char * getStringValue();
	void setStringValue(char * value);
	RegistryVector * getVectorValue();
	void setVectorValue(RegistryVector * value);
	ULONGLONG getUlonglongValue();
	void setUlonglongValue(ULONGLONG value);
	DWORD getDWordValue();
	void setDWordValue(DWORD value);
	WORD getWordValue();
	void setWordValue(WORD value);
	BYTE getByteValue();
	void setByteValue(BYTE value);
	void setStringDescription(char * description);
	void setDwordDescription(DWORD description);
	char * getStringDescription();
	DWORD getDwordDescription();
	int getDescriptionType();
	void clear();
	RegistryNode();
	virtual ~RegistryNode();
	RegistryNode * getNextNode();
	void setNextNode(RegistryNode * node);

	void search(char * text, char * path);
	void search(DWORD value, char * path);
	RegistryNode * copy();
private:
	void clearValue();
	char descType;
	void * description;

	char valueType;
	unsigned int valueSize;
	void * value;

	RegistryNode *nextNode;
};

#endif // !defined(AFX_REGISTRYNODE_H__3A649725_9F08_4387_82AE_80E8F4ECF659__INCLUDED_)
