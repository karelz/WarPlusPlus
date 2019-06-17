// ImageReader.h: interface for the CImageReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEREADER2_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_IMAGEREADER2_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Abstract class for Image readers
class CImageReader2 : public CObject
{
  DECLARE_DYNAMIC(CImageReader2);

public:
  virtual void Close();
	virtual BOOL ReadImage(BYTE *pByte, DWORD dwPitch);
	virtual BOOL ReadInformations();
	virtual void Open(LPCSTR lpszFileName);
	DWORD m_dwHeight;
	DWORD m_dwWidth;
	CImageReader2();
	virtual ~CImageReader2();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

};

#endif // !defined(AFX_IMAGEREADER2_H__A0EDEA62_5845_11D2_8EB5_947204C10000__INCLUDED_)
