// EUnitSurface.h: interface for the CEUnitSurface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITSURFACE_H__D6970FB5_8B08_11D3_A0A8_FA054450EB31__INCLUDED_)
#define AFX_EUNITSURFACE_H__D6970FB5_8B08_11D3_A0A8_FA054450EB31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEUnitSurface : public CImageSurface  
{
  DECLARE_DYNAMIC(CEUnitSurface)

public:
  // restores the image data
	void Restore();

  // constructor & destructor
	CEUnitSurface();
	virtual ~CEUnitSurface();

  int GetXOffset () { return m_nXOffset; }
  int GetYOffset () { return m_nYOffset; }
  void SetXOffset ( int nOffset ) { m_nXOffset = nOffset; }
  void SetYOffset ( int nOffset ) { m_nYOffset = nOffset; }

  DWORD GetFrameDelay () { return m_dwFrameDelay; }
  void SetFrameDelay ( DWORD dwFrameDelay ) { m_dwFrameDelay = dwFrameDelay; }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // the coloring params structure
  struct tagSColoringParams{
    DWORD m_dwSourceColor;  // the source color to be replaced
    DWORD m_dwDestColor;    // the dest color to be used for replaceing
    double m_dbHTolerance;  // toleration for H (if 0 -> no coloring)
    double m_dbSTolerance;  // toleration for S
    double m_dbVTolerance;  // toleration for V
  };
  typedef struct tagSColoringParams SColoringParams;

  // creates the surface from file with given coloring params
	BOOL Create(CArchiveFile file, SColoringParams *pColoringParams);
  // deletes the surface
	virtual void Delete();

private:
  // the params for coloring
  SColoringParams m_sColoringParams;
protected:
  // colors the image
	static void ColorImage(CDDrawSurface *pDDSurface, LPBYTE pSurface, DWORD dwPitch, LPVOID pParam);

  int m_nXOffset, m_nYOffset;
  DWORD m_dwFrameDelay;
};

#endif // !defined(AFX_EUNITSURFACE_H__D6970FB5_8B08_11D3_A0A8_FA054450EB31__INCLUDED_)
