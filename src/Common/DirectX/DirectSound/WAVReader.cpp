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
	SChunkHeader ChunkHeader;
	SWAVHeader WAVHeader;
	char aWAVE [ 4 ];

  m_pFile->Read(&ChunkHeader, sizeof(ChunkHeader));
  
  if(strncmp(ChunkHeader.m_aID, "RIFF", 4)){
    TRACE("WAV file '%s' is not a RIFF file.\n", m_pFile->GetFilePath());
    return FALSE;
  }

	// Now read the WAVE id
  m_pFile->Read ( aWAVE, sizeof ( aWAVE ) );
  if ( strncmp ( aWAVE, "WAVE", 4 ) )
  {
    TRACE("WAV file '%s' is not a WAVE file.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  // Now read the fmt chunk
  m_pFile->Read ( &ChunkHeader, sizeof ( ChunkHeader ) );

  if ( strncmp ( ChunkHeader.m_aID, "fmt ", 4 ) )
  {
    TRACE("WAV file '%s' is not a WAVE file.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  // Now read the fmt
  m_pFile->Read(&WAVHeader, sizeof(WAVHeader));

  if(WAVHeader.m_wFormatTag != 1){
    TRACE("Unknown WAV file format '%s'.\n", m_pFile->GetFilePath());
    return FALSE;
  }

  m_dwBitsPerSample = 8; // Allways 8-bit PCM files
  m_dwChannels = WAVHeader.m_nChannels;
  m_dwSampleRate = WAVHeader.m_nSamplesPerSec;

	// We must seek as far as the end of the fmt chunk
  m_pFile->Seek ( ChunkHeader.m_dwLength - sizeof ( WAVHeader ), CFile::current );

  // No we must find the data chunk
  while ( true )
  {
	  // Read the chunk
	  if ( m_pFile->Read ( &ChunkHeader, sizeof ( ChunkHeader ) ) != sizeof ( ChunkHeader ) )
	  {
		  TRACE ( "Error in WAV file '%s'.\n", m_pFile->GetFilePath () );
		  return FALSE;
	  }
	  // Is it our chunk
	  if ( strncmp ( ChunkHeader.m_aID, "data", 4 ) == 0 )
	  {
		  // OK we've got it
		  break;
	  }
	  // Elseway skip it
	  m_pFile->Seek ( ChunkHeader.m_dwLength, CFile::current );
  }

  m_dwLength = ChunkHeader.m_dwLength;

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

