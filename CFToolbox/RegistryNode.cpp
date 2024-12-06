// RegistryNode.cpp: implementation of the RegistryNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "socketTools.h"
#include "stdafx.h"
#include "RegistryNode.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


RegistryNode::RegistryNode()
{
	descType=-1;
	description=0;
	
	valueType=NODE_VALUE_TYPE_DATA;
	valueSize=0;
	value=0;
	nextNode=0;
}

RegistryNode::~RegistryNode()
{
	clear();
}
void RegistryNode::clearValue()
{
	if (valueType==NODE_VALUE_TYPE_VECTOR) 
	{
		if (value) ((RegistryVector *)(value))->clear();
		value=0;
		return;
	}
	if (value) free(value);
	valueType=NODE_VALUE_TYPE_DATA;
	value=0;
	valueSize=0;
}

void RegistryNode::clear()
{
	if (description)
		free(description);
	descType=-1;
	clearValue();
}

int RegistryNode::getDescriptionType()
{
	return descType;
}

DWORD RegistryNode::getDwordDescription()
{
	return *((DWORD*)description);
}

char * RegistryNode::getStringDescription()
{
	return (char*)description;
}

void RegistryNode::setDwordDescription(DWORD desc)
{
	descType=NODE_DESC_TYPE_DWORD;
	description=malloc(4);
	*((DWORD*)description)=desc;
}

void RegistryNode::setStringDescription(char *desc)
{
	descType=NODE_DESC_TYPE_STRING;
	description=malloc(strlen(desc)+1);
	strcpy((char*)description,desc);
}

void RegistryNode::setByteValue(BYTE value)
{
	valueType=NODE_VALUE_TYPE_BYTE;
	valueSize=1;
	this->value=malloc(valueSize);
	*((BYTE*)this->value)=value;
}

BYTE RegistryNode::getByteValue()
{
	return *((BYTE*)this->value);
}

void RegistryNode::setWordValue(WORD value)
{
	valueType=NODE_VALUE_TYPE_WORD;
	valueSize=2;
	this->value=malloc(valueSize);
	*((WORD*)this->value)=value;
}

WORD RegistryNode::getWordValue()
{
	return *((WORD*)this->value);
}

void RegistryNode::setDWordValue(DWORD value)
{
	valueType=NODE_VALUE_TYPE_DWORD;
	valueSize=4;
	this->value=malloc(valueSize);
	*((DWORD*)this->value)=value;
}

DWORD RegistryNode::getDWordValue()
{
	return *((DWORD*)this->value);
}
void RegistryNode::setUlonglongValue(ULONGLONG value)
{
	valueType=NODE_VALUE_TYPE_ULONGLONG;
	valueSize=8;
	this->value=malloc(valueSize);
	*((ULONGLONG*)this->value)=value;
}

ULONGLONG RegistryNode::getUlonglongValue()
{
	return 	*((ULONGLONG*)this->value);
}

void RegistryNode::setVectorValue(RegistryVector *value)
{
	valueType=NODE_VALUE_TYPE_VECTOR;
	valueSize=-1;	
	this->value=value;
}

RegistryVector * RegistryNode::getVectorValue()
{
	return (RegistryVector *)value;
}


void RegistryNode::setStringValue(char *value)
{
	valueType=NODE_VALUE_TYPE_STRING;
	valueSize=strlen(value)+1;
	this->value=malloc(valueSize);
	strcpy((char*)this->value,value);
}

char * RegistryNode::getStringValue()
{
	return (char*)value;
}

void RegistryNode::setDataValue(char *value, unsigned int size)
{
	valueType=NODE_VALUE_TYPE_DATA;
	valueSize=size;
	this->value=malloc(valueSize);
	memcpy(this->value,value,valueSize);
}

char * RegistryNode::getDataValue()
{
	return (char*)value;
}

unsigned int RegistryNode::getDataValueSize()
{
	return valueSize;
}

void RegistryNode::printXML(char * indent)
{
	if (indent)printf("%s",indent);
	
	char indent2[256];
	memset(indent2,' ',250);
	indent2[(!indent ? 2 : strlen(indent)+2)]=0;
	
	if (descType==NODE_DESC_TYPE_DWORD)
		printf("<node desc=\"%d\">",getDwordDescription());
	else
		printf("<node desc=\"%s\">",getStringDescription());
	char*v="\0";
	switch (valueType)
	{
	case NODE_VALUE_TYPE_BYTE:	    
		printf("%d",getByteValue());break;
	case NODE_VALUE_TYPE_WORD:	    
		printf("%d",getWordValue());break;
	case NODE_VALUE_TYPE_DWORD:	    
		printf("%d",getDWordValue());break;
	case NODE_VALUE_TYPE_ULONGLONG:	
		printf("#%I64x",getUlonglongValue());break;
	case NODE_VALUE_TYPE_STRING:	
		printf("%s",getStringValue());break;
	case NODE_VALUE_TYPE_VECTOR:	
		printf("\n");
		getVectorValue()->printXML(indent2);
		if (indent)printf("%s",indent);break;
	default :	    
		if (valueSize>0)
		{
			printf("<![CDATA[\n");
			hexaPrintf((char*)value,valueSize);
			printf("]]>");
			if (indent)printf("%s",indent);}
		break;
	}
	
	printf("</node>\n");
}

int RegistryNode::getValueType()
{
	return valueType;
}

RegistryNode * RegistryNode::getNextNode()
{
	return nextNode;
}
void RegistryNode::setNextNode(RegistryNode * node)
{
	nextNode=node;
}

/*
#define NODE_VALUE_TYPE_BYTE 0
#define NODE_VALUE_TYPE_WORD 1
#define NODE_VALUE_TYPE_DWORD 2
#define NODE_VALUE_TYPE_ULONGLONG 3
#define NODE_VALUE_TYPE_DATA 4
#define NODE_VALUE_TYPE_STRING 5
#define NODE_VALUE_TYPE_VECTOR 6
*/
void RegistryNode::search(char * text, char * path)
{
	if (getValueType()==NODE_VALUE_TYPE_VECTOR)
	{
		getVectorValue()->search(text,path);
		return;
	}
	if (getValueType()==NODE_VALUE_TYPE_DATA && getDataValueSize()<1000)
	{
		char b[1000];
		memset(b,0,1000);
		memcpy(b,getDataValue(),getDataValueSize());
		if (!stricmp(b,text) || strstr(b,text))
		{
			printf("%s : %s\n",path,b);
		}
	}
	if (getValueType()==NODE_VALUE_TYPE_STRING)
	{
		if (!stricmp(getStringValue(),text) || strstr(getStringValue(),text))
		{
			printf("%s : %s\n",path,getStringValue());
		}
	}
}
void RegistryNode::search(DWORD value, char * path)
{
	if (getValueType()==NODE_VALUE_TYPE_VECTOR)
	{
		getVectorValue()->search(value,path);
		return;
	}
	if (getValueType()<=NODE_VALUE_TYPE_ULONGLONG)
	{
		ULONGLONG value2=getUlonglongValue();
		if (value==value2)
		{
			printf("%s : %d\n",path,value);
		}
	}
}

RegistryNode * RegistryNode::copy()
{
	RegistryNode * node=new RegistryNode();
	switch (getDescriptionType())
	{
	case (NODE_DESC_TYPE_DWORD) :
		node->setDwordDescription(getDwordDescription());
		break;
	case (NODE_DESC_TYPE_STRING) :
		node->setStringDescription(getStringDescription());
		break;	
	}
	
	switch (getValueType())
	{
	case (NODE_VALUE_TYPE_BYTE):
		node->setByteValue(getByteValue());
		break;
	case (NODE_VALUE_TYPE_WORD):
		node->setWordValue(getWordValue());
		break;
	case (NODE_VALUE_TYPE_DWORD):
		node->setDWordValue(getDWordValue());
		break;
	case (NODE_VALUE_TYPE_ULONGLONG):
		node->setUlonglongValue(getUlonglongValue());
		break;
	case (NODE_VALUE_TYPE_DATA):
		node->setDataValue(getDataValue(),getDataValueSize());
		break;
	case (NODE_VALUE_TYPE_STRING):
		node->setStringValue(getStringValue());
		break;
	case (NODE_VALUE_TYPE_VECTOR):
		node->setVectorValue(getVectorValue()->copy());
		break;
	}
	return node;
}