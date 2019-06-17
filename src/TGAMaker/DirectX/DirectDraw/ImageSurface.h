#if !defined(AFX_IMAGESURFACE_H__1B5767A5_488C_11D2_8EB5_CC7204C10000__INCLUDED_)
#define AFX_IMAGESURFACE_H__1B5767A5_488C_11D2_8EB5_CC7204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImageSurface.h : header file
//

#include "DDrawSurface.h"
#include "ImageReader.h"
#include "..\..\Archive\Archive.h"

/////////////////////////////////////////////////////////////////////////////
// CImageSurface command target

class CImageSurface : public CDDrawSurface
{
	DECLARE_DYNAMIC(CImageSurface)

public:

  typedef enum FileTypes{
    FileType_Auto = 0,
    FileType_TIFF = 1,
    FileType_Targa = 2
  }EFileTypes;

	// constructs the object
  CImageSurface();

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual void Restore();
  BOOL Create(){ return CDDrawSurface::Create(); }
	// creates the surface from given file
  // just call SetFile and Create
  BOOL Create(CArchiveFile *pFile, EFileTypes eFileType = FileType_Auto);
  // sets the image key color
	void SetImageKeyColor(DWORD dwKeyColor);
  // sets the image file
	void SetFile(CArchiveFile *pFile, EFileTypes eFileType = FileType_Auto);
  // post create called internally
	virtual BOOL PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease = TRUE);
  // pre create called internally
	virtual BOOL PreCreate(DDSURFACEDESC *ddsd);
  // destructs the object
  virtual ~CImageSurface();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

private:
	// reads the image from the file
  // destroyes the image reader
  BOOL FinishImageReader();
  // creates the image reader and reads informations
  // about the file (size and so on..)
	BOOL StartImageReader();
  // tests if the image contains some transparent pixels
  // it means pixels with key color
	BOOL HasKeyColor();
  // pointer to the image reader object
  // valid only in time of creation
	CImageReader * m_pImageReader;
  // key color wich the image is to have
  // used only before creation
	DWORD m_dwKeyColor;
  // type of the file to load
	EFileTypes m_eFileType;
  // the file from which the image was loaded
  // this is used to reload the image if the surface is lost
	CArchiveFile m_File;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_IMAGESURFACE_H__1B5767A5_488C_11D2_8EB5_CC7204C10000__INCLUDED_)
