// AppIcons.cpp: implementation of the CAppIcons class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "AppIcons.h"
#include "tools.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAppIcons * theUniqueAppIcons=0;

void initAppIcons(GCF * gcf)
{
	if (!theUniqueAppIcons)	new CAppIcons(gcf);
}

CAppIcons * getAppIcons()
{
	return theUniqueAppIcons;
}

CAppIcons::CAppIcons(GCF * gcf)
{
	ids=new HashMapStr(HASHMAP_NODELETE);
	m_iconsList = new CImageList();
    m_iconsList->Create(16,16,ILC_COLOR32|ILC_MASK,3,3);
	reloadIcons(gcf);
	theUniqueAppIcons=this;
}

CAppIcons::~CAppIcons()
{
	delete ids;
	
}

int CAppIcons::getIconId(char *path)
{
	char lwrpath[200];
	strcpy(lwrpath,path);
	strlwr(lwrpath);

	int id=(int)ids->get(lwrpath);
	return id;
}

void CAppIcons::loadIcons(GCF* gcf)
{
	// scans the folder, would be better to load all the listed icons in the CDR.
	DWORD iconId=0; // start from 0  
	char path[1000];
	DWORD iconsRoot=getFileId(gcf,ICONS_PATH);
	if (iconsRoot)
	{
		GCFDirectoryEntry root=gcf->directory->entry[iconsRoot];
		if (root.firstIndex)
		{
			GCFDirectoryEntry entry=gcf->directory->entry[root.firstIndex];
			DWORD next=0;
			do
			{
				// file is in the icons folder
				char * filename=gcf->directory->directoryNames+entry.nameOffset;
				if (strstr(filename,".tga"))
				{
					// is an icon_xxx.tga
					sprintf(path,"%s/%s",ICONS_PATH,filename);
					strlwr(path);
			




					HICON icon=getIconFromGCFTGA(gcf, path);
					*(strstr(path,".tga"))=0;
					if (icon)
					{
						 m_iconsList->Add(icon);
						 iconId++;
						 ids->put(path,(void*)iconId);
					}
				}
			
				next=entry.nextIndex;
				if (next) entry=gcf->directory->entry[next];
			} while (next);
		}
	}
}
 
void CAppIcons::reloadIcons(GCF *gcf)
{
	ids->clear();
//	m_iconsList->DeleteImageList();
	while (m_iconsList->GetImageCount()) m_iconsList->Remove(0);
	
	m_iconsList->Add(AfxGetApp()->LoadIcon(IDI_EMPTY));
	if ((int)gcf>1)
	{
		loadIcons(gcf);
	}
}
