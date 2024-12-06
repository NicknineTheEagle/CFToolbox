// HashMap.h: interface for the HashMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HASHMAPSTR_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_)
#define AFX_HASHMAPSTR_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_

#include "afxsock.h"

#ifdef NEW_HASHMAPS

class HashMapStr  
{
public:
	HashMapStr(int deleteElts=HASHMAP_FREE)
	{
		this->deleteElts=deleteElts;
		hashmap=::newHashMap(true);
	}
	virtual ~HashMapStr()
	{
		clear();
		::deleteHashMap(true,hashmap);
	}

	void clear()
	{
		switch (deleteElts)
		{
		case HASHMAP_NODELETE : ::hashMapClear(true,hashmap);break;
		case HASHMAP_FREE : ::hashMapFreeValues(true,hashmap);break;
		case HASHMAP_DELETE : ::hashMapDeleteValues(true,hashmap);break;
		}
	}

	void remove(char * key)
	{
		switch (deleteElts)
		{
		case HASHMAP_NODELETE : ::hashMapRemove(true,hashmap,(DWORD)key);break;
		case HASHMAP_FREE : ::hashMapFree(true,hashmap,(DWORD)key);break;
		case HASHMAP_DELETE : ::hashMapDelete(true,hashmap,(DWORD)key);break;
		}
	}

	void * get(char * key)
	{
		return ::hashMapGet(true,hashmap,(DWORD)key);
	}

	void put(char * key, void * element)
	{
		::hashMapPut(true,hashmap,(DWORD)key,element);
	}

	int size()
	{
		return ::hashMapSize(true,hashmap);
	}

	int getStrKeys(char ** buffer)
	{
		return ::hashMapGetKeys(true,hashmap,(DWORD*)buffer);
	}

private:
	int deleteElts;
	unsigned long hashmap;
};

#else


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct HashStrElement
{
	char * strKey;
	void * elt;
	HashStrElement * parent;
	HashStrElement * prev;
	HashStrElement * next;
} HashStrElement;

 

class HashMapStr  
{
public:
	void clear();
 
	void remove(char * key);
	void * get(char * key);
	void put(char * key, void * element);
// 	void put(char * key, DWORD element);

	HashMapStr(int deleteElts=HASHMAP_FREE);
	virtual ~HashMapStr();

	int size();
	int getStrKeys(char ** buffer);
 

/*	
	int getStrKeys(char * buffer,char * separator);
	int getDWordKeys(DWORD * buffer, int max);
*/
private:
	int size(HashStrElement * elt);
	int deleteElts;
	HashStrElement * root;
	void remove(HashStrElement * elt);
	int getStrKeys(char ** buffer,int pos,HashStrElement * elt); 
/*
	int getStrKeys(char * buffer,char * separator,HashElement * elt);
	int getDWordKeys(DWORD * buffer, int max,int pos,HashElement * elt);
*/
};

#endif

void freeHashMapGetStrKeys(char** keys,int size);

#endif // !defined(AFX_HASHMAP_H__C85D9EC8_F45B_499D_8ADA_287ABB6D28FF__INCLUDED_)
