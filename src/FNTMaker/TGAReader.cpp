// TGAReader.cpp: implementation of the CTGAReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TGAReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTGAReader::CTGAReader()
{

}

CTGAReader::~CTGAReader()
{

}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CTGAReader::AssertValid() const
{
  CImageReader::AssertValid();
}

void CTGAReader::Dump(CDumpContext &dc) const
{
  CImageReader::Dump(dc);
}

#pragma pack(1)
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


#endif

void CTGAReader::Open(LPCSTR lpszFileName)
{
  CFileException e;
  
  if(!m_File.Open(lpszFileName, CFile::modeRead, &e)){
    TRACE("Error opening file %s while reading TGA format.", lpszFileName);
    THROW(&e);
  }
}


BOOL CTGAReader::ReadInformations()
{
  TGAHeader h;

  m_File.Read(&h, 18);
  if(h.m_nImageType != 2){
    TRACE("Unsupported TGA format.\n");
    return FALSE;
  }

  m_dwHeight = h.m_wHeight;
  m_dwWidth = h.m_wWidth;

  if(h.m_nPixelSize != 24){
    TRACE("Unsupported TGA format (not 24-bit).\n");
    return FALSE;
  }

  m_File.Seek(h.m_nIDFieldLength, CFile::current);

  m_File.Seek(h.m_wColorMapLength * h.m_nColorMapEntrySize, CFile::current);

  return TRUE;
}

BOOL CTGAReader::ReadImage(BYTE * pByte, DWORD dwPitch)
{
  BYTE *line;

  line = new BYTE[m_dwWidth * 3];
  DWORD i;
  for(i = 0; i < m_dwHeight; i++){
    m_File.Read(line, m_dwWidth * 3);
    memcpy(&pByte[(m_dwHeight - i - 1) * dwPitch], line, m_dwWidth * 3);
  }

  delete [] line;

  return TRUE;
}

void CTGAReader::Close()
{
  m_File.Close();
}
