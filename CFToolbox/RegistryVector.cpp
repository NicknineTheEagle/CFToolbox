// RegistryVector.cpp: implementation of the RegistryVector class.
//
//////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include "stdafx.h"
#include "RegistryVector.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RegistryVector::RegistryVector()
{
	firstNode=0;
}

RegistryVector::~RegistryVector()
{
	clear();
}

void RegistryVector::clear()
{
	RegistryNode * node=firstNode;
	while (node)
	{
		RegistryNode * tmp=node;
		node=node->getNextNode();
		tmp->clear();
	}
	firstNode=0;
}

void RegistryVector::addNode(RegistryNode * newNode)
{
	if (!firstNode) 
	{
		firstNode=newNode;
		return;
	}
	RegistryNode * node=firstNode;
	while (node->getNextNode())
	{
		node=node->getNextNode();
	}
	node->setNextNode(newNode);
}

unsigned int RegistryVector::getNbNodes()
{
	unsigned nb=0;
	RegistryNode * node=firstNode;
	while (node)
	{
		nb++;
		node=node->getNextNode();
	}
	return nb;
}

RegistryNode * RegistryVector::getNodeAt(unsigned int pos)
{
	RegistryNode * node=firstNode;

	for (unsigned int ind=0;ind<pos;ind++)
	{
		if (!node) return 0;
		node=node->getNextNode();
	}
	return node;
}

void RegistryVector::printXML(char * indent)
{
	
	if (indent)printf("%s",indent);
	printf("<vector>\n");
	
	char indent2[256];
	memset(indent2,' ',250);
	indent2[(!indent ? 2 : strlen(indent)+2)]=0;
	for (unsigned int ind=0;ind<getNbNodes();ind++)
	{
		getNodeAt(ind)->printXML(indent2);
	}
	if (indent)printf("%s",indent);
	printf("</vector>\n");
}

RegistryNode * RegistryVector::getNode(DWORD desc)
{
	RegistryNode * node=firstNode;
	while (node)
	{
		if (node->getDescriptionType()==NODE_DESC_TYPE_DWORD && node->getDwordDescription()==desc) return node;
		node=node->getNextNode();
	}

	return 0;
}

RegistryNode * RegistryVector::getNode(char * desc)
{

	RegistryNode * node=firstNode;
	while (node)
	{
		if (node->getDescriptionType()==NODE_DESC_TYPE_STRING && !strcmp(node->getStringDescription(),desc)) return node;
		node=node->getNextNode();
	}

	return 0;
}

void RegistryVector::search(char * text, char * path)
{
	int allocated=0;
	if (!path)
	{
		path=(char*)malloc(1000);
		strcpy(path,"/");
		allocated=1;
	}
	char tmp[1000];
	strcpy(tmp,path);
	for (unsigned int ind=0;ind<getNbNodes();ind++)
	{
		RegistryNode * node=getNodeAt(ind);
		if (node->getDescriptionType()==NODE_DESC_TYPE_DWORD)
			sprintf(path,"%s/%d",tmp,node->getDwordDescription());
		else
			sprintf(path,"%s/%d",tmp,node->getStringDescription());
		node->search(text,path);
	}

	if (allocated) free(path);
}
void RegistryVector::search(DWORD value, char * path)
{
	int allocated=0;
	if (!path)
	{
		path=(char*)malloc(1000);
		strcpy(path,"");
		allocated=1;
	}
	char tmp[1000];
	strcpy(tmp,path);
	for (unsigned int ind=0;ind<getNbNodes();ind++)
	{
		RegistryNode * node=getNodeAt(ind);
		if (node->getDescriptionType()==NODE_DESC_TYPE_DWORD)
			sprintf(path,"%s/%d",tmp,node->getDwordDescription());
		else
			sprintf(path,"%s/%d",tmp,node->getStringDescription());
		node->search(value,path);
	}

	if (allocated) free(path);
}

RegistryVector * RegistryVector::copy()
{
	RegistryVector * vec=new RegistryVector();

	for(unsigned int ind=0;ind<getNbNodes();ind++)
	{
		vec->addNode(getNodeAt(ind)->copy());
	}
	return vec;
}

int RegistryVector::removeNode(char * text)
{
	if (!firstNode) return 0;
	if (!strcmp(firstNode->getStringDescription(),text))
	{
		RegistryNode * first=firstNode;
		firstNode=firstNode->getNextNode();
		first->setNextNode(0);
		first->clear();
		return 1;
	}
	RegistryNode * node=firstNode;
	while (node->getNextNode())
	{
		if (!strcmp(node->getNextNode()->getStringDescription(),text))
		{
			RegistryNode * tmp=node->getNextNode();
			node->setNextNode(tmp->getNextNode());
			tmp->setNextNode(0);
			tmp->clear();
			return 1;
		}
		node=node->getNextNode();
	}
	return 0;
}
int RegistryVector::removeNode(DWORD value)
{
	if (!firstNode) return 0;
	if (firstNode->getDwordDescription()==value)
	{
		RegistryNode * first=firstNode;
		firstNode=firstNode->getNextNode();
		first->setNextNode(0);
		first->clear();
		return 1;
	}
	RegistryNode * node=firstNode;
	while (node->getNextNode())
	{
		if (node->getNextNode()->getDwordDescription()==value)
		{
			RegistryNode * tmp=node->getNextNode();
			node->setNextNode(tmp->getNextNode());
			tmp->setNextNode(0);
			tmp->clear();
			return 1;
		}
		node=node->getNextNode();
	}
	return 0;
}
