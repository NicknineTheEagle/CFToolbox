// DownloadManager.h: interface for the CDownloadManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOWNLOADMANAGER_H__918D7722_1E7F_46BA_8374_68B6F58AB8FD__INCLUDED_)
#define AFX_DOWNLOADMANAGER_H__918D7722_1E7F_46BA_8374_68B6F58AB8FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "cdr.h"

#include "HashMap.h"

class CDownloadManager  
{
public:

	CDownloadManager(ContentDescriptionRecord * cdr);
	virtual ~CDownloadManager();

	HashMapDword * getAppList();
	HashMapDword * getCfList();
	HashMapDword * getCfList(int appId);	
	CDRApp getAppDetails(int app);
private:
	ContentDescriptionRecord * cdr;

};

#endif // !defined(AFX_DOWNLOADMANAGER_H__918D7722_1E7F_46BA_8374_68B6F58AB8FD__INCLUDED_)
