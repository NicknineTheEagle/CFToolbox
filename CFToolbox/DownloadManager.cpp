// DownloadManager.cpp: implementation of the CDownloadManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "DownloadManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDownloadManager::CDownloadManager(ContentDescriptionRecord * newCdr)
{
	cdr=newCdr;
}

CDownloadManager::~CDownloadManager()
{
	
}

HashMapDword * CDownloadManager::getAppList()
{
	HashMapDword * result=new HashMapDword(0);
	if (!cdr) return result;
	
	RegistryVector * root=cdr->vector;
	if (!root) return result;
	
	RegistryVector * apps=root->getNode(1)->getVectorValue();
	for (unsigned int ind=0;ind<apps->getNbNodes();ind++)
	{
		RegistryNode * appNode=apps->getNodeAt(ind);
		RegistryNode * node14=appNode->getVectorValue()->getNode(14);
		if (node14)
		{
			if (node14->getVectorValue()->getNode("gamedir") && strlen(node14->getVectorValue()->getNode("developer")->getStringValue())>0)
			{
				
				char * appName=appNode->getVectorValue()->getNode(2)->getStringValue();
				result->put(appNode->getDwordDescription(),appName);
			}
		}
	}
	
	return result;
}
HashMapDword * CDownloadManager::getCfList()
{
	HashMapDword * result=new HashMapDword(0);
	if (!cdr) return result;
	
	RegistryVector * root=cdr->vector;
	if (!root) return result;
	
	RegistryVector * rootCDR=0;
	RegistryVector * gcfsCDR=root->getNode(1)->getVectorValue();
	for (unsigned int ind=0;ind<gcfsCDR->getNbNodes();ind++)
	{
		RegistryNode * gcfCDR=gcfsCDR->getNodeAt(ind);
		RegistryVector * details=gcfCDR->getVectorValue();
		RegistryNode * node14=details->getNode(14);
		if (!node14)
		{
			DWORD appId=details->getNode(1)->getDWordValue();
			char * name=details->getNode(3)->getStringValue();
			char * nameLabel=gcfCDR->getVectorValue()->getNode(2)->getStringValue();
			int lastVersion=gcfCDR->getVectorValue()->getNode(11)->getDWordValue();
			result->put(appId,name);
		}
		else
		{
			if (!(node14->getVectorValue()->getNode("gamedir") && strlen(node14->getVectorValue()->getNode("developer")->getStringValue())>0))
			{
				DWORD appId=details->getNode(1)->getDWordValue();
				char * name=details->getNode(3)->getStringValue();
				char * nameLabel=gcfCDR->getVectorValue()->getNode(2)->getStringValue();
				int lastVersion=gcfCDR->getVectorValue()->getNode(11)->getDWordValue();
				result->put(appId,name);
			}
		}
	}
	
	return result;
}
HashMapDword * CDownloadManager::getCfList(int app)
{
	HashMapDword * result=new HashMapDword(0);
	if (!cdr) return result;
	
	RegistryVector * root=cdr->vector;
	if (!root) return result;
	RegistryVector * gcfsCDR=root->getNode(1)->getVectorValue();
	RegistryNode * appNode=gcfsCDR->getNode(app);
	if (appNode)
	{
		RegistryNode * gcfs=appNode->getVectorValue()->getNode(12);
		if (gcfs)
		{
			RegistryVector * vecGcf=gcfs->getVectorValue();
			for (unsigned int ind=0;ind<vecGcf->getNbNodes();ind++)
			{
				RegistryNode * gcf=vecGcf->getNodeAt(ind)->getVectorValue()->getNode(1);
				int optional=vecGcf->getNodeAt(ind)->getVectorValue()->getNode(3)->getByteValue();
				
				RegistryNode * gcfCDR=gcfsCDR->getNode(gcf->getDWordValue());
				RegistryVector * details=gcfCDR->getVectorValue();
				DWORD appId=details->getNode(1)->getDWordValue();
				char * name=details->getNode(3)->getStringValue();
				char * nameLabel=gcfCDR->getVectorValue()->getNode(2)->getStringValue();
				int lastVersion=gcfCDR->getVectorValue()->getNode(11)->getDWordValue();
				result->put(appId,name);
			}
		}
		
	}
	return result;
}

CDRApp CDownloadManager::getAppDetails(int app)
{
	return getCDRApp(*cdr,app);
}