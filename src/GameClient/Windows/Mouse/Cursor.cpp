// Cursor.cpp: implementation of the CCursor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Cursor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCursor::CCursor()
{
  m_dwHotX = 0; m_dwHotY = 0;
  m_dwFrame = 0;
}

CCursor::~CCursor()
{
}

BOOL CCursor::Create(CArchiveFile File)
{
  CConfigFile CursorFile;

  // open the cursor file
  CursorFile.Create(File);

  // create the animation from this file
  m_Animation.Create(&CursorFile);

  // get the root section
  CConfigFileSection Section = CursorFile.GetSection("");

  // read hot spot
  m_dwHotX = 0;
  m_dwHotY = 0;
  m_dwHotX = Section.GetInteger("HotSpot_X");
  m_dwHotY = Section.GetInteger("HotSpot_Y");

  m_dwFrame = 0;

  return TRUE;
}

void CCursor::Delete()
{
  m_Animation.Delete();
}

BOOL CCursor::OnTimeTick(DWORD dwTime)
{
  m_dwFrame++;
  if(m_dwFrame >= m_Animation.GetFrameNum()) m_dwFrame = 0;

  return TRUE;
}

CDDrawSurface * CCursor::GetCursorBitmap()
{
  return m_Animation.GetFrame(m_dwFrame);
}

DWORD CCursor::GetHotX()
{
  return m_dwHotX;
}

DWORD CCursor::GetHotY()
{
  return m_dwHotY;
}

DWORD CCursor::GetElapseTime()
{
  return m_Animation.GetSpeed();
}