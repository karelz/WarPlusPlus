/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: Animation editor - Data objects
 *      Author: Vit Karas
 * 
 * Description: Frame object class
 * 
 ***********************************************************/

#include "stdafx.h"

#include "AFrame.h"

#include "GameClient\Common\Colors.h"
#include "GameClient\Common\CommonExceptions.h"

#include <math.h>

// RTCI for our objects
IMPLEMENT_DYNAMIC ( CAFrame, CObject );



// ----------------------------------------------------
// Construction
// ----------------------------------------------------

// Constructor
CAFrame::CAFrame ()
{
  // Set empty data
  m_bModified = false;
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_dwFrameDelay = 0;
  m_pColoring = NULL;
  m_pDestinationColor = NULL;
  m_pOriginalSurface = NULL;
  m_szOriginalSurface = CSize ( 0, 0 );
}

// Destructor
CAFrame::~CAFrame ()
{
  // Assert empty data
  ASSERT ( m_bModified == false );
  ASSERT ( m_nXOffset == 0 );
  ASSERT ( m_nYOffset == 0 );
  ASSERT ( m_dwFrameDelay == 0 );
  ASSERT ( m_pOriginalSurface == NULL );
  ASSERT ( m_szOriginalSurface == CSize ( 0, 0 ) );
  ASSERT ( m_pColoring == NULL );
  ASSERT ( m_pDestinationColor == NULL );
}



// ----------------------------------------------------
// Debug functions
// ----------------------------------------------------

#ifdef _DEBUG
  
// Asserts validity of the object's data
void CAFrame::AssertValid () const
{
  // Call the base class
  CObject::AssertValid ();

  // Assert our own data
  ASSERT ( m_cSurface.IsValid () );
  ASSERT ( m_pOriginalSurface != NULL );
  ASSERT ( m_szOriginalSurface.cx != 0 || m_szOriginalSurface.cy != 0 );
  ASSERT ( m_pColoring != NULL );
  ASSERT ( m_pDestinationColor != NULL );
}

// Dumps object's data
void CAFrame::Dump ( CDumpContext & dc ) const
{
  // Call the base class
  CObject::Dump ( dc );

  // Dump our data
  dc << "Offset X : " << m_nXOffset << ", Y : " << m_nYOffset << "\n";
  dc << "Frame delay : " << m_dwFrameDelay << " (milliseconds)\n";
}

#endif


// ----------------------------------------------------
// Creation
// ----------------------------------------------------

// Creates the frame from given archive file
void CAFrame::Create ( CArchiveFile cFile, SColoring * pColoring, DWORD * pDestinationColor )
{
  ASSERT ( pColoring != NULL );
  ASSERT ( pDestinationColor != NULL );

  // Copy parameters
  m_pColoring = pColoring;
  m_pDestinationColor = pDestinationColor;

  // Load the original surface
  LoadSurface ( cFile );
  // And copy it
  CopySurface ();

  m_rcCropRect = m_cSurface.GetAllRect ();

  // No modifications
  m_bModified = false;
}

// Deletes the frame
void CAFrame::Delete ()
{
  // Delete our surfaces
  DestroySurfaces ();

  // Empty our data
  m_dwFrameDelay = 0;
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_pColoring = NULL;
  m_pDestinationColor = NULL;
  m_bModified = false;
}



// ----------------------------------------------------
// Operations
// ----------------------------------------------------

// Crops the frame with given tolerance for alpha channel
// This removes borders of the image which has all pixel with alpha channel value
// less or equal to given tolerance
void CAFrame::Crop ( BYTE nAlphaTolerance )
{
  ASSERT_VALID ( this );

  // We must crop the original file (allways)
  // So first load the original surface from the file
  if ( m_pOriginalSurface != NULL )
  {
    delete m_pOriginalSurface;
    m_pOriginalSurface = NULL;
    m_szOriginalSurface = CSize ( 0, 0 );
  }
  LoadSurface ( m_cFile );

  // So copy it to temp buffer

  // Create the temp surface
  BYTE * pTempSurface = NULL;
  CSize szTempSurface;
  {
    szTempSurface = m_szOriginalSurface;
    pTempSurface = new BYTE [ szTempSurface.cx * szTempSurface.cy * 4 ];

    // Copy the graphics
    memcpy ( pTempSurface, m_pOriginalSurface, szTempSurface.cx * szTempSurface.cy * 4 );
  }

  // Now find the rectangle to crop the image to
  CRect rcCrop;
  {
    // First test top lines
    {
      DWORD dwLine = 0;
      DWORD dwLinePos = 0;

      for ( dwLine = 0; dwLine < (DWORD)m_szOriginalSurface.cy; dwLine ++ )
      {
        for ( dwLinePos = 0; dwLinePos < (DWORD)m_szOriginalSurface.cx; dwLinePos ++ )
        {
          // Test the alpha channel pixel for tolerance
          if ( m_pOriginalSurface [ ( dwLine * m_szOriginalSurface.cx + dwLinePos ) * 4 + 3 ] > nAlphaTolerance )
          {
            // No, it's not in the tolerance -> quit our cycles
            goto TopEndCycle;
          }
        }
      }
TopEndCycle:
      ;
      rcCrop.top = dwLine;
    }

    // Test bottom lines
    {
      DWORD dwLine = 0;
      DWORD dwLinePos = 0;

      for ( dwLine = (DWORD)m_szOriginalSurface.cy - 1; dwLine > 0; dwLine -- )
      {
        for ( dwLinePos = 0; dwLinePos < (DWORD)m_szOriginalSurface.cx; dwLinePos ++ )
        {
          // Test the alpha channel pixel for tolerance
          if ( m_pOriginalSurface [ ( dwLine * m_szOriginalSurface.cx + dwLinePos ) * 4 + 3 ] > nAlphaTolerance )
          {
            // No, it's not in the tolerance -> quit our cycles
            goto BottomEndCycle;
          }
        }
      }
BottomEndCycle:
      ;
      rcCrop.bottom = dwLine + 1;
      if ( rcCrop.bottom < rcCrop.top ) rcCrop.bottom = rcCrop.top;
    }

    // Test left lines
    {
      DWORD dwLine = 0;
      DWORD dwLinePos = 0;

      for ( dwLine = 0; dwLine < (DWORD)m_szOriginalSurface.cx; dwLine ++ )
      {
        for ( dwLinePos = 0; dwLinePos < (DWORD)m_szOriginalSurface.cy; dwLinePos ++ )
        {
          // Test the alpha channel pixel for tolerance
          if ( m_pOriginalSurface [ ( dwLinePos * m_szOriginalSurface.cx + dwLine ) * 4 + 3 ] > nAlphaTolerance )
          {
            // No, it's not in the tolerance -> quit our cycles
            goto LeftEndCycle;
          }
        }
      }
LeftEndCycle:
      ;
      rcCrop.left = dwLine;
    }

    // And test right lines
    {
      DWORD dwLine = 0;
      DWORD dwLinePos = 0;

      for ( dwLine = (DWORD)m_szOriginalSurface.cx - 1; dwLine > 0; dwLine -- )
      {
        for ( dwLinePos = 0; dwLinePos < (DWORD)m_szOriginalSurface.cy; dwLinePos ++ )
        {
          // Test the alpha channel pixel for tolerance
          if ( m_pOriginalSurface [ ( dwLinePos * m_szOriginalSurface.cx + dwLine ) * 4 + 3 ] > nAlphaTolerance )
          {
            // No, it's not in the tolerance -> quit our cycles
            goto RightEndCycle;
          }
        }
      }
RightEndCycle:
      ;
      rcCrop.right = dwLine + 1;
      if ( rcCrop.right < rcCrop.left ) rcCrop.right = rcCrop.left;
    }

    if ( rcCrop.IsRectEmpty () )
    {
      // If empty rect -> just left one pixel (the top left one, for example)
      rcCrop.SetRect ( 0, 0, 1, 1 );
    }
  }

  // Now recreate the original surface
  {
    delete m_pOriginalSurface;
    m_szOriginalSurface = CSize ( 0, 0 );

    m_szOriginalSurface.cx = rcCrop.Width ();
    m_szOriginalSurface.cy = rcCrop.Height ();
    m_pOriginalSurface = new BYTE [ m_szOriginalSurface.cx * m_szOriginalSurface.cy * 4 ];
  }

  // Copy the data
  {
    DWORD dwLine;
    for ( dwLine = 0; dwLine < (DWORD)m_szOriginalSurface.cy; dwLine ++ )
    {
      memcpy ( & (m_pOriginalSurface [ dwLine * m_szOriginalSurface.cx * 4 ] ),
        & ( pTempSurface [ ( ( dwLine + rcCrop.top ) * szTempSurface.cx + rcCrop.left ) * 4 ] ),
        m_szOriginalSurface.cx * 4 );
    }
  }

  // Delete the temp surface
  delete pTempSurface;

  // Copy the graphics to our normal surface
  m_cSurface.Delete ();
  CopySurface ();

  // Set offsets to ake the frame stay at the same place
  m_nXOffset = m_nXOffset - (rcCrop.left - m_rcCropRect.left);
  m_nYOffset = m_nYOffset - (rcCrop.top - m_rcCropRect.top);
  m_rcCropRect = rcCrop;

  SetModified ();
}


// Recomputes coloring
void CAFrame::RecomputeColoring ()
{
  ASSERT_VALID ( this );

  // Delete the surface
  m_cSurface.Delete ();
  // Recreate it from the original one
  CopySurface ();
}

// Reloads the frame from the file it was created from
void CAFrame::Reload ()
{
  ASSERT_VALID ( this );

  // Destroy all surfaces
  DestroySurfaces ();
  // Load the original from the file
  LoadSurface ( m_cFile );
  // Copy it to our surface
  CopySurface ();
}

// Saves the frame to the file it was loaded from
void CAFrame::Save ()
{
  ASSERT_VALID ( this );

  CArchiveFile cFile = m_cFile.GetDataArchive ().CreateFile ( m_cFile.GetFilePath (), CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone );
  // The tga header structure
  STGAHeader sHeader;

  // Fill the header
  sHeader.m_nIDFieldLength = 15;
  sHeader.m_nColorMapType = 0;
  sHeader.m_nImageType = 2;
  sHeader.m_wColorMapOrigin = 0;
  sHeader.m_wColorMapLength = 0;
  sHeader.m_nColorMapEntrySize = 0;
  sHeader.m_wXOrigin = 0;
  sHeader.m_wYOrigin = 0;
  sHeader.m_wWidth = (WORD)m_szOriginalSurface.cx;
  sHeader.m_wHeight = (WORD)m_szOriginalSurface.cy;
  sHeader.m_nPixelSize = 32;
  sHeader.m_nFlags = 8;

  // Write the header
  cFile.Write( &sHeader, 18 );
  cFile.Write( "TheGame format", 15 );

  DWORD dwLine;
  for ( dwLine = 0; dwLine < (DWORD)m_szOriginalSurface.cy; dwLine ++ )
  {
    cFile.Write ( & ( m_pOriginalSurface [ ( m_szOriginalSurface.cy - dwLine - 1 ) * m_szOriginalSurface.cx * 4 ] ),
      m_szOriginalSurface.cx * 4 );
  }

  // We're not modified any more
  SetModified ( false );
}


// ----------------------------------------------------
// Helper functions
// ----------------------------------------------------

// Loads the surface from given file
// The surface must be deleted this time
void CAFrame::LoadSurface ( CArchiveFile cFile )
{
  // Copy the file
  m_cFile = cFile;

  cFile.SeekToBegin ();

  STGAHeader header;
  cFile.Read ( &header, sizeof ( header ) );
  
  if ( header.m_nImageType != 2 )
  {
    CString str;
    str.Format ( "Unsupported TGA format. (%s)\n", cFile.GetFilePath () );
    throw new CStringException ( str );
  }
  
  if ( (header.m_nPixelSize != 24) && (header.m_nPixelSize != 32) )
  {
    CString str;
    str.Format ( "TGA is not 24 or 32 bit. (%s)\n", cFile.GetFilePath () );
    throw new CStringException ( str );
  }
  
  m_szOriginalSurface.cx = header.m_wWidth;
  m_szOriginalSurface.cy = header.m_wHeight;
  
  cFile.Seek ( header.m_nIDFieldLength, CFile::current );
  cFile.Seek ( header.m_wColorMapLength, CFile::current );

  m_pOriginalSurface = new BYTE [ m_szOriginalSurface.cx * m_szOriginalSurface.cy * 4 ];
  
  int i, j;
  BYTE *pLine = new BYTE [ m_szOriginalSurface.cx * 4 ];
  for ( i = 0; i < m_szOriginalSurface.cy; i++ )
  {
    if ( header.m_nPixelSize == 24 )
    {
      cFile.Read ( pLine, m_szOriginalSurface.cx * 3 );
      for ( j = 0; j < m_szOriginalSurface.cx; j ++ )
      {
        m_pOriginalSurface [ ( ( m_szOriginalSurface.cy - i - 1 ) * m_szOriginalSurface.cx + j ) * 4 ] = pLine [ j * 3 ];
        m_pOriginalSurface [ ( ( m_szOriginalSurface.cy - i - 1 ) * m_szOriginalSurface.cx + j ) * 4 + 1 ] = pLine [ j * 3 + 1 ];
        m_pOriginalSurface [ ( ( m_szOriginalSurface.cy - i - 1 ) * m_szOriginalSurface.cx + j ) * 4 + 2 ] = pLine [ j * 3 + 2 ];
        m_pOriginalSurface [ ( ( m_szOriginalSurface.cy - i - 1 ) * m_szOriginalSurface.cx + j ) * 4 + 3 ] = 0xFF;
      }
    }
    if ( header.m_nPixelSize == 32 )
    {
      VERIFY ( cFile.Read ( & ( m_pOriginalSurface [ ( m_szOriginalSurface.cy - i - 1) * m_szOriginalSurface.cx * 4 ] ),
        m_szOriginalSurface.cx * 4 ) == m_szOriginalSurface.cx * 4 );
    }
  }
  delete pLine;
}

// Copies the surface from the original surface and colors it
void CAFrame::CopySurface ()
{
  ASSERT ( !m_cSurface.IsValid () );
  ASSERT ( m_pOriginalSurface != NULL );

  // Set our surface to same attributes
  m_cSurface.SetWidth ( m_szOriginalSurface.cx );
  m_cSurface.SetHeight ( m_szOriginalSurface.cy );
  m_cSurface.SetAlphaChannel ( TRUE );

  // Create our surface
  m_cSurface.Create ();

  // Copy the graphics
  m_cSurface.Fill ( RGB32 ( 0, 0, 0 ) );

  int i, j;
  LPBYTE pData, pAlpha;
  DWORD dwPitch;
  pAlpha = (LPBYTE)m_cSurface.GetAlphaChannel ( FALSE );
  pData = (LPBYTE)m_cSurface.Lock ( m_cSurface.GetAllRect (), 0, dwPitch );

  for ( j = 0; j < m_szOriginalSurface.cy; j ++ )
  {
    for ( i = 0; i < m_szOriginalSurface.cx; i ++ )
    {
      if ( g_pDirectDraw->Is32BitMode () )
      {
        pData [ j * dwPitch + i * 4 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 ];
        pData [ j * dwPitch + i * 4 + 1 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 1 ];
        pData [ j * dwPitch + i * 4 + 2 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 2 ];
        pAlpha [ j * m_szOriginalSurface.cx + i ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 3 ];
      }
      else
      {
        pData [ j * dwPitch + i * 3 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 ];
        pData [ j * dwPitch + i * 3 + 1 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 1 ];
        pData [ j * dwPitch + i * 3 + 2 ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 2 ];
        pAlpha [ j * m_szOriginalSurface.cx + i ] = m_pOriginalSurface [ ( j * m_szOriginalSurface.cx + i ) * 4 + 3 ];
      }
    }
  }
  m_cSurface.Unlock ( pData );
  m_cSurface.ReleaseAlphaChannel ( TRUE );

  // Here color the image
  double dbSrcH, dbSrcS, dbSrcV;
  double dbDstH, dbDstS, dbDstV;

  // if 0 tolerance -> do nothing
  if(m_pColoring->m_dbHTolerance == 0) return;

  // convert our source and destination colors to HSV
  Color_RGB2HSV(R32(m_pColoring->m_dwSourceColor), G32(m_pColoring->m_dwSourceColor),
    B32(m_pColoring->m_dwSourceColor), dbSrcH, dbSrcS, dbSrcV);
  Color_RGB2HSV(R32(*m_pDestinationColor), G32(*m_pDestinationColor),
    B32(*m_pDestinationColor), dbDstH, dbDstS, dbDstV);

  // now go through all pixels in image and color them

  pData = (LPBYTE)m_cSurface.Lock ( m_cSurface.GetAllRect (), 0, dwPitch );
  
  DWORD dwX, dwY, dwWidth, dwHeight, dwOff;
  dwWidth = m_cSurface.GetWidth(); dwHeight = m_cSurface.GetHeight();

  double dbH, dbS, dbV, dbHDif, dbSDif, dbVDif;

  int R, G, B;
  
  if(g_pDirectDraw->Is32BitMode()){
    for(dwY = 0; dwY < dwHeight; dwY++){
      dwOff = dwY * dwPitch;

      for(dwX = 0; dwX < dwWidth; dwX++, dwOff+=4){

        // get the RGB value
        R = pData[dwOff + 2];
        G = pData[dwOff + 1];
        B = pData[dwOff];

        // convert to HSV
        Color_RGB2HSV(R, G, B, dbH, dbS, dbV);

        // compute differences
        dbHDif = dbH - dbSrcH;
        dbSDif = dbS - dbSrcS;
        dbVDif = dbV - dbSrcV;

        // test if we are inside tolerance -> if no do nothing
        if(fabs(dbHDif) > m_pColoring->m_dbHTolerance)
          continue;
        if(fabs(dbSDif) > m_pColoring->m_dbSTolerance)
          continue;
        if(fabs(dbVDif) > m_pColoring->m_dbVTolerance)
          continue;

        // we are in the tolerance -> color it
        // just use the dest color as the base
        // we are in the tolerance -> color it
        // just use the dest color as the base
        dbH = dbDstH + dbHDif;
        if(dbH > 360) dbH -= 360; if(dbH < 0) dbH += 360;
        dbS = dbDstS + dbSDif; 
        dbV = dbDstV + dbVDif;
        if(dbV > 1){
          dbS -= dbV - 1; dbV = 1;
        }
        if(dbV < 0) dbV = 0;
        if(dbS > 1){
          if(dbV > (dbS - 1)) dbV -= dbS - 1; else dbV = 0; 
          dbS = 1;
        }
        if(dbS < 0) dbS = 0;

        // convert the color back to RGB
        Color_HSV2RGB(dbH, dbS, dbV, R, G, B);
        pData[dwOff + 2] = (BYTE)R;
        pData[dwOff + 1] = (BYTE)G;
        pData[dwOff] = (BYTE)B;
      }
    }
  }
  else{
    for(dwY = 0; dwY < dwHeight; dwY++){
      dwOff = dwY * dwPitch;

      for(dwX = 0; dwX < dwWidth; dwX++, dwOff+=3){

        // get the RGB value
        R = pData[dwOff + 2];
        G = pData[dwOff + 1];
        B = pData[dwOff];

        // convert to HSV
        Color_RGB2HSV(R, G, B, dbH, dbS, dbV);

        // compute differences
        dbHDif = dbH - dbSrcH;
        dbSDif = dbS - dbSrcS;
        dbVDif = dbV - dbSrcV;

        // test if we are inside tolerance -> if no do nothing
        if(fabs(dbHDif) > m_pColoring->m_dbHTolerance)
          continue;
        if(fabs(dbSDif) > m_pColoring->m_dbSTolerance)
          continue;
        if(fabs(dbVDif) > m_pColoring->m_dbVTolerance)
          continue;

        // we are in the tolerance -> color it
        // just use the dest color as the base
        dbH = dbDstH + dbHDif;
        if(dbH > 360) dbH -= 360; if(dbH < 0) dbH += 360;
        dbS = dbDstS + dbSDif; 
        dbV = dbDstV + dbVDif;
        if(dbV > 1){
          dbS -= dbV - 1; dbV = 1;
        }
        if(dbV < 0) dbV = 0;
        if(dbS > 1){
          if(dbV > (dbS - 1)) dbV -= dbS - 1; else dbV = 0; 
          dbS = 1;
        }
        if(dbS < 0) dbS = 0;

        // convert the color back to RGB
        Color_HSV2RGB(dbH, dbS, dbV, R, G, B);
        pData[dwOff + 2] = (BYTE)R;
        pData[dwOff + 1] = (BYTE)G;
        pData[dwOff] = (BYTE)B;
      }
    }
  }

  m_cSurface.Unlock ( pData );
}

// Deletes the surfaces for this frame
void CAFrame::DestroySurfaces ()
{
  // Just delete the surface
  m_cSurface.Delete ();
  if ( m_pOriginalSurface != NULL )
  {
    delete m_pOriginalSurface;
    m_pOriginalSurface = NULL;
    m_szOriginalSurface = CSize ( 0, 0 );
  }
}
