// TGAReader.h: interface for the CTGAReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TGAREADER_H__64AF7620_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
#define AFX_TGAREADER_H__64AF7620_7B24_11D2_AB5A_99CC716C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ImageReader2.h"

class CTGAReader2 : public CImageReader2  
{
public:
	virtual void Close();
	virtual BOOL ReadImage(BYTE * pByte, DWORD dwPitch);
	virtual BOOL ReadInformations();
	virtual void Open(LPCSTR lpszFileName);
	CTGAReader2();
	virtual ~CTGAReader2();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	CFile m_File;
	DWORD m_nPixelSize;
};

#endif // !defined(AFX_TGAREADER_H__64AF7620_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
