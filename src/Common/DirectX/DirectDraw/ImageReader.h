// ImageReader.h: interface for the CImageReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEREADER_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_IMAGEREADER_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DDrawSurface.h"

// Abstract class for Image readers
class CImageReader : public CObject
{
  DECLARE_DYNAMIC(CImageReader);

public:
	BOOL m_bAlphaChannel;
  virtual void Close();
	virtual BOOL ReadImage(CDDrawSurface *lpSurface);
	virtual BOOL ReadInformations();
	virtual void Open(CArchiveFile *pFile, void (*FinishRGBProc)(CDDrawSurface *, LPBYTE, DWORD, LPVOID), LPVOID pFinishRGBProcParam);
	DWORD m_dwHeight;
	DWORD m_dwWidth;
	CImageReader();
	virtual ~CImageReader();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

protected:
  CArchiveFile *m_pFile;
  void (*m_pFinishRGBProc)(CDDrawSurface *, LPBYTE, DWORD, LPVOID);
  LPVOID m_pFinishRGBProcParam;
};

#endif // !defined(AFX_IMAGEREADER_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_)
