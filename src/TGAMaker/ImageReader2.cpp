// ImageReader.cpp: implementation of the CImageReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageReader2.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CImageReader2, CObject);

CImageReader2::CImageReader2()
{
  // some sizes - will be filled by ReadInformations
  m_dwWidth = 1; m_dwHeight = 1;
}

CImageReader2::~CImageReader2()
{
}

// Opens the image file
void CImageReader2::Open(LPCSTR lpszFileName)
{
}

// Reads informations about image - it has to fill
// m_dwWidth and m_dwHeight with the sizes of the image
BOOL CImageReader2::ReadInformations()
{
  return TRUE;
}

// Reads the image to a surface
BOOL CImageReader2::ReadImage(BYTE * pByte, DWORD dwPitch)
{
  return TRUE;
}

// Closes the reading of the image
void CImageReader2::Close()
{
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CImageReader2::AssertValid() const
{
  CObject::AssertValid();
}

void CImageReader2::Dump(CDumpContext & dc) const
{
  CObject::Dump(dc);
}

#endif