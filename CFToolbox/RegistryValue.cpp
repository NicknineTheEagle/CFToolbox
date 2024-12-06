// RegistryValue.cpp: implementation of the RegistryValue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "malloc.h"
#include "string.h"
#include "RegistryValue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RegistryValue::RegistryValue()
{
	name=0;
	type=-1;
	value=0;
/*	intValue=0;
	stringValue=0;

	dataValue=0;
	dataValueSize=-1;*/
	nextValue=0;
}

RegistryValue::~RegistryValue()
{
	
	if (name) free(name);
	if (value) free(value);
	/*if (dataValue) free(dataValue);
	if (stringValue) free(stringValue);*/
}

void RegistryValue::setType(int type)
{
	this->type=type;
}

int RegistryValue::getType()
{
	return type;
}

char * RegistryValue::getName()
{
	return name;
}

void RegistryValue::setName(char *newName)
{
	if (name) free(name);
	name=(char*)malloc(strlen(newName)+1);
	strcpy(name,newName);
}

void RegistryValue::setString(char *newValue)
{
	setType(TYPE_STRING);
	if (value) free(value);
	dataValueSize=strlen(newValue)+1;
	value= malloc(dataValueSize);
	strcpy((char*)value,newValue);
}

char * RegistryValue::getString()
{
	return (char*)value;
}

void RegistryValue::setDWORD(DWORD newValue)
{
	setType(TYPE_DWORD);
	
	if (value) free(value);
	value= malloc(4);
	*(DWORD*)value=newValue;
}
void RegistryValue::setULONGLONG(ULONGLONG newValue)
{
setData((char*)&newValue,8);
}	
DWORD RegistryValue::getDWORD()
{
	return *(DWORD*)value;
}
ULONGLONG RegistryValue::getULONGLONG()
{
	return *(ULONGLONG*)getData();
}

void RegistryValue::setData(char *newValue, unsigned long size)
{
	setType(TYPE_DATA);
	if (value) free(value);
	value=malloc(size);
	memcpy(value,newValue,size);
	dataValueSize=size;
}

char * RegistryValue::getData()
{
	return (char*)value;
}

DWORD RegistryValue::getDataSize()
{
	return dataValueSize;	
}

RegistryValue * RegistryValue::getNextValue()
{
	return nextValue;
}

void RegistryValue::setNextValue(RegistryValue * next)
{
	nextValue=next;
}