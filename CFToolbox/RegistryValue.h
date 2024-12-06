// RegistryValue.h: interface for the RegistryValue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRYVALUE_H__B52F6742_7359_4DF2_940D_8ECBBEE63D14__INCLUDED_)
#define AFX_REGISTRYVALUE_H__B52F6742_7359_4DF2_940D_8ECBBEE63D14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "afxsock.h"

#define TYPE_STRING 0
#define TYPE_DWORD  1
#define TYPE_DATA   2


class RegistryValue  
{
public:	

	DWORD getDataSize();
	char * getData();
	void setData(char * value, DWORD size);
	DWORD getDWORD();
	ULONGLONG getULONGLONG();
	void setULONGLONG(ULONGLONG newValue);
	void setDWORD(DWORD value);
	char * getString();
	void setString(char * value);
	void setName(char * name);
	char * getName();
	int getType();
		void setType(int type);
	RegistryValue();
	virtual ~RegistryValue();

	RegistryValue * getNextValue();
	void setNextValue(RegistryValue * next);
private:
	char * name;
	DWORD dataValueSize;
	//char * dataValue_;
	//DWORD intValue_;
	//char * stringValue_;
	void * value;
	char type;


	RegistryValue * nextValue;
};

#endif // !defined(AFX_REGISTRYVALUE_H__B52F6742_7359_4DF2_940D_8ECBBEE63D14__INCLUDED_)
