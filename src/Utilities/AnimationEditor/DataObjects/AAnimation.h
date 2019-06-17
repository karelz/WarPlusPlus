/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: Animation editor - Data objects
 *      Author: Vit Karas
 * 
 * Description: Animation object class
 * 
 ***********************************************************/

#ifndef _ANIMATIONEDITOR_AANIMATION_H_
#define _ANIMATIONEDITOR_AANIMATION_H_

#include "Structures.h"
#include "AFrame.h"

class CAAnimation : public CObject
{
  // RTCI for our objects
  DECLARE_DYNAMIC ( CAAnimation );

public:
// Construction
  // Constructor
  CAAnimation ();
  // Destructor
  virtual ~CAAnimation ();

// Debug functions
#ifdef _DEBUG
  // Asserts validity of the object's data
  virtual void AssertValid () const;
  // Dumps object's data
  virtual void Dump ( CDumpContext & dc ) const;
#endif

// Creation
  // Creates empty animation
  void Create ();
  // Creates the frame from given archive file
  void Create ( CArchiveFile cFile );
  // Deletes the frame
  virtual void Delete ();

// Operations
  // Saves the animation to given file
  void Save ( CArchiveFile cFile );

  // Inserts new frame from given file
  // This will insert the frame before specified index
  // If the index is behind the end of animation, the frame will be inserted to the end
  CAFrame * InsertFrame ( CArchiveFile cFile, DWORD dwBeforeIndex );

  // Removes and deletes the frame on specified index from the animation
  void DeleteFrame ( DWORD dwIndex );

  // Set frame X offset
  void SetFrameXOffset ( DWORD dwIndex, int nXOffset ) { ASSERT_VALID ( this ); ASSERT ( dwIndex < m_dwFrameCount );
    FindFrame ( dwIndex )->SetXOffset ( nXOffset ); SetModified (); }
  // Set frame Y offset
  void SetFrameYOffset ( DWORD dwIndex, int nYOffset ) { ASSERT_VALID ( this ); ASSERT ( dwIndex < m_dwFrameCount );
    FindFrame ( dwIndex )->SetYOffset ( nYOffset ); SetModified (); }

  // Set frame delay
  void SetFrameDelay ( DWORD dwIndex, DWORD dwDelay ) { ASSERT_VALID ( this ); ASSERT ( dwIndex < m_dwFrameCount );
    FindFrame ( dwIndex )->SetFrameDelay ( dwDelay ); SetModified (); }

  // Crop the frame
  void CropFrame ( DWORD dwIndex, BYTE nAlphaTolerance ) { ASSERT_VALID ( this ); ASSERT ( dwIndex < m_dwFrameCount );
    FindFrame ( dwIndex )->Crop ( nAlphaTolerance ); }

  // Moves given frame before the previous frame
  void MoveFrameLeft ( DWORD dwIndex );
  // Moves given frame after the next frame
  void MoveFrameRight ( DWORD dwIndex );

// Get/Set functions
  // Returns number of frames in the animation
  DWORD GetFrameCount () const { ASSERT_VALID ( this ); return m_dwFrameCount; }

  // Returns default frame delay for the animation
  // 0 - statci image
  DWORD GetDefaultFrameDelay () const { ASSERT_VALID ( this ); return m_dwDefaultFrameDelay; }
  // Sets default frame delay for the animation
  // 0 - static image
  void SetDefaultFrameDelay ( DWORD dwDefaultFrameDelay ) { ASSERT_VALID ( this );
    m_dwDefaultFrameDelay = dwDefaultFrameDelay; SetModified (); }

  // Returns the loop flag
  bool GetLoop () const { ASSERT_VALID ( this ); return m_bLoop; }
  // Sets the loop flag
  void SetLoop ( bool bLoop ) { ASSERT_VALID ( this ); m_bLoop = bLoop; SetModified (); }

  // Returns destination color
  DWORD GetDestinationColor () const { ASSERT_VALID ( this ); return m_dwDestinationColor; }
  // Sets new destination color
  void SetDestinationColor ( DWORD dwDestinationColor );

  // Returns pointer to the coloring structure
  // You should call ReleaseColoring when you end working with it
  SColoring * GetColoring () { ASSERT_VALID ( this ); return &m_sColoring; }
  // This recomputes whole animation with new coloring
  void ReleaseColoring ();

  // Returns true if the object was modified from the last save
  bool GetModified () const { ASSERT_VALID ( this ); return m_bModified; }
  // Sets the modification flag
  void SetModified ( bool bModified = true ) { ASSERT_VALID ( this ); m_bModified = bModified; }

  // Returns X graphical offset of the whole animation
  int GetXOffset () const { ASSERT_VALID ( this ); return m_nXOffset; }
  // Sets new X graphical offset of the whole animation
  void SetXOffset ( int nXOffset ) { ASSERT_VALID ( this ); m_nXOffset = nXOffset; SetModified (); }
  // Returns Y graphical offset of the whole animation
  int GetYOffset () const { ASSERT_VALID ( this ); return m_nYOffset; }
  // Sets new Y graphical offset of the whole animation
  void SetYOffset ( int nYOffset ) { ASSERT_VALID ( this ); m_nYOffset = nYOffset; SetModified (); }

  // Returns frame for given index
  const CAFrame * GetFrame ( DWORD dwIndex ) const { ASSERT_VALID ( this ); return FindFrame ( dwIndex ); }

  // Returns true if someting is modified (animation or any frame)
  bool IsModified () const;

private:
// Data members
  // Number of frames in the animation
  DWORD m_dwFrameCount;
  // Default frame delay
  DWORD m_dwDefaultFrameDelay;
  // true if the animation is about to loop
  bool m_bLoop;

  // List of frames in the animation
  CAFrame * m_pFrames;

  // Modification flag
  bool m_bModified;

  // The X graphical offset of the whole animation
  int m_nXOffset;
  // The Y graphical offset of the whole animation
  int m_nYOffset;

  // Coloring structure for the animation
  SColoring m_sColoring;
  // Destination color for the animation
  DWORD m_dwDestinationColor;

// Helper functions
  // Finds frame by index
  CAFrame * FindFrame ( DWORD dwIndex ) const;
};

#endif // _ANIMATIONEDITOR_AANIMATION_H_