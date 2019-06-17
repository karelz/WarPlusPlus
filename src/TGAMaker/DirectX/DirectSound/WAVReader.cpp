// WAVReader.cpp: implementation of the CWAVReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WAVReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWAVReader, CSoundReader);

CWAVReader::CWAVReader()
{

}

CWAVReader::~CWAVReader()
{

}

#ifdef _DEBUG

void CWAVReader::AssertValid() const
{
  CSoundReader::AssertValid();
}

void CWAVReader::Dump(CDumpContext &dc) const
{
  CSoundReader::Dump(dc);
}

#endif

void CWAVReader::Open(CArchiveFile *pFile)
{
  CSoundReader::Open(pFile);

  m_pFile->Open();  
}

void CWAVReader::Close()
{
  m_pFile->Close();
  
  CSoundReader::Close();
}

BOOL CWAVReader::ReadInformations()
{
  SRIFFHeader RHeader;
  SWAVHeader WAVHeader;
  SDataHeader DataHeader;

  m_pFile->Read(&RHeader, sizeof(RHeader));
  
  if(strncmp(RHeader.m_ID, "RIFF", 4)){
    TRACE("WAV file '%s' is not a RIFF file.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  m_pFile->Read(&WAVHeader, sizeof(WAVHeader));

  if(strncmp(WAVHeader.m_ID, "WAVE", 4)){
    TRACE("WAV file '%s' is not a WAVE file.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  if(strncmp(WAVHeader.m_FormatID, "fmt ", 4)){
    TRACE("Error in WAV file format '%s'.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  if(WAVHeader.m_wFormatTag != 1){
    TRACE("Unknown WAV file format '%s'.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  m_dwBitsPerSample = 8; // Allways 8-bit PCM files
  m_dwChannels = WAVHeader.m_nChannels;
  m_dwSampleRate = WAVHeader.m_nSamplesPerSec;

  m_pFile->Read(&DataHeader, sizeof(DataHeader));

  if(strncmp(DataHeader.m_ID, "data", 4)){
    TRACE("Error in WAV file format '%s'.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  m_dwLength = DataHeader.m_dwLength;

  return TRUE;
}

BOOL CWAVReader::ReadSound(CDSoundBuffer *pSBuffer)
{
  BYTE *pData;

  pData = new BYTE[m_dwLength];
  if(pData == NULL){
    AfxThrowMemoryException();
  }

  m_pFile->Read(pData, m_dwLength);

  pSBuffer->WriteToBuffer(pData, m_dwLength, 0);

  delete pData;

  return TRUE;
}

