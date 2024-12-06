// HashMap.cpp: implementation of the HashMap class.
//
//////////////////////////////////////////////////////////////////////
 
#include "HashMap.h" 

#ifndef NEW_HASHMAPS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HashMapStr::HashMapStr(int deleteElts)
{
	this->deleteElts=deleteElts;
	root=0;
}

void deleteKeys(HashStrElement * elt,int deleteElts)
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
	if (elt->strKey) free(elt->strKey);
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

HashMapStr::~HashMapStr()
{
	clear();
}

 
HashStrElement * getKey(HashStrElement *elt,char * key, int create)
{
	int strcmpv=strcmp(elt->strKey,key);
	if (!strcmpv) return elt;
	if (strcmpv>0)
	{
		if (elt->next) return getKey(elt->next,key,create);
		if (!create) return 0;
		elt->next=new HashStrElement;
		elt->next->parent=elt;
		elt->next->strKey=(char*)malloc(strlen(key)+1);
		strcpy(elt->next->strKey,key);
		
		elt->next->prev=0;
		elt->next->next=0;
		return elt->next;
	}
	if (strcmpv<0)
	{
		if (elt->prev) return getKey(elt->prev,key,create);
		if (!create) return 0;
		elt->prev=new HashStrElement;
		elt->prev->parent=elt;
		elt->prev->strKey=(char*)malloc(strlen(key)+1);
		strcpy(elt->prev->strKey,key);
		
		elt->prev->prev=0;
		elt->prev->next=0;
		return elt->prev;
	}
	// impossible
	return 0;
}
 

void HashMapStr::put(char * key, void *element)
{
	if (!root)
	{
		root=new HashStrElement;
		root->parent=0;
		root->strKey=(char*)malloc(strlen(key)+1);
		strcpy(root->strKey,key);
		root->elt=element;
		root->prev=0;
		root->next=0;
	}
	else
	{
		HashStrElement * elt=root;
		
		while (strcmp(elt->strKey,key)) elt=getKey(elt,key,1);
		
		elt->elt=element;
	}
}
/*
void HashMapStr::put(char * key, DWORD element)
{
	if (!root)
	{
		root=new HashStrElement;
		root->parent=0;
		root->strKey=(char*)malloc(strlen(key)+1);
		strcpy(root->strKey,key);
		root->elt=(void*)element;
		root->prev=0;
		root->next=0;
	}
	else
	{
		HashStrElement * elt=root;
		
		while (strcmp(elt->strKey,key)) elt=getKey(elt,key,1);
		
		elt->elt=(void*)element;
	}
}
*/ 
void * HashMapStr::get(char * key)
{
	if (!root) return 0;
	HashStrElement * elt=getKey(root,key,0);
	if (!elt) return 0;
	return elt->elt;
}

int HashMapStr::getStrKeys(char ** buffer,int pos,HashStrElement * elt)
{
	int nbBefore=0;
	int nbAfter=0;

	if (elt->prev)
	{
		nbBefore=getStrKeys(buffer,pos,elt->prev);
	} 
	
	if (buffer) 
	{
		buffer[pos+nbBefore]=(char*)malloc(strlen(elt->strKey)+1);
		strcpy(buffer[pos+nbBefore],elt->strKey);
		//buffer[pos+nbBefore]=elt->strKey;
	}
	
	if (elt->next)
	{
		nbAfter=getStrKeys(buffer,pos+nbBefore+1,elt->next);
	}

	return nbBefore+1+nbAfter;

}

int HashMapStr::getStrKeys(char ** buffer)
{
	if (!root) return 0;
	return getStrKeys(buffer,0,root);
}
 


void HashMapStr::remove(char * key)
{
	if (!root) return;
	HashStrElement * elt=getKey(root,key,0);
	if (!elt) return;

	remove(elt);
}
 
void HashMapStr::remove(HashStrElement * elt)
{
		HashStrElement * newTree=0;

		if (elt->prev)
		{ // there is a prev, adding next at maximum of it
			newTree=elt->prev;
			HashStrElement * addTo=newTree;

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
	if (elt->strKey) free(elt->strKey);
	delete elt;
}

void HashMapStr::clear( )
{
	deleteKeys(root,deleteElts);
	delete root;
	root=0;
}

int HashMapStr::size( )
{
	return size(root);
}

int HashMapStr::size(HashStrElement *elt)
{
	if (!elt) return 0;
	return size(elt->prev)+1+size(elt->next);
}

#endif

void freeHashMapGetStrKeys(char** keys,int size)
{
	for (int ind=0;ind<size;ind++)
	{
		if (keys[ind]) free(keys[ind]);
		keys[ind]=0;
	}
	free(keys);
}