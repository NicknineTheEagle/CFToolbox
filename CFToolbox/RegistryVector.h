// RegistryVector.h: interface for the RegistryVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRYVECTOR_H__1B7B0B13_9705_48FF_893C_AFBE1112D906__INCLUDED_)
#define AFX_REGISTRYVECTOR_H__1B7B0B13_9705_48FF_893C_AFBE1112D906__INCLUDED_
#include "afxsock.h"

class RegistryVector;

#include "RegistryNode.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class RegistryVector  
{
public:
	void printXML(char * indent=0);
	RegistryNode * getNode(DWORD desc);
	RegistryNode * getNode(char * desc);
	RegistryNode * getNodeAt(unsigned int pos);
	unsigned int getNbNodes();
	void addNode(RegistryNode * node);
	void search(char * text, char * path=0);
	void search(DWORD value, char * path=0);
	int removeNode(char * name);
	int removeNode(DWORD value);
	RegistryVector * copy();	
	void clear();
	RegistryVector();
	virtual ~RegistryVector();

private:
	RegistryNode * firstNode;
	//unsigned int nbNodes;

};

#endif // !defined(AFX_REGISTRYVECTOR_H__1B7B0B13_9705_48FF_893C_AFBE1112D906__INCLUDED_)
