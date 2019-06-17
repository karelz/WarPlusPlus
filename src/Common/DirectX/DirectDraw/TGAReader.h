// TGAReader.h: interface for the CTGAReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TGAREADER_H__64AF7621_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
#define AFX_TGAREADER_H__64AF7621_7B24_11D2_AB5A_99CC716C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ImageReader.h"

class CTGAReader : public CImageReader  
{
public:
	virtual void Close();
	virtual BOOL ReadImage(CDDrawSurface *lpSurface);
	DWORD m_dwImageOffset;
	virtual BOOL ReadInformations();
	virtual void Open(CArchiveFile *pFile, void (*FinishRGBProc)(CDDrawSurface *, LPBYTE, DWORD, LPVOID), LPVOID pFinishRGBProcParam);
	CTGAReader();
	virtual ~CTGAReader();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// targa header
#pragma pack(1) // for odd adresses of words
typedef struct{
  BYTE m_nIDFieldLength;
  BYTE m_nColorMapType;
  BYTE m_nImageType;
  WORD m_wColorMapOrigin;
  WORD m_wColorMapLength;
  BYTE m_nColorMapEntrySize;
  WORD m_wXOrigin;
  WORD m_wYOrigin;
  WORD m_wWidth;
  WORD m_wHeight;
  BYTE m_nPixelSize;
  BYTE m_nFlags;
}TGAHeader;
#pragma pack()

private:
};

#endif // !defined(AFX_TGAREADER_H__64AF7621_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
