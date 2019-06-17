// TGAConvert.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TGAConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

bool g_bInverseAlphaChannel = false;
BYTE g_nAlphaTolerance = 0;

CWinApp theApp;

bool ConvertOneTGA ( CString strSourceImage, CString strSourceAlphaImage, CString strOutputImage );

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
    CString strSourceMask;
    CString strOutputPath;

    if ( argc < 3 )
    {
      cout << "TGAConvert: usage\n\n";
      cout << "  TGAConvert.exe [flags] [SourceMask] [OutputPath]\n\n";
      cout << "  flags : -I   Invert the alpha channel\n";
      cout << "        : -Txx Specifies alpha tolerance to use when cropping\n";
      return 0;
    }

    int nArg;
    for ( nArg = 1; nArg < argc; nArg++ )
    {
      CString strArg = argv [ nArg ];

      if ( (strArg [ 0 ] == '/') || (strArg [ 0 ] == '-') )
      {
        strArg = strArg.Mid ( 1 );

        if ( strArg [ 0 ] == 'I' || strArg [ 0 ] == 'i' )
        {
          g_bInverseAlphaChannel = true;
        }
        else if ( strArg [ 0 ] == 'T' || strArg [ 0 ] == 't' )
        {
          g_nAlphaTolerance = atoi ( strArg.Mid ( 1 ) );
        }
      }
      else
      {
        if ( !strSourceMask.IsEmpty () )
        {
          strOutputPath = strArg;
        }
        else
        {
          strSourceMask = strArg;
        }
      }
    }

    if ( strOutputPath.IsEmpty () )
    {
      cout << "Yout must specify output path.\n";
      return 0;
    }

    try
    {

      CFileFind cFind;
      cFind.FindFile ( strSourceMask, 0 );
      while ( cFind.FindNextFile () )
      {
        if ( cFind.IsDirectory () ) continue;
        if ( cFind.IsDots () ) continue;

        CString strSourceImage, strSourceAlphaImage, strOutputImage;

        strSourceImage = cFind.GetFilePath ();
        int nPos = strSourceImage.ReverseFind ( '.' );
        if ( nPos == -1 )
          strSourceAlphaImage = strSourceImage + "T";
        else
        {
          if ( strSourceImage [ nPos - 1] == 'T' ) continue;
          strSourceAlphaImage = strSourceImage.Left ( nPos ) + "T" + strSourceImage.Mid ( nPos );
        }

        strOutputImage = strOutputPath + "\\" + cFind.GetFileName ();

        CString strAlphaName;
        nPos = strSourceAlphaImage.ReverseFind ( '\\' );
        if ( nPos == -1 )
          strAlphaName = strSourceAlphaImage;
        else
          strAlphaName = strSourceAlphaImage.Mid ( nPos + 1 );
        cout << (LPCSTR)cFind.GetFileName () << " + " << (LPCSTR)strAlphaName << "....\n";

        if ( !ConvertOneTGA ( strSourceImage, strSourceAlphaImage, strOutputImage ) ) return 0;
      }

    }
    catch ( CException * e)
    {
      char txt [1024];

      e->GetErrorMessage ( txt, 1023, NULL );
      e->Delete ();

      cout << txt << "\n";
    }
	}

	return nRetCode;
}

#pragma pack(1)
typedef struct tagSTGAHeader{
  BYTE m_nIDFieldLength;
  BYTE m_nColorMapType;
  BYTE m_nImageType;
  WORD m_wColorMapOrigin;
  WORD m_wColorMapLength;
  BYTE m_nColorMapEntrySize;
  WORD m_wXOrigin;
  WORD m_wYOrigin;
  WORD m_wWidth;
  WORD m_wHeight;
  BYTE m_nPixelSize;
  BYTE m_nFlags;
}TGAHeader;
#pragma pack()

bool ConvertOneTGA ( CString strSourceImage, CString strSourceAlphaImage, CString strOutputImage )
{
  BYTE * pSourceImage = NULL, * pSourceAlphaImage = NULL;
  CSize szSourceImage, szSourceAlphaImage;

  CFileException * pFileException = new CFileException ();
  // Open the source image
  CFile cSourceImage;
  if ( !cSourceImage.Open ( strSourceImage, CFile::modeRead, pFileException ) )
    throw pFileException;

  // Open the source alpha image
  CFile cSourceAlphaImage;
  if ( !cSourceAlphaImage.Open ( strSourceAlphaImage, CFile::modeRead, pFileException ) )
    throw pFileException;

  // First read the normal image
  {
    TGAHeader header;
    cSourceImage.Read ( &header, sizeof ( header ) );

    if ( header.m_nImageType != 2 )
    {
      printf ( "Unsupported TGA format. (%s)\n", strSourceImage );
      return false;
    }

    if ( (header.m_nPixelSize != 24) && (header.m_nPixelSize != 32) )
    {
      printf ( "TGA is not 24 or 32 bit. (%s)\n", strSourceImage );
      return false;
    }

    szSourceImage.cx = header.m_wWidth;
    szSourceImage.cy = header.m_wHeight;

    cSourceImage.Seek ( header.m_nIDFieldLength, CFile::current );
    cSourceImage.Seek ( header.m_wColorMapLength, CFile::current );

    pSourceImage = new BYTE [ szSourceImage.cx * szSourceImage.cy * 3 ];

    int i, j;
    BYTE *pLine = new BYTE [ szSourceImage.cx * 4 ];
    for ( i = 0; i < szSourceImage.cy; i++ )
    {
      if ( header.m_nPixelSize == 24 )
      {
        cSourceImage.Read ( &pSourceImage [ (szSourceImage.cy - i - 1) * szSourceImage.cx * 3 ], szSourceImage.cx * 3 );
      }
      if ( header.m_nPixelSize == 32 )
      {
        cSourceImage.Read ( pLine, szSourceImage.cx * 4 );
        for ( j = 0; j < szSourceImage.cx; j ++ )
        {
          pSourceImage [ (szSourceImage.cy - i - 1) * szSourceImage.cx * 3 + j * 3 ] = pLine [ j * 4 ];
          pSourceImage [ (szSourceImage.cy - i - 1) * szSourceImage.cx * 3 + j * 3 + 1 ] = pLine [ j * 4 + 1 ];
          pSourceImage [ (szSourceImage.cy - i - 1) * szSourceImage.cx * 3 + j * 3 + 2 ] = pLine [ j * 4 + 2 ];
        }
      }
    }
    delete pLine;
  }

  // Then read the alpha image
  {
    TGAHeader header;
    cSourceAlphaImage.Read ( &header, sizeof ( header ) );

    if ( header.m_nImageType != 2 )
    {
      printf ( "Unsupported TGA format. (%s)\n", strSourceImage );
      return false;
    }

    if ( (header.m_nPixelSize != 24) && (header.m_nPixelSize != 32) )
    {
      printf ( "TGA is not 24 or 32 bit. (%s)\n", strSourceImage );
      return false;
    }

    szSourceAlphaImage.cx = header.m_wWidth;
    szSourceAlphaImage.cy = header.m_wHeight;

    if ( szSourceAlphaImage != szSourceImage )
    {
      printf ( "The source RGB image (%s) and the source alpha image (%s) are not the same size.\n", strSourceImage, strSourceAlphaImage );
      return false;
    }

    cSourceAlphaImage.Seek ( header.m_nIDFieldLength, CFile::current );
    cSourceAlphaImage.Seek ( header.m_wColorMapLength, CFile::current );

    pSourceAlphaImage = new BYTE [ szSourceAlphaImage.cx * szSourceAlphaImage.cy ];

    int i, j;
    BYTE *pLine = new BYTE [ szSourceAlphaImage.cx * 4 ];
    for ( i = 0; i < szSourceAlphaImage.cy; i++ )
    {
      if ( header.m_nPixelSize == 24 )
      {
        cSourceAlphaImage.Read ( pLine, szSourceAlphaImage.cx * 3 );
        // Convert the alpha
        for ( j = 0; j < szSourceImage.cx; j++ )
        {
          if ( g_bInverseAlphaChannel )
          {
            pSourceAlphaImage [ (szSourceAlphaImage.cy - i - 1) * szSourceAlphaImage.cx + j ] =
              255 - (pLine [ j * 3 ] + pLine [ j * 3 + 1 ] + pLine [ j * 3 + 2 ]) / 3;
          }
          else
          {
            pSourceAlphaImage [ (szSourceAlphaImage.cy - i - 1) * szSourceAlphaImage.cx + j ] =
              (pLine [ j * 3 ] + pLine [ j * 3 + 1 ] + pLine [ j * 3 + 2 ]) / 3;
          }
        }
      }
      if ( header.m_nPixelSize == 32 )
      {
        cSourceImage.Read ( pLine, szSourceImage.cx * 4 );
        // Convert the alpha
        for ( j = 0; j < szSourceImage.cx; j ++ )
        {
          if ( g_bInverseAlphaChannel )
          {
            pSourceAlphaImage [ (szSourceAlphaImage.cy - i - 1) * szSourceAlphaImage.cx + j ] =
              255 - (pLine [ j * 4 ] + pLine [ j * 4 + 1 ] + pLine [ j * 4 + 2 ]) / 3;
          }
          else
          {
            pSourceAlphaImage [ (szSourceAlphaImage.cy - i - 1) * szSourceAlphaImage.cx + j ] =
              (pLine [ j * 4 ] + pLine [ j * 4 + 1 ] + pLine [ j * 4 + 2 ]) / 3;
          }
        }
      }
    }
    delete pLine;
  }

  // Now crop the image
  CRect rcCrop;
  {
    {
      // First test top lines
      {
        int dwLine = 0;
        int dwLinePos = 0;
        
        for ( dwLine = 0; dwLine < szSourceAlphaImage.cy; dwLine ++ )
        {
          for ( dwLinePos = 0; dwLinePos < szSourceAlphaImage.cx; dwLinePos ++ )
          {
            // Test the alpha channel pixel for tolerance
            if ( pSourceAlphaImage [ dwLine * szSourceAlphaImage.cx + dwLinePos ] > g_nAlphaTolerance )
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
        int dwLine = 0;
        int dwLinePos = 0;
        
        for ( dwLine = szSourceAlphaImage.cy - 1; dwLine > 0; dwLine -- )
        {
          for ( dwLinePos = 0; dwLinePos < szSourceAlphaImage.cx; dwLinePos ++ )
          {
            // Test the alpha channel pixel for tolerance
            if ( pSourceAlphaImage [ dwLine * szSourceAlphaImage.cx + dwLinePos ] > g_nAlphaTolerance )
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
        int dwLine = 0;
        int dwLinePos = 0;
        
        for ( dwLine = 0; dwLine < szSourceAlphaImage.cx; dwLine ++ )
        {
          for ( dwLinePos = 0; dwLinePos < szSourceAlphaImage.cy; dwLinePos ++ )
          {
            // Test the alpha channel pixel for tolerance
            if ( pSourceAlphaImage [ dwLinePos * szSourceAlphaImage.cx + dwLine ] > g_nAlphaTolerance )
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
        int dwLine = 0;
        int dwLinePos = 0;
        
        for ( dwLine = szSourceAlphaImage.cx - 1; dwLine > 0; dwLine -- )
        {
          for ( dwLinePos = 0; dwLinePos < szSourceAlphaImage.cy; dwLinePos ++ )
          {
            // Test the alpha channel pixel for tolerance
            if ( pSourceAlphaImage [ dwLinePos * szSourceAlphaImage.cx + dwLine ] > g_nAlphaTolerance )
            {
              // No, it's not in the tolerance -> quit our cycles
              goto RightEndCycle;
            }
          }
        }
RightEndCycle:
        ;
        rcCrop.right = dwLine - 1;
        if ( rcCrop.right < rcCrop.left ) rcCrop.right = rcCrop.left;
      }
      
      if ( rcCrop.IsRectEmpty () )
      {
        // If empty rect -> just left one pixel (the top left one, for example)
        rcCrop.SetRect ( 0, 0, 1, 1 );
      }
    }
  }

  // Now we have in the rcCrop the rect of the new image
  // So copy the data to the result image
  BYTE * pOutputImage = new BYTE [ rcCrop.Width () * rcCrop.Height () * 4 ];
  {
    int i, j;
    for ( i = 0; i < rcCrop.Height (); i++ )
    {
      for ( j = 0; j < rcCrop.Width (); j++ )
      {
        pOutputImage [ (rcCrop.Height () - i - 1) * rcCrop.Width () * 4 + j * 4 ] = pSourceImage [ ( i + rcCrop.top ) * szSourceImage.cx * 3 + ( j + rcCrop.left ) * 3 ];
        pOutputImage [ (rcCrop.Height () - i - 1) * rcCrop.Width () * 4 + j * 4 + 1 ] = pSourceImage [ ( i + rcCrop.top ) * szSourceImage.cx * 3 + ( j + rcCrop.left ) * 3 + 1 ];
        pOutputImage [ (rcCrop.Height () - i - 1) * rcCrop.Width () * 4 + j * 4 + 2 ] = pSourceImage [ ( i + rcCrop.top ) * szSourceImage.cx * 3 + ( j + rcCrop.left ) * 3 + 2 ];
        pOutputImage [ (rcCrop.Height () - i - 1) * rcCrop.Width () * 4 + j * 4 + 3 ] = pSourceAlphaImage [ ( i + rcCrop.top ) * szSourceAlphaImage.cx + ( j + rcCrop.left ) ];
      }
    }
  }

  // Write the output tga image
  {
    CFile cOutputImage;
    if ( !cOutputImage.Open ( strOutputImage, CFile::modeCreate | CFile::modeWrite, pFileException ) )
      throw pFileException;

    TGAHeader header;
    memset ( &header, 0, sizeof ( header ) );
    header.m_nIDFieldLength = 15;
    header.m_nColorMapType = 0;
    header.m_nImageType = 2;
    header.m_wColorMapOrigin = 0;
    header.m_wColorMapLength = 0;
    header.m_nColorMapEntrySize = 0;
    header.m_wXOrigin = 0;
    header.m_wYOrigin = 0;
    header.m_wWidth = (WORD)rcCrop.Width ();
    header.m_wHeight = (WORD)rcCrop.Height ();
    header.m_nPixelSize = 32;
    header.m_nFlags = 8;
    cOutputImage.Write( &header, 18);
    cOutputImage.Write("TheGame format", 15);

    cOutputImage.Write ( pOutputImage, rcCrop.Width () * rcCrop.Height () * 4 );

    cOutputImage.Close ();
  }

  if ( pOutputImage != NULL )
  {
    delete pOutputImage;
  }
  if ( pSourceAlphaImage != NULL )
  {
    delete pSourceAlphaImage;
  }
  if ( pSourceImage != NULL )
  {
    delete pSourceImage;
  }
  cSourceAlphaImage.Close ();
  cSourceImage.Close ();

  pFileException->Delete ();

  return true;
}