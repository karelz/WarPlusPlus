// TIFFReader.h: interface for the CTIFFReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIFFREADER_H__BB7316C2_57D8_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_TIFFREADER_H__BB7316C2_57D8_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ImageReader.h"

class CTIFFReader : public CImageReader
{
  DECLARE_DYNAMIC(CTIFFReader);

private:
	BOOL m_bInformationsValid;
	BOOL m_bOpened;
	BOOL ReadStripOffsets();
	BOOL ReadStripFormat();
	BOOL ReadCompresionType();
	BOOL ReadPixelFormat();
  typedef struct{
    WORD NumberFormat;
    WORD Version;
    DWORD FirstIFD;
  } HEADER;
  
  typedef struct{
    WORD Tag;
    WORD Type;
    DWORD Length;
    DWORD Value;
  } IFD_ENTRY;

  enum IFDTypes{
    IFDT_BYTE = 1,
    IFDT_ASCII = 2,
    IFDT_SHORT = 3,
    IFDT_LONG = 4,
    IFDT_RATIONAL = 5
  };

  enum Compresions{
    Compresion_None = 1,
    Compresion_CCITT = 2,
    Compresion_LZW = 5,
    Compresion_PackBits = 32773
  };

	BOOL ReadSizes();
	WORD m_nIFDEntriesCount;
	IFD_ENTRY *m_lpIFDEntries;
	void ConvertNumber(DWORD & dword);
	void ConvertNumber(WORD &word);
	BYTE m_nNumberFormat;
	BOOL FindIFDEntry(WORD wTag, IFD_ENTRY *lpIFDEntry);

public:
	DWORD * m_lpStripOffsets;
	DWORD * m_lpStripByteCounts;
	DWORD m_dwStripsPerImage;
	DWORD m_dwRowsPerStrip;
	WORD m_wCompresion;
	WORD m_wABits;
	WORD m_wBBits;
	WORD m_wGBits;
	WORD m_wRBits;
	WORD m_nSamplesPerPixel;

	virtual BOOL ReadImage(CDDrawSurface *lpSurface);
  virtual BOOL ReadInformations();
	virtual void Open(CArchiveFile *pFile);
	virtual void Close();
	CTIFFReader();
	virtual ~CTIFFReader();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

};

#endif // !defined(AFX_TIFFREADER_H__BB7316C2_57D8_11D2_8EB5_947204C10000__INCLUDED_)
