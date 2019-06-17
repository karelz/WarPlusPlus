#include "stdafx.h"
#include "CUnitSurface.h"

#include "..\Common\Colors.h"
#include <math.h>

// Constructor
CCUnitSurface::CCUnitSurface()
{
  m_pColoringParams = NULL;
  m_nRefCount = 0;
  m_nXOffset = 0;
  m_nYOffset = 0;
  m_dwFrameDelay = 0;
}

// Destructor
CCUnitSurface::~CCUnitSurface()
{
}

// Debug functions
#ifdef _DEBUG

void CCUnitSurface::AssertValid() const
{
  CImageSurface::AssertValid();
  ASSERT(m_pColoringParams != NULL);
}

void CCUnitSurface::Dump(CDumpContext &dc) const
{
  CImageSurface::Dump(dc);
}

#endif


// Implementation ----------------------------------------------

// Creates the object - loads it from the disk
void CCUnitSurface::Create(CArchiveFile file, CCUnitSurface::tagSColoringParams *pColoringParams,
                           int nXOffset, int nYOffset, DWORD dwFrameDelay )
{
  ASSERT(pColoringParams != NULL);

  m_pColoringParams = pColoringParams;
  m_nXOffset = nXOffset;
  m_nYOffset = nYOffset;
  m_dwFrameDelay = dwFrameDelay;

  CImageSurface::SetFinishRGBProc(ColorImage, pColoringParams);
  // create the surface (load the image)
  CImageSurface::Create(file);
}

// Deletes the object
void CCUnitSurface::Delete()
{
  // just call the base class
  CImageSurface::Delete();

  // clear coloring params
  m_pColoringParams = NULL;
}

void CCUnitSurface::Restore()
{
  // first restore the image
  CImageSurface::Restore();
}

void CCUnitSurface::ColorImage(CDDrawSurface *pDDSurface, LPBYTE pSurface, DWORD dwPitch, LPVOID pParam)
{
  int nSrcH, nSrcS, nSrcV;
  int nDstH, nDstS, nDstV;

  SColoringParams *pColoringParams = (SColoringParams *)pParam;

  // if 0 tolerance -> do nothing
  if(pColoringParams->m_dbHTolerance == 0) return;

  // convert our source and destination colors to HSV
  Color_RGB2HSV_int(R32(pColoringParams->m_dwSourceColor), G32(pColoringParams->m_dwSourceColor),
    B32(pColoringParams->m_dwSourceColor), nSrcH, nSrcS, nSrcV);
  Color_RGB2HSV_int(R32(pColoringParams->m_dwDestColor), G32(pColoringParams->m_dwDestColor),
    B32(pColoringParams->m_dwDestColor), nDstH, nDstS, nDstV);

  // now go through all pixels in image and color them

  // lock the surface
  LPBYTE pData = pSurface;
  
  DWORD dwX, dwY, dwWidth, dwHeight, dwOff;
  dwWidth = pDDSurface->GetWidth(); dwHeight = pDDSurface->GetHeight();

  int nH, nS, nV;
  int nHTolerance=(int)pColoringParams->m_dbHTolerance;
  int nSTolerance=(int)(pColoringParams->m_dbSTolerance*255.0);
  int nVTolerance=(int)(pColoringParams->m_dbVTolerance*255.0);

  int nHDif, nSDif, nVDif;

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
        Color_RGB2HSV_int(R, G, B, nH, nS, nV);

        // compute differences
        nHDif = nH - nSrcH;
        nSDif = nS - nSrcS;
        nVDif = nV - nSrcV;

        // test if we are inside tolerance -> if no do nothing
        if(abs(nHDif) > nHTolerance)
          continue;
        if(abs(nSDif) > nSTolerance)
          continue;
        if(abs(nVDif) > nVTolerance)
          continue;

        // we are in the tolerance -> color it
        // just use the dest color as the base
        nH = nDstH + nHDif;
        if(nH > 360) nH -= 360; if(nH < 0) nH += 360;
        nS = nDstS + nSDif; 
        nV = nDstV + nVDif;
        if(nV > 255){
          nS -= nV - 255; nV = 255;
        }
        if(nV < 0) nV = 0;
        if(nS > 255){
          if(nV > (nS - 255)) nV -= nS - 255; else nV = 0; 
          nS = 255;
        }
        if(nS < 0) nS = 0;

        // convert the color back to RGB
        Color_HSV2RGB_int(nH, nS, nV, R, G, B);
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
        Color_RGB2HSV_int(R, G, B, nH, nS, nV);

        // compute differences
        nHDif = nH - nSrcH;
        nSDif = nS - nSrcS;
        nVDif = nV - nSrcV;

        // test if we are inside tolerance -> if no do nothing
        if(abs(nHDif) > nHTolerance)
          continue;
        if(abs(nSDif) > nSTolerance)
          continue;
        if(abs(nVDif) > nVTolerance)
          continue;

        // we are in the tolerance -> color it
        // just use the dest color as the base
        nH = nDstH + nHDif;
        if(nH > 360) nH -= 360; if(nH < 0) nH += 360;
        nS = nDstS + nSDif; 
        nV = nDstV + nVDif;
        if(nV > 255){
          nS -= nV - 255; nV = 255;
        }
        if(nV < 0) nV = 0;
        if(nS > 255){
          if(nV > (nS - 255)) nV -= nS - 255; else nV = 0; 
          nS = 255;
        }
        if(nS < 0) nS = 0;

        // convert the color back to RGB
        Color_HSV2RGB_int(nH, nS, nV, R, G, B);
        pData[dwOff + 2] = (BYTE)R;
        pData[dwOff + 1] = (BYTE)G;
        pData[dwOff] = (BYTE)B;
      }
    }
  }
}