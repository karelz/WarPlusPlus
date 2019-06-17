/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: Animation editor - Data objects
 *      Author: Vit Karas
 * 
 * Description: Frame object class
 * 
 ***********************************************************/

#ifndef _ANIMATIONEDITOR_AFRAME_H_
#define _ANIMATIONEDITOR_AFRAME_H_

#include "Structures.h"

class CAFrame : public CObject
{
  // RTCI for our objects
  DECLARE_DYNAMIC ( CAFrame );

public:
// Construction
  // Constructor
  CAFrame ();
  // Destructor
  virtual ~CAFrame ();

// Debug functions
#ifdef _DEBUG
  // Asserts validity of the object's data
  virtual void AssertValid () const;
  // Dumps object's data
  virtual void Dump ( CDumpContext & dc ) const;
#endif

// Creation
  // Creates the frame from given archive file
  void Create ( CArchiveFile cFile, SColoring * pColoring, DWORD * pDestinationColor );
  // Deletes the frame
  virtual void Delete ();

// Operations
  // Crops the frame with given tolerance for alpha channel
  // This removes borders of the image which has all pixel with alpha channel value
  // less or equal to given tolerance
  void Crop ( BYTE nAlphaTolerance );

  // Recomputes coloring
  void RecomputeColoring ();

  // Reloads the frame from the file it was created from
  void Reload ();

  // Saves the frame to the file it was loaded from
  void Save ();

// Get/Set functions
  // Returns pointer to the graphical surface
  CDDrawSurface * GetSurface () const { ASSERT_VALID ( this ); return const_cast < CDDrawSurface * > ( (CDDrawSurface *)&m_cSurface ) ; }

  // Returns X graphical offset
  int GetXOffset () const { ASSERT_VALID ( this ); return m_nXOffset; }
  // Returns Y graphical offset
  int GetYOffset () const { ASSERT_VALID ( this ); return m_nYOffset; }
  // Sets new X graphical offset
  void SetXOffset ( int nOffset ) { ASSERT_VALID ( this ); m_nXOffset = nOffset; }
  // Sets new Y graphical offset
  void SetYOffset ( int nOffset ) { ASSERT_VALID ( this ); m_nYOffset = nOffset; }

  // Returns the frame delay (in milliseconds)
  DWORD GetFrameDelay () const { ASSERT_VALID ( this ); return m_dwFrameDelay; }
  // Sets the frame delat (in milliseconds)
  void SetFrameDelay ( DWORD dwFrameDelay ) { ASSERT_VALID ( this ); m_dwFrameDelay = dwFrameDelay; }

  // Returns path to the file (and the filename)
  CString GetFilePath () const { ASSERT_VALID ( this ); return m_cFile.GetFilePath (); }

  // Returns true if the object was modified from the last save
  bool IsModified () const { ASSERT_VALID ( this ); return m_bModified; }
  // Sets the modification flag
  void SetModified ( bool bModified = true ) { ASSERT_VALID ( this ); m_bModified = bModified; }

private:
// Data members
  // The original image surface (this one isloaded from the file, and sometimes it's cropped)
  BYTE *m_pOriginalSurface;
  CSize m_szOriginalSurface;
  // Archive file from which was the original surface loaded
  CArchiveFile m_cFile;
  // The surface for this frame (it's already colored)
  CScratchSurface m_cSurface;

  // Graphical offset of this frame
  int m_nXOffset, m_nYOffset;

  // The last crop rectangle ( it means the rect that is the m_cSurface out of the whole
  // m_pOriginalSurface)
  CRect m_rcCropRect;

  // Frame delay (in milliseconds)
  DWORD m_dwFrameDelay;

  // Modification flag
  bool m_bModified;

  // The coloring structure for this frame (it's shared by all frames in the animation)
  SColoring * m_pColoring;
  // Destination color for this frame (it's shared by all frames in the animation)
  DWORD * m_pDestinationColor;

// Helper functions
  // Loads the original surface from given file
  // The surface must be deleted this time
  void LoadSurface ( CArchiveFile cFile );
  // Copies the surface from the original surface and colors it
  // The surface must be deleted
  void CopySurface ();
  // Deletes the surfaces for this frame
  void DestroySurfaces ();


  // Next frame in the list of frames on the animation
  CAFrame * m_pNext;
  friend class CAAnimation;
};

#endif // _ANIMATIONEDITOR_AFRAME_H_