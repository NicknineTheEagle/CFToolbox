// Download.h: interface for the CDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOWNLOAD_H__764A3BE9_815B_4007_BA41_F3B032C052D9__INCLUDED_)
#define AFX_DOWNLOAD_H__764A3BE9_815B_4007_BA41_F3B032C052D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDownload  
{
public:
	CDownload();
	virtual ~CDownload();
	
private:
	DWORD cfId;
	char * account;
	char * filePath;

	int paused;
	int error;

	

};

#endif // !defined(AFX_DOWNLOAD_H__764A3BE9_815B_4007_BA41_F3B032C052D9__INCLUDED_)
