// TIFFReader.cpp: implementation of the CTIFFReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TIFFReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CTIFFReader, CImageReader);

CTIFFReader::CTIFFReader()
{
  m_bOpened = FALSE;
  m_bInformationsValid = FALSE;
}

CTIFFReader::~CTIFFReader()
{
  if(m_bInformationsValid) Close();
}

void CTIFFReader::Open(CArchiveFile *pFile, void (*FinishRGBProc)(CDDrawSurface *, LPBYTE, DWORD, LPVOID), LPVOID pFinishRGBParam)
{
  CImageReader::Open(pFile, FinishRGBProc, pFinishRGBParam);
  m_pFile->Open();
  m_bOpened = TRUE;
}

BOOL CTIFFReader::ReadInformations()
{
  HEADER header;

  if(!m_bOpened){ ASSERT(NULL); return FALSE; }
  if(m_bInformationsValid) Close();

  m_pFile->SeekToBegin();
  m_pFile->Read(&header, sizeof(header));

  if(header.NumberFormat == 0x4949) m_nNumberFormat = 0;
  if(header.NumberFormat == 0x4D4D) m_nNumberFormat = 1;
  ConvertNumber(header.Version);
  ConvertNumber(header.FirstIFD);

  if(header.Version != 0x002A) return FALSE;

  m_pFile->Seek(header.FirstIFD, CFile::begin);
  
  m_pFile->Read(&m_nIFDEntriesCount, 2);
  ConvertNumber(m_nIFDEntriesCount);
  
  m_lpIFDEntries = new IFD_ENTRY[m_nIFDEntriesCount];
  m_pFile->Read(m_lpIFDEntries, m_nIFDEntriesCount * sizeof(IFD_ENTRY));

  m_wCompresion = 1;

  if(!ReadSizes()) return FALSE;
  if(!ReadPixelFormat()) return FALSE;
  if(!ReadCompresionType()) return FALSE;
  if(!ReadStripFormat()) return FALSE;
  if(!ReadStripOffsets()) return FALSE;

  m_bInformationsValid = TRUE;

  return TRUE;
}

void CTIFFReader::Close()
{
  if(!m_bInformationsValid) return;

  delete [] m_lpIFDEntries;

  delete [] m_lpStripByteCounts;
  delete [] m_lpStripOffsets;

  m_pFile->Close();

  m_bInformationsValid = FALSE;
}

void CTIFFReader::ConvertNumber(WORD & word)
{
  if(m_nNumberFormat) word = ((word & 0xFF00) >> 8) | ((word & 0x00FF) << 8);
}

void CTIFFReader::ConvertNumber(DWORD & dword)
{
  if(m_nNumberFormat) dword = ((dword & 0xFF000000) >> 24) | ((dword & 0x00FF0000) >> 8) | ((dword & 0x0000FF00) << 8) | ((dword & 0x000000FF) << 24);
}

BOOL CTIFFReader::FindIFDEntry(WORD wTag, IFD_ENTRY *lpIFDEntry)
{
  int i;
  IFD_ENTRY *lpPos;
  WORD *pTag, Tag;

  for(i = 0; i < m_nIFDEntriesCount; i++){
    lpPos = m_lpIFDEntries + i;

    pTag = (WORD *)lpPos;
    Tag = *pTag;

    ConvertNumber(Tag);
    if(Tag == wTag){
      CopyMemory(lpIFDEntry, lpPos, sizeof(IFD_ENTRY));
      ConvertNumber(lpIFDEntry->Tag);
      ConvertNumber(lpIFDEntry->Type);
      ConvertNumber(lpIFDEntry->Length);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL CTIFFReader::ReadSizes()
{
  IFD_ENTRY entry;

  if(!FindIFDEntry(0x0100, &entry)) return FALSE;
  if(entry.Type = IFDT_SHORT){
    WORD *w;

    w = (WORD *)&entry.Value;
    ConvertNumber(*w);

    m_dwWidth = *w;
  }
  if(entry.Type = IFDT_LONG){
    ConvertNumber(entry.Value);

    m_dwWidth = entry.Value;
  }

  if(!FindIFDEntry(0x0101, &entry)) return FALSE;
  if(entry.Type = IFDT_SHORT){
    WORD *w;

    w = (WORD *)&entry.Value;
    ConvertNumber(*w);

    m_dwHeight = *w;
  }
  if(entry.Type = IFDT_LONG){
    ConvertNumber(entry.Value);

    m_dwHeight = entry.Value;
  }

  return TRUE;
}

BOOL CTIFFReader::ReadPixelFormat()
{
  IFD_ENTRY entry;

  if(!FindIFDEntry(0x0115, &entry)) return FALSE;
  WORD *w = (WORD *)&entry.Value;
  ConvertNumber(*w);
  m_nSamplesPerPixel = *w;

  if(!FindIFDEntry(0x0102, &entry)) return FALSE;
  ConvertNumber(entry.Value);
  m_pFile->Seek(entry.Value, CFile::begin);
  WORD *lpW;

  lpW = new WORD[entry.Length];
  m_pFile->Read(lpW, entry.Length * 2);
  m_wRBits = lpW[0];
  m_wGBits = lpW[1];
  m_wBBits = lpW[2];
  m_wABits = lpW[3];
  delete [] lpW;

  return TRUE;
}

BOOL CTIFFReader::ReadCompresionType()
{
  IFD_ENTRY entry;

  if(!FindIFDEntry(0x0103, &entry)) return FALSE;
  WORD *w = (WORD *)&entry.Value;
  ConvertNumber(*w);
  m_wCompresion = *w;

  return TRUE;
}

BOOL CTIFFReader::ReadStripFormat()
{
  IFD_ENTRY entry;

  if(!FindIFDEntry(0x0116, &entry)) return FALSE;
  if(entry.Type = IFDT_SHORT){
    WORD *w;

    w = (WORD *)&entry.Value;
    ConvertNumber(*w);

    m_dwRowsPerStrip = *w;
  }
  if(entry.Type = IFDT_LONG){
    ConvertNumber(entry.Value);

    m_dwRowsPerStrip = entry.Value;
  }

  m_dwStripsPerImage = (m_dwHeight + m_dwRowsPerStrip - 1) / m_dwRowsPerStrip;

  if(!FindIFDEntry(0x0117, &entry)) return FALSE;
  if(entry.Length == 1){
    m_lpStripByteCounts = new DWORD[1];

    if(entry.Type = IFDT_SHORT){
      WORD *w;

      w = (WORD *)&entry.Value;
      ConvertNumber(*w);

      m_lpStripByteCounts[0] = *w;
    }
    if(entry.Type = IFDT_LONG){
      ConvertNumber(entry.Value);

      m_lpStripByteCounts[0] = entry.Value;
    }
  }
  else{
    ConvertNumber(entry.Value);
    m_pFile->Seek(entry.Value, CFile::begin);
    if(entry.Type == IFDT_SHORT){
      WORD *w;

      m_lpStripByteCounts = new DWORD[m_dwStripsPerImage];
    
      w = new WORD[m_dwStripsPerImage];
      m_pFile->Read(w, m_dwStripsPerImage * 2);
      for(DWORD i = 0; i < m_dwStripsPerImage; i++){
        ConvertNumber(w[i]);
        m_lpStripByteCounts[i] = w[i];
      }
      delete [] w;
    }
    if(entry.Type == IFDT_LONG){
      m_lpStripByteCounts = new DWORD[m_dwStripsPerImage];
 
      m_pFile->Read(m_lpStripByteCounts, m_dwStripsPerImage * 4);
      for(DWORD i = 0; i < m_dwStripsPerImage; i++){
        ConvertNumber(m_lpStripByteCounts[i]);
      }
    }
  }

  return TRUE;
}

BOOL CTIFFReader::ReadStripOffsets()
{
  IFD_ENTRY entry;

  if(!FindIFDEntry(0x0111, &entry)) return FALSE;
  if(entry.Length == 1){
    m_lpStripOffsets = new DWORD[1];

    if(entry.Type = IFDT_SHORT){
      WORD *w;

      w = (WORD *)&entry.Value;
      ConvertNumber(*w);

      m_lpStripOffsets[0] = *w;
    }
    if(entry.Type = IFDT_LONG){
      ConvertNumber(entry.Value);

      m_lpStripOffsets[0] = entry.Value;
    }
  }
  else{
    ConvertNumber(entry.Value);
    m_pFile->Seek(entry.Value, CFile::begin);
    if(entry.Type == IFDT_SHORT){
      WORD *w;

      m_lpStripOffsets = new DWORD[m_dwStripsPerImage];
    
      w = new WORD[m_dwStripsPerImage];
      m_pFile->Read(w, m_dwStripsPerImage * 2);
      for(DWORD i = 0; i < m_dwStripsPerImage; i++){
        ConvertNumber(w[i]);
        m_lpStripOffsets[i] = w[i];
      }
      delete [] w;
    }
    if(entry.Type == IFDT_LONG){
      m_lpStripOffsets = new DWORD[m_dwStripsPerImage];
 
      m_pFile->Read(m_lpStripOffsets, m_dwStripsPerImage * 4);
      for(DWORD i = 0; i < m_dwStripsPerImage; i++){
        ConvertNumber(m_lpStripOffsets[i]);
      }
    }
  }

  return TRUE;

}

BOOL CTIFFReader::ReadImage(CDDrawSurface * lpSurface)
{
  DWORD dwStrip;
  CRect rect;

  if(!m_bInformationsValid) return FALSE;

  for(dwStrip = 0; dwStrip < m_dwStripsPerImage; dwStrip++){
    switch(m_wCompresion){
    case Compresion_None:
      DWORD Rows;

      Rows = m_dwRowsPerStrip;
      if((m_dwHeight - (m_dwRowsPerStrip * dwStrip)) < Rows)
        Rows = m_dwHeight - (m_dwRowsPerStrip * dwStrip);

      rect.SetRect(0, m_dwRowsPerStrip * dwStrip, m_dwWidth - 1,
        m_dwRowsPerStrip * dwStrip + Rows - 1);

      LPBYTE lpS;
      DWORD dwPitch;
      lpS = (LPBYTE)lpSurface->Lock(&rect, 0, dwPitch);
//      lpS = new BYTE[m_lpStripByteCounts[dwStrip];
//      dwPitch = m_dwWidth;
      
      DWORD x, y, pos, pos2;
      LPBYTE lpImg;

      lpImg = new BYTE[m_lpStripByteCounts[dwStrip]];
      m_pFile->Seek(m_lpStripOffsets[dwStrip], CFile::begin);
      m_pFile->Read(lpImg, m_lpStripByteCounts[dwStrip]);
      pos = 0;

      if(g_pDirectDraw->Is32BitMode()){
        for(y = 0; y < Rows; y++){
          pos2 = y * dwPitch;
          for(x = 0; x < m_dwWidth; x++, pos2 += 4){
            lpS[pos2] = lpImg[pos + 2];     // Reverse order of RGB
            lpS[pos2 + 1] = lpImg[pos + 1];
            lpS[pos2 + 2] = lpImg[pos];
            pos += m_nSamplesPerPixel; // next pixel
          }
        }
      }
      else{
        for(y = 0; y < Rows; y++){
          pos2 = y * dwPitch;
          for(x = 0; x < m_dwWidth; x++, pos2 += 3){
            lpS[pos2] = lpImg[pos + 2];     // Reverse order of RGB
            lpS[pos2 + 1] = lpImg[pos + 1];
            lpS[pos2 + 2] = lpImg[pos];
            pos += m_nSamplesPerPixel; // next pixel
          }
        }
      }

      delete [] lpImg;

      if(m_pFinishRGBProc != NULL){
        m_pFinishRGBProc(lpSurface, lpS, dwPitch, m_pFinishRGBProcParam);
      }

      lpSurface->Unlock(lpS);
      break;
    case Compresion_LZW:
      break;
    }
  }

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CTIFFReader::AssertValid() const
{
  CImageReader::AssertValid();
}

void CTIFFReader::Dump(CDumpContext & dc) const
{
  CImageReader::Dump(dc);
}

#endif