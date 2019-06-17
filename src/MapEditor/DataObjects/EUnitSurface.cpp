// EUnitSurface.cpp: implementation of the CEUnitSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitSurface.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitSurface, CImageSurface)

CEUnitSurface::CEUnitSurface()
{
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_dwFrameDelay = 0;
}

CEUnitSurface::~CEUnitSurface()
{
}

#ifdef _DEBUG

void CEUnitSurface::AssertValid() const
{
  CImageSurface::AssertValid();
}

void CEUnitSurface::Dump(CDumpContext &dc) const
{
  CImageSurface::Dump(dc);
}

#endif


BOOL CEUnitSurface::Create(CArchiveFile file, SColoringParams *pColoringParams)
{
  // copy coloring parameters
  memcpy(&m_sColoringParams, pColoringParams, sizeof(m_sColoringParams));

  CImageSurface::SetFinishRGBProc(ColorImage, (LPVOID)&m_sColoringParams);

  // create the surface (load the image)
  if(!CImageSurface::Create(file)) return FALSE;

  return TRUE;
}

void CEUnitSurface::Delete()
{
  // delete the surface
  CImageSurface::Delete();
}

void CEUnitSurface::Restore()
{
  // first restore the image
  CImageSurface::Restore();
}

// colors the image with our parameters (m_sColoringParams)
void CEUnitSurface::ColorImage(CDDrawSurface *pDDSurface, LPBYTE pSurface, DWORD dwPitch, LPVOID pParam)
{
  double dbSrcH, dbSrcS, dbSrcV;
  double dbDstH, dbDstS, dbDstV;

  SColoringParams *pColoringParams = (SColoringParams *)pParam;

  // if 0 tolerance -> do nothing
  if(pColoringParams->m_dbHTolerance == 0) return;

  // convert our source and destination colors to HSV
  Color_RGB2HSV(R32(pColoringParams->m_dwSourceColor), G32(pColoringParams->m_dwSourceColor),
    B32(pColoringParams->m_dwSourceColor), dbSrcH, dbSrcS, dbSrcV);
  Color_RGB2HSV(R32(pColoringParams->m_dwDestColor), G32(pColoringParams->m_dwDestColor),
    B32(pColoringParams->m_dwDestColor), dbDstH, dbDstS, dbDstV);

  // now go through all pixels in image and color them

  LPBYTE pData;
  pData = pSurface;
  
  DWORD dwX, dwY, dwWidth, dwHeight, dwOff;
  dwWidth = pDDSurface->GetWidth(); dwHeight = pDDSurface->GetHeight();

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
        if(fabs(dbHDif) > pColoringParams->m_dbHTolerance)
          continue;
        if(fabs(dbSDif) > pColoringParams->m_dbSTolerance)
          continue;
        if(fabs(dbVDif) > pColoringParams->m_dbVTolerance)
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
        if(fabs(dbHDif) > pColoringParams->m_dbHTolerance)
          continue;
        if(fabs(dbSDif) > pColoringParams->m_dbSTolerance)
          continue;
        if(fabs(dbVDif) > pColoringParams->m_dbVTolerance)
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
}
