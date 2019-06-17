// EMap.cpp: implementation of the CEMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMap.h"

#include "..\MapEditorDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MINIMAP_WIDTH 200
#define MINIMAP_HEIGHT 200

CUnitSkillTypeCollection *g_pUnitSkillTypeCollection = NULL;

// Here declare the version global variables
// Version number of currently loading map file
DWORD g_dwMapFileVersion = 0;
// Version number of currently loading mapex file (library)
DWORD g_dwMapexFileVersion = 0;
// Version number of currently loading unit type file (library)
DWORD g_dwUnitTypeFileVersion = 0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEMap, CObject)

CEMap::CEMap()
{
  m_pMapSquares = NULL;
  m_bDeleted = TRUE;
  m_bModified = FALSE;

  m_bDrawGrid = FALSE;
  m_dwGridSize = 8;

  m_bDrawLandTypes = FALSE;

  m_dwSelectedLevel = 1;
  m_pULSelection = NULL;
  m_pUnitSelection = NULL;

  memset(m_aResources, 0, sizeof(CEResource *) * RESOURCE_COUNT);
  memset(m_aInvisibilities, 0, sizeof(CEInvisibility *) * INVISIBILITY_COUNT);
}

CEMap::~CEMap()
{
  Delete();
}

#ifdef _DEBUG

void CEMap::AssertValid() const
{
  CObject::AssertValid();

  if(!m_bDeleted){
    ASSERT(m_pMapSquares != NULL);
  }
}

void CEMap::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CEMap::Create(CArchiveFile MapFile, CProgressCtrl *pProgress)
{
  // First load the version header
  SFileVersionHeader sFileVersionHeader;
  
  try{

    MapFile.Open();
    
    // Load the version header
    {
      if(MapFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader)) < sizeof(sFileVersionHeader)){
        // the file is too small -> load it as the old format
        g_dwMapFileVersion = MAP_FILE_VERSION(0, 9);
      }
      else{
        // compare the ID
        {
          BYTE aFileID[16] = MAP_FILE_IDENTIFIER;
          if(memcmp(aFileID, sFileVersionHeader.m_aMapFileID, 16) != 0){
            g_dwMapFileVersion = MAP_FILE_VERSION(0, 9);
            goto FormatDone;
          }
        }
      
        // OK the ID is the same
        // Compare the version number
        if(sFileVersionHeader.m_dwFormatVersion > CURRENT_MAP_FILE_VERSION){
          // It's newer format
          // Try to load it as compatible
          if(sFileVersionHeader.m_dwCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION){
            // It all new, we're too old to load this file
            throw int(2);
          }
        
          // We can still load it
          // Just look at it as our version
          g_dwMapFileVersion = CURRENT_MAP_FILE_VERSION;
        }
        else{
          // It's older or our version -> normal load
          g_dwMapFileVersion = sFileVersionHeader.m_dwFormatVersion;
        }
      }
    }

FormatDone:
    ;

    if(g_dwMapFileVersion >= MAP_FILE_VERSION(1, 0))
    {
      // Newer format

      SMapHeader h;
      CString strLibraryPath;

      if(pProgress != NULL) pProgress->SetPos(0);

      m_bModified = FALSE;

      m_UnitSkillTypeCollection.Create();
      g_pUnitSkillTypeCollection = &m_UnitSkillTypeCollection;
      CreateSpecialBuffers();
      CreateMiniMap();

      if(MapFile.Read(&h, sizeof(h)) != sizeof(h)) throw int(1);

      m_dwWidth = h.m_dwWidth;  m_dwSquareWidth = m_dwWidth / MAPSQUARE_WIDTH;
      m_dwHeight = h.m_dwHeight; m_dwSquareHeight = m_dwHeight / MAPSQUARE_HEIGHT;
      m_strName = h.m_pName;
      m_strDescription = h.m_pDescription;
      m_strLandTypesFile = h.m_pLandTypesFile;

      if(!m_strLandTypesFile.IsEmpty()){
        CArchiveFile file;
        BOOL bFound = FALSE;
        CString strFileName = m_strLandTypesFile;

    LoadLandTypes:;
        try{
          file = CDataArchive::GetRootArchive()->CreateFile(strFileName);
          LoadLandTypes(file);
          bFound = TRUE;
        }
        catch(CException *e){
          e->Delete();
        }

        if(!bFound){
          CString str;
          str.Format("Nebyl nalezen soubor definující typy povrchù '%s'.\nPokusíte se ho nalézt sami ?", m_strLandTypesFile);
          if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
            throw (int)1;

          CFileDialog dlg(TRUE, "*.landtypes", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
            "Typy povrchù (*.landtypes)|*.landtypes|Všechny soubory|*.*||", AfxGetMainWnd());
          str.Format("Najít %s", m_strLandTypesFile);
          dlg.m_ofn.lpstrTitle = str;

          if(dlg.DoModal() != IDOK) goto AfterLandTypes;

          {
            CString strPath = dlg.GetPathName();
            int nPos = strPath.ReverseFind('\\');
            if(nPos == -1) strLibraryPath.Empty();
            else strLibraryPath = strPath.Left(nPos + 1);
          }

          strFileName = dlg.GetPathName();

          goto LoadLandTypes;
        }
      }
      else{
        CLandType *pLandType;
        pLandType = new CLandType;
        pLandType->Create(0, "(transparentní)", RGB(255, 255, 255));
        m_listLandTypes.AddHead(pLandType);
      }
    AfterLandTypes:
      if(pProgress != NULL) pProgress->SetPos(2);

      // Load script sets
      if(g_dwMapFileVersion >= MAP_FILE_VERSION(1, 1)){
        CEScriptSet *pScriptSet;
        int i;
        for(i = 0; i < (int)h.m_dwScriptSetCount; i++){
          pScriptSet = new CEScriptSet();
          try{
            pScriptSet->Create(MapFile);
          }
          catch(CException *e){
            e->Delete();
            // If some error just dont't add it
            delete pScriptSet;
            continue;
          }
          m_listScriptSet.AddTail(pScriptSet);
        }
      }

      {
        int i;
        SMapexLibraryNode node;
        CEMapexLibrary *pMapexLibrary;
        CDataArchive LibraryArchive;
        for(i = 0; i < (int)h.m_dwMapexLibraryCount; i++){
          MapFile.Read(&node, sizeof(node));
          BOOL bFound = FALSE;
          CString strFileName = node.m_pFullName;

          try{
            LibraryArchive.Create(node.m_pFullName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
            bFound = TRUE;
          }
          catch(CException *e){
            e->Delete();
          }
      
          if(!bFound){
            strFileName = strLibraryPath + node.m_pFileName;
    LoadMapexLibrary:;
            try{
              LibraryArchive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
              bFound = TRUE;
            }
            catch(CException *e){
              e->Delete();
            }
        
            if(!bFound){
              CString str;
              str.Format("Nebyla nalezena knihovna mapexù '%s'.\nPokusíte se ji nalézt sami ?", node.m_pFileName);
        
              if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
                throw (int)1;
        
              CFileDialog dlg(TRUE, "*.mlb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                "Knihovny mapexù (*.mlb)|*.mlb|Všechny soubory|*.*||", AfxGetMainWnd());
              str.Format("Najít %s", node.m_pFileName);
              dlg.m_ofn.lpstrTitle = str;
              dlg.m_ofn.lpstrInitialDir = strLibraryPath;
              if(dlg.DoModal() != IDOK) continue;
        
              CString strLibFile = dlg.GetPathName();

              {
                int nPos = strLibFile.ReverseFind('\\');
                strLibraryPath = strLibFile.Left(nPos + 1);
              }
        
              m_bModified = TRUE;
              strFileName = strLibFile;

              goto LoadMapexLibrary;
            }
          }
      
          pMapexLibrary = new CEMapexLibrary;
          pMapexLibrary->Create(LibraryArchive, FALSE, this);
      
          m_listLibraries.AddHead(pMapexLibrary);
      
          if(pProgress != NULL) pProgress->SetPos(2 + 50 * (i + 1) / h.m_dwMapexLibraryCount);
        }
      }

      // here load civilizations
      {
        int i;
        CECivilization *pCivilization;
        for(i = 0; i < (int)h.m_dwCivilizationsCount; i++){
          pCivilization = new CECivilization();

          pCivilization->Create(MapFile, this);
          m_listCivilizations.AddTail(pCivilization);
        }

        if((m_listCivilizations.GetHeadPosition() == NULL) || (m_listCivilizations.GetHead()->GetID() != 0)){
          // no system civilization -> create one
          CECivilization *pSystemCiv;
          pSystemCiv = new CECivilization();
          pSystemCiv->Create(0, "[Systémová civilizace]", RGB32(200, 200, 200));
          m_listCivilizations.AddHead(pSystemCiv);
        }
      }
      if(pProgress != NULL) pProgress->SetPos(55);

      // now load unit libraries
      {
        int i;
        SUnitTypeLibraryNode node;
        CEUnitLibrary *pUnitLibrary;
        CDataArchive LibraryArchive;
        for(i = 0; i < (int)h.m_dwUnitTypeLibraryCount; i++){
          MapFile.Read(&node, sizeof(node));
          BOOL bFound = FALSE;
          CString strFileName = node.m_pFullName;

          try{
            LibraryArchive.Create(node.m_pFullName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
            bFound = TRUE;
          }
          catch(CException *e){
            e->Delete();
          }

          if(!bFound){
            strFileName = strLibraryPath + node.m_pFileName;
    LoadUnitTypeLibrary:;
            try{
              LibraryArchive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
              bFound = TRUE;
            }
            catch(CException *e){
              e->Delete();
            }
      
            if(!bFound){
              CString str;
              str.Format("Nebyla nalezena knihovna jednotek '%s'.\nPokusíte se ji nalézt sami ?", node.m_pFileName);
      
              if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
                throw (int)1;
      
              CFileDialog dlg(TRUE, "*.ulb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                "Knihovny jednotek (*.ulb)|*.ulb|Všechny soubory|*.*||", AfxGetMainWnd());
              str.Format("Najít %s", node.m_pFileName);
              dlg.m_ofn.lpstrTitle = str;
              dlg.m_ofn.lpstrInitialDir = strLibraryPath;
              if(dlg.DoModal() != IDOK) continue;
      
              CString strLibFile = dlg.GetPathName();

              {
                int nPos = strLibFile.ReverseFind('\\');
                strLibraryPath = strLibFile.Left(nPos + 1);
              }
      
              m_bModified = TRUE;
              strFileName = strLibFile;

              goto LoadUnitTypeLibrary;
            }
          }
      
          pUnitLibrary = new CEUnitLibrary;
          pUnitLibrary->Create(LibraryArchive, FALSE, this);
      
          m_listUnitLibraries.AddHead(pUnitLibrary);
  
          if(pProgress != NULL) pProgress->SetPos(55 + (40 * (i + 1) / h.m_dwUnitTypeLibraryCount));
        }
      }

      // now skip the map square offsets
      MapFile.Seek(sizeof(DWORD) * m_dwSquareWidth * m_dwSquareHeight, CFile::current);

      // now load map squares
      m_pMapSquares = new LPMapSquare[m_dwSquareWidth * m_dwSquareHeight];

      DWORD dwX, dwY;
      CEMapSquare *pSquare;
      for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
        for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
          pSquare = new CEMapSquare();
          pSquare->Create(MapFile);
          m_pMapSquares[dwX + dwY * m_dwSquareWidth] = pSquare;
        }
      }

      if(pProgress != NULL) pProgress->SetPos(97);

      // now load unit instances
      {
        CEUnit *pUnit;

        DWORD i;

        for(i = 0; i < h.m_dwUnitCount; i++){
          pUnit = new CEUnit();
          pUnit->Create(MapFile);
          InsertUnit(pUnit);
        }
      }

      // Load resources
      if(pProgress != NULL) pProgress->SetPos(99);
      {
        CEResource *pResource;

        int i;

        for(i = 0; i < RESOURCE_COUNT; i++){
          pResource = new CEResource();
          pResource->Create(i, MapFile);
          m_aResources[i] = pResource;
        }
      }

      // Load Visibilities
      {
        CEInvisibility *pInvisibility;
        int i;
        for(i = 0; i < INVISIBILITY_COUNT; i++){
          pInvisibility = new CEInvisibility();
          pInvisibility->Create(i, MapFile);
          m_aInvisibilities[i] = pInvisibility;
        }
      }

      // Load find path graphs
      if(g_dwMapFileVersion >= MAP_FILE_VERSION(1, 2))
      {
        CEFindPathGraph *pGraph;
        int i;
        for(i = 0; i < (int)h.m_dwFindPathGraphCount; i++){
          pGraph = new CEFindPathGraph();
          pGraph->Create(MapFile);
          m_listFindPathGraphs.AddHead(pGraph);
        }
      }
	  // And create the fly one
	  m_cFlyFindPathGraph.Create ( 0, "[Bez omezení]" );

      if(pProgress != NULL) pProgress->SetPos(100);


    }
    else{
      // The old format
      MapFile.SeekToBegin();

      SMapHeader_Old h;
      CString strLibraryPath;

      if(pProgress != NULL) pProgress->SetPos(0);

      m_bModified = FALSE;

      m_UnitSkillTypeCollection.Create();
      g_pUnitSkillTypeCollection = &m_UnitSkillTypeCollection;
      CreateSpecialBuffers();
      CreateMiniMap();

      if(MapFile.Read(&h, sizeof(h)) != sizeof(h)) throw int(1);

      m_dwWidth = h.m_dwWidth;  m_dwSquareWidth = m_dwWidth / MAPSQUARE_WIDTH;
      m_dwHeight = h.m_dwHeight; m_dwSquareHeight = m_dwHeight / MAPSQUARE_HEIGHT;
      m_strName = h.m_pName;
      m_strDescription = h.m_pDescription;
      m_strLandTypesFile = h.m_pLandTypesFile;

      if(!m_strLandTypesFile.IsEmpty()){
        CArchiveFile file;
        BOOL bFound = FALSE;
        CString strFileName = m_strLandTypesFile;

    LoadLandTypes_Old:;
        try{
          file = CDataArchive::GetRootArchive()->CreateFile(strFileName);
          LoadLandTypes(file);
          bFound = TRUE;
        }
        catch(CException *e){
          e->Delete();
        }

        if(!bFound){
          CString str;
          str.Format("Nebyl nalezen soubor definující typy povrchù '%s'.\nPokusíte se ho nalézt sami ?", m_strLandTypesFile);
          if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
            throw (int)1;

          CFileDialog dlg(TRUE, "*.landtypes", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
            "Typy povrchù (*.landtypes)|*.landtypes|Všechny soubory|*.*||", AfxGetMainWnd());
          str.Format("Najít %s", m_strLandTypesFile);
          dlg.m_ofn.lpstrTitle = str;

          if(dlg.DoModal() != IDOK) goto AfterLandTypes_Old;

          {
            CString strPath = dlg.GetPathName();
            int nPos = strPath.ReverseFind('\\');
            if(nPos == -1) strLibraryPath.Empty();
            else strLibraryPath = strPath.Left(nPos + 1);
          }

          strFileName = dlg.GetPathName();

          goto LoadLandTypes_Old;
        }
      }
      else{
        CLandType *pLandType;
        pLandType = new CLandType;
        pLandType->Create(0, "(transparentní)", RGB(255, 255, 255));
        m_listLandTypes.AddHead(pLandType);
      }
    AfterLandTypes_Old:
      if(pProgress != NULL) pProgress->SetPos(2);

      {
        int i;
        SMapexLibraryNode_Old node;
        CEMapexLibrary *pMapexLibrary;
        CDataArchive LibraryArchive;
        for(i = 0; i < (int)h.m_dwMapexLibraryCount; i++){
          MapFile.Read(&node, sizeof(node));
          BOOL bFound = FALSE;
          CString strFileName = node.m_pFullName;

          try{
            LibraryArchive.Create(node.m_pFullName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
            bFound = TRUE;
          }
          catch(CException *e){
            e->Delete();
          }
      
          if(!bFound){
            strFileName = strLibraryPath + node.m_pFileName;
    LoadMapexLibrary_Old:;
            try{
              LibraryArchive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
              bFound = TRUE;
            }
            catch(CException *e){
              e->Delete();
            }
        
            if(!bFound){
              CString str;
              str.Format("Nebyla nalezena knihovna mapexù '%s'.\nPokusíte se ji nalézt sami ?", node.m_pFileName);
        
              if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
                throw (int)1;
        
              CFileDialog dlg(TRUE, "*.mlb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                "Knihovny mapexù (*.mlb)|*.mlb|Všechny soubory|*.*||", AfxGetMainWnd());
              str.Format("Najít %s", node.m_pFileName);
              dlg.m_ofn.lpstrTitle = str;
              dlg.m_ofn.lpstrInitialDir = strLibraryPath;
              if(dlg.DoModal() != IDOK) continue;
        
              CString strLibFile = dlg.GetPathName();

              {
                int nPos = strLibFile.ReverseFind('\\');
                strLibraryPath = strLibFile.Left(nPos + 1);
              }
        
              m_bModified = TRUE;
              strFileName = strLibFile;

              goto LoadMapexLibrary_Old;
            }
          }
      
          pMapexLibrary = new CEMapexLibrary;
          pMapexLibrary->Create(LibraryArchive, FALSE, this);
      
          m_listLibraries.AddHead(pMapexLibrary);
      
          if(pProgress != NULL) pProgress->SetPos(2 + 50 * (i + 1) / h.m_dwMapexLibraryCount);
        }
      }

      // here load civilizations
      {
        int i;
        CECivilization *pCivilization;
        for(i = 0; i < (int)h.m_dwCivilizationsCount; i++){
          pCivilization = new CECivilization();

          pCivilization->Create(MapFile, this);
          m_listCivilizations.AddTail(pCivilization);
        }

        if((m_listCivilizations.GetHeadPosition() == NULL) || (m_listCivilizations.GetHead()->GetID() != 0)){
          // no system civilization -> create one
          CECivilization *pSystemCiv;
          pSystemCiv = new CECivilization();
          pSystemCiv->Create(0, "[Systémová civilizace]", RGB32(200, 200, 200));
          m_listCivilizations.AddHead(pSystemCiv);
        }
      }
      if(pProgress != NULL) pProgress->SetPos(55);

      // now load unit libraries
      {
        int i;
        SUnitTypeLibraryNode_Old node;
        CEUnitLibrary *pUnitLibrary;
        CDataArchive LibraryArchive;
        for(i = 0; i < (int)h.m_dwUnitTypeLibraryCount; i++){
          MapFile.Read(&node, sizeof(node));
          BOOL bFound = FALSE;
          CString strFileName = node.m_pFullName;

          try{
            LibraryArchive.Create(node.m_pFullName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
            bFound = TRUE;
          }
          catch(CException *e){
            e->Delete();
          }

          if(!bFound){
            strFileName = strLibraryPath + node.m_pFileName;
    LoadUnitTypeLibrary_Old:;
            try{
              LibraryArchive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
              bFound = TRUE;
            }
            catch(CException *e){
              e->Delete();
            }
      
            if(!bFound){
              CString str;
              str.Format("Nebyla nalezena knihovna jednotek '%s'.\nPokusíte se ji nalézt sami ?", node.m_pFileName);
      
              if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
                throw (int)1;
      
              CFileDialog dlg(TRUE, "*.ulb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                "Knihovny jednotek (*.ulb)|*.ulb|Všechny soubory|*.*||", AfxGetMainWnd());
              str.Format("Najít %s", node.m_pFileName);
              dlg.m_ofn.lpstrTitle = str;
              dlg.m_ofn.lpstrInitialDir = strLibraryPath;
              if(dlg.DoModal() != IDOK) continue;
      
              CString strLibFile = dlg.GetPathName();

              {
                int nPos = strLibFile.ReverseFind('\\');
                strLibraryPath = strLibFile.Left(nPos + 1);
              }
      
              m_bModified = TRUE;
              strFileName = strLibFile;

              goto LoadUnitTypeLibrary_Old;
            }
          }
      
          pUnitLibrary = new CEUnitLibrary;
          pUnitLibrary->Create(LibraryArchive, FALSE, this);
      
          m_listUnitLibraries.AddHead(pUnitLibrary);
  
          if(pProgress != NULL) pProgress->SetPos(55 + (40 * (i + 1) / h.m_dwUnitTypeLibraryCount));
        }
      }

      // now skip the map square offsets
      MapFile.Seek(sizeof(DWORD) * m_dwSquareWidth * m_dwSquareHeight, CFile::current);

      // now load map squares
      m_pMapSquares = new LPMapSquare[m_dwSquareWidth * m_dwSquareHeight];

      DWORD dwX, dwY;
      CEMapSquare *pSquare;
      for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
        for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
          pSquare = new CEMapSquare();
          pSquare->Create(MapFile);
          m_pMapSquares[dwX + dwY * m_dwSquareWidth] = pSquare;
        }
      }

      if(pProgress != NULL) pProgress->SetPos(97);

      // now load unit instances
      {
        CEUnit *pUnit;

        DWORD i;

        for(i = 0; i < h.m_dwUnitCount; i++){
          pUnit = new CEUnit();
          pUnit->Create(MapFile);
          InsertUnit(pUnit);
        }
      }

      // Load resources
      if(pProgress != NULL) pProgress->SetPos(99);
      {
        CEResource *pResource;

        int i;

        for(i = 0; i < RESOURCE_COUNT; i++){
          pResource = new CEResource();
          pResource->Create(i, MapFile);
          m_aResources[i] = pResource;
        }
      }

      // Load Visibilities
      {
        CEInvisibility *pInvisibility;
        int i;
        for(i = 0; i < INVISIBILITY_COUNT; i++){
          pInvisibility = new CEInvisibility();
          pInvisibility->Create(i, MapFile);
          m_aInvisibilities[i] = pInvisibility;
        }
      }

	  // And create the fly one find path graph
	  m_cFlyFindPathGraph.Create ( 0, "[Bez omezení]" );

      if(pProgress != NULL) pProgress->SetPos(100);

    }
  }
  catch(int nErr){
    switch(nErr){
    case 2:
      AfxMessageBox("Neznámá verze souboru mapy.");
      break;
    default:
      AfxMessageBox("Mapa nemohla být naètena.");
    };
    MapFile.Close();
    m_bDeleted = TRUE;
    return FALSE;
  }

  MapFile.Close();

  m_bBL1Visible = TRUE; m_bBL2Visible = TRUE; m_bBL3Visible = TRUE; m_bULVisible = TRUE;

  m_bDeleted = FALSE;

  return TRUE;
}

void CEMap::Delete(CProgressCtrl *pProgress)
{
  POSITION pos;
  CEMapexLibrary *pMapexLibrary;

  m_bDeleted = TRUE;

  if(pProgress != NULL) pProgress->SetPos(0);

  pos = m_listLibraries.GetHeadPosition();
  while(pos != NULL){
    pMapexLibrary = m_listLibraries.GetNext(pos);

    pMapexLibrary->Delete();
    delete pMapexLibrary;
  }
  m_listLibraries.RemoveAll();
  if(pProgress != NULL) pProgress->SetPos(40);

  CEUnitLibrary *pUnitLibrary;
  pos = m_listUnitLibraries.GetHeadPosition();
  while(pos != NULL){
    pUnitLibrary = m_listUnitLibraries.GetNext(pos);

    pUnitLibrary->Delete();
    delete pUnitLibrary;
  }
  m_listUnitLibraries.RemoveAll();
  if(pProgress != NULL) pProgress->SetPos(80);

  if(m_pMapSquares != NULL){
    DWORD dwX, dwY;
    CEMapSquare *pSquare;
    for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
      for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
        pSquare = GetMapSquare(dwX, dwY);
        pSquare->Delete();
        delete pSquare;
      }
    }
    delete m_pMapSquares;
    m_pMapSquares = NULL;
  }
  if(pProgress != NULL) pProgress->SetPos(90);

  {
    CECivilization *pCivilization;
    pos = m_listCivilizations.GetHeadPosition();
    while(pos != NULL){
      pCivilization = m_listCivilizations.GetNext(pos);
      pCivilization->Delete();
      delete pCivilization;
    }
    m_listCivilizations.RemoveAll();
  }
  if(pProgress != NULL) pProgress->SetPos(95);

  {
    CEUnit *pUnit;

    pos = m_listUnits.GetHeadPosition();
    while(pos != NULL){
      pUnit = m_listUnits.GetNext(pos);
      pUnit->Delete();
      delete pUnit;
    }
    m_listUnits.RemoveAll();
  }
  if(pProgress != NULL) pProgress->SetPos(100);
    
  DeleteLandTypes();

  {
    // Delete resources
    int i;
    for(i = 0; i < RESOURCE_COUNT; i++){
      if(m_aResources[i] != NULL){
        m_aResources[i]->Delete();
        delete m_aResources[i];
        m_aResources[i] = NULL;
      }
    }
  }

  {
    // Delete visibilities
    int i;
    for(i = 0; i < INVISIBILITY_COUNT; i++){
      if(m_aInvisibilities[i] != NULL){
        m_aInvisibilities[i]->Delete();
        delete m_aInvisibilities[i];
        m_aInvisibilities[i] = NULL;
      }
    }
  }

  {
    // Delete find path graphs
    POSITION pos;
    CEFindPathGraph *pGraph;
    pos = m_listFindPathGraphs.GetHeadPosition();
    while(pos != NULL){
      pGraph = m_listFindPathGraphs.GetNext(pos);

      pGraph->Delete();
      delete pGraph;
    }
    m_listFindPathGraphs.RemoveAll();
  }

  {
    // delete script sets
    POSITION pos;
    CEScriptSet *pSet;
    pos = m_listScriptSet.GetHeadPosition();
    while(pos != NULL){
      pSet = m_listScriptSet.GetNext(pos);

      pSet->Delete();
      delete pSet;
    }
    m_listScriptSet.RemoveAll();
  }

  m_mapMapexes.RemoveAll();
  m_mapUnitTypes.RemoveAll();

  DeleteSpecialBuffers();
  DeleteMiniMap();
  g_pUnitSkillTypeCollection = NULL;
  m_UnitSkillTypeCollection.Delete();
}

BOOL CEMap::Create(DWORD dwWidth, DWORD dwHeight)
{
  m_strName.Empty();
  m_strDescription.Empty();

  m_bModified = TRUE;

  m_UnitSkillTypeCollection.Create();
  g_pUnitSkillTypeCollection = &m_UnitSkillTypeCollection;
  CreateSpecialBuffers();
  CreateMiniMap();

  m_dwWidth = dwWidth; m_dwHeight = dwHeight;
  m_dwSquareWidth = m_dwWidth / MAPSQUARE_WIDTH;
  m_dwSquareHeight = m_dwHeight / MAPSQUARE_HEIGHT;

  m_pMapSquares = new LPMapSquare[m_dwSquareWidth * m_dwSquareHeight];
  DWORD dwX, dwY;
  CEMapSquare *pSquare;
  for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
    for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
      pSquare = new CEMapSquare();
      pSquare->Create();
      m_pMapSquares[dwX + dwY * m_dwSquareWidth] = pSquare;
    }
  }

  // create transparent land type
  CLandType *pLandType;
  pLandType = new CLandType;
  pLandType->Create(0, "(transparentní)", RGB(255, 255, 255));
  m_listLandTypes.AddHead(pLandType);

  // create system civilization
  CECivilization *pSystemCiv;
  pSystemCiv = new CECivilization();
  pSystemCiv->Create(0, "[System civilization]", RGB32(200, 200, 200));
  m_listCivilizations.AddHead(pSystemCiv);

  // Create empty resources
  {
    CEResource *pResource;
    int i;
    CString strName;
    for(i = 0; i < RESOURCE_COUNT; i++){
      pResource = new CEResource();
      strName.Format("Surovina %d", i + 1);
      pResource->Create(i, strName);
      m_aResources[i] = pResource;
    }
  }

  // Create empty visibilities
  {
    CEInvisibility *pInvisibility;
    int i;
    for(i = 0; i < INVISIBILITY_COUNT; i++){
      pInvisibility = new CEInvisibility();
      pInvisibility->Create(i);
      m_aInvisibilities[i] = pInvisibility;
    }
  }

  // And create the fly one find path graph
  m_cFlyFindPathGraph.Create ( 0, "[Bez omezení]" );

  m_bBL1Visible = TRUE; m_bBL2Visible = TRUE; m_bBL3Visible = TRUE; m_bULVisible = TRUE;

  m_bDeleted = FALSE;

  return TRUE;
}

void CEMap::SaveMap(CArchiveFile MapFile)
{
  MapFile.Open();

  // Save the version header
  {
    SFileVersionHeader sFileVersionHeader;
    BYTE aFileID[16] = MAP_FILE_IDENTIFIER;

    memset(&sFileVersionHeader, 0, sizeof(sFileVersionHeader));
    memcpy(sFileVersionHeader.m_aMapFileID, aFileID, 16);
    sFileVersionHeader.m_dwFormatVersion = CURRENT_MAP_FILE_VERSION;
    sFileVersionHeader.m_dwCompatibleFormatVersion = COMPATIBLE_MAP_FILE_VERSION;

    MapFile.Write(&sFileVersionHeader, sizeof(sFileVersionHeader));
  }

  
  SMapHeader h;

  memset(&h, 0, sizeof(h));
  h.m_dwWidth = m_dwWidth;
  h.m_dwHeight = m_dwHeight;
  strncpy(h.m_pName, m_strName, 50);
  strncpy(h.m_pDescription, m_strDescription, 1000);
  h.m_dwMapexLibraryCount = (DWORD)m_listLibraries.GetCount();
  strncpy(h.m_pLandTypesFile, m_strLandTypesFile, 255);
  h.m_dwCivilizationsCount = (DWORD)m_listCivilizations.GetCount();
  h.m_dwUnitTypeLibraryCount = (DWORD)m_listUnitLibraries.GetCount();
  h.m_dwUnitCount = (DWORD)m_listUnits.GetCount();
  h.m_dwScriptSetCount = (DWORD)m_listScriptSet.GetCount();
  h.m_dwFindPathGraphCount = (DWORD)m_listFindPathGraphs.GetCount();
  h.m_dwMapVersion = GetNewUnitTypeID();
  {
    C256BitArray aUsedLandTypes;
    aUsedLandTypes.FillArray(FALSE);

    POSITION pos;
    CLandType *pLandType;
    pos = m_listLandTypes.GetHeadPosition();
    while(pos != NULL){
      pLandType = m_listLandTypes.GetNext(pos);
      aUsedLandTypes.SetAt(pLandType->GetID(), TRUE);
    }

    memcpy(h.m_aUsedLandTypes, aUsedLandTypes.GetData(), 8);
  }
  
  MapFile.Write(&h, sizeof(h));

  // write scriptsets
  {
    POSITION pos = m_listScriptSet.GetHeadPosition();
    CEScriptSet *pScriptSet;
    while(pos != NULL){
      pScriptSet = m_listScriptSet.GetNext(pos);
      pScriptSet->Save(MapFile);
    }
  }

  // save mapex libraries
  POSITION pos;
  CEMapexLibrary *pMapexLibrary;
  SMapexLibraryNode node;
  pos = m_listLibraries.GetHeadPosition();
  while(pos != NULL){
    pMapexLibrary = m_listLibraries.GetNext(pos);

    pMapexLibrary->SaveLibrary();

    memset(&node, 0, sizeof(node));
    strncpy(node.m_pFileName, pMapexLibrary->GetFileName(), 255);
    strncpy(node.m_pFullName, pMapexLibrary->GetFullPath(), 1023);
    MapFile.Write(&node, sizeof(node));
  }

  // save civilizations
  {
    CECivilization *pCivilization;
    pos = m_listCivilizations.GetHeadPosition();
    while(pos != NULL){
      pCivilization = m_listCivilizations.GetNext(pos);

      pCivilization->SaveToFile(MapFile);
    }
  }

  // save unit libraries
  {
    POSITION pos;
    CEUnitLibrary *pUnitLibrary;
    SUnitTypeLibraryNode node;
    pos = m_listUnitLibraries.GetHeadPosition();
    while(pos != NULL){
      pUnitLibrary = m_listUnitLibraries.GetNext(pos);
      
      pUnitLibrary->SaveLibrary();
      
      memset(&node, 0, sizeof(node));
      strncpy(node.m_pFileName, pUnitLibrary->GetFileName(), 255);
      strncpy(node.m_pFullName, pUnitLibrary->GetFullPath(), 1023);
      MapFile.Write(&node, sizeof(node));
    }
  }
  
  // write mapsquare offsets
  DWORD *aMapSquareOffsets;
  try{
    aMapSquareOffsets = new DWORD[m_dwSquareHeight * m_dwSquareWidth];
    {
      DWORD dwCurOffset = 0;
      DWORD dwX, dwY;
      CEMapSquare *pSquare;
      
      for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
        for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
          aMapSquareOffsets[dwY * m_dwSquareWidth + dwX] = dwCurOffset;
          pSquare = GetMapSquare(dwX, dwY);
          dwCurOffset += pSquare->GetSaveSize();
        }
      }
    }
    MapFile.Write(aMapSquareOffsets, m_dwSquareHeight * m_dwSquareWidth * sizeof(DWORD));
    delete aMapSquareOffsets;
  }
  catch(CException *){
    delete aMapSquareOffsets;
    throw;
  }

  // write mapsquares
  DWORD dwX, dwY;
  CEMapSquare *pSquare;
  for(dwY = 0; dwY < m_dwSquareHeight; dwY++){
    for(dwX = 0; dwX < m_dwSquareWidth; dwX++){
      pSquare = GetMapSquare(dwX, dwY);
      pSquare->SaveToFile(MapFile); // don;t have to lock it cause the minimap can only read it
    }
  }

  // write units
  {
    CEUnit *pUnit;
    POSITION pos = m_listUnits.GetHeadPosition();
    while(pos != NULL){
      pUnit = m_listUnits.GetNext(pos);
      pUnit->Save(MapFile);
    }
  }

  // write resources
  {
    int i;
    for(i = 0; i < RESOURCE_COUNT; i++){
      m_aResources[i]->Save(MapFile);
    }
  }

  // write visibilities
  {
    int i;
    for(i = 0; i < INVISIBILITY_COUNT; i++){
      m_aInvisibilities[i]->Save(MapFile);
    }
  }

  // write find path graphs
  {
    POSITION pos = m_listFindPathGraphs.GetHeadPosition();
    CEFindPathGraph *pGraph;
    while(pos != NULL){
      pGraph = m_listFindPathGraphs.GetNext(pos);
      pGraph->Save(MapFile);
    }
  }

  MapFile.Close();
}

CString CEMap::GetName()
{
  return m_strName;
}

void CEMap::SetName(CString strName)
{
  m_strName = strName;
}

CString CEMap::GetDescription()
{
  return m_strDescription;
}

void CEMap::SetDescription(CString strDescription)
{
  m_strDescription = strDescription;
}

CEMapexLibrary *CEMap::NewMapexLibrary(CString strFileName)
{
  CDataArchive Archive;

  Archive.Create(strFileName, CArchiveFile::modeCreate | CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveFile);

  CEMapexLibrary *pMapexLibrary;
  pMapexLibrary = new CEMapexLibrary();
  if(!pMapexLibrary->Create(Archive, TRUE, this)){
    throw new CStringException("Can't create new library.");
  }

  m_listLibraries.AddHead(pMapexLibrary);

  return pMapexLibrary;
}

CEUnitLibrary * CEMap::NewUnitLibrary(CString strFileName)
{
  CDataArchive Archive;

  Archive.Create(strFileName, CArchiveFile::modeCreate | CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveFile);

  CEUnitLibrary *pUnitLibrary;
  pUnitLibrary = new CEUnitLibrary();
  if(!pUnitLibrary->Create(Archive, TRUE, this)){
    throw new CStringException("Can't create new library.");
  }

  m_listUnitLibraries.AddHead(pUnitLibrary);

  return pUnitLibrary;
}

POSITION CEMap::GetFirstMapexLibraryPosition()
{
  return m_listLibraries.GetHeadPosition();
}

POSITION CEMap::GetFirstUnitLibraryPosition()
{
  return m_listUnitLibraries.GetHeadPosition();
}

CEMapexLibrary * CEMap::GetNextMapexLibrary(POSITION &pos)
{
  return m_listLibraries.GetNext(pos);
}

CEUnitLibrary * CEMap::GetNextUnitLibrary(POSITION &pos)
{
  return m_listUnitLibraries.GetNext(pos);
}

void CEMap::DeleteMapexLibrary(CEMapexLibrary *pMapexLibrary)
{
  ASSERT_VALID(pMapexLibrary);

  POSITION pos = m_listLibraries.Find(pMapexLibrary);

  pMapexLibrary->Delete();
  m_listLibraries.RemoveAt(pos);
}

void CEMap::DeleteUnitLibrary(CEUnitLibrary *pUnitLibrary)
{
  ASSERT_VALID(pUnitLibrary);

  POSITION pos = m_listUnitLibraries.Find(pUnitLibrary);

  pUnitLibrary->Delete();
  m_listUnitLibraries.RemoveAt(pos);
}

void CEMap::AddMapex(CEMapex *pMapex)
{
  m_mapMapexes.SetAt(pMapex->GetID(), pMapex);
}

void CEMap::DeleteMapex(CEMapex *pMapex)
{
  m_mapMapexes.RemoveKey(pMapex->GetID());
}

void CEMap::AddUnitType(CEUnitType *pUnitType)
{
  m_mapUnitTypes.SetAt(pUnitType->GetID(), pUnitType);
}

void CEMap::DeleteUnitType(CEUnitType *pUnitType)
{
  m_mapUnitTypes.RemoveKey(pUnitType->GetID());
}


void CEMap::OpenMapexLibrary(CString strFileName)
{
  CDataArchive Archive;

  if(!Archive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone )){
    CString str;
    str.Format("Can't open mapex library archive '%s'.", strFileName);
    throw new CStringException(str);
  }

  CEMapexLibrary *pMapexLibrary;
  pMapexLibrary = new CEMapexLibrary();

  if(!pMapexLibrary->Create(Archive, FALSE, this)){
    CString str;
    str.Format("Can't load mapex library '%s'.", strFileName);
    throw new CStringException(str);
  }

  m_listLibraries.AddHead(pMapexLibrary);
}

void CEMap::OpenUnitLibrary(CString strFileName)
{
  CDataArchive Archive;

  if(!Archive.Create(strFileName, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone )){
    CString str;
    str.Format("Can't open unit library archive '%s'.", strFileName);
    throw new CStringException(str);
  }

  CEUnitLibrary *pUnitLibrary;
  pUnitLibrary = new CEUnitLibrary();

  if(!pUnitLibrary->Create(Archive, FALSE, this)){
    CString str;
    str.Format("Can't load unit library '%s'.", strFileName);
    throw new CStringException(str);
  }

  m_listUnitLibraries.AddHead(pUnitLibrary);
}

void CEMap::FillScriptSetComboBox(CComboBox *pComboBox)
{
  POSITION pos = m_listScriptSet.GetHeadPosition();
  CEScriptSet *pScriptSet;
  pComboBox->ResetContent();


  int i = 0;
  pComboBox->InsertString(i, "[žádný]");
  pComboBox->SetItemData(i, 0);
  i++;
  while(pos != NULL){
    pScriptSet = m_listScriptSet.GetNext(pos);
    pComboBox->InsertString(i, pScriptSet->GetName());
    pComboBox->SetItemData(i, (DWORD)pScriptSet);

    i++;
  }
}

void CEMap::FillScriptSetListCtrl(CListCtrl *pListCtrl)
{
  pListCtrl->DeleteAllItems();

  int i, j = 0;
  POSITION pos = m_listScriptSet.GetHeadPosition();
  CEScriptSet *pScriptSet;
  while(pos != NULL){
    pScriptSet = m_listScriptSet.GetNext(pos);

    i = pListCtrl->InsertItem(j, pScriptSet->GetName(), 0);
    pListCtrl->SetItemData(i, (DWORD)pScriptSet);

    j++;
  }
}

void CEMap::FillFindPathGraphListCtrl(CListCtrl *pListCtrl)
{
  pListCtrl->DeleteAllItems();

  int i, j = 0;
  POSITION pos = m_listFindPathGraphs.GetHeadPosition();
  CEFindPathGraph *pGraph;

  i = pListCtrl->InsertItem ( j, m_cFlyFindPathGraph.GetName (), 0);
  pListCtrl->SetItemData( i, (DWORD)&m_cFlyFindPathGraph );
  j++;

  while(pos != NULL){
    pGraph = m_listFindPathGraphs.GetNext(pos);

    i = pListCtrl->InsertItem(j, pGraph->GetName(), 0);
    pListCtrl->SetItemData(i, (DWORD)pGraph);

    j++;
  }
}

void CEMap::FillMapexLibrariesComboBox(CComboBox *pComboBox)
{
  int nSel = pComboBox->GetCurSel();
  pComboBox->ResetContent();

  POSITION pos;
  CEMapexLibrary *pMapexLibrary;
  int i;

  pos = m_listLibraries.GetHeadPosition();
  i = 0;
  while(pos != NULL){
    pMapexLibrary = m_listLibraries.GetNext(pos);

    pComboBox->InsertString(i, pMapexLibrary->GetName());
    pComboBox->SetItemData(i, (DWORD)pMapexLibrary);

    i++;
  }
  pComboBox->SetCurSel(nSel);
}

void CEMap::FillUnitLibrariesComboBox(CComboBox *pComboBox)
{
  int nSel = pComboBox->GetCurSel();
  pComboBox->ResetContent();

  POSITION pos;
  CEUnitLibrary *pUnitLibrary;
  int i;

  pos = m_listUnitLibraries.GetHeadPosition();
  i = 0;
  while(pos != NULL){
    pUnitLibrary = m_listUnitLibraries.GetNext(pos);

    pComboBox->InsertString(i, pUnitLibrary->GetName());
    pComboBox->SetItemData(i, (DWORD)pUnitLibrary);

    i++;
  }
  pComboBox->SetCurSel(nSel);
}

void CEMap::FillUnitTypesComboBox(CComboBox *pComboBox)
{
  POSITION pos;

  pComboBox->ResetContent();

  pos = m_mapUnitTypes.GetStartPosition();
  CEUnitType *pUnitType;
  DWORD dwID;
  int i = 0;
  while(pos != NULL){
    m_mapUnitTypes.GetNextAssoc(pos, dwID, pUnitType);
    pComboBox->InsertString(i, pUnitType->GetName());
    pComboBox->SetItemData(i, (DWORD)pUnitType);

    i++;
  }
}

void CEMap::FillCivilizationsComboBox(CComboBox *pComboBox)
{
  int nSel = pComboBox->GetCurSel();
  pComboBox->ResetContent();

  POSITION pos;
  CECivilization *pCivilization;
  int i;

  pos = m_listCivilizations.GetHeadPosition();
  i = 0;
  while(pos != NULL){
    pCivilization = m_listCivilizations.GetNext(pos);

    pComboBox->InsertString(i, pCivilization->GetName());
    pComboBox->SetItemData(i, (DWORD)pCivilization);

    i++;
  }
  pComboBox->SetCurSel(nSel);
}

DWORD CEMap::GetNewMapexID()
{
  CTime time;
  time = CTime::GetCurrentTime();

  return (DWORD)time.GetTime();
}

DWORD CEMap::GetNewCivilizationID()
{
  CTime time;
  time = CTime::GetCurrentTime();

  return (DWORD)time.GetTime();
}

DWORD CEMap::GetNewUnitTypeID()
{
  CTime time;
  time = CTime::GetCurrentTime();

  return (DWORD)time.GetTime();
}

void CEMap::DeleteLandTypes()
{
  POSITION pos;
  CLandType *pLandType;
  pos = m_listLandTypes.GetHeadPosition();
  while(pos != NULL){
    pLandType=  m_listLandTypes.GetNext(pos);

    pLandType->Delete();
    delete pLandType;
  }
  m_listLandTypes.RemoveAll();

  m_strLandTypesFile.Empty();
}

void CEMap::LoadLandTypes(CArchiveFile file)
{
  CConfigFile CfgFile, *pCfgFile;

  DeleteLandTypes();

  try{
    CfgFile.Create(file);
    pCfgFile = &CfgFile;

    CConfigFileSection RootSection = CfgFile.GetRootSection();

    CFG_BEGIN(pCfgFile);

    DWORD dwCount, dwPos;
    DWORD dwID, dwColor;
    CAnimation *pAnimation;
    CString strTag, strName;
    CLandType *pLandType;

    // add transparent land type
    pLandType = new CLandType();
    pLandType->Create(0, "(transparentní)", 0xFFFFFF);
    m_listLandTypes.AddHead(pLandType);

    CFG_LONG("Count", dwCount);
    for(dwPos = 1; dwPos <= dwCount; dwPos++){
      pAnimation = NULL;

      strTag.Format("Land%d_ID", dwPos);
      CFG_LONG(strTag, dwID);
      if((dwID > 255) || (dwID < 1)){
        CString str;
        str.Format("Invalid land type ID in file '%s'.\nMust be in range 1 .. 255 .", file.GetFilePath());
        throw new CStringException(str);
      }
      if(GetLandType((BYTE)dwID) != NULL){
        CString str;
        str.Format("Duplicit land type ID %d , in file '%s'.\nMust be in range 1 .. 255 .", dwID, file.GetFilePath());
        throw new CStringException(str);
      }

      strTag.Format("Land%d_Name", dwPos);
      strName = RootSection.GetString(strTag);

      strTag.Format("Land%d_Graphics", dwPos);
      CFG_ANIMATION_O(strTag, pAnimation);
      strTag.Format("Land%d_Color", dwPos);
      CFG_COLOR(strTag, dwColor);

      pLandType = new CLandType();
      if(pAnimation != NULL)
        pLandType->Create((BYTE)dwID, strName, pAnimation, dwColor);
      else
        pLandType->Create((BYTE)dwID, strName, dwColor);
      m_listLandTypes.AddTail(pLandType);
    }

    CFG_END();

    m_strLandTypesFile = file.GetFilePath();
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
    DeleteLandTypes();
    m_strLandTypesFile.Empty();
  }

  CfgFile.Delete();
}

CLandType * CEMap::GetLandType(BYTE nID)
{
  POSITION pos;
  CLandType *pLandType;
  pos = m_listLandTypes.GetHeadPosition();
  while(pos != NULL){
    pLandType = m_listLandTypes.GetNext(pos);
    if(pLandType->GetID() == nID) return pLandType;
  }
  return NULL;
}

void CEMap::FillLandTypesListCtrl(CListCtrl *pListCtrl)
{
  POSITION pos;
  CLandType *pLandType;
  int i, nItem;

  pListCtrl->DeleteAllItems();

  pos = m_listLandTypes.GetHeadPosition();
  i = 0;
  while(pos != NULL){
    pLandType = m_listLandTypes.GetNext(pos);
    nItem = pListCtrl->InsertItem(i, pLandType->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pLandType);

    i++;
  }
}

void CEMap::FillCivilizationsListCtrl(CListCtrl *pListCtrl)
{
  POSITION pos;
  CECivilization *pCivilization;
  int i, nItem;

  pListCtrl->DeleteAllItems();

  pos = m_listCivilizations.GetHeadPosition();
  i = 0;
  while(pos != NULL){
    pCivilization = m_listCivilizations.GetNext(pos);
    nItem = pListCtrl->InsertItem(i, pCivilization->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pCivilization);

    i++;
  }
}

void CEMap::SaveLibraries()
{
  POSITION pos;
  CEMapexLibrary *pLibrary;
  pos = m_listLibraries.GetHeadPosition();
  while(pos != NULL){
    pLibrary = m_listLibraries.GetNext(pos);
    pLibrary->SaveLibrary();
  }
}

void CEMap::SaveUnitLibraries()
{
  POSITION pos;
  CEUnitLibrary *pLibrary;
  pos = m_listUnitLibraries.GetHeadPosition();
  while(pos != NULL){
    pLibrary = m_listUnitLibraries.GetNext(pos);
    pLibrary->SaveLibrary();
  }
}

CEMapex * CEMap::GetMapex(DWORD dwID)
{
  CEMapex *pMapex;
  if(!m_mapMapexes.Lookup(dwID, pMapex)) return NULL;
  return pMapex;
}

CEUnitType * CEMap::GetUnitType(DWORD dwID)
{
  CEUnitType *pUnitType;
  if(!m_mapUnitTypes.Lookup(dwID, pUnitType)) return NULL;
  return pUnitType;
}

void CEMap::DrawView(DWORD dwXPos, DWORD dwYPos, CRect &rcDest, CDDrawSurface *pSurface)
{
  pSurface->Fill(0, &rcDest);

  DWORD dwXSize, dwYSize;
  dwXSize = rcDest.Width() / 16 + 1;
  dwYSize = rcDest.Height() / 16 + 1;

  // now construct the list of all sqaures that mapexes from can touch our rect
  CTypedPtrList<CPtrList, CEMapSquare *> m_listDrawSquares;
  int nX, nY;
  // we have to start one square above the affected one
  // becauese mapexes can override square boundaries
  for(nY = dwYPos / MAPSQUARE_HEIGHT - 1; nY <= (int)((dwYPos + dwYSize) / MAPSQUARE_HEIGHT); nY++){
    if((nY < 0) || (nY >= (int)m_dwSquareHeight)) continue;
    // the same trick with starting index as for Y coord
    for(nX = dwXPos / MAPSQUARE_WIDTH - 1; nX <= (int)((dwXPos + dwXSize) / MAPSQUARE_WIDTH); nX++){
      if((nX < 0) || (nX >= (int)m_dwSquareWidth)) continue;

      m_listDrawSquares.AddHead(GetMapSquare(nX, nY));
    }
  }

  // no square locking -> we're only reading it -> OK

  // make draw background levels
  DWORD dwLevel;
  CTypedPtrList<CPtrList, CEMapexInstance *> m_listMapexes;
  POSITION pos;
  CEMapSquare *pSquare;
  CEMapexInstance *pMapexInstance;
  CPoint ptTopLeft;

  for(dwLevel = 1; dwLevel <= 3; dwLevel++){

    if((dwLevel == 1) && !m_bBL1Visible) continue;
    if((dwLevel == 2) && !m_bBL2Visible) continue;
    if((dwLevel == 3) && !m_bBL3Visible) continue;

    // collect all mapexes for this level from draw squares
    m_listMapexes.RemoveAll();

    pos = m_listDrawSquares.GetHeadPosition();
    while(pos != NULL){
      pSquare = m_listDrawSquares.GetNext(pos);
      pSquare->AppendBLMapexes(dwLevel, &m_listMapexes);
    }

    // now draw them
    pos = m_listMapexes.GetHeadPosition();
    while(pos != NULL){
      pMapexInstance = m_listMapexes.GetNext(pos);
      ptTopLeft.x = rcDest.left + (pMapexInstance->GetXPosition() - dwXPos) * 16;
      ptTopLeft.y = rcDest.top + (pMapexInstance->GetYPosition() - dwYPos) * 16;

      if((m_pSelection != NULL) && (m_pSelection->Includes(pMapexInstance))){
        if(m_bDrawLandTypes)
          pMapexInstance->DrawLandTypesSelected(ptTopLeft, &rcDest, pSurface, &m_LandTypeBuffer, &m_SelectionLayer);
        else
          pMapexInstance->DrawSelected(ptTopLeft, &rcDest, pSurface, &m_MapexBuffer, &m_SelectionLayer);
      }
      else{
        if(m_bDrawLandTypes)
          pMapexInstance->DrawLandTypes(ptTopLeft, &rcDest, pSurface);
        else
          pMapexInstance->Draw(ptTopLeft, &rcDest, pSurface);
      }
    }

    if(dwLevel == m_dwSelectedLevel){
      // draw the grid
      if(m_bDrawGrid){
        // first horizontal lines
        DWORD dwX, dwY;
        int nLeft, nXScreenPos, nYScreenPos;
        CRect rcLine;
        CSize sizeLine;
        
        sizeLine.cx = m_HorizontalGridLine.GetWidth();
        sizeLine.cy = m_HorizontalGridLine.GetHeight();
        rcLine.left = 0; rcLine.top = 0; rcLine.bottom = sizeLine.cy;
        for(dwY = dwYPos; dwY < (dwYPos + dwYSize); dwY++){
          if((dwY % m_dwGridSize) != 0) continue;
          
          nYScreenPos = (dwY - dwYPos) * 16 + rcDest.top;
          nLeft = rcDest.Width();
          nXScreenPos = rcDest.left;
          while(nLeft > 0){
            if(nLeft > sizeLine.cx)
              rcLine.right = sizeLine.cx;
            else
              rcLine.right = nLeft;
            pSurface->Paste(nXScreenPos, nYScreenPos, &m_HorizontalGridLine, &rcLine);
            nLeft -= rcLine.Width();
            nXScreenPos += rcLine.Width();;
          }
        }
        
        // and then vertical lines
        sizeLine.cx = m_VerticalGridLine.GetWidth();
        sizeLine.cy = m_VerticalGridLine.GetHeight();
        rcLine.left = 0; rcLine.top = 0; rcLine.right = sizeLine.cx;
        for(dwX = dwXPos; dwX < (dwXPos + dwXSize); dwX++){
          if((dwX % m_dwGridSize) != 0) continue;
          
          nXScreenPos = (dwX - dwXPos) * 16 + rcDest.left;
          nLeft = rcDest.Height();
          nYScreenPos = rcDest.top;
          while(nLeft > 0){
            if(nLeft > sizeLine.cy)
              rcLine.bottom = sizeLine.cy;
            else
              rcLine.bottom = nLeft;
            pSurface->Paste(nXScreenPos, nYScreenPos, &m_VerticalGridLine, &rcLine);
            nLeft -= sizeLine.cy;
            nYScreenPos += sizeLine.cy;
          }
        }
      }
    }
  }
  // now draw the Unit level
  {
    if(!m_bULVisible) goto NoUnitLevel;

    CTypedPtrList<CPtrList, CEULMapexInstance *> m_listULMapexes;
    CTypedPtrList<CPtrList, CEUnit *> m_listUnits;
    CEULMapexInstance *pULMapex;
    CEUnit *pUnit;

    // collect all mapexes & units for this level from draw squares
    m_listULMapexes.RemoveAll();
    m_listUnits.RemoveAll();

    pos = m_listDrawSquares.GetHeadPosition();
    while(pos != NULL){
      pSquare = m_listDrawSquares.GetNext(pos);
      // this will also sort the list (first one -> the smallest z pos -> draw first)
      pSquare->AppendULMapexes(&m_listULMapexes);
      // this will also sort the list
      pSquare->AppendUnits(&m_listUnits);
    }

    POSITION posMapex, posUnits;

    // now draw them
    posMapex = m_listULMapexes.GetHeadPosition();
    posUnits = m_listUnits.GetHeadPosition();
    while((posMapex != NULL) || (posUnits != NULL)){
      if(posMapex != NULL) pULMapex = m_listULMapexes.GetAt(posMapex);
      else pULMapex = NULL;

      if(posUnits != NULL) pUnit = m_listUnits.GetAt(posUnits);
      else pUnit = NULL;

      if((pUnit == NULL) && (pULMapex == NULL)) continue;

      if(pUnit == NULL)
        goto ULMapexDraw;
      if(pULMapex == NULL)
        goto UnitDraw;

      if(pUnit->GetZPos() < pULMapex->GetZPosition()){
UnitDraw:
      ;

        ptTopLeft.x = rcDest.left + (pUnit->GetXPos() - dwXPos) * 16 + pUnit->GetXTransition();
        ptTopLeft.y = rcDest.top + (pUnit->GetYPos() - dwYPos) * 16 + pUnit->GetYTransition();

        if(m_pUnitSelection == pUnit){
          if(!m_bDrawLandTypes){
            pUnit->DrawSelected(ptTopLeft, &rcDest, pSurface, &m_MapexBuffer, &m_SelectionLayer);
          }
        }
        else{
          if(!m_bDrawLandTypes){
            pUnit->Draw(ptTopLeft, &rcDest, pSurface);
          }
        }
        
        m_listUnits.GetNext(posUnits);
      }
      else{
ULMapexDraw:
      ;
      
        ptTopLeft.x = rcDest.left + (pULMapex->GetXPosition() - dwXPos) * 16;
        ptTopLeft.y = rcDest.top + (pULMapex->GetYPosition() - dwYPos) * 16;
      
        if(m_pULSelection == pULMapex){
          if(m_bDrawLandTypes)
            pULMapex->DrawLandTypesSelected(ptTopLeft, &rcDest, pSurface, &m_LandTypeBuffer, &m_SelectionLayer);
          else
            pULMapex->DrawSelected(ptTopLeft, &rcDest, pSurface, &m_MapexBuffer, &m_SelectionLayer);
        }
        else{
          if(m_bDrawLandTypes)
            pULMapex->DrawLandTypes(ptTopLeft, &rcDest, pSurface);
          else
            pULMapex->Draw(ptTopLeft, &rcDest, pSurface);
        }

        m_listULMapexes.GetNext(posMapex);
      }
    }

    if(dwLevel == 4){
      // draw the grid
      if(m_bDrawGrid){
        // first horizontal lines
        DWORD dwX, dwY;
        int nLeft, nXScreenPos, nYScreenPos;
        CRect rcLine;
        CSize sizeLine;
        
        sizeLine.cx = m_HorizontalGridLine.GetWidth();
        sizeLine.cy = m_HorizontalGridLine.GetHeight();
        rcLine.left = 0; rcLine.top = 0; rcLine.bottom = sizeLine.cy;
        for(dwY = dwYPos; dwY < (dwYPos + dwYSize); dwY++){
          if((dwY % m_dwGridSize) != 0) continue;
          
          nYScreenPos = (dwY - dwYPos) * 16 + rcDest.top;
          nLeft = rcDest.Width();
          nXScreenPos = rcDest.left;
          while(nLeft > 0){
            if(nLeft > sizeLine.cx)
              rcLine.right = sizeLine.cx;
            else
              rcLine.right = nLeft;
            pSurface->Paste(nXScreenPos, nYScreenPos, &m_HorizontalGridLine, &rcLine);
            nLeft -= rcLine.Width();
            nXScreenPos += rcLine.Width();;
          }
        }
        
        // and then vertical lines
        sizeLine.cx = m_VerticalGridLine.GetWidth();
        sizeLine.cy = m_VerticalGridLine.GetHeight();
        rcLine.left = 0; rcLine.top = 0; rcLine.right = sizeLine.cx;
        for(dwX = dwXPos; dwX < (dwXPos + dwXSize); dwX++){
          if((dwX % m_dwGridSize) != 0) continue;
          
          nXScreenPos = (dwX - dwXPos) * 16 + rcDest.left;
          nLeft = rcDest.Height();
          nYScreenPos = rcDest.top;
          while(nLeft > 0){
            if(nLeft > sizeLine.cy)
              rcLine.bottom = sizeLine.cy;
            else
              rcLine.bottom = nLeft;
            pSurface->Paste(nXScreenPos, nYScreenPos, &m_VerticalGridLine, &rcLine);
            nLeft -= sizeLine.cy;
            nYScreenPos += sizeLine.cy;
          }
        }
      }
    }
  }
NoUnitLevel:
  ;

  // now draw the selection rectangle
  if(!m_rcSelection.IsRectEmpty())
  {
    int nXLeft, nYLeft;
    int nPos1, nPos2;
    CRect rcPart, rcBuffer(0, 0, 0, 0);
    
    // first draw the horizontal lines
    nXLeft = m_rcSelection.Width() * 16;
    nPos1 = (m_rcSelection.top - dwYPos) * 16 + rcDest.top;
    nPos2 = (m_rcSelection.bottom - dwYPos) * 16 + rcDest.top;
    rcPart.left = (m_rcSelection.left - dwXPos) * 16 + rcDest.left;
    rcBuffer.bottom = m_HorizontalSelectionLine.GetHeight();
    while(nXLeft > 0){
      if(nXLeft > (int)m_HorizontalSelectionLine.GetWidth())
        rcPart.right = rcPart.left + m_HorizontalSelectionLine.GetWidth();
      else
        rcPart.right = rcPart.left + nXLeft;

      rcBuffer.right = rcPart.Width();
      pSurface->Paste(rcPart.left, nPos1, &m_HorizontalSelectionLine, &rcBuffer);
      pSurface->Paste(rcPart.left, nPos2, &m_HorizontalSelectionLine, &rcBuffer);

      nXLeft -= rcPart.Width();
      rcPart.left += rcPart.Width();
    }

    // than draw the vertical lines
    nYLeft = m_rcSelection.Height() * 16;
    nPos1 = (m_rcSelection.left - dwXPos) * 16 + rcDest.left;
    nPos2 = (m_rcSelection.right - dwXPos) * 16 + rcDest.left;
    rcPart.top = (m_rcSelection.top - dwYPos) * 16 + rcDest.top;
    rcBuffer.right = m_VerticalSelectionLine.GetWidth();
    while(nYLeft > 0){
      if(nYLeft > (int)m_VerticalSelectionLine.GetHeight())
        rcPart.bottom = rcPart.top + m_VerticalSelectionLine.GetHeight();
      else
        rcPart.bottom = rcPart.top + nYLeft;

      rcBuffer.bottom = rcPart.Height();
      pSurface->Paste(nPos1, rcPart.top, &m_VerticalSelectionLine, &rcBuffer);
      pSurface->Paste(nPos2, rcPart.top, &m_VerticalSelectionLine, &rcBuffer);

      nYLeft -= rcPart.Height();
      rcPart.top += rcPart.Height();
    }
  }
}

void CEMap::CreateSpecialBuffers()
{
  m_MapexBuffer.SetWidth(100);
  m_MapexBuffer.SetHeight(100);
  m_MapexBuffer.SetAlphaChannel(TRUE);
  m_MapexBuffer.Create();

  m_SelectionLayer.SetWidth(100);
  m_SelectionLayer.SetHeight(100);
  m_SelectionLayer.SetAlphaChannel(TRUE);
  m_SelectionLayer.Create();
  m_SelectionLayer.Fill(RGB32(335, 106, 255));
  LPBYTE pAlpha = m_SelectionLayer.GetAlphaChannel();
  memset(pAlpha, 80, 100 * 100);
  m_SelectionLayer.ReleaseAlphaChannel(TRUE);

  m_HorizontalGridLine.SetWidth(100);
  m_HorizontalGridLine.SetHeight(1);
  m_HorizontalGridLine.SetAlphaChannel(TRUE);
  m_HorizontalGridLine.Create();
  m_HorizontalGridLine.Fill(RGB32(0, 255, 0));
  pAlpha = m_HorizontalGridLine.GetAlphaChannel();
  memset(pAlpha, 80, 100 * 1);
  m_HorizontalGridLine.ReleaseAlphaChannel(TRUE);

  m_VerticalGridLine.SetWidth(1);
  m_VerticalGridLine.SetHeight(100);
  m_VerticalGridLine.SetAlphaChannel(TRUE);
  m_VerticalGridLine.Create();
  m_VerticalGridLine.Fill(RGB32(0, 255, 0));
  pAlpha = m_VerticalGridLine.GetAlphaChannel();
  memset(pAlpha, 80, 1 * 100);
  m_VerticalGridLine.ReleaseAlphaChannel(TRUE);

  m_LandTypeBuffer.SetWidth(16);
  m_LandTypeBuffer.SetHeight(16);
  m_LandTypeBuffer.Create();

  m_HorizontalSelectionLine.SetWidth(100);
  m_HorizontalSelectionLine.SetHeight(1);
  m_HorizontalSelectionLine.SetAlphaChannel(TRUE);
  m_HorizontalSelectionLine.Create();
  m_HorizontalSelectionLine.Fill(RGB32(255, 0, 0));
  pAlpha = m_HorizontalSelectionLine.GetAlphaChannel();
  memset(pAlpha, 150, 100 * 1);
  m_HorizontalSelectionLine.ReleaseAlphaChannel(TRUE);

  m_VerticalSelectionLine.SetWidth(1);
  m_VerticalSelectionLine.SetHeight(100);
  m_VerticalSelectionLine.SetAlphaChannel(TRUE);
  m_VerticalSelectionLine.Create();
  m_VerticalSelectionLine.Fill(RGB32(255, 0, 0));
  pAlpha = m_VerticalSelectionLine.GetAlphaChannel();
  memset(pAlpha, 150, 1 * 100);
  m_VerticalSelectionLine.ReleaseAlphaChannel(TRUE);

  m_pSelection = NULL;
}

void CEMap::DeleteSpecialBuffers()
{
  m_MapexBuffer.Delete();
  m_SelectionLayer.Delete();
  m_HorizontalGridLine.Delete();
  m_VerticalGridLine.Delete();
  m_LandTypeBuffer.Delete();
  m_HorizontalSelectionLine.Delete();
  m_VerticalSelectionLine.Delete();
}

CMapexInstanceSelection * CEMap::GetSelection()
{
  return m_pSelection;
}

void CEMap::SetSelection(CMapexInstanceSelection *pSelection)
{
  m_pSelection = pSelection;
}

CEMapexInstance * CEMap::GetMapexFromCell(DWORD dwX, DWORD dwY, CEMapexInstance *pPrevSel)
{
  ASSERT(dwX < m_dwWidth); ASSERT(dwY < m_dwHeight);

  // first get all mapexes instance which can touch the given coords
  // in the level
  CTypedPtrList<CPtrList, CEMapexInstance *> listMapexInstances;

  CTypedPtrList<CPtrList, CEMapexInstance *> listPosibles;

  CEMapexInstance *pResult = NULL;

  DWORD dwLevel;
  POSITION pos;
  CEMapexInstance *pMapexInstance;
  for(dwLevel = 3; dwLevel >= 1; dwLevel--){

    if((dwLevel == 1) && (!m_bBL1Visible)) continue;
    if((dwLevel == 2) && (!m_bBL2Visible)) continue;
    if((dwLevel == 3) && (!m_bBL3Visible)) continue;

    listMapexInstances.RemoveAll();
    // get all mapexes
    GetMapSquareFromCell(dwX, dwY)->AppendBLMapexes(dwLevel, &listMapexInstances);
    if(dwX >= MAPSQUARE_WIDTH){
      GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY)->AppendBLMapexes(dwLevel, &listMapexInstances);
      if(dwY >= MAPSQUARE_HEIGHT){
        GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY - MAPSQUARE_HEIGHT)->AppendBLMapexes(dwLevel, &listMapexInstances);
      }
    }
    if(dwY >= MAPSQUARE_HEIGHT){
      GetMapSquareFromCell(dwX, dwY - MAPSQUARE_HEIGHT)->AppendBLMapexes(dwLevel, &listMapexInstances);
    }

    // go through them and find the one which is crossing our coords
    // iterate backwards to do it same as the drawing works
    pos = listMapexInstances.GetTailPosition();
    while(pos != NULL){
      pMapexInstance = listMapexInstances.GetPrev(pos);
      if(pMapexInstance->PtInMapex(dwX, dwY)){
        pResult = pMapexInstance;
        listPosibles.AddTail(pResult);
        // if the land type is not transparent -> return it
        // if not try to find other below
      }
    }
  }

  if(pPrevSel == NULL){
    // try to find the first one to select
    pos = listPosibles.GetHeadPosition();
    while(pos != NULL){
      pMapexInstance = listPosibles.GetNext(pos);
      if(pMapexInstance->GetMapex()->GetLandType(dwX - pMapexInstance->GetXPosition(),
        dwY - pMapexInstance->GetYPosition()) != 0){
        return pMapexInstance;
      }
    }
  }

  // try to find the prev sel in those posibles
  pos = listPosibles.GetHeadPosition();
  while(pos != NULL){
    pMapexInstance = listPosibles.GetNext(pos);
    if(pMapexInstance == pPrevSel){
      if(pos != NULL)
        return listPosibles.GetAt(pos);
      else{
        return listPosibles.GetHead();
      }
    }
  }
  // not found
  if(listPosibles.GetHeadPosition() != NULL)
    return listPosibles.GetHead();
  return NULL;
}

CEULMapexInstance * CEMap::GetULMapexFromCell(DWORD dwX, DWORD dwY)
{
  ASSERT(dwX < m_dwWidth); ASSERT(dwY < m_dwHeight);

  // first get all mapexes instance which can touch the given coords
  // in the level
  CTypedPtrList<CPtrList, CEULMapexInstance *> listMapexInstances;
  POSITION pos;
  CEULMapexInstance *pMapexInstance;
  CEULMapexInstance *pResult = NULL;

  listMapexInstances.RemoveAll();
  // get all mapexes
  GetMapSquareFromCell(dwX, dwY)->AppendULMapexes(&listMapexInstances);
  if(dwX >= MAPSQUARE_WIDTH){
    GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY)->AppendULMapexes(&listMapexInstances);
    if(dwY >= MAPSQUARE_HEIGHT){
      GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY - MAPSQUARE_HEIGHT)->AppendULMapexes(&listMapexInstances);
    }
  }
  if(dwY >= MAPSQUARE_HEIGHT){
    GetMapSquareFromCell(dwX, dwY - MAPSQUARE_HEIGHT)->AppendULMapexes(&listMapexInstances);
  }

  // go through them and find the one which is crossing our coords
  // iterate backwards to do it same as the drawing works
  pos = listMapexInstances.GetTailPosition();
  while(pos != NULL){
    pMapexInstance = listMapexInstances.GetPrev(pos);
    if(pMapexInstance->PtInMapex(dwX, dwY)){
      pResult = pMapexInstance;
      // if the land type is not transparent -> return it
      // if not try to find other below
      if(pMapexInstance->GetMapex()->GetLandType(dwX - pMapexInstance->GetXPosition(),
        dwY - pMapexInstance->GetYPosition()) != 0){
        return pResult;
      }
    }
  }
  return pResult;
}

CEUnit * CEMap::GetUnitFromCell(DWORD dwX, DWORD dwY)
{
  ASSERT(dwX < m_dwWidth); ASSERT(dwY < m_dwHeight);

  // first get all units which can touch the given coords
  // in the level
  CTypedPtrList<CPtrList, CEUnit *> listUnitInstances;
  POSITION pos;
  CEUnit *pUnitInstance;
  CEUnit *pResult = NULL;

  listUnitInstances.RemoveAll();
  // get all mapexes
  GetMapSquareFromCell(dwX, dwY)->AppendUnits(&listUnitInstances);
  if(dwX >= MAPSQUARE_WIDTH){
    GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY)->AppendUnits(&listUnitInstances);
    if(dwY >= MAPSQUARE_HEIGHT){
      GetMapSquareFromCell(dwX - MAPSQUARE_WIDTH, dwY - MAPSQUARE_HEIGHT)->AppendUnits(&listUnitInstances);
    }
  }
  if(dwY >= MAPSQUARE_HEIGHT){
    GetMapSquareFromCell(dwX, dwY - MAPSQUARE_HEIGHT)->AppendUnits(&listUnitInstances);
  }

  // go through them and find the one which is crossing our coords
  // iterate backwards to do it same as the drawing works
  pos = listUnitInstances.GetTailPosition();
  while(pos != NULL){
    pUnitInstance = listUnitInstances.GetPrev(pos);
    if(pUnitInstance->PtInUnit(dwX, dwY)){
      pResult = pUnitInstance;
      return pResult;
    }
  }
  return pResult;
}

BOOL CEMap::IsDeleted()
{
  return m_bDeleted;
}

BOOL CEMap::IsModified()
{
  return m_bModified;
}

void CEMap::SetModified(BOOL bModified)
{
  m_bModified = bModified;
}

BOOL CEMap::GetDrawGrid()
{
  return m_bDrawGrid;
}

void CEMap::SetDrawGrid(BOOL bDraw)
{
  m_bDrawGrid = bDraw;
}

DWORD CEMap::GetGridSize()
{
  return m_dwGridSize;
}

void CEMap::SetGridSize(DWORD dwGridSize)
{
  m_dwGridSize = dwGridSize;
}

void CEMap::LockSquare(CEMapSquare *pSquare)
{
RetryIt:
  ;
  m_semaphoreSquareLock.Lock();
  if(!pSquare->m_bLock){
    pSquare->m_bLock = TRUE;
    m_semaphoreSquareLock.Unlock();
    return;
  }
  m_semaphoreSquareLock.Unlock();

  // if the lock is unavailable -> wait for next square release to check it again
  m_eventSquareLockReleased.Lock();
  goto RetryIt;
}

void CEMap::ReleaseSquare(CEMapSquare *pSquare)
{
  m_semaphoreSquareLock.Lock();
  if(pSquare->m_bLock){
    pSquare->m_bLock = FALSE; // lets others use it
    m_eventSquareLockReleased.PulseEvent(); // notify about our change
  }
  m_semaphoreSquareLock.Unlock();
}

void CEMap::GetExclusiveAccess()
{
  m_semaphoreExlusive.Lock();
}

void CEMap::ReleaseExclusiveAccess()
{
  m_semaphoreExlusive.Unlock();
}

void CEMap::CreateMiniMap()
{
  m_MiniMap.SetWidth(MINIMAP_WIDTH);
  m_MiniMap.SetHeight(MINIMAP_HEIGHT);
  m_MiniMap.Create();
  m_MiniMap.Fill(RGB32(0, 0, 0));

  m_MiniMapBuffer.SetWidth(MINIMAP_WIDTH);
  m_MiniMapBuffer.SetHeight(MINIMAP_HEIGHT);
  m_MiniMapBuffer.Create();
}

void CEMap::DeleteMiniMap()
{
  m_MiniMapBuffer.Delete();
  m_MiniMap.Delete();
}

CDDrawSurface * CEMap::GetMiniMap()
{
  m_semaphoreMiniMap.Lock();
  return &m_MiniMap;
}

void CEMap::ReleaseMiniMap()
{
  m_semaphoreMiniMap.Unlock();
}

CRect CEMap::GetMiniMapRect()
{
  return m_rcMiniMap;
}

void CEMap::SetVisibleRect(CRect rcRect)
{
  if(rcRect.right > (int)m_dwWidth) rcRect.OffsetRect(m_dwWidth - rcRect.right, 0);
  if(rcRect.bottom > (int)m_dwHeight) rcRect.OffsetRect(0, m_dwHeight - rcRect.bottom);
  if(rcRect.left < 0) rcRect.OffsetRect(-rcRect.left, 0);
  if(rcRect.top < 0) rcRect.OffsetRect(0, -rcRect.top);

  m_pMiniMap->SetVisibleRect(rcRect);
}

void CEMap::SetMiniMapRect(CRect rcRect)
{
  CRect rcIntr, rcUpdate;

  if(rcRect.right > (int)m_dwWidth) rcRect.OffsetRect(m_dwWidth - rcRect.right, 0);
  if(rcRect.bottom > (int)m_dwHeight) rcRect.OffsetRect(0, m_dwHeight - rcRect.bottom);
  if(rcRect.left < 0) rcRect.OffsetRect(-rcRect.left, 0);
  if(rcRect.top < 0) rcRect.OffsetRect(0, -rcRect.top);

  rcIntr.IntersectRect(&m_rcMiniMap, &rcRect);
  CPoint ptTrans;
  ptTrans.x = rcRect.left - m_rcMiniMap.left;
  ptTrans.y = rcRect.top - m_rcMiniMap.top;
/*  ptTrans.x = rcIntr.left - m_rcMiniMap.left;
  ptTrans.y = rcIntr.top - m_rcMiniMap.top;
  if(ptTrans.x == 0) ptTrans.x = rcRect.left - rcIntr.left;
  if(ptTrans.y == 0) ptTrans.y = rcRect.top - rcIntr.top;*/
  // ptTrans is now the transition of the map
  // if its positive the new rect (rcRect) is down-right from the old rect (m_rcMiniMap)
  // and to the other sides as well

  LockMiniMapMove();

  int nZoom = (int)m_pMiniMap->GetZoom();
  // now copy the graphics in the map
  CDDrawSurface *pMiniMap = GetMiniMap();
  rcUpdate.left = (rcIntr.left - m_rcMiniMap.left) / nZoom;
  rcUpdate.top = (rcIntr.top - m_rcMiniMap.top) / nZoom;
  rcUpdate.right = (rcIntr.right - m_rcMiniMap.left) / nZoom;
  rcUpdate.bottom = (rcIntr.bottom - m_rcMiniMap.top) / nZoom;
  CRect rcBuf(0, 0, rcUpdate.Width(), rcUpdate.Height());
  m_MiniMapBuffer.Paste(0, 0, pMiniMap, &rcUpdate);
  pMiniMap->Paste(rcUpdate.left - (ptTrans.x / nZoom), rcUpdate.top - (ptTrans.y / nZoom),
    &m_MiniMapBuffer, &rcBuf);
  ReleaseMiniMap();

  if(ptTrans.x < 0){
    rcUpdate.left = rcRect.left;
    rcUpdate.right = m_rcMiniMap.left;
    if(ptTrans.y < 0){
      rcUpdate.top = rcRect.top;
      rcUpdate.bottom = rcRect.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
      rcUpdate.bottom = m_rcMiniMap.top;
      rcUpdate.left = m_rcMiniMap.left;
      rcUpdate.right = rcRect.right;
      m_pMiniMap->UpdateRect(rcUpdate);
    }
    else{
      rcUpdate.top = rcRect.top;
      rcUpdate.bottom = rcRect.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
      rcUpdate.left = m_rcMiniMap.left;
      rcUpdate.right = rcRect.right;
      rcUpdate.top = m_rcMiniMap.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
    }
  }
  else{
    rcUpdate.left = m_rcMiniMap.right;
    rcUpdate.right = rcRect.right;
    if(ptTrans.y < 0){
      rcUpdate.top = rcRect.top;
      rcUpdate.bottom = rcRect.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
      rcUpdate.left = rcRect.left;
      rcUpdate.right = m_rcMiniMap.right;
      rcUpdate.bottom = m_rcMiniMap.top;
      m_pMiniMap->UpdateRect(rcUpdate);
    }
    else{
      rcUpdate.top = rcRect.top;
      rcUpdate.bottom = rcRect.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
      rcUpdate.left = rcRect.left;
      rcUpdate.right = m_rcMiniMap.right;
      rcUpdate.top = m_rcMiniMap.bottom;
      m_pMiniMap->UpdateRect(rcUpdate);
    }
  }
  m_rcMiniMap = rcRect;
  UnlockMiniMapMove();

//  m_pMiniMap->UpdateRect(m_rcMiniMap);
}

void CEMap::SetMiniMap(CMiniMap *pMiniMap)
{
  m_pMiniMap = pMiniMap;
}

BOOL CEMap::SetSelectionRectangle(CRect *pSelection)
{
  BOOL bDiffers = TRUE;
  if(pSelection == NULL){
    if(m_rcSelection.IsRectEmpty()) bDiffers = FALSE;
  }
  else{
    if(pSelection->EqualRect(&m_rcSelection)) bDiffers = FALSE;
  }
  if(pSelection == NULL)
    m_rcSelection.SetRectEmpty();
  else
    m_rcSelection = *pSelection;

  return bDiffers;
}

void CEMap::SetULSelection(CEULMapexInstance *pSel)
{
  m_pULSelection = pSel;
}

void CEMap::SetUnitSelection(CEUnit *pUnit)
{
  m_pUnitSelection = pUnit;
}

void CEMap::AddCivilization(CECivilization *pCivilization)
{
  m_listCivilizations.AddTail(pCivilization);
}

void CEMap::DeleteCivilization(CECivilization *pCivilization)
{
  POSITION pos;
  CECivilization *pCiv;
  pos = m_listCivilizations.GetHeadPosition();
  while(pos != NULL){
    pCiv = m_listCivilizations.GetAt(pos);
    if(pCiv == pCivilization){
      m_listCivilizations.RemoveAt(pos);
      break;
    }
    m_listCivilizations.GetNext(pos);
  }
  pCivilization->Delete();
  delete pCivilization;
}

CECivilization * CEMap::GetCivilization(DWORD dwID)
{
  POSITION pos;
  CECivilization *pCiv;
  pos = m_listCivilizations.GetHeadPosition();
  while(pos != NULL){
    pCiv = m_listCivilizations.GetAt(pos);
    if(pCiv->GetID() == dwID) return pCiv;
    m_listCivilizations.GetNext(pos);
  }
  return NULL;
}

void CEMap::InsertUnit(CEUnit *pUnit)
{
  // first insert unit to the list of all units
  m_listUnits.AddHead(pUnit);

  // find the map square to insert the unit to
  GetMapSquareFromCell(pUnit->GetXPos(), pUnit->GetYPos())->AddUnit(pUnit);
}

void CEMap::DeleteUnit(CEUnit *pUnit)
{
  // find and remove it from our list
  POSITION pos;
  pos = m_listUnits.GetHeadPosition();
  while(pos != NULL){
    if(m_listUnits.GetAt(pos) == pUnit){
      m_listUnits.RemoveAt(pos);
      break;
    }
    m_listUnits.GetNext(pos);
  }

  // remove it from the map square
  GetMapSquareFromCell(pUnit->GetXPos(), pUnit->GetYPos())->DeleteUnit(pUnit);
}

void CEMap::RebuildLibraries()
{
  try{
    // first go through all mapex libraries
    {
      POSITION pos;
      CEMapexLibrary *pMapexLibrary;

      pos = m_listLibraries.GetHeadPosition();
      while(pos != NULL){
        pMapexLibrary = m_listLibraries.GetNext(pos);
        pMapexLibrary->Rebuild();
      }
    }

    // then rebuild unittype libraries
    {
      POSITION pos;
      CEUnitLibrary *pUnitTypeLibrary;

      pos = m_listUnitLibraries.GetHeadPosition();
      while(pos != NULL){
        pUnitTypeLibrary = m_listUnitLibraries.GetNext(pos);
        pUnitTypeLibrary->Rebuild();
      }
    }
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
  }
}

BOOL CEMap::IsCivUnique(CECivilization *pCiv)
{
  CECivilization *pC;
  POSITION pos;

  pos = m_listCivilizations.GetHeadPosition();
  while(pos != NULL){
    pC = m_listCivilizations.GetNext(pos);

    if(pC == pCiv) continue;
    if(!pC->GetName().CompareNoCase(pCiv->GetName()))
      return FALSE;
    if(pC->GetColor() == pCiv->GetColor()) return FALSE;
  }
  return TRUE;
}

BOOL CEMap::CheckValid()
{
  // check unit types
  {
    POSITION pos;
    CEUnitType *pUnitType;
    DWORD dwKey;

    pos = m_mapUnitTypes.GetStartPosition();
    while(pos != NULL){
      m_mapUnitTypes.GetNextAssoc(pos, dwKey, pUnitType);
      if(!pUnitType->CheckValid()) return FALSE;
    }
  }

  // check mapexes
  {
    POSITION pos;
    CEMapex *pMapex;
    DWORD dwKey;

    pos = m_mapMapexes.GetStartPosition();
    while(pos != NULL){
      m_mapMapexes.GetNextAssoc(pos, dwKey, pMapex);

      if(!pMapex->CheckValid()) return FALSE;
    }
  }

  // check civilizations
  {
    POSITION pos;
    CECivilization *pCiv;

    pos = m_listCivilizations.GetHeadPosition();
    while(pos != NULL){
      pCiv = m_listCivilizations.GetNext(pos);
      if(!pCiv->CheckValid()) return FALSE;
    }
  }

  // check resources
  {
    int i;
    for(i = 0; i < RESOURCE_COUNT; i++){
      if(!m_aResources[i]->CheckValid()) return FALSE;
      if(!m_aResources[i]->GetName().IsEmpty()){
        int j;
        for(j = 0; j < RESOURCE_COUNT; j++){
          if((j != i) && (!m_aResources[j]->GetName().IsEmpty())){
            if(m_aResources[i]->GetName() == m_aResources[j]->GetName()){
              AfxMessageBox("Suroviny nesmìjí mít stejná jména.");
              return FALSE;
            }
          }
        }
      }
    }
  }

  // check visibilities
  {
    int i;
    for(i = 0; i < INVISIBILITY_COUNT; i++){
      if(!m_aInvisibilities[i]->CheckValid()) return FALSE;
      if(!m_aInvisibilities[i]->GetName().IsEmpty()){
        int j;
        for(j = 0; j < INVISIBILITY_COUNT; j++){
          if((j != i) && (!m_aInvisibilities[j]->GetName().IsEmpty())){
            if(m_aInvisibilities[j]->GetName() == m_aInvisibilities[i]->GetName()){
              AfxMessageBox("Viditelnosti nesmìjí mít stejná jména.");
              return FALSE;
            }
          }
        }
      }
    }
  }

  // Check unit instances
  {
RetryUnitInstances:;
    POSITION pos = m_listUnits.GetHeadPosition();
    CEUnit *pUnit;
    while(pos != NULL){
      pUnit = m_listUnits.GetNext(pos);

      // If the unit is not valid -> remove it from the map
      if(!pUnit->CheckValid()){
        CEMapSquare *pSquare;
        pSquare = GetMapSquareFromCell(pUnit->GetXPos(), pUnit->GetYPos());
        LockSquare(pSquare);
        DeleteUnit(pUnit);
        ReleaseSquare(pSquare);

        pUnit->Delete();
        delete pUnit;
        goto RetryUnitInstances;
      }
    }
  }

  // Check script sets
  {
    POSITION pos1, pos2;
    CEScriptSet *pSet1, *pSet2;
    pos1 = m_listScriptSet.GetHeadPosition();
    while(pos1 != NULL){
      pSet1 = m_listScriptSet.GetNext(pos1);
      pos2 = m_listScriptSet.GetHeadPosition();
      while(pos2 != NULL){
        pSet2 = m_listScriptSet.GetNext(pos2);
        if((pSet2 != pSet1) && (pSet2->GetFileName().CompareNoCase(pSet1->GetFileName()) == 0)){
          AfxMessageBox("Dva zadané skriptsety nesmìjí ukazujé na stejný soubor, nebo na soubory, které se stejnì jmenují.");
          return FALSE;
        }
      }
    }
  }


  return TRUE;
}

CEScriptSet *CEMap::GetScriptSet(DWORD dwID)
{
  POSITION pos = m_listScriptSet.GetHeadPosition();
  CEScriptSet *pScriptSet = NULL;
  while(pos != NULL){
    pScriptSet = m_listScriptSet.GetNext(pos);
    if ( pScriptSet->GetID () == dwID ) break;
  }
  return pScriptSet;
}

void CEMap::DeleteScriptSet(CEScriptSet *pScriptSet)
{
  POSITION pos = m_listScriptSet.Find(pScriptSet);
  if(pos != NULL){
    m_listScriptSet.RemoveAt(pos);
  }

  // Notify civilizations
  CECivilization *pCiv;
  pos = m_listCivilizations.GetHeadPosition();
  while(pos != NULL){
    pCiv = m_listCivilizations.GetNext(pos);

    if(pCiv->GetScriptSet() == pScriptSet){
      pCiv->SetScriptSet(NULL);
    }
  }

  pScriptSet->Delete();
  delete pScriptSet;
}

void CEMap::AddScriptSet(CEScriptSet *pScriptSet)
{
  m_listScriptSet.AddHead(pScriptSet);
}

void CEMap::DeleteFindPathGraph(CEFindPathGraph *pGraph)
{
  POSITION pos = m_listFindPathGraphs.Find(pGraph);
  if(pos != NULL){
    m_listFindPathGraphs.RemoveAt(pos);
  }

  pGraph->Delete();
  delete pGraph;
}

void CEMap::AddFindPathGraph(CEFindPathGraph *pGraph)
{
  m_listFindPathGraphs.AddHead(pGraph);
}

// Changes the size of the map
// the given sizes is in mapcells and must be a multiple of map square
// size
void CEMap::SetSize ( DWORD dwWidth, DWORD dwHeight )
{
  ASSERT ( dwWidth % MAPSQUARE_WIDTH == 0 );
  ASSERT ( dwHeight % MAPSQUARE_HEIGHT == 0 );
  ASSERT ( dwWidth > 0 );
  ASSERT ( dwHeight > 0 );

  DWORD dwNewSquareWidth = dwWidth / MAPSQUARE_WIDTH, dwNewSquareHeight = dwHeight / MAPSQUARE_HEIGHT;
  // First we'll allocate new array of mapsquares
  CEMapSquare * * pNewMapSquares = ( CEMapSquare ** ) new BYTE [ sizeof ( CEMapSquare *) * dwNewSquareWidth * dwNewSquareHeight ];
  memset ( pNewMapSquares, 0, sizeof ( CEMapSquare *) * dwNewSquareWidth * dwNewSquareHeight );

  // Now copy all mapsquares that can stay the same
  {
    DWORD dwX, dwY;
    DWORD dwXSize = m_dwSquareWidth, dwYSize = m_dwSquareHeight;
    if ( dwXSize > dwNewSquareWidth ) dwXSize = dwNewSquareWidth;
    if ( dwYSize > dwNewSquareHeight ) dwYSize = dwNewSquareHeight;

    for ( dwY = 0; dwY < dwYSize; dwY ++ )
    {
      for ( dwX = 0; dwX < dwXSize; dwX ++ )
      {
        // Just copy the pointer
        pNewMapSquares [ dwX + dwY * dwNewSquareWidth ] = m_pMapSquares [ dwX + dwY * m_dwSquareWidth ];
        // And write NULL to the old array
        m_pMapSquares [ dwX + dwY * m_dwSquareWidth ] = NULL;
      }
    }
  }

  // Now go through old mapsquare (the ones that remains in the old array)
  // and delete them, all mapexes in them and also all units in them
  {
    DWORD dwX, dwY;
    CEMapSquare * pMapSquare;
    CTypedPtrList< CPtrList, CEUnit * > listUnits;
    for ( dwY = 0; dwY < m_dwSquareHeight; dwY ++ )
    {
      for ( dwX = 0; dwX < m_dwSquareWidth; dwX ++ )
      {
        pMapSquare = m_pMapSquares [ dwX + dwY * m_dwSquareWidth ];
        if ( pMapSquare != NULL )
        {
          // Delete all units owned by the mapsquare
          listUnits.RemoveAll ();
          pMapSquare->AppendUnits ( &listUnits );

          POSITION pos = listUnits.GetHeadPosition ();
          CEUnit * pUnit;
          while ( pos != NULL )
          {
            pUnit = listUnits.GetNext ( pos );
            // Just remove it from our list
            POSITION pos2 = m_listUnits.GetHeadPosition ();
            while ( pos2 != NULL )
            {
              if ( m_listUnits.GetAt ( pos2 ) == pUnit )
              {
                m_listUnits.RemoveAt ( pos2 );
                break;
              }
              m_listUnits.GetNext ( pos2 );
            }
            ASSERT ( pos2 != NULL );
            // Delete the unit itself
            pUnit->Delete ();
            delete pUnit;
          }

          // Delete the mapsquare itself
          // this will also delete all mapex instances in the mapsquare
          pMapSquare->Delete ();
          delete pMapSquare;
        }
      }
    }
  }

  // Now create new mapsquares if some is needed
  {
    DWORD dwX, dwY;
    CEMapSquare * pMapSquare = NULL;
    for ( dwY = 0; dwY < dwNewSquareHeight; dwY ++ )
    {
      for ( dwX = 0; dwX < dwNewSquareWidth; dwX ++ )
      {
        // If there is a NULL there -> need new mapsquare
        if ( pNewMapSquares [ dwX + dwY * dwNewSquareWidth ] == NULL )
        {
          // Create the new mapsquare
          pMapSquare = new CEMapSquare ();
          pMapSquare->Create ();

          // Write it to the array
          pNewMapSquares [ dwX + dwY * dwNewSquareWidth ] = pMapSquare;
        }
      }
    }
  }

  // Now delete the old table
  delete m_pMapSquares;

  // And remember the new one
  m_pMapSquares = pNewMapSquares;

  // And now overwrite the sizes
  m_dwWidth = dwWidth; m_dwHeight = dwHeight;
  m_dwSquareWidth = m_dwWidth / MAPSQUARE_WIDTH;
  m_dwSquareHeight = m_dwHeight / MAPSQUARE_HEIGHT;
}
