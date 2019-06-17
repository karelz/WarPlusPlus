// MapexToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexToolBar.h"

#include "..\DataObjects\EMap.h"
#include "..\MapEditorDoc.h"
#include "..\Dialogs\SelectMapexDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_MAPEXBUTTONS 20100

/////////////////////////////////////////////////////////////////////////////
// CMapexToolBar

CMapexToolBar::CMapexToolBar()
{
}

CMapexToolBar::~CMapexToolBar()
{
}


BEGIN_MESSAGE_MAP(CMapexToolBar, CToolBar)
	//{{AFX_MSG_MAP(CMapexToolBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
  ON_COMMAND_RANGE(ID_MAPEXBUTTONS, ID_MAPEXBUTTONS + 19, OnMapexButton)
  ON_NOTIFY_RANGE(CImageTBButton::N_DOUBLECLICKED, ID_MAPEXBUTTONS, ID_MAPEXBUTTONS + 19, OnMapexButtonDblClk)
END_MESSAGE_MAP()

DWORD CMapexToolBar::m_aMapexesSelected[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int CMapexToolBar::m_nMapexSelected = 0;

/////////////////////////////////////////////////////////////////////////////
// CMapexToolBar message handlers

int CMapexToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CMapexToolBar::OnDestroy() 
{
	CToolBar::OnDestroy();
}

void CMapexToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CToolBar::OnSize(nType, cx, cy);
	
  int i;
  CRect rcItem;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    GetItemRect(i, &rcItem);
    m_aButtons[i]->SetWindowPos(NULL, rcItem.left, rcItem.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }
}

void CMapexToolBar::CreateButtons()
{
  int i;
  CRect rcItem;
  DeleteButtons();
  m_aButtons.SetSize(20);
  CMapexTBButton *pButton;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    GetItemRect(i, &rcItem);
    pButton = new CMapexTBButton();
    pButton->Create(rcItem, this, ID_MAPEXBUTTONS + i);
    m_aButtons[i] = pButton;
  }
}

void CMapexToolBar::DeleteButtons()
{
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(m_aButtons[i]->GetMapex() != NULL)
      m_aMapexesSelected[i] = m_aButtons[i]->GetMapex()->GetID();
    else
      m_aMapexesSelected[i] = 0;
    if(m_aButtons[i]->GetSelected())
      m_nMapexSelected = i;

    m_aButtons[i]->DestroyWindow();
    delete m_aButtons[i];
    m_aButtons[i] = NULL;
  }
  m_aButtons.RemoveAll();
}

void CMapexToolBar::SaveState(CString strSection)
{
  CString strVariable;
  CWinApp *pApp = AfxGetApp();

  int i;
  for(i = 0; i < 20; i++){
    strVariable.Format("Mapex%d", i);
    pApp->WriteProfileInt(strSection, strVariable, m_aMapexesSelected[i]);
  }
  pApp->WriteProfileInt(strSection, "SelectedMapex", m_nMapexSelected);
}

void CMapexToolBar::LoadState(CString strSection)
{
  CString strVariable;
  CWinApp *pApp = AfxGetApp();

  int i;
  for(i = 0; i < 20; i++){
    strVariable.Format("Mapex%d", i);
    m_aMapexesSelected[i] = pApp->GetProfileInt(strSection, strVariable, 0);
  }
  m_nMapexSelected = pApp->GetProfileInt(strSection, "SelectedMapex", 0);
}

void CMapexToolBar::SetSelectedMapexes(CEMap *pMap)
{
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(pMap == NULL)
      m_aButtons[i]->SetMapex(NULL);
    else
      m_aButtons[i]->SetMapex(pMap->GetMapex(m_aMapexesSelected[i]));
  }
  if(m_aButtons.GetSize() > 0){
    m_aButtons[m_nMapexSelected]->SetSelected(TRUE);
    AppGetActiveDocument()->SetSelectedMapex(m_aButtons[m_nMapexSelected]->GetMapex());
  }
  Invalidate();
}

void CMapexToolBar::OnMapexButton(UINT nID)
{
  int nIndex = nID - ID_MAPEXBUTTONS;
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(i == nIndex)
      m_aButtons[i]->SetSelected(TRUE);
    else
      m_aButtons[i]->SetSelected(FALSE);
  }
  CEMapex *pMapex;
  pMapex = m_aButtons[nIndex]->GetMapex();
  AppGetActiveDocument()->SetSelectedMapex(pMapex);
}

void CMapexToolBar::OnMapexButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult)
{
  int nIndex = nID - ID_MAPEXBUTTONS;

  CSelectMapexDlg dlg;
  dlg.Create(AppGetActiveMap());

  if(dlg.DoModal() == IDOK){
    m_aButtons[nIndex]->SetMapex(dlg.GetSelected());
    AppGetActiveDocument()->SetSelectedMapex(dlg.GetSelected());
    if(dlg.GetSelected() != NULL)
      m_aMapexesSelected[nIndex] = dlg.GetSelected()->GetID();
    else
      m_aMapexesSelected[nIndex] = 0;
  }

  *pResult = 0;
}

void CMapexToolBar::ResetSelection()
{
  int i;

  // deselect all buttons
  for(i = 0; i < m_aButtons.GetSize(); i++){
    m_aButtons[i]->SetSelected(FALSE);
  }
}
