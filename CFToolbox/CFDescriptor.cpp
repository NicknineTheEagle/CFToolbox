// CFDescriptor.cpp: implementation of the CCFDescriptor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "CFDescriptor.h"
#include "CSClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCFDescriptor::CCFDescriptor(ContentDescriptionRecord cdr, CDRApp app, char * _fileName, GCF *gcf, bool accurateCompletion)
{	
	isMoreThan4GB=false;
	this->accurateCompletion=accurateCompletion;
	runningTask=0;
	fileName=0;
	path=0;
	commonPath=0;
	decryptionKey=0;
	description=0;
	strcpy(information,"");
	
	fileName=(char*)malloc(strlen(_fileName)+1);
	strcpy(fileName,_fileName);

	description=(char*)malloc(strlen(app.name)+1);
	strcpy(description,app.name);

	
	path=(char*)malloc(strlen(gcf->_fileName)+1);
	strcpy(path,gcf->_fileName);

	fileId=gcf->appId;
	fileSize=gcf->fileSize;

	
    fileVersion=gcf->appVersion;
	cdrVersion=app.currentVersionId;
	nbBlocks=gcf->dataBlocks->blockCount;

	downloadedBlocks=gcf->dataBlocks->blocksUsed;
	isNcf=gcf->cfType>1;//app.appOfManifestOnlyCache;
	realSize=getGCFRealSize(gcf); 
	char root[1000];
	strcpy(root,gcf->_fileName);
	*strrchr(root,'\\')=0;

	
	CDRAppVersion version=getCDRAppVersion(app,getCDRAppNbVersions(app)-1);
	if (version.isEncryptionKeyAvailable)
	{
		decryptionKey=(char*)malloc(strlen(version.depotEncryptionKey)+1);
		strcpy(decryptionKey,version.depotEncryptionKey);
	}

	
	if (isNcf)
	{
		CDRApp manifest=getCDRApp(cdr,app.appOfManifestOnlyCache);
/*
 		commonPath=(char*)malloc(strlen(manifest.installDirName)+1);
		strcpy(commonPath,manifest.installDirName);
*/		 
	/*
		RegistryVector * vector=getCDRAppUserDefinedData(manifest);  
 	
		if (vector)
		{
			RegistryNode * gamedir=vector->getNode("gamedir");
			if (gamedir) 
			{
		 
				commonPath=(char*)malloc(strlen(gamedir->getStringValue())+1);
				strcpy(commonPath,gamedir->getStringValue());
			}
		}*/
 		commonPath=(char*)malloc(strlen(manifest.installDirName)+1);
		strcpy(commonPath,manifest.installDirName);

	}
	else isMoreThan4GB=(fileSize64(path)>0xFFFFFFFF);


	completion=getGCFCompletion(gcf,root,commonPath,accurateCompletion);	
}

CCFDescriptor::CCFDescriptor(ContentDescriptionRecord cdr, CDRApp app, char *_root, bool accurateCompletion)
{	
	isMoreThan4GB=false;
	this->accurateCompletion=accurateCompletion;
	runningTask=0;
	fileName=0;
	path=0;
	commonPath=0;
	decryptionKey=0;
	description=0;
	strcpy(information,"");
	
  	fileName=(char*)malloc(strlen(app.installDirName)+4+1);
	sprintf(fileName,"%s.%s",app.installDirName,(app.appOfManifestOnlyCache ? "ncf" : "gcf"));
	strlwr(fileName);

	description=(char*)malloc(strlen(app.name)+1);
	strcpy(description,app.name);

	
	path=(char*)malloc(strlen(_root)+1+strlen(fileName)+1);
	sprintf(path,"%s\\%s",_root,fileName);

	fileId=app.appId;
	fileSize=app.maxCacheFileSizeMb*1024*1024;

	
    fileVersion=app.currentVersionId;
	cdrVersion=app.currentVersionId;
	nbBlocks=0;

	downloadedBlocks=0;
	isNcf=app.appOfManifestOnlyCache;//app.appOfManifestOnlyCache;
	realSize=app.maxCacheFileSizeMb*1024*1024;
  	
	CDRAppVersion version=getCDRAppVersion(app,getCDRAppNbVersions(app)-1);
	if (version.isEncryptionKeyAvailable)
	{
		decryptionKey=(char*)malloc(strlen(version.depotEncryptionKey)+1);
		strcpy(decryptionKey,version.depotEncryptionKey);
	}
	
	if (isNcf)
	{
		CDRApp manifest=getCDRApp(cdr,app.appOfManifestOnlyCache);
/*
 		commonPath=(char*)malloc(strlen(manifest.installDirName)+1);
		strcpy(commonPath,manifest.installDirName);
*/		 
	/*
		RegistryVector * vector=getCDRAppUserDefinedData(manifest);  
 	
		if (vector)
		{
			RegistryNode * gamedir=vector->getNode("gamedir");
			if (gamedir) 
			{
		 
				commonPath=(char*)malloc(strlen(gamedir->getStringValue())+1);
				strcpy(commonPath,gamedir->getStringValue());
			}
		}*/
 		commonPath=(char*)malloc(strlen(manifest.installDirName)+1);
		strcpy(commonPath,manifest.installDirName);

	}
	else isMoreThan4GB=(fileSize64(path)>0xFFFFFFFF);

	completion=0;
}

CCFDescriptor::~CCFDescriptor()
{
	if (description) free(description);
	if (fileName) free(fileName);
	if (path) free(path);
	if (commonPath) free(commonPath);
	if (decryptionKey) free(decryptionKey);
}

void CCFDescriptor::update()
{
	isMoreThan4GB=false;
	
	GCF * gcf=parseGCF(path);
	if ((int)gcf==0) return; // file is not existing or locked
	if ((int)gcf==1)
	{
		// corrupted
		completion=0;
		return;
	}
	fileId=gcf->appId;
	fileSize=gcf->fileSize;
    fileVersion=gcf->appVersion;
	realSize=getGCFRealSize(gcf);

	nbBlocks=gcf->dataBlocks->blockCount;
	downloadedBlocks=gcf->dataBlocks->blocksUsed;

	isNcf=gcf->cfType>1;//app.appOfManifestOnlyCache;
	char root[1000];
	strcpy(root,gcf->_fileName);
	*strrchr(root,'\\')=0;
	completion=getGCFCompletion(gcf,root,commonPath,accurateCompletion);
	
	if (!isNcf) isMoreThan4GB=(fileSize64(path)>0xFFFFFFFF);
	
	deleteGCF(gcf);
}
