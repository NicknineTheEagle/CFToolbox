// HashMap.cpp: implementation of the HashMap class.
//
//////////////////////////////////////////////////////////////////////
 
#include "HashMap.h"

#ifndef NEW_HASHMAPS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HashMapDword::HashMapDword(int deleteElts)
{
	this->deleteElts=deleteElts;
	root=0;
}

void deleteKeys(HashDwordElement * elt,int deleteElts)
{
	if (!elt) return ;
	if (elt->elt)
	{
		switch (deleteElts)
		{
		case HASHMAP_FREE : free(elt->elt);break;
		case HASHMAP_DELETE : delete elt->elt;break;
		}
	}
 
	if (elt->prev)
	{
		deleteKeys(elt->prev,deleteElts);
		delete elt->prev;
	}
	if (elt->next)
	{
		deleteKeys(elt->next,deleteElts);
		delete elt->next;
	}
}

HashMapDword::~HashMapDword()
{
	clear();
}


HashDwordElement * getKey(HashDwordElement *elt,DWORD key, int create)
{
	if (elt->key==key) return elt;
	if (key>elt->key)
	{
		if (elt->next) return getKey(elt->next,key,create);
		if (!create) return 0;
		elt->next=new HashDwordElement;
		elt->next->parent=elt;
		
		elt->next->key=key;
 
		elt->next->prev=0;
		elt->next->next=0;
		elt->next->elt=0;
		
		return elt->next;
	}
	if (key<elt->key)
	{
		if (elt->prev) return getKey(elt->prev,key,create);
		if (!create) return 0;
		elt->prev=new HashDwordElement;
		
		elt->prev->parent=elt;

		elt->prev->key=key;
 
		elt->prev->prev=0;
		elt->prev->next=0;
		elt->prev->elt=0;
		return elt->prev;
	}
	// impossible
	return 0;
}
 

void HashMapDword::put(DWORD key, void *element)
{
	if (!root)
	{
		root=new HashDwordElement;
		root->parent=0;
		root->key=key;
 
		root->elt=element;
		root->prev=0;
		root->next=0;
	}
	else
	{
		HashDwordElement * elt=root;
		
		while (elt->key!=key) elt=getKey(elt,key,1);
		
		elt->elt=element;
	}
}
 
void * HashMapDword::get(DWORD key)
{
	if (!root) return 0;
	HashDwordElement * elt=getKey(root,key,0);
	if (!elt) return 0;
	return elt->elt;
} 
 

int HashMapDword::getDWordKeys(DWORD * buffer,int pos,HashDwordElement * elt)
{
	int nbBefore=0;
	int nbAfter=0;

	if (elt->prev)
	{
		nbBefore=getDWordKeys(buffer,pos,elt->prev);
	} 
	
	if (buffer) buffer[pos+nbBefore]=elt->key;
	
	if (elt->next)
	{
		nbAfter=getDWordKeys(buffer,pos+nbBefore+1,elt->next);
	}

	return nbBefore+1+nbAfter;
}


int HashMapDword::getDWordKeys(DWORD * buffer)
{
	if (!root) return 0;
	return getDWordKeys(buffer,0,root);
}

void HashMapDword::remove(DWORD key)
{
	if (!root) return;
	HashDwordElement * elt=getKey(root,key,0);
	if (!elt) return;

	remove(elt);
}
 
 
void HashMapDword::remove(HashDwordElement * elt)
{
		HashDwordElement * newTree=0;

		if (elt->prev)
		{ // there is a prev, adding next at maximum of it
			newTree=elt->prev;
			HashDwordElement * addTo=newTree;

			while (addTo->next) addTo=addTo->next;
			addTo->next=elt->next;
			if (elt->next) elt->next->parent=addTo;
		}
		else
		if (elt->next)
		{ // there is a next, but no prev
			newTree=elt->next;
		}
		
		if (newTree) newTree->parent=elt->parent;

		if (!elt->parent)
		{
			root=newTree;
		}
		else
		{
			if (elt->parent->prev==elt) elt->parent->prev=newTree;
			if (elt->parent->next==elt) elt->parent->next=newTree;
		}

	if (elt->elt)
	{
		switch (deleteElts)
		{
		case HASHMAP_FREE : free(elt->elt);break;
		case HASHMAP_DELETE : delete elt->elt;break;
		}
	}
 
	delete elt;
}

void HashMapDword::clear( )
{
	deleteKeys(root,deleteElts);
	delete root;
	root=0;
}

int HashMapDword::size( )
{
	return size(root);
}

int HashMapDword::size(HashDwordElement *elt)
{
	if (!elt) return 0;
	return size(elt->prev)+1+size(elt->next);
}
 
#endif