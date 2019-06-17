// ResourceControlInner.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ResourceControlInner.h"

#include "Common\Map\Map.h"
#include "..\DataObjects\EMap.h"

#include "..\Dialogs\EditResourcesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceControlInner

CResourceControlInner::CResourceControlInner()
{
  m_pResources = NULL;
}

CResourceControlInner::~CResourceControlInner()
{
}


BEGIN_MESSAGE_MAP(CResourceControlInner, CStatic)
	//{{AFX_MSG_MAP(CResourceControlInner)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceControlInner message handlers

void CResourceControlInner::ResetText()
{
  CEMap *pMap = AppGetActiveMap();
  if(pMap == NULL){
    SetWindowText("");
    return;
  }
  if(m_pResources == NULL){
    SetWindowText("");
  }
  else{
    BOOL bOne = TRUE;
    DWORD dwMax = 0, dwMaxIndex = 0;
    DWORD i;
    for(i = 0; i < RESOURCE_COUNT; i++){
      if((dwMax != NULL) && (m_pResources[i] != NULL)) bOne = FALSE;
      if(dwMax < (DWORD)(abs(m_pResources[i]))){
        dwMax = abs(m_pResources[i]);
        dwMaxIndex = i;
      }
    }

    CString str;
    str.Format("%d %s ", m_pResources[dwMaxIndex], pMap->GetResource(dwMaxIndex)->GetName());
    if(!bOne) str += "(atd...)";

    if(dwMax == 0) str = "(žádné)";
    SetWindowText(str);
  }
}
void CResourceControlInner::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CEditResourcesDlg dlg;
  int aResources[RESOURCE_COUNT];

  memcpy(aResources, m_pResources, RESOURCE_COUNT * sizeof(int));

  dlg.m_pResources = aResources;
  dlg.m_strTitle = m_strTitle;
  if(dlg.DoModal() == IDOK){
    memcpy(m_pResources, aResources, RESOURCE_COUNT * sizeof(int));
    ResetText();
  }
  
	CStatic::OnLButtonDown(nFlags, point);
}
