

#include "Tools.h"
#include "direct.h"
#include "tga.h"
#include "gcf.h"

#include <tlhelp32.h>

#include "wingdi.h"
bool BrowseForFolder(HWND hOwner, char * title, CString& folderpath)
{
	// Create a pointer to a MALLOC (memory allocation object)
	// then get the Shell Malloc.
	IMalloc* pMalloc = 0;
	if(::SHGetMalloc(&pMalloc) != NOERROR)
		return false;
	
	// Now create BROWSEINFO structure, to tell the shell how
	// to display the dialog.
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	
	bi.hwndOwner = hOwner;
	bi.ulFlags=BIF_EDITBOX | BIF_RETURNONLYFSDIRS|BIF_VALIDATE;
	
	bi.lpszTitle = title;
	
	// Now show the dialog and get the itemIDList for the selected folder.
	LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);
	if(pIDL == NULL)
		return false;
	
	// Now create a buffer to store the path, thne get it.
	char buffer[_MAX_PATH];
	if(::SHGetPathFromIDList(pIDL, buffer) == 0)
		return false;
	
	// Finally, set the string to the path, and return true.
	folderpath = buffer;
	return true;
}

bool BrowsForFile(char  szFilters[],bool isOpen,char * title, CString &m_path)
{
	char cwd[1001];
	_getcwd(cwd,1000);
	
	CFileDialog fileDlg (isOpen, "" , title, OFN_HIDEREADONLY, szFilters, NULL);
	
	
	if( fileDlg.DoModal ()==IDOK )
	{
		_chdir(cwd);
		m_path= fileDlg.GetPathName();
		
		return true;	
	}
    _chdir(cwd);
	return false;
}

char * renderTime(DWORD time, char * buffer)
{

	if (time<60)
	{
		sprintf(buffer,"%ds",time);
		return buffer;
	}
	if (time<60*60)
	{
		sprintf(buffer,"%dm %ds",time/60,time%60);
		return buffer;
	}
	time/=60;
	if (time<24*60)
	{
		sprintf(buffer,"%dh %dm",time/60,time%60);
		return buffer;
	}
	time/=24;
	sprintf(buffer,"%dd %dh",time/24,time%24);
	return buffer;
	
}

char * renderSize(signed __int64 size, char * buffer,bool isMoreThan4GB)
{
	if (isMoreThan4GB) 
	{
		strcpy(buffer,"more than 4GB");
		return buffer;
	}	
	if (size<1000)
	{
		sprintf(buffer,"%d Bytes",size);
		return buffer;
	}
	double size2=((double)size)/1024.0;
	if (size2<1000)
	{
		sprintf(buffer,"%4.2f KBytes",size2);
		return buffer;
	}	
	size2/=1024.0;
	if (size2<1000)
	{
		sprintf(buffer,"%4.2f MBytes",size2);
		return buffer;
	} 
	size2/=1024.0;
	sprintf(buffer,"%4.2f GBytes",size2);
	return buffer;
}

float alphaBlend(float color,float color2,float alpha)
{
	return color*alpha+(1.0f-alpha)*color2;
}

DWORD alphaBlend(DWORD color, DWORD background, int negative)
{
	DWORD bR=(background)&0xFF;
	DWORD bG=(background>>8)&0xFF;
	DWORD bB=(background>>16)&0xFF;
	DWORD cR=(color)&0xFF;
	DWORD cG=(color>>8)&0xFF;
	DWORD cB=(color>>16)&0xFF;
	if (negative)
	{
		cR=255-cR;
		cG=255-cG;
		cB=255-cB;
	}
	float alpha=(float)((color>>24)&0xFF)/(float)0xFF;
	
	DWORD r=(DWORD)alphaBlend((float)cR,(float)bR,alpha);
	DWORD g=(DWORD)alphaBlend((float)cG,(float)bG,alpha);
	DWORD b=(DWORD)alphaBlend((float)cB,(float)bB,alpha);
	return (b<<16)+(g<<8)+r;
}

HICON createIconFromRGBA(char * rgba,DWORD dwWidth  ,DWORD dwHeight, DWORD backgroundColor=0x00FFFFFF )
{
	//  HDC hMemDC;
	
    BITMAPV4HEADER bi;
    HBITMAP hBitmap;//, hOldBitmap;
    void *lpBits;
    DWORD x,y;
    HICON hAlphaIcon = NULL;
	
	
    ZeroMemory(&bi,sizeof(BITMAPV4HEADER));
    bi.bV4Size           = sizeof(BITMAPV4HEADER);
    bi.bV4Width           = dwWidth;
    bi.bV4Height          = dwHeight;
    bi.bV4Planes = 1;
    bi.bV4BitCount = 32;
	bi.bV4V4Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.
	
	bi.bV4RedMask   =  0x000000FF;
    bi.bV4GreenMask =  0x0000FF00;
    bi.bV4BlueMask  =  0x00FF0000;
    bi.bV4AlphaMask =  0xFF000000; 
	HDC hdc;
    hdc = GetDC(NULL);
	
    // Create the DIB section with an alpha channel.
    hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, 
        (void **)&lpBits, NULL, (DWORD)0);
	
	ReleaseDC(NULL,hdc);
	
	
    // Create an empty mask bitmap.
	
    HBITMAP hMaskBitmap = CreateBitmap(dwWidth,dwHeight,1,1,NULL);
	
	
    // Set the alpha values for each pixel in the cursor so that
    // the complete cursor is semi-transparent.
	int negative=1; // check if full white will ask for a negative icon
	DWORD *origin=(DWORD*)rgba;
	for (x=0;x<dwWidth;x++)
		for (y=0;y<dwHeight;y++)
		{
			if (((*origin)&0x00FFFFFF)!=0x00FFFFFF)
			{
				negative=0;
				break;
			}
			origin++;
		}
		
		DWORD *lpdwPixel;
		origin=(DWORD*)rgba;
		lpdwPixel = (DWORD *)lpBits;
		for (x=0;x<dwWidth;x++)
			for (y=0;y<dwHeight;y++)
			{
				*lpdwPixel=alphaBlend(*origin,backgroundColor,negative);
				*lpdwPixel&=0x00FFFFFF;
				lpdwPixel++;
				origin++;
			}
			
			ICONINFO ii;
			ii.fIcon = TRUE;
			ii.xHotspot = 0;
			ii.yHotspot = 0;
			ii.hbmMask = hMaskBitmap;
			ii.hbmColor = hBitmap;
			
			// Create the alpha cursor with the alpha DIB section.
			hAlphaIcon = CreateIconIndirect(&ii);
			
			DeleteObject(hBitmap);          
			DeleteObject(hMaskBitmap); 
			
			return hAlphaIcon;
}

HICON getIconFromGCFTGA(GCF * gcf, char * iconpath)
{
	HICON icon=0;
	if ((int)gcf>1)
	{
		// winui.gcf is available and valid
		DWORD iconId=getFileId(gcf,iconpath);
		
		if (iconId)
		{ // icon is in the gcf
			GCFDirectoryEntry iconEntry=gcf->directory->entry[iconId];
			DWORD buffSize=iconEntry.itemSize;
			buffSize=((int)(buffSize/0x8000)+1)*0x8000+0x10;
			char * iconData=(char*)malloc(buffSize);
			
			DWORD size=copyGcfFileContentToBuffer(gcf,iconId,iconData,iconEntry.itemSize,"CF Manager",0);
			if (size)
			{
				TGA * tga=parseTGA(iconData,size);
				if (tga && tga->pixels && tga->width && tga->height)
				{
					icon=createIconFromRGBA((char*)tga->pixels,tga->width,tga->height);
				}
				free(tga->pixels);
				free(tga);
			}
			free(iconData);
		}
	}
	return icon;
}

int isRunningProcess(char * exe)
{
	HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	
	PROCESSENTRY32 processInfo;
	
	processInfo.dwSize=sizeof(PROCESSENTRY32);
	int nb=0;
	while(Process32Next(hSnapShot,&processInfo)!=FALSE)
	{
		if (!stricmp(processInfo.szExeFile,exe))
		{
			nb++;
		}
	}
	
	CloseHandle(hSnapShot);
	return nb;
}

