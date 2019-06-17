#ifndef CUNITSURFACE_H_
#define CUNITSURFACE_H_

// Class representing one unit frame
class CCUnitSurface : public CImageSurface
{

friend class CCUnitSurfaceManager;

public:
  // constructor & destructor
  CCUnitSurface();
  virtual ~CCUnitSurface();


  // Returns X offset of the frame
  int GetXOffset () { return m_nXOffset; }
  // Returns Y offset of the frame
  int GetYOffset () { return m_nYOffset; }

  // Returns frame delay
  int GetFrameDelay () { return m_dwFrameDelay; }

  // debug functions
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
	void Create(CArchiveFile file, SColoringParams *pColoringParams, int nXOffset, int nYOffset, DWORD dwFrameDelay );
  // deletes the surface
	virtual void Delete();

private:
// Private methods
  static void ColorImage(CDDrawSurface *pDDSurface, LPBYTE pSurface, DWORD dwPitch, LPVOID pParam);

// Data
  // the params for coloring
  SColoringParams *m_pColoringParams;

  // The X offset of this frame
  int m_nXOffset;
  int m_nYOffset;

  // The frame delay for this frame
  DWORD m_dwFrameDelay;

protected:
  // Restore the image
  virtual void Restore();

// methods for cooperation with CImageSurfaceManager
private:
    // Rises the refcount
    void AddRef() { ASSERT(m_nRefCount>=0); m_nRefCount++; }

    // Lowers the refcount
    void Release() { ASSERT(m_nRefCount>0); m_nRefCount--; }

    // Returns the reference count
    int GetRefCount() { return m_nRefCount; }

// Data for cooperation with CImageSurfaceManager
private:
    // Number of references to the surface
    int m_nRefCount;
};

#endif