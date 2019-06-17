// SoundReader.cpp: implementation of the CSoundReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSoundReader, CObject);

CSoundReader::CSoundReader()
{
  m_pFile = NULL;

  m_dwBitsPerSample = 8;
  m_dwChannels = 1;
  m_dwLength = 0;
  m_dwSampleRate = 22050;
}

CSoundReader::~CSoundReader()
{
}

#ifdef _DEBUG

void CSoundReader::AssertValid() const
{
  CObject::AssertValid();
}

void CSoundReader::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CSoundReader::Open(CArchiveFile *pFile)
{
  m_pFile = pFile;
}

void CSoundReader::Close()
{
  m_pFile = NULL;
}

BOOL CSoundReader::ReadInformations()
{
  return TRUE;
}

BOOL CSoundReader::ReadSound(CDSoundBuffer *pSBuffer)
{
  return TRUE;
}
