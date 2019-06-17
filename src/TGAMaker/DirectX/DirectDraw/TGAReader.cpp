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
  m_bAlphaChannel = FALSE;
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

#endif

void CTGAReader::Open(CArchiveFile *pFile)
{
  CImageReader::Open(pFile);

  m_pFile->Open();
}


BOOL CTGAReader::ReadInformations()
{
  TGAHeader h;

  m_pFile->Read(&h, 18);

// for now - we can read only RGB uncompressed format
  if(h.m_nImageType != 2){
    TRACE("Unsupported TGA format.\n");
    return FALSE;
  }

  m_dwHeight = h.m_wHeight;
  m_dwWidth = h.m_wWidth;

  m_bAlphaChannel = FALSE;
  if((h.m_nPixelSize != 24) && (h.m_nPixelSize != 32)){
    TRACE("Unsupported TGA format (not 24-bit or 32-bit).\n");
    return FALSE;
  }
  // 32 - bit Targa has 8 more bits -> alpha channel
  // be carefull, you can make 32-bit Targa with no alpha channel
  if(h.m_nPixelSize == 32){
    m_bAlphaChannel = TRUE;
  }

  m_dwImageOffset = 18 + h.m_nIDFieldLength +
    (h.m_wColorMapLength * h.m_nColorMapEntrySize);

  return TRUE;
}

BOOL CTGAReader::ReadImage(CDDrawSurface *lpSurface)
{
  CRect rect;
  DWORD pos1, pos2, pos3;
  DWORD i, j, dwPitch;
  BYTE *pLine;
  BYTE *pAlphaChannel;
  LPBYTE lpS;

  rect.SetRect(0, 0, m_dwWidth - 1,
    m_dwHeight - 1);

  m_pFile->Seek(m_dwImageOffset, CFile::begin);
  
  pAlphaChannel = lpSurface->GetAlphaChannel();
  lpS = (LPBYTE)lpSurface->Lock(&rect, 0, dwPitch);

  if(m_bAlphaChannel){ // -> 32-bit Targa
    pLine = new BYTE[m_dwWidth * 4];
    for(i = 0; i < m_dwHeight; i++){
      pos1 = (m_dwHeight - i - 1) * dwPitch;
      pos3 = (m_dwHeight - i - 1) * lpSurface->GetWidth();
      pos2 = 0;
      m_pFile->Read(pLine, m_dwWidth * 4);
      if(g_pDirectDraw->Is32BitMode()){
        if(pAlphaChannel == NULL){
          for(j = 0; j < m_dwWidth; j++, pos1+=4, pos2+=4){
            lpS[pos1] = pLine[pos2];
            lpS[pos1+1] = pLine[pos2+1];
            lpS[pos1+2] = pLine[pos2+2];
          }
        }
        else{
          for(j = 0; j < m_dwWidth; j++, pos1+=4, pos2+=4, pos3++){
            lpS[pos1] = pLine[pos2];
            lpS[pos1+1] = pLine[pos2+1];
            lpS[pos1+2] = pLine[pos2+2];
            pAlphaChannel[pos3] = pLine[pos2+3];
          }
        }
      }
      else{
        if(pAlphaChannel == NULL){
          for(j = 0; j < m_dwWidth; j++, pos1+=3, pos2+=4){
            lpS[pos1] = pLine[pos2];
            lpS[pos1+1] = pLine[pos2+1];
            lpS[pos1+2] = pLine[pos2+2];
          }
        }
        else{
          for(j = 0; j < m_dwWidth; j++, pos1+=3, pos2+=4, pos3++){
            lpS[pos1] = pLine[pos2];
            lpS[pos1+1] = pLine[pos2+1];
            lpS[pos1+2] = pLine[pos2+2];
            pAlphaChannel[pos3] = pLine[pos2+3];
          }
        }
      }
    }
    delete [] pLine;
  }
  else{ // no alpha -> 24-bit Targa
    pLine = new BYTE[m_dwWidth * 3];
    for(i = 0; i < m_dwHeight; i++){
      pos1 = (m_dwHeight - i - 1) * dwPitch;
      pos2 = 0;
      m_pFile->Read(pLine, m_dwWidth * 3);
      for(j = 0; j < m_dwWidth; j++, pos1+=3, pos2+=3){
        lpS[pos1] = pLine[pos2];
        lpS[pos1+1] = pLine[pos2+1];
        lpS[pos1+2] = pLine[pos2+2];
      }
    }
    delete [] pLine;
  }

  lpSurface->Unlock(lpS);

  return TRUE;
}

void CTGAReader::Close()
{
  m_pFile->Close();
}
