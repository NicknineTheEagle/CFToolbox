// CFDescriptor.h: interface for the CCFDescriptor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFDESCRIPTOR_H__DECAE85E_E006_445B_899D_F065E46BEAD7__INCLUDED_)
#define AFX_CFDESCRIPTOR_H__DECAE85E_E006_445B_899D_F065E46BEAD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cdr.h"
#include "gcf.h"
#include "Task.h"

class CCFDescriptor  
{
public:
	void update();
	CCFDescriptor(ContentDescriptionRecord cdr, CDRApp app, char * fileName, GCF * gcf, bool accurateCompletion);
	CCFDescriptor(ContentDescriptionRecord cdr, CDRApp app, char * root, bool accurateCompletion);

	virtual ~CCFDescriptor();
	
	DWORD fileId;
	char * description;
	char * fileName;
	char * path;

	char * commonPath;
	char * decryptionKey;

	bool isMoreThan4GB;
	DWORD fileVersion;
	DWORD cdrVersion;
	DWORD nbBlocks;
	DWORD downloadedBlocks;
	DWORD fileSize;
	BOOL isNcf;
	DWORD realSize;
	float completion;
	CTask * runningTask;
	char information[200];
	bool accurateCompletion;
};

#endif // !defined(AFX_CFDESCRIPTOR_H__DECAE85E_E006_445B_899D_F065E46BEAD7__INCLUDED_)
