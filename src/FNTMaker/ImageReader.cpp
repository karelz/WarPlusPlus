// ImageReader.cpp: implementation of the CImageReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CImageReader, CObject);

CImageReader::CImageReader()
{
  // some sizes - will be filled by ReadInformations
  m_dwWidth = 1; m_dwHeight = 1;
}

CImageReader::~CImageReader()
{
}

// Opens the image file
void CImageReader::Open(LPCSTR lpszFileName)
{
}

// Reads informations about image - it has to fill
// m_dwWidth and m_dwHeight with the sizes of the image
BOOL CImageReader::ReadInformations()
{
  return TRUE;
}

// Reads the image to a surface
BOOL CImageReader::ReadImage(BYTE * pByte, DWORD dwPitch)
{
  return TRUE;
}

// Closes the reading of the image
void CImageReader::Close()
{
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CImageReader::AssertValid() const
{
  CObject::AssertValid();
}

void CImageReader::Dump(CDumpContext & dc) const
{
  CObject::Dump(dc);
}

#endif