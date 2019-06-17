// FontObject.cpp: implementation of the CFontObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FontObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFontObject *g_pSystemFont = NULL;

IMPLEMENT_DYNAMIC(CFontObject, CObject)

CFontObject::CFontObject()
{
  m_dwColor = 0; // set the black color as default
}

CFontObject::~CFontObject()
{

}

/////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CFontObject::AssertValid() const
{
  CObject::AssertValid();
}

void CFontObject::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);

  dc << "Font color : " << m_dwColor << "\n";
}

#endif


BOOL CFontObject::Create(DWORD dwColor)
{
  m_dwColor = dwColor;

  return TRUE;
}

void CFontObject::Delete()
{

}

void CFontObject::SetColor(DWORD dwColor)
{
  m_dwColor = dwColor;
}

DWORD CFontObject::GetColor()
{
  return m_dwColor;
}

CSize CFontObject::GetTextSize(CString strText)
{
  return CSize(0, 0);
}

CSize CFontObject::GetCharSize(char Character)
{
  return CSize(0, 0);
}