// UnitSkillTypeCollection.cpp: implementation of the CUnitSkillTypeCollection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitSkillTypeCollection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "..\SkillTypes\ExampleSkillTypeDlg.h"
#include "..\SkillTypes\EMoveSkillTypeDlg.h"
#include "..\SkillTypes\EBulletAttackSkillTypeDlg.h"
#include "..\SkillTypes\EBulletDefenseSkillTypeDlg.h"
#include "..\SkillTypes\EMakeSkillTypeDlg.h"
#include "..\SkillTypes\EMineSkillTypeDlg.h"
#include "..\SkillTypes\EResourceSkillTypeDlg.h"
#include "..\SkillTypes\EStorageSkillTypeDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitSkillTypeCollection::CUnitSkillTypeCollection()
{

}

CUnitSkillTypeCollection::~CUnitSkillTypeCollection()
{

}

void CUnitSkillTypeCollection::Create()
{
  // here create new instance of your skill type dlg
  CExampleSkillTypeDlg *pExmapleSkillTypeDlg;
  pExmapleSkillTypeDlg = new CExampleSkillTypeDlg();
  // call the create method on it
  pExmapleSkillTypeDlg->Create();
  // add it to the list of skill types
  m_listSkillTypeDlgs.AddHead(pExmapleSkillTypeDlg);

  CEMoveSkillTypeDlg *pEMoveSkillTypeDlg;
  pEMoveSkillTypeDlg = new CEMoveSkillTypeDlg();
  pEMoveSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEMoveSkillTypeDlg);

  CEBulletAttackSkillTypeDlg *pEBulletAttackSkillTypeDlg;
  pEBulletAttackSkillTypeDlg = new CEBulletAttackSkillTypeDlg();
  pEBulletAttackSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEBulletAttackSkillTypeDlg);

  CEBulletDefenseSkillTypeDlg *pEBulletDefenseSkillTypeDlg;
  pEBulletDefenseSkillTypeDlg = new CEBulletDefenseSkillTypeDlg();
  pEBulletDefenseSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEBulletDefenseSkillTypeDlg);

  CEMakeSkillTypeDlg *pEMakeSkillTypeDlg;
  pEMakeSkillTypeDlg = new CEMakeSkillTypeDlg();
  pEMakeSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEMakeSkillTypeDlg);

  CEMineSkillTypeDlg *pEMineSkillTypeDlg;
  pEMineSkillTypeDlg = new CEMineSkillTypeDlg();
  pEMineSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEMineSkillTypeDlg);

  CEResourceSkillTypeDlg *pEResourceSkillTypeDlg;
  pEResourceSkillTypeDlg = new CEResourceSkillTypeDlg();
  pEResourceSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEResourceSkillTypeDlg);

  CEStorageSkillTypeDlg *pEStorageSkillTypeDlg;
  pEStorageSkillTypeDlg = new CEStorageSkillTypeDlg();
  pEStorageSkillTypeDlg->Create();
  m_listSkillTypeDlgs.AddHead(pEStorageSkillTypeDlg);
}

void CUnitSkillTypeCollection::Delete()
{
  POSITION pos;

  CUnitSkillTypeDlg *pDlg;
  pos = m_listSkillTypeDlgs.GetHeadPosition();
  while(pos != NULL){
    pDlg = m_listSkillTypeDlgs.GetNext(pos);
    pDlg->Delete();
    delete pDlg;
  }
  m_listSkillTypeDlgs.RemoveAll();
}

void CUnitSkillTypeCollection::FillListCtrl(CListCtrl *pListCtrl)
{
  POSITION pos;
  CUnitSkillTypeDlg *pDlg;
  int nItem ,i;

  pListCtrl->DeleteAllItems();
  pos = m_listSkillTypeDlgs.GetHeadPosition();
  i = 0 ;
  while(pos != NULL){
    pDlg = m_listSkillTypeDlgs.GetNext(pos);

    nItem = pListCtrl->InsertItem(i, pDlg->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pDlg);
    i++;
  }
}

CUnitSkillTypeDlg * CUnitSkillTypeCollection::GetByName(CString strName)
{
  POSITION pos;
  CUnitSkillTypeDlg *pDlg;
  pos = m_listSkillTypeDlgs.GetHeadPosition();
  while(pos != NULL){
    pDlg = m_listSkillTypeDlgs.GetNext(pos);
    if(pDlg->GetName() == strName){
      return pDlg;
    }
  }
  return NULL;
}
