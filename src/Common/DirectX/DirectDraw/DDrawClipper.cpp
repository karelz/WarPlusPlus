// DDrawClipper.cpp: implementation of the CDDrawClipper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DDrawClipper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDDrawClipper::CDDrawClipper()
{
  m_lpDDClipper = NULL;
  m_pNextClipper = NULL;
  m_pDirectDraw = NULL;
  m_bRemoveFromList = FALSE;
}

CDDrawClipper::~CDDrawClipper()
{
  Delete();
}

#ifdef _DEBUG

void CDDrawClipper::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_lpDDClipper != NULL);
}

void CDDrawClipper::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);

  dc << "IDirectDrawClipper interface : " << m_lpDDClipper << "\n";
}

#endif

void CDDrawClipper::Delete()
{
  if(m_lpDDClipper != NULL){
    m_lpDDClipper->Release();
    m_lpDDClipper = NULL;

    if(m_bRemoveFromList){
      ASSERT(m_pDirectDraw != NULL);

      if(m_pDirectDraw) m_pDirectDraw->DeleteClipper(this);
    }
    m_bRemoveFromList = FALSE;
  }
}

BOOL CDDrawClipper::Create()
{
  if(g_pDirectDraw == NULL){
    ASSERT(FALSE);
    return FALSE;
  }

  if(!g_pDirectDraw->CreateClipper(this)) return FALSE;

  return TRUE;
}

BOOL CDDrawClipper::IsClipListChanged()
{
  BOOL bResult;

  m_hResult = m_lpDDClipper->IsClipListChanged(&bResult);
  DIRECTDRAW_ERROR(m_hResult);

  return bResult;
}

void CDDrawClipper::SetHWnd(CWnd *pWnd)
{
  ASSERT_VALID(this);
  ASSERT(pWnd != NULL);
  
  m_hResult = m_lpDDClipper->SetHWnd(0, pWnd->GetSafeHwnd());
  DIRECTDRAW_ERROR(m_hResult);
}

BOOL CDDrawClipper::Create(CWnd *pWnd)
{
  if(!Create()) return FALSE;

  SetHWnd(pWnd);

  return TRUE;
}
