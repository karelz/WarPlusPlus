/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: Animation editor - Data objects
 *      Author: Vit Karas
 * 
 * Description: Animation object class
 * 
 ***********************************************************/

#include "stdafx.h"

#include "AAnimation.h"

#include "GameClient\Common\CommonExceptions.h"

// RTCI for our objects
IMPLEMENT_DYNAMIC ( CAAnimation, CObject );


// ----------------------------------------------------
// Construction
// ----------------------------------------------------

// Constructor
CAAnimation::CAAnimation ()
{
  // Set empty data
  m_pFrames = NULL;
  m_dwFrameCount = 0;
  m_dwDefaultFrameDelay = 0;
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_bLoop = false;
  m_bModified = false;
  m_dwDestinationColor = RGB32 ( 0, 0, 0 );
  memset ( &m_sColoring, 0, sizeof ( m_sColoring ) );
}

// Destructor
CAAnimation::~CAAnimation ()
{
  // Assert empty data
  ASSERT ( m_pFrames == NULL );
  ASSERT ( m_dwFrameCount == 0 );
  ASSERT ( m_dwDefaultFrameDelay == 0 );
  ASSERT ( m_nXOffset == 0 );
  ASSERT ( m_nYOffset == 0 );
  ASSERT ( m_bLoop == false );
  ASSERT ( m_bModified == false );
  ASSERT ( m_dwDestinationColor == RGB32 ( 0, 0, 0 ) );
}


// ----------------------------------------------------
// Debug functions
// ----------------------------------------------------

#ifdef _DEBUG

// Asserts validity of the object's data
void CAAnimation::AssertValid () const
{
  // Call the base class
  CObject::AssertValid ();

  // Assert our own data
  DWORD dwFrame;
  CAFrame * pFrame = m_pFrames;
  for ( dwFrame = 0; dwFrame < m_dwFrameCount; dwFrame ++ )
  {
    ASSERT ( pFrame != NULL );
    pFrame = pFrame->m_pNext;
  }
}

// Dumps object's data
void CAAnimation::Dump ( CDumpContext & dc ) const
{
  // Call the base class
  CObject::Dump ( dc );

  // Dump our own data
  dc << "Frame count : " << m_dwFrameCount << "\n";
  dc << "Default frame delay : " << m_dwDefaultFrameDelay << "\n";
  dc << "Loop : " << m_bLoop << "\n";
  dc << "Offset X : " << m_nXOffset << ", Y : " << m_nYOffset << "\n";
  dc << "Destination color : " << (void *)m_dwDestinationColor << "\n";
}

#endif



// ----------------------------------------------------
// Creation
// ----------------------------------------------------

// Creates empty animation
void CAAnimation::Create ()
{
  m_dwFrameCount = 0;
  m_pFrames = NULL;
  m_dwDefaultFrameDelay = 50;
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_bLoop = true;
  m_bModified = true;
  m_dwDestinationColor = RGB32 ( 220, 0, 0 );
  m_sColoring.m_dwSourceColor = RGB32 ( 220, 0, 220 );
  m_sColoring.m_dbHTolerance = 0; // No coloring by default
  m_sColoring.m_dbSTolerance = 0.05;
  m_sColoring.m_dbVTolerance = 1;
}

// Creates the frame from given archive file
void CAAnimation::Create ( CArchiveFile cFile )
{
  // Load the animation from the file
  // First open it as config file
  CConfigFile cConfigFile;
  cConfigFile.Create ( cFile );

  // Get the root section
  CConfigFileSection cRootSection = cConfigFile.GetRootSection ();

  // Destination color - default
  m_dwDestinationColor = RGB32 ( 220, 0, 0 );

  // Set some defaults
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_bLoop = true;
  m_sColoring.m_dwSourceColor = RGB32 ( 220, 0, 220 );
  m_sColoring.m_dbHTolerance = 0; // No coloring by default
  m_sColoring.m_dbSTolerance = 0.05;
  m_sColoring.m_dbVTolerance = 1;

  // Read it
  try
  {
    int nHlp;
    // Read number of frames
    nHlp = cRootSection.GetInteger ( "FrameNum" );
    if ( nHlp < 0 ) throw new CStringException ( "Illegal number of frames." );
    m_dwFrameCount = (DWORD)nHlp;

    // Read the default frame delay (speed)
    nHlp = cRootSection.GetInteger ( "Speed" );
    if ( nHlp < 0 ) throw new CStringException ( "Illegal speed." );
    m_dwDefaultFrameDelay = nHlp;

    // Read loop flag
    if ( cRootSection.IsVariable ( "Loop" ) )
    {
      m_bLoop = cRootSection.GetBoolean ( "Loop" ) ? true : false;
    }

    // Read graphical offsets
    if ( cRootSection.IsVariable ( "XOffset" ) )
    {
      m_nXOffset = cRootSection.GetInteger ( "XOffset" );
    }
    if ( cRootSection.IsVariable ( "YOffset" ) )
    {
      m_nYOffset = cRootSection.GetInteger ( "YOffset" );
    }

    // Read coloring
    if ( cRootSection.IsSubsection ( "Coloring" ) )
    {
      CConfigFileSection cColoringSection = cRootSection.GetSubsection ( "Coloring" );

      if ( cColoringSection.IsVariable ( "HTolerance" ) )
      {
        m_sColoring.m_dbHTolerance = cColoringSection.GetReal ( "HTolerance" );
      }
      if ( cColoringSection.IsVariable ( "STolerance" ) )
      {
        m_sColoring.m_dbSTolerance = cColoringSection.GetReal ( "STolerance" );
      }
      if ( cColoringSection.IsVariable ( "VTolerance" ) )
      {
        m_sColoring.m_dbVTolerance = cColoringSection.GetReal ( "VTolerance" );
      }

      if ( cColoringSection.IsVariable ( "SourceColor" ) )
      {
        m_sColoring.m_dwSourceColor = cColoringSection.GetInteger ( "SourceColor" );
      }
    }

    // Read frames
    DWORD dwFrame;
    CAFrame * pFrame, ** pFrameInsert = &m_pFrames;
    CString strFrameFileName;

    // get the path for frames
    CString strPath;
    strPath = cFile.GetFileDir ();
    if ( !strPath.IsEmpty () ) strPath += "\\";

    // get the archive
    CDataArchive cArchive;
    cArchive = cFile.GetDataArchive ();

    CString strTagName;
    for ( dwFrame = 0; dwFrame < m_dwFrameCount; dwFrame ++ )
    {
      pFrame = NULL;

      try
      {
        // Get the file name
        strTagName.Format ( "Frame%d", dwFrame );
        strFrameFileName = cRootSection.GetString ( strTagName );
        if ( strFrameFileName.Find ( ":" ) == -1 )
          strFrameFileName = strPath + strFrameFileName;

        // Create new frame object
        pFrame = new CAFrame ();
        pFrame->m_pNext = NULL;
        pFrame->Create ( cArchive.CreateFile ( strFrameFileName, CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone ), &m_sColoring, &m_dwDestinationColor );

        // Read the frame delay
        strTagName.Format ( "Frame%d_Delay", dwFrame );
        if ( cRootSection.IsVariable ( strTagName ) )
        {
          pFrame->SetFrameDelay ( cRootSection.GetInteger ( strTagName ) );
        }

        // Read the frame offsets
        strTagName.Format ( "Frame%d_XOffset", dwFrame );
        if ( cRootSection.IsVariable ( strTagName ) )
        {
          pFrame->SetXOffset ( cRootSection.GetInteger ( strTagName ) );
        }
        strTagName.Format ( "Frame%d_YOffset", dwFrame );
        if ( cRootSection.IsVariable ( strTagName ) )
        {
          pFrame->SetYOffset ( cRootSection.GetInteger ( strTagName ) );
        }
      }
      catch ( CException * )
      {
        if ( pFrame != NULL )
        {
          pFrame->Delete ();
          delete pFrame;
        }
        throw;
      }

      // Add the frame to the list
      // (To the end)
      *pFrameInsert = pFrame;
      pFrameInsert = & ( pFrame->m_pNext );
    }
  }
  catch ( CException * )
  {
    Delete ();
    throw;
  }

  m_bModified = false;
}

// Deletes the frame
void CAAnimation::Delete ()
{
  // Delete all frames
  CAFrame * pFrame = m_pFrames, * pDel;
  while ( pFrame != NULL )
  {
    pDel = pFrame;
    pFrame = pFrame->m_pNext;

    pDel->Delete ();
    delete pDel;
  }
  m_pFrames = NULL;
  m_dwFrameCount = 0;
  m_dwDefaultFrameDelay = 0;
  m_dwDestinationColor = RGB32 ( 0, 0, 0 );
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_bLoop = false;
  m_bModified = false;
}



// ----------------------------------------------------
// Operations
// ----------------------------------------------------

// Saves the animation to given file
void CAAnimation::Save ( CArchiveFile cFile )
{
  ASSERT_VALID ( this );

  CString strHlp;

  // Write the frame num
  strHlp.Format ( "FrameNum = %d;\n", m_dwFrameCount );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  // Write the speed
  strHlp.Format ( "Speed = %d;\n", m_dwDefaultFrameDelay );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  // Write the loop flag
  strHlp.Format ( "Loop = %s;\n\n", (m_bLoop) ? "True" : "False" );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  // Write graphical offsets
  strHlp.Format ( "XOffset = %d;\nYOffset = %d;\n\n\n", m_nXOffset, m_nYOffset );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  // Write the frames
  DWORD dwFrame;
  CString strPath = cFile.GetFileDir ();
  if ( !strPath.IsEmpty () ) strPath += "\\";

  CString strFrameFileName;
  CAFrame * pFrame = m_pFrames;
  for ( dwFrame = 0; dwFrame < m_dwFrameCount; dwFrame ++ )
  {
    ASSERT_VALID ( pFrame );

    // First save the frame if it was modified before
    if ( pFrame->IsModified () )
      pFrame->Save ();

    strFrameFileName = pFrame->GetFilePath ();

    // Is our path the prefix to the frame path ?
    if ( strFrameFileName.Left ( strPath.GetLength () ).CompareNoCase ( strPath ) == 0 )
    {
      // Yes it's the prefix
      // Modify the frame path only to the relative path
      strFrameFileName = strFrameFileName.Mid ( strPath.GetLength () );
    }

    // Write the row
    strHlp.Format ( "Frame%d = \"%s\";\n", dwFrame, strFrameFileName );
    cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

    // Write the frame delay
    strHlp.Format ( "Frame%d_Delay = %d;\n", dwFrame, pFrame->GetFrameDelay () );
    cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

    // Write the frame offsets
    strHlp.Format ( "Frame%d_XOffset = %d;\nFrame%d_YOffset = %d;\n\n", dwFrame, pFrame->GetXOffset (), dwFrame, pFrame->GetYOffset () );
    cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

    // Go to the next frame
    pFrame = pFrame->m_pNext;
  }

  // Now write the coloring section
  strHlp.Format ( "\n[Coloring]\n" );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  strHlp.Format ( "SourceColor = 0x0%06X;\n", m_sColoring.m_dwSourceColor );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );
  strHlp.Format ( "HTolerance = %f;\n", m_sColoring.m_dbHTolerance );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );
  strHlp.Format ( "STolerance = %f;\n", m_sColoring.m_dbSTolerance );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );
  strHlp.Format ( "VTolerance = %f;\n", m_sColoring.m_dbVTolerance );
  cFile.Write ( (LPCSTR)strHlp, strHlp.GetLength () );

  // No modification no
  m_bModified = false;
}


// Inserts new frame from given file
// This will insert the frame before specified index
// If the index is behind the end of animation, the frame will be inserted to the end
CAFrame * CAAnimation::InsertFrame ( CArchiveFile cFile, DWORD dwBeforeIndex )
{
  ASSERT_VALID ( this );
  ASSERT ( dwBeforeIndex <= m_dwFrameCount );

  CAFrame * pFrame = NULL;

  // First create the frame
  try
  {
    pFrame = new CAFrame ();

    pFrame->Create ( cFile, &m_sColoring, &m_dwDestinationColor );
  }
  catch ( CException * )
  {
    if ( pFrame != NULL )
    {
      pFrame->Delete ();
      delete pFrame;
    }
    throw;
  }

  // Now insert it to our list
  CAFrame * * pInsert = &m_pFrames;
  DWORD dwIndex = 0;
  while ( *pInsert != NULL )
  {
    if ( dwIndex == dwBeforeIndex )
      break;

    pInsert = &( (*pInsert)->m_pNext );
    dwIndex++;
  }

  // Insert it
  pFrame->m_pNext = (*pInsert);
  *pInsert = pFrame;

  // Increase number of frames
  m_dwFrameCount ++ ;

  SetModified ();

  // Return the new frame
  return pFrame;
}

// Removes and deletes the frame on specified index from the animation
void CAAnimation::DeleteFrame ( DWORD dwIndex )
{
  ASSERT_VALID ( this );
  ASSERT ( dwIndex < m_dwFrameCount );

  // Find the frame
  CAFrame ** pFind = &m_pFrames, *pFrame;
  DWORD dwI = 0;
  while ( (*pFind) != NULL )
  {
    if ( dwI == dwIndex )
      break;

    pFind = & ( (*pFind)->m_pNext );
    dwI ++;
  }
  ASSERT ( (*pFind) != NULL );

  pFrame = (*pFind);

  // Remove it from the list
  *pFind = pFrame->m_pNext;

  // Delete it
  pFrame->Delete ();
  delete pFrame;

  m_dwFrameCount--;

  SetModified ();
}


// Moves given frame before the previous frame
void CAAnimation::MoveFrameLeft ( DWORD dwIndex )
{
  ASSERT_VALID ( this );
  ASSERT ( dwIndex < m_dwFrameCount );

  // We can't move the frame if it's already the first
  if ( dwIndex == 0 ) return;

  // Find it in the list
  CAFrame * * pPrev = &m_pFrames, *pFrame;
  ASSERT ( *pPrev != NULL );
  DWORD dwFrame = 1;
  while ( (*pPrev)->m_pNext != NULL )
  {
    if ( dwFrame == dwIndex )
      break;
    dwFrame ++;
    pPrev = &((*pPrev)->m_pNext);
  }

  // Move it
  pFrame = (*pPrev)->m_pNext;
  (*pPrev)->m_pNext = pFrame->m_pNext;
  pFrame->m_pNext = (*pPrev);
  *pPrev = pFrame;
}

// Moves given frame after the next frame
void CAAnimation::MoveFrameRight ( DWORD dwIndex )
{
  ASSERT_VALID ( this );
  ASSERT ( dwIndex < m_dwFrameCount );

  // We can't move the frame if it's already the last
  if ( dwIndex - 1 == m_dwFrameCount ) return;

  CAFrame * * pPrev = &m_pFrames, *pFrame;
  DWORD dwFrame = 0;
  while ( (*pPrev) != NULL )
  {
    if ( dwFrame == dwIndex )
      break;
    dwFrame ++;
    pPrev = &((*pPrev)->m_pNext);
  }

  // Move it
  pFrame = (*pPrev);
  *pPrev = pFrame->m_pNext;
  pFrame->m_pNext = pFrame->m_pNext->m_pNext;
  (*pPrev)->m_pNext = pFrame;
}




// ----------------------------------------------------
// Get/Set functions
// ----------------------------------------------------

// Sets new destination color
void CAAnimation::SetDestinationColor ( DWORD dwDestinationColor )
{
  ASSERT_VALID ( this );

  // Set new destination color
  m_dwDestinationColor = dwDestinationColor;

  // Go through all frames and set them the color
  CAFrame * pFrame = m_pFrames;
  while ( pFrame != NULL )
  {
    pFrame->RecomputeColoring ();
    pFrame = pFrame->m_pNext;
  }
}

// This recomputes whole animation with new coloring
void CAAnimation::ReleaseColoring ()
{
  ASSERT_VALID ( this );

  // Go through all frames and recompute coloring
  CAFrame * pFrame = m_pFrames;
  while ( pFrame != NULL )
  {
    pFrame->RecomputeColoring ();
    pFrame = pFrame->m_pNext;
  }
}

// Returns true if someting is modified (animation or any frame)
bool CAAnimation::IsModified () const
{
  ASSERT_VALID ( this );

  if ( GetModified () ) return true;

  // Go through all frames
  CAFrame * pFrame = m_pFrames;
  while ( pFrame != NULL )
  {
    if ( pFrame->IsModified () ) return true;
    pFrame = pFrame->m_pNext;
  }
  return false;
}




// ----------------------------------------------------
// Helper functions
// ----------------------------------------------------

// Finds frame by index
CAFrame * CAAnimation::FindFrame ( DWORD dwIndex ) const
{
  DWORD dwI = 0;
  CAFrame *pFrame = m_pFrames;
  while ( pFrame != NULL )
  {
    if ( dwI == dwIndex )
      break;
    pFrame = pFrame->m_pNext;
    dwI ++;
  }
  return pFrame;
}
