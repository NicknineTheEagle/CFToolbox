// RegistryKey.cpp: implementation of the RegistryKey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "malloc.h"
#include "string.h"
#include "stdio.h"
#include "memory.h"
#include "socketTools.h"
#include "RegistryKey.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RegistryKey::RegistryKey()
{
	previousSibiling=0;
	nextSibiling=0;
	name=0;
	parent=0;
	firstChild=0;
	value=0;
}

RegistryKey::~RegistryKey()
{
	if (name) free(name);
	name=0;
	RegistryValue * v=value;
	while (v)
	{
		RegistryValue * tmp=v;
		v=v->getNextValue();
		delete tmp;
	}
	if (firstChild) 
	{
		firstChild->parent=0;
		delete firstChild;
	}
	if (nextSibiling) 
	{
		nextSibiling->previousSibiling=0;
		delete nextSibiling;
	}
	if (previousSibiling) 
	{
		previousSibiling->nextSibiling=0;
		delete previousSibiling;
	}
	
}

char * RegistryKey::getName()
{
	return name;
}

void RegistryKey::setName(char *newName)
{
	//printf("setName:%s\n",newName);
	if (name) free(name);
	name=(char*)malloc(strlen(newName)+1);
	strcpy(name,newName);
}

RegistryKey * RegistryKey::getParent()
{
	return parent;
	
}

void RegistryKey::setParent(RegistryKey *newParent)
{
	parent=newParent;
}

RegistryKey * RegistryKey::getPreviousSibiling()
{
	return previousSibiling;
}

RegistryKey * RegistryKey::getNextSibiling()
{
	return nextSibiling;
}

RegistryKey * RegistryKey::getFirstSibiling()
{
	if (!previousSibiling) return this;
	return (previousSibiling->getFirstSibiling());
}

RegistryKey * RegistryKey::getLastSibiling()
{
	if (!nextSibiling) return this;
	return (nextSibiling->getLastSibiling());
}

void RegistryKey::setPreviousSibiling(RegistryKey *sibiling)
{
	if (sibiling) sibiling->nextSibiling=this;
	previousSibiling=sibiling;
}

void RegistryKey::setNextSibiling(RegistryKey *sibiling)
{
	if (sibiling) sibiling->previousSibiling=this;
	nextSibiling=sibiling;
}

void RegistryKey::insertPreviousSibiling(RegistryKey *sibiling)
{
	sibiling->setPreviousSibiling(previousSibiling);
	sibiling->setNextSibiling(this);
	previousSibiling=sibiling;
}

void RegistryKey::insertNextSibiling(RegistryKey *sibiling)
{
	sibiling->setPreviousSibiling(this);
	sibiling->setNextSibiling(nextSibiling);
	nextSibiling=sibiling;
}

void RegistryKey::setFirstChild(RegistryKey *child)
{
	firstChild=child;
	if (child) child->setParent(this);
}

RegistryKey * RegistryKey::getFirstChild()
{
	return firstChild;
}

void RegistryKey::addChild(RegistryKey *child)
{
	//printf("addChild:%s\n",child->getName());
	if (!firstChild) setFirstChild(child);
	else
		getFirstChild()->getLastSibiling()->setNextSibiling(child);
}

RegistryKey* RegistryKey::remove()
{
	if (parent && this==parent->getFirstChild())
	{
		parent->setFirstChild(nextSibiling);
	}
	if (previousSibiling)
	{
		previousSibiling->setNextSibiling(nextSibiling);
	}
	
	if (nextSibiling)
	{
		nextSibiling->setPreviousSibiling(previousSibiling);
	}
	nextSibiling=0;
	previousSibiling=0;
	return this;
}

RegistryKey * RegistryKey::getChild(char *name)
{
	if (!firstChild) return 0;
	RegistryKey * child=firstChild->getFirstSibiling();
	do
	{
		if (!strcmp(name,child->getName())) return child;
		child=child->nextSibiling;
	} while (child);
	return 0;
}

RegistryKey * RegistryKey::getKey(char * path)
{
	if (!path || strlen(path)==0)
		return this;
	char * pos=strchr(path,'/');
	if (pos==path)
	{
		char * current=path+1;
		
		char * currentPos=strchr(current,'/');
		int len=0;
		if (!currentPos) 
			len=strlen(current);
		else
			len=currentPos-current;
		char * elt=(char*)malloc(len+1);
		memcpy(elt,current,len);
		elt[len]=0;
		int diff=strcmp(name,elt);
		free(elt);
		if (diff) return 0;
		if (!currentPos) return this;

		path=currentPos+1;
		pos=strchr(path,'/');
	}
	char * elt=path;
	if (pos>0)
	{
		elt=(char*)malloc(pos+1-path);
		memcpy(elt,path,pos-path);
		elt[pos-path]=0;
		pos++;
	}
	RegistryKey * next=getChild(elt);
	if (pos>0) free(elt);
	if (!next) return 0;
	return next->getKey(pos );
}
RegistryKey * RegistryKey::setKey(char * path)
{
	//printf("%s %s\n",name,path);
	if (!path|| strlen(path)==0)
		return this;
	char * pos=strchr(path,'/');
	if (pos==path)
	{
		char * current=path+1;
	
		char * currentPos=strchr(current,'/');
		int len=0;
		if (!currentPos) 
			len=strlen(current);
		else
			len=currentPos-current;
		char * elt=(char*)malloc(len+1);
		memcpy(elt,current,len);
		elt[len]=0;
		
		int diff=strcmp(name,elt);
		free(elt);
		if (diff) return 0;
		if (!currentPos) return this;
		path=currentPos+1;
		pos=strchr(path,'/');
	}
	char * elt=path;
	if (pos>0)
	{
		elt=(char*)malloc(pos+1-path);
		memcpy(elt,path,pos-path);
		elt[pos-path]=0;
		pos++;
	}
	RegistryKey * next=getChild(elt);
	
	if (!next) 
	{
		next=new RegistryKey();
		next->setName(elt);
		addChild(next);
	}
	if (pos>0) free(elt);
	return next->setKey(pos );
}
void RegistryKey::addValue(RegistryValue *newValue)
{
	if (value && !strcmp(newValue->getName(),value->getName()))
	{
		RegistryValue * v=value;
		value=v->getNextValue();
		delete v;
	}
	
	if (!value) 
	{
		value=newValue;
		return;
	}
	RegistryValue * v=value;
	while (v->getNextValue())
	{
		if (!strcmp(newValue->getName(),v->getNextValue()->getName()))
		{
			newValue->setNextValue(v->getNextValue()->getNextValue());
			delete v->getNextValue();
			v->setNextValue(newValue);
			return;
		}
		v=v->getNextValue();
	}
	v->setNextValue(newValue);
}
RegistryValue * RegistryKey::getFirstValue()
{
	return value;
}
RegistryValue * RegistryKey::getValue(char * name)
{
	RegistryValue * v=value;
	while (v)
	{
		if (!strcmp(name,v->getName()))
		{
			return v;
		}
		v=v->getNextValue();
	}

	return 0;
}
void RegistryKey::removeValue(char *name)
{
	if (!value) 
	{
		return;
	}
	if (!strcmp(name,value->getName()))
	{
		RegistryValue * v=value;
		value=v->getNextValue();
		v->setNextValue(0);
		delete v;
		return;
	}
	RegistryValue * v=value;
	while (v->getNextValue())
	{
		if (!strcmp(name,v->getNextValue()->getName()))
		{
			RegistryValue * tmp=v->getNextValue();
			v->setNextValue(v->getNextValue()->getNextValue());
			tmp->setNextValue(0);
			delete tmp;
			return;
		}
		v=v->getNextValue();
	}
}
/*
void RegistryKey::setValue(RegistryValue * value)
{
	removeValue(value->getName());
	addValue(value);
}
unsigned int RegistryKey::getNbValues()
{
	return nbValues;
}


RegistryValue * RegistryKey::valueAt(unsigned int ind)
{
	return values[ind];
}

void RegistryKey::removeValueAt(unsigned int ind)
{
	delete values[ind];

	for (unsigned int i=ind;i<nbValues-1;i++)
	{
		values[i]=values[i+1];
	}
	nbValues--;
}*/

void RegistryKey::printXML(int withHex, char * indent)
{
	if (indent)printf("%s",indent);
	printf("<%s",name);
	RegistryValue * v=value;
	while (v)
	{
		//char*v="\0";
		switch (v->getType())
		{
		case 0:	printf("\n    %s=\"%s\"",v->getName(),v->getString());break;
		case 1:	printf("\n    %s=\"%d\"",v->getName(),v->getDWORD());break;
		case 2:	printf("\n    %s=\"#\n",v->getName(),"");printf("<![CDATA[ ");
		if (withHex)	hexaPrintf(v->getData(),v->getDataSize());
			else	printf("####");
			printf(" ]]>");
			printf("\"");
			break;
		}
		v=v->getNextValue();
	}
	printf(">\n");
	
	char indent2[256];
	memset(indent2,' ',250);
	indent2[(!indent ? 2 : strlen(indent)+2)]=0;
	
	
	RegistryKey * child=firstChild;
	while (child)
	{
		child->printXML(withHex,indent2);
		child=child->getNextSibiling();
	}
	
	if (indent)printf("%s",indent);
	printf("</%s>\n",name);
}