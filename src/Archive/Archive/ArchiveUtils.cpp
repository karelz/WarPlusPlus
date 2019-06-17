// ArchiveUtils.cpp: implementation of the CArchiveUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveUtils.h"

#include "../Compression/CompressEngine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveUtils::CArchiveUtils()
{

}

CArchiveUtils::~CArchiveUtils()
{

}

BOOL CArchiveUtils::IsCompressed(CString strFileName)
{
	
	CString strCmpName;
	strFileName.TrimRight();
	strFileName.TrimRight('\\');
	strCmpName=strFileName+CCompressEngine::GetDefaultArchiveExtension();
	
	CFileFind finder;		
	BOOL bFound=TRUE;
	bFound=bFound && finder.FindFile(strCmpName);
	if(bFound) {
		finder.FindNextFile();
		bFound=bFound && (!finder.IsDirectory() && !finder.IsDots());
	}
	return bFound;
}

BOOL CArchiveUtils::IsFile(LPCTSTR lpcszFileName)
{
	CString strFileName;
	CFileFind finder;
	
	if(!finder.FindFile(lpcszFileName)) return FALSE;
	finder.FindNextFile();
	return finder.IsDirectory()?FALSE:TRUE;
}

BOOL CArchiveUtils::IsDirectory(LPCTSTR lpcszDirectoryName)
{
	CString strFileName;
	CFileFind finder;
	
	if(!finder.FindFile(lpcszDirectoryName)) return FALSE;
	finder.FindNextFile();
	return finder.IsDirectory();
}

CString CArchiveUtils::AddExtension(LPCTSTR lpcszDirectoryName) {
	CString strName=lpcszDirectoryName;
	if(!IsFile(strName) && IsFile(strName+CCompressEngine::GetDefaultArchiveExtension())) {
		strName+=CCompressEngine::GetDefaultArchiveExtension();
	}
	return strName;
}

void CArchiveUtils::ClipExtension(CString &strName) {
	CString strExt=strName.Mid(strName.GetLength()-CCompressEngine::GetDefaultArchiveExtension().GetLength());
	strExt.MakeLower();
	if(strExt==CCompressEngine::GetDefaultArchiveExtension()) {
		strName=strName.Left(strName.GetLength()-CCompressEngine::GetDefaultArchiveExtension().GetLength());
	}
}

BOOL CArchiveUtils::HasExtension(CString strName) {
	strName.MakeLower();
	if(strName.GetLength()>=CCompressEngine::GetDefaultArchiveExtension().GetLength() && 
	   strName.Right(CCompressEngine::GetDefaultArchiveExtension().GetLength())==CCompressEngine::GetDefaultArchiveExtension()) {
		return TRUE;
	} else
		return FALSE;
}

CString CArchiveUtils::ConcatenatePaths(CString strPath1, CString strPath2) {
    if(strPath1.IsEmpty() || strPath2.IsEmpty())
        return strPath1 + strPath2;
    else {
	    strPath1.TrimRight();
	    strPath1.TrimRight('\\');
	    strPath1.TrimRight('/');
	    strPath2.TrimLeft();
	    strPath2.TrimLeft('\\');
	    strPath2.TrimLeft('/');
	    return strPath1 + "\\" + strPath2;
    }
}