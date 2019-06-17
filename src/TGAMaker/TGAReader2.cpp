// TGAReader.cpp: implementation of the CTGAReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TGAReader2.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTGAReader2::CTGAReader2()
{

}

CTGAReader2::~CTGAReader2()
{

}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CTGAReader2::AssertValid() const
{
  CImageReader2::AssertValid();
}

void CTGAReader2::Dump(CDumpContext &dc) const
{
  CImageReader2::Dump(dc);
}

#endif

#pragma pack(1)
typedef struct tagSTGAHeader{
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

void CTGAReader2::Open(LPCSTR lpszFileName)
{
  CFileException e;
  
  if(!m_File.Open(lpszFileName, CFile::modeRead, &e)){
    TRACE("Error opening file %s while reading TGA format.", lpszFileName);
    THROW(&e);
  }
}


BOOL CTGAReader2::ReadInformations()
{
  TGAHeader h;

  m_File.Read(&h, 18);
  if(h.m_nImageType != 2){
    TRACE("Unsupported TGA format.\n");
    return FALSE;
  }

  m_dwHeight = h.m_wHeight;
  m_dwWidth = h.m_wWidth;
  m_nPixelSize = h.m_nPixelSize;

  if((m_nPixelSize != 24) && (m_nPixelSize != 32)){
    TRACE("Unsupported TGA format (not 24-bit or 32-bit).\n");
    return FALSE;
  }

  m_File.Seek(h.m_nIDFieldLength, CFile::current);

  m_File.Seek(h.m_wColorMapLength * h.m_nColorMapEntrySize, CFile::current);

  return TRUE;
}

BOOL CTGAReader2::ReadImage(BYTE * pByte, DWORD dwPitch)
{
  BYTE *line;

  line = new BYTE[m_dwWidth * 4];
  DWORD i, j;
  for(i = 0; i < m_dwHeight; i++){
    if(m_nPixelSize == 24){
      m_File.Read(line, m_dwWidth * 3);
      memcpy(&pByte[(m_dwHeight - i - 1) * dwPitch], line, m_dwWidth * 3);
    }
    if(m_nPixelSize == 32){
      m_File.Read(line, m_dwWidth * 4);
      for(j = 0; j < m_dwWidth; j++){
        pByte[(m_dwHeight - i - 1) * dwPitch + j * 3] = line[j * 4];
        pByte[(m_dwHeight - i - 1) * dwPitch + j * 3 + 1] = line[j * 4 + 1];
        pByte[(m_dwHeight - i - 1) * dwPitch + j * 3 + 2] = line[j * 4 + 2];
      }
    }
  }

  delete [] line;

  return TRUE;
}

void CTGAReader2::Close()
{
  m_File.Close();
}
