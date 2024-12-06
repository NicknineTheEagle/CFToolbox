// HashMap.h: interface for the HashMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HASHMAP_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_)
#define AFX_HASHMAP_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_

#include "afxsock.h"
#include "cftoolbox_dll.h"

//#define NEW_HASHMAPS

#define HASHMAP_NODELETE 0
#define HASHMAP_FREE 1
#define HASHMAP_DELETE 2

#ifdef NEW_HASHMAPS

class HashMapDword  
{
public:
	HashMapDword(int deleteElts=HASHMAP_FREE)
	{
		this->deleteElts=deleteElts;
		hashmap=::newHashMap(false);
	}
	virtual ~HashMapDword()
	{
		clear();
		::deleteHashMap(false,hashmap);
	}

	void clear()
	{
		switch (deleteElts)
		{
		case HASHMAP_NODELETE : ::hashMapClear(false,hashmap);break;
		case HASHMAP_FREE : ::hashMapFreeValues(false,hashmap);break;
		case HASHMAP_DELETE : ::hashMapDeleteValues(false,hashmap);break;
		}
	}

	void remove(DWORD key)
	{
		switch (deleteElts)
		{
		case HASHMAP_NODELETE : ::hashMapRemove(false,hashmap,key);break;
		case HASHMAP_FREE : ::hashMapFree(false,hashmap,key);break;
		case HASHMAP_DELETE : ::hashMapDelete(false,hashmap,key);break;
		}
	}

	void * get(DWORD key)
	{
		return ::hashMapGet(false,hashmap,key);
	}

	void put(DWORD key, void * element)
	{
		::hashMapPut(false,hashmap,key,element);
	}

	int size()
	{
		return ::hashMapSize(false,hashmap);
	}

	int getDWordKeys(DWORD * buffer)
	{
		return ::hashMapGetKeys(false,hashmap,buffer);
	}

private:
	int deleteElts;
	unsigned long hashmap;
};

#else


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct HashDwordElement
{
	DWORD key;
	void * elt;
	HashDwordElement * parent;
	HashDwordElement * prev;
	HashDwordElement * next;
} HashDwordElement;



class HashMapDword  
{
public:
	void clear();
	void remove(DWORD key);
	void * get(DWORD key);

	void put(DWORD key, void * element);

	HashMapDword(int deleteElts=HASHMAP_FREE);
	virtual ~HashMapDword();

	int size();
	int getDWordKeys(DWORD * buffer);

/*	
	int getStrKeys(char * buffer,char * separator);
	int getDWordKeys(DWORD * buffer, int max);
*/
private:
	int size(HashDwordElement * elt);
	int deleteElts;
	HashDwordElement * root;
	void remove(HashDwordElement * elt);
	int getDWordKeys(DWORD * buffer,int pos,HashDwordElement * elt);
/*
	int getStrKeys(char * buffer,char * separator,HashElement * elt);
	int getDWordKeys(DWORD * buffer, int max,int pos,HashElement * elt);
*/
};

#endif

#include "HashMapStr.h"

#endif // !defined(AFX_HASHMAP_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_)

