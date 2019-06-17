// EUnitType.cpp: implementation of the CEUnitType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitType.h"

#include "..\MapFormats.h"

#include "EUnitAppearance.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitType, CObject)

CEUnitType::CEUnitType()
{
  // set some defaults
  m_dwLifeMax = 10;
  m_dwViewRadius = 30;
  m_dwFlags = UnitTypeFlags_Selectable;
  m_dwZPos = 35000;

  int i;
  CString str;
  for(i = 0; i < 8; i ++){
    str.Format("Mód %d", i+1);
    m_aModeNames[i] = str;
    m_aPathGraphs[i] = 0;
    m_aMoveWidth[i] = 2;
    m_aDefaultAppearances[i] = 0;
    m_aInvisibilityFlags[i] = 0;
    m_aDetectionFlags[i] = 0;
  }

  m_pSelectionMarkAppearance = NULL;
}

CEUnitType::~CEUnitType()
{
}

#ifdef _DEBUG

void CEUnitType::AssertValid() const
{
  CObject::AssertValid();
}

void CEUnitType::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


void CEUnitType::Delete()
{
  POSITION pos;
  CEUnitAppearanceType *pAppearance;

  // delete appearances
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppearance = m_listAppearances.GetNext(pos);
    pAppearance->Delete();
    delete pAppearance;
  }
  m_listAppearances.RemoveAll();

  if(m_pSelectionMarkAppearance != NULL){
    m_pSelectionMarkAppearance->Delete();
    delete m_pSelectionMarkAppearance;
    m_pSelectionMarkAppearance = NULL;
  }

  CEUnitSkillType *pSkillType;
  pos = m_listSkillTypes.GetHeadPosition();
  while(pos != NULL){
    pSkillType = m_listSkillTypes.GetNext(pos);
    pSkillType->Delete();
    delete pSkillType;
  }
  m_listSkillTypes.RemoveAll();
}

BOOL CEUnitType::Create(DWORD dwID)
{
  m_dwID = dwID;

  // Create empty mark appearance
  m_pSelectionMarkAppearance = new CEUnitAppearanceType();
  m_pSelectionMarkAppearance->Create(0, this);
  m_pSelectionMarkAppearance->SetName("Selection mark");
  m_pSelectionMarkAppearance->SetArchive(m_Archive);

  return TRUE;
}

BOOL CEUnitType::Create(CArchiveFile file)
{
  if(g_dwUnitTypeFileVersion < MAP_FILE_VERSION(1, 0)){
    SUnitTypeHeader_Old h;

    file.Read(&h, sizeof(h));

    m_strUnitFileName = file.GetFilePath();

    m_dwID = h.m_dwID;
    m_strName = h.m_pName;
    m_dwLifeMax = h.m_dwLifeMax;
    m_dwViewRadius = h.m_dwViewRadius;
    m_dwFlags = h.m_dwFlags;
    m_dwZPos = h.m_dwZPos;

    DWORD i;
    for(i = 0; i < 8; i++){
      m_aMoveWidth[i] = h.m_aMoveWidth[i];
      m_aModeNames[i] = h.m_aModeNames[i];
      m_aDefaultAppearances[i] = h.m_aDefaultAppearances[i];
      m_aInvisibilityFlags[i] = h.m_aInvisibilityFlags[i];
      m_aDetectionFlags[i] = h.m_aDetectionFlags[i];

      // copy the bit data
      m_aPathGraphs[i] = 0;
    }

    // read all appearances (create and load from file)
    CEUnitAppearanceType *pAppearance;
    for(i = 0; i < h.m_dwAppearancesNumber; i++){
      pAppearance = new CEUnitAppearanceType();
      pAppearance->Create(file, this);
      pAppearance->SetArchive(file.GetDataArchive());
      if(pAppearance->GetID() == 0){
        m_pSelectionMarkAppearance = pAppearance;
      }
      else{
        m_listAppearances.AddHead(pAppearance);
      }
    }

    if(m_pSelectionMarkAppearance == NULL){
      m_pSelectionMarkAppearance = new CEUnitAppearanceType();
      m_pSelectionMarkAppearance->Create(0, this);
      m_pSelectionMarkAppearance->SetName("Selection mark");
      m_pSelectionMarkAppearance->SetArchive(m_Archive);
    }

    // read all skill types
    CEUnitSkillType *pSkillType;
    for(i = 0; i < h.m_dwSkillTypesCount; i++){
      pSkillType = new CEUnitSkillType();
      pSkillType->Create(file, this);
      m_listSkillTypes.AddTail(pSkillType);
    }
  }
  else if ( g_dwUnitTypeFileVersion <= MAP_FILE_VERSION (1, 3))
  {
    SUnitTypeHeader1p3 h;

    file.Read(&h, sizeof(h));

    m_strUnitFileName = file.GetFilePath();

    m_dwID = h.m_dwID;
    m_strName = h.m_pName;
    m_dwLifeMax = h.m_dwLifeMax;
    m_dwViewRadius = h.m_dwViewRadius;
    m_dwFlags = h.m_dwFlags;
    m_dwZPos = h.m_dwZPos;

    DWORD i;
    for(i = 0; i < 8; i++){
      m_aMoveWidth[i] = h.m_aMoveWidth[i];
      m_aModeNames[i] = h.m_aModeNames[i];
      m_aDefaultAppearances[i] = h.m_aDefaultAppearances[i];
      m_aInvisibilityFlags[i] = h.m_aInvisibilityFlags[i];
      m_aDetectionFlags[i] = h.m_aDetectionFlags[i];

      // copy the bit data
      m_aPathGraphs[i] = 0;
    }

    // read all appearances (create and load from file)
    CEUnitAppearanceType *pAppearance;
    for(i = 0; i < h.m_dwAppearancesNumber; i++){
      pAppearance = new CEUnitAppearanceType();
      pAppearance->Create(file, this);
      pAppearance->SetArchive(file.GetDataArchive());
      if(pAppearance->GetID() == 0){
        m_pSelectionMarkAppearance = pAppearance;
      }
      else{
        m_listAppearances.AddHead(pAppearance);
      }
    }

    if(m_pSelectionMarkAppearance == NULL){
      m_pSelectionMarkAppearance = new CEUnitAppearanceType();
      m_pSelectionMarkAppearance->Create(0, this);
      m_pSelectionMarkAppearance->SetName("Selection mark");
      m_pSelectionMarkAppearance->SetArchive(m_Archive);
    }

    // read all skill types
    CEUnitSkillType *pSkillType;
    for(i = 0; i < h.m_dwSkillTypesCount; i++){
      pSkillType = new CEUnitSkillType();
      pSkillType->Create(file, this);
      m_listSkillTypes.AddTail(pSkillType);
    }
  }
  else
  {
    SUnitTypeHeader h;

    file.Read(&h, sizeof(h));

    m_strUnitFileName = file.GetFilePath();

    m_dwID = h.m_dwID;
    m_strName = h.m_pName;
    m_dwLifeMax = h.m_dwLifeMax;
    m_dwViewRadius = h.m_dwViewRadius;
    m_dwFlags = h.m_dwFlags;
    m_dwZPos = h.m_dwZPos;

    DWORD i;
    for(i = 0; i < 8; i++){
      m_aMoveWidth[i] = h.m_aMoveWidth[i];
      m_aModeNames[i] = h.m_aModeNames[i];
      m_aDefaultAppearances[i] = h.m_aDefaultAppearances[i];
      m_aInvisibilityFlags[i] = h.m_aInvisibilityFlags[i];
      m_aDetectionFlags[i] = h.m_aDetectionFlags[i];
      m_aPathGraphs[i] = h.m_aPathGraphs[i];
    }

    // read all appearances (create and load from file)
    CEUnitAppearanceType *pAppearance;
    for(i = 0; i < h.m_dwAppearancesNumber; i++){
      pAppearance = new CEUnitAppearanceType();
      pAppearance->Create(file, this);
      pAppearance->SetArchive(file.GetDataArchive());
      if(pAppearance->GetID() == 0){
        m_pSelectionMarkAppearance = pAppearance;
      }
      else{
        m_listAppearances.AddHead(pAppearance);
      }
    }

    if(m_pSelectionMarkAppearance == NULL){
      m_pSelectionMarkAppearance = new CEUnitAppearanceType();
      m_pSelectionMarkAppearance->Create(0, this);
      m_pSelectionMarkAppearance->SetName("Selection mark");
      m_pSelectionMarkAppearance->SetArchive(m_Archive);
    }

    // read all skill types
    CEUnitSkillType *pSkillType;
    for(i = 0; i < h.m_dwSkillTypesCount; i++){
      pSkillType = new CEUnitSkillType();
      pSkillType->Create(file, this);
      m_listSkillTypes.AddTail(pSkillType);
    }
  }

  return TRUE;
}

void CEUnitType::SaveUnit(CDataArchive Archive)
{
  m_strUnitFileName.Format("Unit%010d.header", m_dwID);

  CArchiveFile UnitFile;
  UnitFile = Archive.CreateFile(m_strUnitFileName, CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);

  SUnitTypeHeader h;

  memset(&h, 0, sizeof(h));
  h.m_dwID = m_dwID;
  strncpy(h.m_pName, m_strName, 30);
  h.m_dwLifeMax = m_dwLifeMax;
  h.m_dwViewRadius = m_dwViewRadius;
  h.m_dwAppearancesNumber = m_listAppearances.GetCount() + 1; // 1 - selection mark
  h.m_dwSkillTypesCount = m_listSkillTypes.GetCount();
  h.m_dwFlags = m_dwFlags;
  h.m_dwZPos = m_dwZPos;

  DWORD i;
  for(i = 0; i < 8; i++){
    h.m_aMoveWidth[i] = m_aMoveWidth[i];
    strncpy(h.m_aModeNames[i], m_aModeNames[i], 30);
    h.m_aDefaultAppearances[i] = m_aDefaultAppearances[i];
    h.m_aInvisibilityFlags[i] = m_aInvisibilityFlags[i];
    h.m_aDetectionFlags[i] = m_aDetectionFlags[i];
    h.m_aPathGraphs[i] = m_aPathGraphs[i];
  }

  UnitFile.Write(&h, sizeof(h));

  // first save the selection appearance
  ASSERT(m_pSelectionMarkAppearance != NULL);
  m_pSelectionMarkAppearance->SaveToFile(UnitFile);

  // save all appearances to this file
  POSITION pos;
  CEUnitAppearanceType *pAppearance;
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppearance = m_listAppearances.GetNext(pos);
    pAppearance->SaveToFile(UnitFile);
  }

  CEUnitSkillType *pSkillType;
  pos = m_listSkillTypes.GetHeadPosition();
  while(pos != NULL){
    pSkillType = m_listSkillTypes.GetNext(pos);
    pSkillType->SaveToFile(UnitFile);
  }

  UnitFile.Close();
}

CEUnitAppearanceType * CEUnitType::GetDefaultAppearanceType()
{
  if(m_listAppearances.GetCount() == 0) return NULL;
  // go through all modes and try to find some default app set
  DWORD dwMode;
  CEUnitAppearanceType *pApp;
  for(dwMode = 0; dwMode < 8; dwMode++){
    pApp = GetAppearanceType(GetDefaultAppearance(dwMode));
    if(pApp != NULL) return pApp;
  }
  pApp = m_listAppearances.GetHead();
  return pApp;
}

void CEUnitType::SetArchive(CDataArchive Archive)
{
  m_Archive = Archive;
}

void CEUnitType::RemoveGraphics(CDataArchive Archive)
{
  POSITION pos;

  CEUnitAppearance *pApp;
  CEUnitAppearanceType *pAppType;

  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppType = m_listAppearances.GetNext(pos);

    pApp = new CEUnitAppearance();
    pApp->Create(pAppType, 0, Archive);

    pApp->RemoveGraphics(Archive);
    pApp->Delete();
    delete pApp;
  }

  pApp = new CEUnitAppearance();
  pApp->Create(m_pSelectionMarkAppearance, 0, Archive);
  pApp->RemoveGraphics(Archive);
  pApp->Delete();
  delete pApp;
}

void CEUnitType::LoadGraphics()
{
  // load all appearances, it means create instances of them
  POSITION pos;
  CEUnitAppearanceType *pAppType;
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppType = m_listAppearances.GetNext(pos);
    pAppType->CreateInstance();
  }

  m_pSelectionMarkAppearance->CreateInstance();
}

void CEUnitType::ReleaseGraphics()
{
  // free the appearance previosly loaded by the LoadGraphics
  POSITION pos;
  CEUnitAppearanceType *pAppType;
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppType = m_listAppearances.GetNext(pos);
    pAppType->ClearCache();
  }

  m_pSelectionMarkAppearance->ClearCache();
}

void CEUnitType::FillAppearancesListCtrl(CListCtrl *pListCtrl, BOOL bHidden)
{
  int i, nItem;
  CEUnitAppearanceType *pAppType;

  pListCtrl->DeleteAllItems();
  POSITION pos;
  i = 0;
  
  if(bHidden){
    // Add the selection mark
    nItem = pListCtrl->InsertItem(i++, m_pSelectionMarkAppearance->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)m_pSelectionMarkAppearance);
  }

  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppType = m_listAppearances.GetNext(pos);
    nItem = pListCtrl->InsertItem(i, pAppType->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pAppType);
    i++;
  }
}

void CEUnitType::FillAppearancesCombo(CComboBox *pComboBox, BOOL bHidden)
{
  int nItem;
  CEUnitAppearanceType *pAppType;

  if(bHidden){
    nItem = pComboBox->AddString(m_pSelectionMarkAppearance->GetName());
    pComboBox->SetItemData(nItem, (DWORD)m_pSelectionMarkAppearance);
  }

  pComboBox->ResetContent();
  POSITION pos;
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pAppType = m_listAppearances.GetNext(pos);
    nItem = pComboBox->AddString(pAppType->GetName());
    pComboBox->SetItemData(nItem, (DWORD)pAppType);
  }
}

CEUnitAppearanceType * CEUnitType::NewAppearance()
{
  CEUnitAppearanceType *pAppType;
  pAppType = new CEUnitAppearanceType();
  pAppType->Create(GetNewAppearanceID(), this);
  pAppType->SetArchive(m_Archive);
  m_listAppearances.AddHead(pAppType);

  return pAppType;
}

DWORD CEUnitType::GetNewAppearanceID()
{
  CTime time;
  time = CTime::GetCurrentTime();

  return (DWORD)time.GetTime();
}

void CEUnitType::DeleteAppearance(CEUnitAppearanceType *pAppType)
{
  POSITION pos;
  // remnove it from the list
  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    if(m_listAppearances.GetAt(pos) == pAppType){
      m_listAppearances.RemoveAt(pos);
      break;
    }
    m_listAppearances.GetNext(pos);
  }

  // remove the graphics
  CEUnitAppearance *pApp;
  pApp = new CEUnitAppearance();
  pApp->Create(pAppType, 0, m_Archive);
  pApp->RemoveGraphics(m_Archive);
  pApp->Delete();
  delete pApp;

  // delete it
  pAppType->Delete();
  delete pAppType;
}

void CEUnitType::FillModesCombo(CComboBox *pComboBox)
{
  DWORD i;
  int nItem;
  pComboBox->ResetContent();

  for(i = 0; i < 8; i++){
    nItem = pComboBox->AddString(m_aModeNames[i]);
    pComboBox->SetItemData(nItem, i);
  }
}

void CEUnitType::FillModesListCtrl(CListCtrl *pListCtrl)
{
  int i, nItem;

  pListCtrl->DeleteAllItems();
  i = 0;
  for(i = 0; i < 8; i++){
    nItem = pListCtrl->InsertItem(i, m_aModeNames[i]);
    pListCtrl->SetItemData(nItem, (DWORD)i);
  }
}

CEUnitSkillType * CEUnitType::AddSkillType(CString strName)
{
  CEUnitSkillType *pSkillType;

  pSkillType = new CEUnitSkillType();
  pSkillType->Create(strName, this);
  m_listSkillTypes.AddTail(pSkillType);
  return pSkillType;
}

void CEUnitType::DeleteSkillType(CEUnitSkillType *pSkillType)
{
  POSITION pos;

  // first remove it from the list
  pos = m_listSkillTypes.GetHeadPosition();
  while(pos != NULL){
    if(m_listSkillTypes.GetAt(pos) == pSkillType){
      m_listSkillTypes.RemoveAt(pos);
      break;
    }
    m_listSkillTypes.GetNext(pos);
  }

  // delete it
  pSkillType->Delete();
  delete pSkillType;
}

CEUnitAppearanceType * CEUnitType::GetAppearanceType(DWORD dwID)
{
  POSITION pos;
  CEUnitAppearanceType *pType;

  pos = m_listAppearances.GetHeadPosition();
  while(pos != NULL){
    pType = m_listAppearances.GetNext(pos);
    if(pType->GetID() == dwID) return pType;
  }

  if(dwID == m_pSelectionMarkAppearance->GetID()) return m_pSelectionMarkAppearance;

  return NULL;
}

void CEUnitType::MoveSkillTypeUp(CEUnitSkillType *pSkillType)
{
  POSITION pos, pos2;
  pos = m_listSkillTypes.Find(pSkillType);
  pos2 = pos;
  m_listSkillTypes.GetPrev(pos);
  if(pos == NULL) return;
  m_listSkillTypes.RemoveAt(pos2);
  m_listSkillTypes.InsertBefore(pos, pSkillType);
}

void CEUnitType::MoveSkillTypeDown(CEUnitSkillType *pSkillType)
{
  POSITION pos, pos2;
  pos = m_listSkillTypes.Find(pSkillType);
  pos2 = pos;
  m_listSkillTypes.GetNext(pos);
  if(pos == NULL) return;
  m_listSkillTypes.RemoveAt(pos2);
  m_listSkillTypes.InsertAfter(pos, pSkillType);
}

CEUnitSkillType *CEUnitType::GetPrevSkillType(CEUnitSkillType *pSkillType)
{
  POSITION pos;
  pos = m_listSkillTypes.Find(pSkillType);
  m_listSkillTypes.GetPrev(pos);
  if(pos == NULL) return NULL;
  return m_listSkillTypes.GetAt(pos);
}

CEUnitSkillType *CEUnitType::GetNextSkillType(CEUnitSkillType *pSkillType)
{
  POSITION pos;
  pos = m_listSkillTypes.Find(pSkillType);
  m_listSkillTypes.GetNext(pos);
  if(pos == NULL) return NULL;
  return m_listSkillTypes.GetAt(pos);
}

BOOL CEUnitType::CheckValid()
{
  if(m_strName.IsEmpty()){
    AfxMessageBox("Jednotka nemùže mít prázdné jméno.");
    return FALSE;
  }

  if ( m_dwLifeMax == 0 )
  {
    CString str;
	str.Format ( "Jednotka '%s' nemùže mít nula životù.", GetName () );
	return FALSE;
  }

  {
    int nPos, nLen = m_strName.GetLength();
    for(nPos = 0; nPos < nLen; nPos++){
      if((m_strName[nPos] < 32) || (m_strName[nPos] >= 128)){
        AfxMessageBox("Jméno typu jednotky nesmí obsahovat diakritiku ani kontrolní znaky.\nDoporuèujeme použít anglická jména");
        return FALSE;
      }
    }
  }

  if(m_dwZPos == 0){
    CString str;
    str = "Jednotka '";
    str += m_strName;
    str += "' nemùže mít výšku 0.";
    AfxMessageBox(str);
    return FALSE;
  }

  if(m_listSkillTypes.GetCount() == 0){
    CString str;
    str = "Jednotka '";
    str += m_strName;
    str += "' musí mít alespoò jednu dovednost.";
    AfxMessageBox(str);
    return FALSE;
  }

  // go through all appearances and ask them
  {
    POSITION pos;
    CEUnitAppearanceType *pApp;
    pos = m_listAppearances.GetHeadPosition();
    while(pos != NULL){
      pApp = m_listAppearances.GetNext(pos);
      if(!pApp->CheckValid()) return FALSE;
    }
    if(!m_pSelectionMarkAppearance->CheckValid()) return FALSE;
  }

  if(m_aDefaultAppearances[0] == 0){
    CString str;
    str = "Výchozí vzhled jednoky " + m_strName + "v prvním módu musí být zadaný.";
    AfxMessageBox(str);
    return FALSE;
  }

  // go through all skills and ask them
  {
    POSITION pos;
    CEUnitSkillType *pSkill;
    pos = m_listSkillTypes.GetHeadPosition();
    while(pos != NULL){
      pSkill = m_listSkillTypes.GetNext(pos);
      if(!pSkill->CheckValid()) return FALSE;
    }
  }

  return TRUE;
}