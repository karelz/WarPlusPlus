// WAVReader.h: interface for the CWAVReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVREADER_H__C3B6B345_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_)
#define AFX_WAVREADER_H__C3B6B345_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SoundReader.h"

class CWAVReader : public CSoundReader  
{
  DECLARE_DYNAMIC(CWAVReader);

public:
	virtual BOOL ReadSound(CDSoundBuffer *pSBuffer);
	virtual BOOL ReadInformations();
	virtual void Close();
	virtual void Open(CArchiveFile *pFile);
  // constructs the object
	CWAVReader();
  // destructs the object
	virtual ~CWAVReader();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  struct tagSRIFFHeader{
    char m_ID[4];
    DWORD m_dwLength;
  };
  typedef struct tagSRIFFHeader SRIFFHeader;

  struct tagSWAVHeader{
    char m_ID[4];
    char m_FormatID[4];
    DWORD m_dwLength;
    WORD m_wFormatTag;
    WORD m_nChannels;
    DWORD m_nSamplesPerSec;
    DWORD m_nAvgBytesPerSec;
    WORD m_nBlockAlign;
    WORD m_FormatSpecific;
  };
  typedef struct tagSWAVHeader SWAVHeader;

  struct tagSDataHeader{
    char m_ID[4];
    DWORD m_dwLength;
  };
  typedef struct tagSDataHeader SDataHeader;
};

#endif // !defined(AFX_WAVREADER_H__C3B6B345_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_)
