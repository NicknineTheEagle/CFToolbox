// AppIcons.h: interface for the CAppIcons class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APPICONS_H__300FE3CD_845B_47B1_98F7_041986B9CE4E__INCLUDED_)
#define AFX_APPICONS_H__300FE3CD_845B_47B1_98F7_041986B9CE4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "gcf.h"
#include "HashMap.h"

#define ICONS_PATH "steam/games" // today, only 1 folder with TGA icons;

class CAppIcons  
{
public:
	void reloadIcons(GCF * winui);

	CAppIcons(GCF * winui);
	virtual ~CAppIcons();
	int getIconId(char * path);
	CImageList * m_iconsList;
private :
	HashMapStr * ids;
	void loadIcons(GCF* gcf);
};

void initAppIcons(GCF * gcf);
CAppIcons * getAppIcons();

#endif // !defined(AFX_APPICONS_H__300FE3CD_845B_47B1_98F7_041986B9CE4E__INCLUDED_)
