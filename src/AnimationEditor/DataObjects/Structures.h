/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: Animation editor - Data objects
 *      Author: Vit Karas
 * 
 * Description: Structure definitions
 * 
 ***********************************************************/

#ifndef _ANIMATIONEDITOR_STRUCTURES_H_
#define _ANIMATIONEDITOR_STRUCTURES_H_

// Structure containing all coloring params
struct SColoring
{
  DWORD m_dwSourceColor;  // the source color to be replaced
  double m_dbHTolerance;  // toleration for H (if 0 -> no coloring)
  double m_dbSTolerance;  // toleration for S
  double m_dbVTolerance;  // toleration for V
};


// The Targa (*.tga) file header
#pragma pack(1)
struct STGAHeader{
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
};
#pragma pack()

#endif // _ANIMATIONEDITOR_STRUCTURES_H_