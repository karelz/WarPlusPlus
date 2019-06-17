// SoundBuffer.h: interface for the CSoundBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDBUFFER_H__9ABBEDB3_A3F6_11D2_ABBA_E4A802FA4A60__INCLUDED_)
#define AFX_SOUNDBUFFER_H__9ABBEDB3_A3F6_11D2_ABBA_E4A802FA4A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSoundBuffer.h"
#include "..\..\Archive\Archive.h"

class CSoundReader;

class CSoundBuffer : public CDSoundBuffer  
{
  DECLARE_DYNAMIC(CSoundBuffer);

public:
  typedef enum FileTypes{
    FileType_Auto = 0,
    FileType_WAVE = 1
  } EFileType;

	virtual BOOL PostCreate(LPDIRECTSOUNDBUFFER lpDSBuffer, BOOL bCallRelease = TRUE);
	virtual BOOL PreCreate(DSBUFFERDESC *pDSBD, PCMWAVEFORMAT *lppcmwf);
	void SetFile(CArchiveFile *pFile, EFileType eFileType = FileType_Auto);
  
  CSoundBuffer();
	virtual ~CSoundBuffer();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	CSoundReader * m_pSoundReader;
	EFileType m_eFileType;
	CArchiveFile m_File;
};

#endif // !defined(AFX_SOUNDBUFFER_H__9ABBEDB3_A3F6_11D2_ABBA_E4A802FA4A60__INCLUDED_)
