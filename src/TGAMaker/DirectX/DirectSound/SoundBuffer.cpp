// SoundBuffer.cpp: implementation of the CSoundBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundBuffer.h"

#include "SoundReader.h"
#include "WAVReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSoundBuffer, CDSoundBuffer);

CSoundBuffer::CSoundBuffer()
{
  m_pSoundReader = NULL;
}

CSoundBuffer::~CSoundBuffer()
{
  if(m_pSoundReader) delete m_pSoundReader;
}

#ifdef _DEBUG

void CSoundBuffer::AssertValid() const
{
  CDSoundBuffer::AssertValid();
}

void CSoundBuffer::Dump(CDumpContext &dc) const
{
  CDSoundBuffer::Dump(dc);
}

#endif

void CSoundBuffer::SetFile(CArchiveFile *pFile, EFileType eFileType)
{
  CString strFileName;

  m_File = *pFile;
  strFileName = pFile->GetFileName();

  if(m_lpDSoundBuffer != NULL) return; // if created -> no effect

  m_eFileType = eFileType;
  if(eFileType == FileType_Auto){
    char *hlp;
    hlp = strrchr((LPCSTR)strFileName, '.');
    if(hlp != NULL){
      hlp++;
      if(!strcmpi(hlp, "wav")) m_eFileType = FileType_WAVE;
    }
  }
}

BOOL CSoundBuffer::PreCreate(DSBUFFERDESC *pDSBD, PCMWAVEFORMAT *lppcmwf)
{
  if(m_pSoundReader) delete m_pSoundReader;
  m_pSoundReader = NULL;

  // if the dummy one - no file reading or so
  if(m_bDummy) return TRUE;

  switch(m_eFileType){
  case FileType_WAVE:
    m_pSoundReader = new CWAVReader();
    break;
  }

  if(m_pSoundReader == NULL){
    TRACE("Unknown sound file format '%s'.\n", m_File.GetFilePath());
    return FALSE;
  }

  m_pSoundReader->Open(&m_File);
  if(!m_pSoundReader->ReadInformations()) return FALSE;

  m_dwBitsPerSample = m_pSoundReader->m_dwBitsPerSample;
  m_dwChannels = m_pSoundReader->m_dwChannels;
  m_dwSampleRate = m_pSoundReader->m_dwSampleRate;
  m_dwLength = m_pSoundReader->m_dwLength;
  
  if(!CDSoundBuffer::PreCreate(pDSBD, lppcmwf)) return FALSE;

  return TRUE;
}


BOOL CSoundBuffer::PostCreate(LPDIRECTSOUNDBUFFER lpDSBuffer, BOOL bCallRelease)
{
  if(!CDSoundBuffer::PostCreate(lpDSBuffer, bCallRelease)) return FALSE;

  if(m_bDummy) return TRUE;

  if(!m_pSoundReader){
    ASSERT(FALSE); // I can't image this situation
    return FALSE;  // FATAL error in calling function
  }
  ASSERT_VALID(m_pSoundReader);

  if(!m_pSoundReader->ReadSound(this)) return FALSE;

  m_pSoundReader->Close();

  delete m_pSoundReader; m_pSoundReader = NULL;

  return TRUE;
}
