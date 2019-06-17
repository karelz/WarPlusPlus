// SoundReader.h: interface for the CSoundReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDREADER_H__C3B6B343_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_)
#define AFX_SOUNDREADER_H__C3B6B343_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSoundBuffer.h"

class CSoundReader : public CObject  
{
  DECLARE_DYNAMIC(CSoundReader);

public:
	DWORD m_dwSampleRate;
	DWORD m_dwLength;
	DWORD m_dwChannels;
	DWORD m_dwBitsPerSample;
	virtual BOOL ReadSound(CDSoundBuffer *pSBuffer);
	virtual BOOL ReadInformations();
	virtual void Close();
	virtual void Open(CArchiveFile *pFile);
	// constructs the object
  CSoundReader();
  // destroys the object
  virtual ~CSoundReader();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	CArchiveFile * m_pFile;
};

#endif // !defined(AFX_SOUNDREADER_H__C3B6B343_A3AB_11D2_ABB9_D4C2A7305D60__INCLUDED_)
