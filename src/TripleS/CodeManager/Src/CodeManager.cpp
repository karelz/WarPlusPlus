// CodeManager.cpp: implementation of the CCodeManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../CodeManager.h"

#include "LexDefaultUnit.h"

#include "GameServer\Civilization\ZCivilization.h"
#include "GameServer\GameServer\SFileManager.h"

#include "Archive\Archive\DataArchiveDirContents.h"
#include "Archive\Archive\DataArchiveFileDirectoryItem.h"

// parser fot the interface of default unit
int SDefUnitparse(void *pCodeMan);

// Defaultni jednotka (typ ve skriptu)
CIUnitType *CCodeManager::m_pDefaultUnitType = NULL;

// Pocet CodeManageru (kdyz klesne na 0, defaultni jednotka se musi uvolnit)
LONG CCodeManager::m_nCodeManagersCount = 0;
CIUnitType* CCodeManager::m_pHelperDefaultUnit = NULL;
// aktualni metoda
CIMethod* CCodeManager::m_pDefaultUnitMethod = NULL;
// aktualni typu argumentu metody
CArray<CIType, CIType&> CCodeManager::m_aDefaultUnitMethodArgs;
// Lexikalni analyzator
CLexDefaultUnit* CCodeManager::m_pLexDefaultUnit = NULL; 

//////////////////////////////////////////////////////////////////////

CStringTableItem* LowerCase( CStringTableItem *pName, bool bAddRef = false)
{
    CString str = (CString)*pName;
    str.MakeLower();
    return g_StringTable.AddItem( str, bAddRef);
}

//////////////////////////////////////////////////////////////////////
// CCodeManager::CUnitInheritanceInfo
//////////////////////////////////////////////////////////////////////

CCodeManager::CUnitInheritanceInfo::CUnitInheritanceInfo(CStringTableItem *stiUnitName, 
        CUnitInheritanceInfo *pParent, bool bDefined)
{
    m_stiUnitName = stiUnitName->AddRef();

    ASSERT( m_pParent != NULL);
    m_pParent = pParent;
    m_bDefined = bDefined;
}

//////////////////////////////////////////////////////////////////////

CCodeManager::CUnitInheritanceInfo::~CUnitInheritanceInfo()
{
    m_stiUnitName->Release();
}

//////////////////////////////////////////////////////////////////////
// CCodeManager::CScript
//////////////////////////////////////////////////////////////////////

CCodeManager::CScript::CScript()
{
    m_stiName = NULL;
    m_eScriptType = SCRIPT_UNKNOWN;
}

CCodeManager::CScript::~CScript()
{
    if (m_stiName!=NULL) m_stiName->Release();
}

//////////////////////////////////////////////////////////////////////
// CCodeManager
//////////////////////////////////////////////////////////////////////

CCodeManager::CCodeManager()
{
    m_mutexUnitInheritanceInfo.Lock();
    m_tpUnitInheritanceInfo.InitHashTable(UNITINHERITANCE_TABLE_SIZE);
    m_mutexUnitInheritanceInfo.Unlock();

	m_ptpPersistentLoadScripts = NULL;

	// inicializace hashovaci tabulky m_tpScripts
    m_mutexScripts.Lock();
    m_tpScripts.InitHashTable(SCRIPTS_HASHTABLE_SIZE);
    m_mutexScripts.Unlock();

    m_pZCivilization = NULL;

    m_bCreated = false;

	// increment codemanager count
    ::InterlockedIncrement( &m_nCodeManagersCount);
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::~CCodeManager()
{
   if (m_bCreated) 
   {
	   Delete();
   }

   // decrement codemanager count
   if (::InterlockedDecrement( &m_nCodeManagersCount) == 0)
   {
       ASSERT( CCodeManager::m_pDefaultUnitType != NULL);
       delete CCodeManager::m_pDefaultUnitType;
       CCodeManager::m_pDefaultUnitType = NULL;
   }
}

//////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CCodeManager::AssertValid() const
{
    CNotifier::AssertValid();
    ASSERT(m_bCreated);
}
#endif
//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::Create(CString &strUserScriptsPath, CString &strScriptSetPath, CZCivilization *pZCivilization)
{
    ASSERT(!m_bCreated);
    //ASSERT( pZCivilization != NULL); kdyz je code manager pouzity v ScriptSetMaker, pak zadna civilizace neni

    // inicializuj notifiera a napoj se na interpret
    if (!CNotifier::Create()) throw new CStringException("Cannot create CodeManager.");

    DWORD attr;

    m_CodeManagerForCompiler.Init( this);

// CESTY
	strUserScriptsPath.TrimRight();
	strUserScriptsPath.TrimRight('\\');

    ::CreateDirectory( strUserScriptsPath, NULL);
    if ((attr=::GetFileAttributes(strUserScriptsPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
	    throw new CStringException("Cannot create directory %s.", strUserScriptsPath);

    // cesta k user bytekodum
    m_strCodeUserPath.Format("%s\\%s\\", strUserScriptsPath, DIRECTORY_BYTECODE);
    ::CreateDirectory( m_strCodeUserPath, NULL);
    if ((attr=::GetFileAttributes(m_strCodeUserPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
	    throw new CStringException("Cannot create directory %s.", m_strCodeUserPath);


    // cesta k user zdrojakum
    m_strSourceUserPath.Format("%s\\%s\\", strUserScriptsPath, DIRECTORY_SOURCE);
    ::CreateDirectory( m_strSourceUserPath, NULL);
    if ((attr=::GetFileAttributes(m_strSourceUserPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strSourceUserPath);

    // cesta k system bytekodum
    m_strCodeSystemPath.Format("%s\\%s\\", strScriptSetPath, DIRECTORY_BYTECODE);
    ::CreateDirectory( m_strCodeSystemPath, NULL);
    if ((attr=::GetFileAttributes(m_strCodeSystemPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strCodeSystemPath);

    // cesta k system zdrojakum
    m_strSourceSystemPath.Format("%s\\%s\\", strScriptSetPath, DIRECTORY_SOURCE);
    ::CreateDirectory( m_strSourceSystemPath, NULL);
    if ((attr=::GetFileAttributes(m_strSourceSystemPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strSourceSystemPath);

    // stejne adresare -> chyba
    if (m_strCodeUserPath==m_strCodeSystemPath) 
        throw new CStringException("The user and script set directories cannot be the same.");
    if (m_strSourceUserPath==m_strSourceSystemPath) 
        throw new CStringException("The user and script set source directories cannot be the same.");

    // copy toolbars to user directory
    {
      CDataArchive cDest;
      cDest.Create ( m_strSourceUserPath );
      CDataArchive cSource;
      cSource.Create ( m_strSourceSystemPath );

      cDest.AppendDir ( "Toolbars", cSource, "Toolbars", appendRecursive );
    }

	try
	{
		// naloadovani interface defaultni unity (pokud jiz neni naloadovana)
        CreateDefaultUnit();
		
		// vytvoreni informace o dedicnosti jednotek
 		CreateUnitInheritanceInfo();
	}
	catch (CStringException *e)
	{
		Delete();
		throw e;
	}

    // civilizace
    m_pZCivilization = pZCivilization;

    if (pZCivilization != NULL)
	{
		ASSERT( pZCivilization->m_pInterpret != NULL);
		Connect( pZCivilization->m_pInterpret, CODEMANAGER_NOTIFIER_ID);
	}

    m_bCreated = true;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::Delete()
{
    // uklid po padu pri loadovani hry
    if ( g_bGameLoading)
    {
        if ( m_ptpPersistentLoadScripts != NULL)
        {
            delete m_ptpPersistentLoadScripts;
            m_ptpPersistentLoadScripts = NULL;
        }
    }

    RemoveAll();

    // smazani informaci o dedicnosti
    POSITION pos;
    CStringTableItem *stiName;
    CUnitInheritanceInfo *pUIInfo;
    
    m_mutexUnitInheritanceInfo.Lock();

    pos=m_tpUnitInheritanceInfo.GetStartPosition();
    while (pos!=NULL)
    {
        m_tpUnitInheritanceInfo.GetNextAssoc(pos, stiName, pUIInfo);
        stiName->Release();
        delete pUIInfo;
    }
    m_tpUnitInheritanceInfo.RemoveAll();

    m_mutexUnitInheritanceInfo.Unlock();

    m_pZCivilization = NULL;

    // notifier
    CNotifier::Delete();

    m_bCreated = false;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::RemoveAll()
{
    POSITION pos;
    CStringTableItem *pSTI;
    CScript *pScript;

    m_mutexScripts.Lock();

    pos=m_tpScripts.GetStartPosition();
    while (pos!=NULL)
    {
        m_tpScripts.GetNextAssoc(pos, pSTI, pScript);
        pSTI->Release();

        switch (pScript->m_eScriptType)
        {
        case SCRIPT_UNIT:
			// defaultni jednotku neuvolnovat
			if ( pScript->m_pIUnitType != CCodeManager::m_pDefaultUnitType)
			{
				delete pScript->m_pIUnitType;
			}
            break;
        case SCRIPT_FUNCTION:
            delete pScript->m_pGlobalFunc;
            break;
        case SCRIPT_STRUCTURE:
            delete pScript->m_pStructType;
            break;
        }

        delete pScript;
    }
    m_tpScripts.RemoveAll();

    m_mutexScripts.Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::CreateUnitInheritanceInfo()
{
    ASSERT(!m_bCreated);

    // Zamek - cela metoda je zamcena !!!
    m_mutexUnitInheritanceInfo.Lock();

    // vytvorit strom dedicnosti
    WIN32_FIND_DATA FD;
    HANDLE hFindFile;
    CFile file;
    CFileException *pFE=new CFileException();
    short length,i;
    char lpszName[MAX_IDENTIFIER_LENGTH+1], lpszParentName[MAX_IDENTIFIER_LENGTH+1];
    CStringTableItem *stiName, *stiParentName;
    CUnitInheritanceInfo  *pUIInfo, *pParentUIInfo;
    CString strPath, strCodeName;

    // vytvoreni zaznamu pro default unitu
    stiName = g_StringTable.AddItem(DEFAULT_UNIT_PARENT);
    pUIInfo = new CUnitInheritanceInfo(stiName, NULL, true);

    m_tpUnitInheritanceInfo.SetAt( LowerCase( stiName, true), pUIInfo);

    stiName->Release();

    for (i=0; i<2; i++)
    {
        if (i==0) strPath=m_strCodeUserPath;
        else strPath=m_strCodeSystemPath;
        
        hFindFile=::FindFirstFile(strPath+"*."FILE_CODE_EXTENSION_UNIT, &FD);
        while (hFindFile!=INVALID_HANDLE_VALUE)
        {
            if (file.Open(strPath+FD.cFileName, CFile::modeRead | CFile::shareDenyWrite, pFE))
            {
                // nacteni a kontrola hlavicky
                file.Read(lpszName, sizeof(UNITFILE_FIRTSBYTES)-1);
                lpszName[sizeof(UNITFILE_FIRTSBYTES)-1]=0;

                if (strcmp(lpszName, UNITFILE_FIRTSBYTES)==0)
                {
                    // nacteni jmena jednotky
                    file.Read(&length, sizeof(short));
                    file.Read(lpszName, length);
                    lpszName[length]=0;

                    // kontrola, zda jmeno jednotky je stejne jako
                    // jmeno souboru
                    strCodeName=FD.cFileName;
                    strCodeName=strCodeName.Left(strCodeName.Find("."));

                    if (strCodeName!=lpszName)
                    {
                        m_mutexUnitInheritanceInfo.Unlock();
                        throw new CStringException("File %s must have the same name as "
                        "the unit '%s' defined inside.", FD.cFileName, lpszName);
                    }

                    // nacteni jmena predka
                    file.Read(&length, sizeof(short));
                    file.Read(lpszParentName, length);
                    lpszParentName[length]=0;

                    // pridani do seznamu:
                    stiName=g_StringTable.AddItem(lpszName);
                    stiParentName=g_StringTable.AddItem(lpszParentName);
                  
                    // nalezeni/pridani rodice
                    if (!m_tpUnitInheritanceInfo.Lookup( LowerCase( stiParentName), pParentUIInfo))
                    {
                        pParentUIInfo=new CUnitInheritanceInfo(stiParentName, NULL, false);
                        m_tpUnitInheritanceInfo.SetAt( LowerCase( stiParentName, true), pParentUIInfo);
                    }

                    // nalezeni/pridani jednotky
                    if (m_tpUnitInheritanceInfo.Lookup( LowerCase( stiName), pUIInfo))
                    {
                        if (!pUIInfo->m_bDefined)
                        {
                            pUIInfo->m_bDefined=true;
                            pUIInfo->m_pParent=pParentUIInfo;
                        }
                        else 
                        {
                            m_mutexUnitInheritanceInfo.Unlock();
                            throw new CStringException("Unit '%s' already exists, duplicate found in %s.",
                                lpszName, FD.cFileName);
                        }
                    }
                    else 
                    {
                        pUIInfo=new CUnitInheritanceInfo(stiName, pParentUIInfo);
                        m_tpUnitInheritanceInfo.SetAt( LowerCase( stiName, true), pUIInfo);
                    }

                    stiName->Release();
                    stiParentName->Release();
                }

                file.Close();
            }
            else 
            {
                m_mutexUnitInheritanceInfo.Unlock();
                throw pFE; 
            }

            if (!::FindNextFile(hFindFile, &FD)) break;
        }
        ::FindClose(hFindFile);
    }
    
    pFE->Delete();

    // kontrola, zda nejsou ve stromu dedicnosti nedefinovane jednotky nebo cykly
    POSITION pos;
	CUnitInheritanceInfo *pUIInfoPom;

    pos=m_tpUnitInheritanceInfo.GetStartPosition();
    while (pos!=NULL)
    {
		// definovana
        m_tpUnitInheritanceInfo.GetNextAssoc(pos, stiName, pUIInfo);
        if (!pUIInfo->m_bDefined)
        {
		    m_mutexUnitInheritanceInfo.Unlock();
            throw new CStringException("undefined unit '%s' used as a parent unit", 
				(CString)*stiName);
        }

		// kontrola cyklu v dedicnosti
		pUIInfoPom = pUIInfo;
		i=50;
		while (i>0 && pUIInfoPom->m_stiUnitName != m_pDefaultUnitType->m_stiName)
		{
			pUIInfoPom = pUIInfoPom->m_pParent;
			ASSERT( pUIInfoPom != NULL);
		}

		if (i==0)
		{
		    m_mutexUnitInheritanceInfo.Unlock();
			throw new CStringException("circular unit inheritance (check unit '%s')", (CString)*pUIInfo->m_stiUnitName);
		}
    }

    // Odemceni cele metody
    m_mutexUnitInheritanceInfo.Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////

// TODO: smazat
extern CString g_strRootPath;

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::CreateDefaultUnit()
{
    ASSERT(!m_bCreated);

   	if ( CCodeManager::m_pDefaultUnitType == NULL)
    {
        CCodeManager::LoadDefaultUnit();
        ASSERT( CCodeManager::m_pDefaultUnitType != NULL);
    }
    
	// pridani do seznamu skriptu
	CScript *pScript = new CScript();
	pScript->m_eScriptType = SCRIPT_UNIT;
	pScript->m_pIUnitType = CCodeManager::m_pDefaultUnitType;
	pScript->m_stiName = CCodeManager::m_pDefaultUnitType->m_stiName->AddRef();

	m_mutexScripts.Lock();
	m_tpScripts.SetAt( LowerCase(CCodeManager::m_pDefaultUnitType->m_stiName, true), pScript);
	m_mutexScripts.Unlock();
}

////////////////////////////////////////////////////////////////////////////////////
// ACTIONS  -  metody volane behem parsovani souboru DEFAULTUNIT_FILE
// (to probiha pri vytvareni CodeManagera), naplnuji tridu m_pDefaultUnit.
////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitBegin(CString &strName)
{
    ASSERT(m_pHelperDefaultUnit != NULL);

    m_pHelperDefaultUnit->m_stiName = g_StringTable.AddItem(strName);
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitEnd()
{
    ASSERT(m_pHelperDefaultUnit != NULL);

    m_pHelperDefaultUnit->m_bLoaded = true;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitMember(EScriptType ScType, CString &strType, CString &strName, int nArrayLevel)
{
    ASSERT(m_pHelperDefaultUnit != NULL);

    CStringTableItem *stiName = g_StringTable.AddItem(strName);
    CStringTableItem *stiType = g_StringTable.AddItem(strType);
    CIMember *pMember;
    
    if (m_pHelperDefaultUnit->m_tpMembers.Lookup(stiName, pMember))
    {
        stiName->Release();
        throw new CStringException(DEFAULTUNIT_FILE"(%d): duplicate member name", 
            m_pLexDefaultUnit->GetLineNo());
    }

    pMember = new CIMember;

    switch (ScType)
    {
    case SCRIPT_STRUCTURE:
        pMember->m_DataType.SetStruct(stiType);
        break;
    case SCRIPT_OBJECT:
        ASSERT (GetSOTypeFromString((CString)*stiType) != SO_UNKNOWN);
        pMember->m_DataType.SetObject( GetSOTypeFromString((CString)*stiType));
        break;
    case SCRIPT_UNIT:
        pMember->m_DataType.SetUnit(stiType);
        break;
    case SCRIPT_UNKNOWN:
        pMember->m_DataType.Set(strType);
    }

    stiType->Release();

    if (nArrayLevel == -1) pMember->m_DataType.MakeSet();

    while (nArrayLevel>0)
	{
		pMember->m_DataType.MakeArray();
		nArrayLevel--;
	}

    pMember->m_nHeapOffset = (int)-1;

    m_pHelperDefaultUnit->m_tpMembers.SetAt(stiName, pMember);
    m_pHelperDefaultUnit->m_nMemberCount++;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitMethodBegin(EScriptType ScType, CString &strRetType, int nArrayLevel, CString &strName)
{
    ASSERT(m_pHelperDefaultUnit != NULL);

    CStringTableItem *stiName = g_StringTable.AddItem(strName);
    CStringTableItem *stiRetType = g_StringTable.AddItem(strRetType);

    
    if (m_pHelperDefaultUnit->m_tpMethods.Lookup(stiName, m_pDefaultUnitMethod))
    {
        stiName->Release();
        throw new CStringException(DEFAULTUNIT_FILE"(%d): duplicate method name", 
            m_pLexDefaultUnit->GetLineNo());
    }

    m_pDefaultUnitMethod = new CIMethod();

    m_pDefaultUnitMethod->m_stiName = stiName->AddRef();
    
    switch (ScType)
    {
    case SCRIPT_STRUCTURE:
        m_pDefaultUnitMethod->m_ReturnType.SetStruct(stiRetType);
        break;
    case SCRIPT_OBJECT:
        ASSERT (GetSOTypeFromString((CString)*stiRetType) != SO_UNKNOWN);
        m_pDefaultUnitMethod->m_ReturnType.SetObject( GetSOTypeFromString((CString)*stiRetType));
        break;
    case SCRIPT_UNIT:
        m_pDefaultUnitMethod->m_ReturnType.SetUnit(stiRetType);
        break;
    case SCRIPT_UNKNOWN:
        m_pDefaultUnitMethod->m_ReturnType.Set(strRetType);
    }

    if (nArrayLevel != 0)
    {
        // mnozina
        if (nArrayLevel == -1)
        {
            m_pDefaultUnitMethod->m_ReturnType.MakeSet();
        }
        // pole
        else
        {
            while (nArrayLevel > 0)
            {
                m_pDefaultUnitMethod->m_ReturnType.MakeArray();
                nArrayLevel--;
            }
        }
    }



    stiRetType->Release();

    m_pHelperDefaultUnit->m_tpMethods.SetAt(stiName, m_pDefaultUnitMethod);

    m_pHelperDefaultUnit->m_nMethodCount++;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitMethodEnd()
{
    int i;

    ASSERT(m_pHelperDefaultUnit != NULL);

    if (m_pDefaultUnitMethod->m_nArgCount!=NULL)
        m_pDefaultUnitMethod->m_apArgAndLocalVars = new CIType[m_pDefaultUnitMethod->m_nArgCount];
    else m_pDefaultUnitMethod->m_apArgAndLocalVars = NULL;
    for (i=0; i<m_pDefaultUnitMethod->m_nArgCount; i++)
        m_pDefaultUnitMethod->m_apArgAndLocalVars[i] = m_aDefaultUnitMethodArgs[i];

    m_pDefaultUnitMethod->m_nArgAndLocalVarCount = m_pDefaultUnitMethod->m_nArgCount;

    m_pDefaultUnitMethod = NULL;
}  

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::OnDefaultUnitArgument(EScriptType ScType, CString &strType, int nArrayLevel)
{
    ASSERT(m_pHelperDefaultUnit != NULL);

    CStringTableItem *stiType = g_StringTable.AddItem(strType);

    switch (ScType)
    {
    case SCRIPT_STRUCTURE:
        m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].SetStruct(stiType);
        break;
    case SCRIPT_OBJECT:
        ASSERT (GetSOTypeFromString((CString)*stiType) != SO_UNKNOWN);
        m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].SetObject( GetSOTypeFromString((CString)*stiType) );
        break;
    case SCRIPT_UNIT:
        m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].SetUnit(stiType);
        break;
    case SCRIPT_UNKNOWN:
        m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].Set(strType);
    }

    stiType->Release();

    if (nArrayLevel == -1) 
        m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].MakeSet();

    while (nArrayLevel>0)
	{
		m_aDefaultUnitMethodArgs[m_pDefaultUnitMethod->m_nArgCount].MakeArray();
		nArrayLevel--;
	}

    m_pDefaultUnitMethod->m_nArgCount++;
}

/// END OD ACTIONS ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

CIUnitType* CCodeManager::GetUnitType(CStringTableItem *stiName)
{
    CScript *pScript;

    m_mutexScripts.Lock();
    if (m_tpScripts.Lookup( LowerCase( stiName), pScript))
    {
        m_mutexScripts.Unlock();
        
        if (!pScript->IsUnit()) return NULL;
        return pScript->m_pIUnitType;
    }
    else
    {
        m_mutexScripts.Unlock();

        if ((pScript=LoadUnit(stiName))!=NULL) 
        {
            return pScript->m_pIUnitType;
        }
        else return NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

CIStructureType* CCodeManager::GetStructureType(CStringTableItem *stiName)
{
    CScript *pScript;

    m_mutexScripts.Lock();

    if (m_tpScripts.Lookup( LowerCase( stiName), pScript))
    {
        m_mutexScripts.Unlock();

        if (!pScript->IsStructure()) return NULL;
        return pScript->m_pStructType;
    }
    else
    {
        m_mutexScripts.Unlock();

        if ((pScript = LoadStructure(stiName)) == NULL) return NULL;
        else return pScript->m_pStructType;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

CIGlobalFunction* CCodeManager::GetGlobalFunction(CStringTableItem *stiName)
{
    CScript *pScript;

    m_mutexScripts.Lock();

    if (m_tpScripts.Lookup( LowerCase( stiName), pScript))
    {
        m_mutexScripts.Unlock();

        if (!pScript->IsGlobalFunction()) return NULL;
        return pScript->m_pGlobalFunc;
    }
    else
    {
        m_mutexScripts.Unlock();

        if ((pScript = LoadGlobalFunction(stiName))==NULL) return NULL;
        else return pScript->m_pGlobalFunc;
    }
}


//////////////////////////////////////////////////////////////////////////////////////
    
CCompilerOutput* CCodeManager::UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName)
{
	CString strFilename;

    m_mutexCodeUpload.Lock();

	// vytvoreni jmena
	switch (Type)
	{
	case SCRIPT_UNIT: 
		strFilename=m_strCodeUserPath + strName + "." + FILE_CODE_EXTENSION_UNIT;

		ASSERT( !strParentName.IsEmpty());

		CStringTableItem *stiName;
		CStringTableItem *stiParentName;
		CUnitInheritanceInfo *pUIInfo, *pUIParentInfo;
		bool bCheckInheritance, bError;

		bCheckInheritance = false;
		bError = false;
		stiName = g_StringTable.AddItem( strName);
		stiParentName = g_StringTable.AddItem( strParentName);

// uprava inheritance info a kontrola dedicnosti
		m_mutexUnitInheritanceInfo.Lock();

		// uz byla definovana
		if (m_tpUnitInheritanceInfo.Lookup( LowerCase( stiName), pUIInfo))
		{
			ASSERT( pUIInfo->m_pParent != NULL);
			ASSERT( pUIInfo->m_bDefined);

			// znemil se predek
			if (pUIInfo->m_pParent->m_stiUnitName != stiParentName)
			{
				// neznamy predek
				if (!m_tpUnitInheritanceInfo.Lookup( LowerCase( stiParentName), pUIParentInfo))
				{
					ASSERT( m_pZCivilization != NULL);
					m_pZCivilization->ReportError("Unknown ancestor '%s' of unit '%s'.", strParentName, strName);
					bError = true;
				}
				// znamy predek
				else
				{
					ASSERT( pUIParentInfo->m_bDefined);
					pUIInfo->m_pParent = pUIParentInfo;
					bCheckInheritance = true;
				}
			}
		}
		// nebyla definovana
		else
		{
			// neznamy predek
			if (!m_tpUnitInheritanceInfo.Lookup( LowerCase( stiParentName), pUIParentInfo))
			{
				ASSERT( m_pZCivilization != NULL);
				m_pZCivilization->ReportError("Unknown ancestor '%s' of unit '%s'.", strParentName, strName);
				bError = true;
			}
			// znamy predek
			else
			{
				pUIInfo = new CUnitInheritanceInfo( stiName, pUIParentInfo);
				m_tpUnitInheritanceInfo.SetAt( LowerCase( stiName, true), pUIInfo);
				bCheckInheritance = true;
			}
		}

		// kontrola dedicnosti
		if ( bCheckInheritance)
		{
			// kontrola zacykleni
			int i = 50;
			while ( i>0 && pUIInfo->m_stiUnitName != m_pDefaultUnitType->m_stiName)
			{
				pUIInfo = pUIInfo->m_pParent;
				ASSERT( pUIInfo != NULL);
			}

			if (i==0)
			{
				ASSERT( m_pZCivilization != NULL);
				m_pZCivilization->ReportError("Circular unit inheritance (check unit '%s')", strName);
				bError = true;
			}
		}

		m_mutexUnitInheritanceInfo.Unlock();

		if (bError) 
		{
			m_mutexCodeUpload.Unlock();
			return NULL;
		}

		break;
	case SCRIPT_FUNCTION:
		strFilename=m_strCodeUserPath + strName + "." + FILE_CODE_EXTENSION_FUNCTION;
		break;
	case SCRIPT_STRUCTURE:
        strFilename=m_strCodeUserPath + strName + "." + FILE_CODE_EXTENSION_STRUCTURE;
		break;
    default:
		ASSERT(false);
	}

    m_strCodeUpdateFilename = strFilename;
    m_stiCodeUpdateName = g_StringTable.AddItem( strName);
    m_CodeUpdateType = Type;

    // docasne se ulozi do souboru s pridanym znakem ~
	strFilename+="~";

    try
    {
	    if (m_CodeStdOut.Open(strFilename)) return &m_CodeStdOut;
	}
    catch (CException *e)
    {
        m_mutexCodeUpload.Unlock();
    	throw e;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

bool CCodeManager::UploadScriptCodeEnd()
{
	m_CodeStdOut.Close();

    switch (m_CodeUpdateType)
    {
    case SCRIPT_UNIT:
        DoUpdateUnit();
        break;
    case SCRIPT_FUNCTION:
        DoUpdateFunction();
        break;
    case SCRIPT_STRUCTURE:
        DoUpdateStruct();
        break;
    default:
        ASSERT(false);
    }

    m_stiCodeUpdateName->Release();

    m_mutexCodeUpload.Unlock();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
    
CCompilerOutput* CCodeManager::UploadScriptSourceStart(CString &strFilename)
{
	CString strPath;

    m_mutexSourceUpload.Lock();

	// vytvoreni jmena
	strPath=m_strSourceUserPath+strFilename;

    m_strSourceUpdateFilename=strPath;

    // docasne se ulozi do souboru s pridanym znakem ~
	strPath+="~";

    try
    {
	    if (m_SourceStdOut.Open(strPath)) return &m_SourceStdOut;
	}
    catch (CException *e)
    {
        m_mutexSourceUpload.Unlock();
    	throw e;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

bool CCodeManager::UploadScriptSourceEnd()
{
	m_SourceStdOut.Close();

    this->MoveFile( m_strSourceUpdateFilename+"~", m_strSourceUpdateFilename);

	m_mutexSourceUpload.Unlock();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

CCompilerInputFile* CCodeManager::DownloadScriptSourceStart(ESourceType eSourceType, CString &strFilename)
{                                                                                            
    m_mutexSourceDownload.Lock();

	CString strPath;
	
	if (eSourceType == USER_SOURCE) strPath.Format("%s\\%s", m_strSourceUserPath, strFilename);
	else 
	{
		ASSERT( eSourceType == SYSTEM_SOURCE);
		strPath.Format("%s\\%s", m_strSourceSystemPath, strFilename);
	}

    CFileException *pFE = new CFileException;

	if (m_SourceStdIn.Open(strPath)) 
    {
        pFE->Delete();
        return &m_SourceStdIn;
    }
    else
    {
	    m_mutexSourceDownload.Unlock();
    	throw pFE;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DownloadScriptEnd()
{
    m_SourceStdIn.Close();
    m_mutexSourceDownload.Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::GetChangedScripts(  CString &strChangedScriptsFilenames, CString &strScriptPath)
{
    WIN32_FIND_DATA FD;
    HANDLE hFindFile;
    CString strPath;
    DWORD dwAttr;
    
    strPath.Format( "%s*.%s", m_strSourceUserPath, FILE_SOURCE_EXTENSION);

    strChangedScriptsFilenames.Empty();
    
    hFindFile=::FindFirstFile(strPath, &FD);

    while (hFindFile!=INVALID_HANDLE_VALUE)
    {
        if ( strcmp(FD.cFileName, ".") == 0 || strcmp(FD.cFileName, "..") == 0) 
        {
            if (!::FindNextFile(hFindFile, &FD)) break;
            continue;
        }
        
        dwAttr = ::GetFileAttributes( m_strSourceUserPath + "\\" + FD.cFileName);

        if ((dwAttr & FILE_ATTRIBUTE_READONLY) == 0) 
        {
            if (strChangedScriptsFilenames.IsEmpty()) strChangedScriptsFilenames = FD.cFileName;
            else 
            {
                strChangedScriptsFilenames += '\n';
                strChangedScriptsFilenames += FD.cFileName;
            }
        }

        if (!::FindNextFile(hFindFile, &FD)) break;
    }

    ::FindClose(hFindFile);

    strScriptPath.Empty();
    if (!strChangedScriptsFilenames.IsEmpty()) strScriptPath = m_strSourceUserPath;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::ListSourceFiles(ESourceType nSourceType, CString &strResult)
{
    // vytvorit strom dedicnosti
    WIN32_FIND_DATA FD;
    HANDLE hFindFile;
    CString strPath, str;
    CFileStatus FS;

    if (nSourceType == USER_SOURCE) strPath=m_strSourceUserPath + "\\*.";
    else strPath=m_strSourceSystemPath + "\\*.";
    strPath += FILE_SOURCE_EXTENSION;
    
    strResult.Empty();
    
    hFindFile=::FindFirstFile(strPath, &FD);
    while (hFindFile!=INVALID_HANDLE_VALUE)
    {
        if ( strcmp(FD.cFileName, ".") == 0 || strcmp(FD.cFileName, "..") == 0) 
        {
            if (!::FindNextFile(hFindFile, &FD)) break;
            continue;
        }
        
        ASSERT( FD.nFileSizeHigh == 0);
        str.Format("%s\\%d\\%s\\%s", FD.cFileName, FD.nFileSizeLow, CTime(FD.ftLastWriteTime).Format("%d.%m.%y"),  
            CTime(FD.ftLastWriteTime).Format("%H:%M"));
        
        strResult += str;

        if (!::FindNextFile(hFindFile, &FD)) break;
        else  strResult += "|" ;
    }

    ::FindClose(hFindFile);
}

//////////////////////////////////////////////////////////////////////////////////////

EScriptType CCodeManager::LookForScriptOnDisk(CStringTableItem *stiName, 
      CString *pstrFoundFilename, EScriptType nScriptType /*= SCRIPT_UNKNOWN*/)
{
    CString strFileMask =(CString)*stiName;
    CString strExt, strPath;
    WIN32_FIND_DATA FD;
    HANDLE hSearchHandle;
    int i;
    
    if (nScriptType==SCRIPT_UNKNOWN)
    {
        strFileMask += ".*";

        for (i=0; i<2; i++)
        {
            strPath = (i==0) ? (m_strCodeUserPath + strFileMask)
                              :(m_strCodeSystemPath + strFileMask);

            if ((hSearchHandle = ::FindFirstFile(strPath, &FD)) ==INVALID_HANDLE_VALUE) continue;
            else
            {
                ::FindClose( hSearchHandle);
                // dle pripony zjisti typ
                strFileMask = FD.cFileName;
                strExt = strFileMask.Right(strFileMask.GetLength() - ((CString)*stiName).GetLength() - 1);

                if (pstrFoundFilename!=NULL)
                    *pstrFoundFilename = (i==0) ? (m_strCodeUserPath + FD.cFileName)
                                                 :(m_strCodeSystemPath + FD.cFileName);

                if (strExt == FILE_CODE_EXTENSION_UNIT) return SCRIPT_UNIT;
                else if (strExt == FILE_CODE_EXTENSION_FUNCTION) return SCRIPT_FUNCTION;
                else if (strExt == FILE_CODE_EXTENSION_STRUCTURE) return SCRIPT_STRUCTURE;
                else 
                {
                    ASSERT(false);
                    return SCRIPT_UNKNOWN;
                }
            }
        }
        return SCRIPT_UNKNOWN;
    }
    else
    {
        switch (nScriptType)
        {
        case SCRIPT_UNIT: 
            strFileMask += "."FILE_CODE_EXTENSION_UNIT;
            break;
        case SCRIPT_FUNCTION:
            strFileMask += "."FILE_CODE_EXTENSION_FUNCTION;
            break;
        case SCRIPT_STRUCTURE:
            strFileMask += "."FILE_CODE_EXTENSION_STRUCTURE;
            break;
        default:
            ASSERT(false);
        }

        for (i=0; i<2; i++)
        {
            strPath = (i==0) ? (m_strCodeUserPath + strFileMask)
                              :(m_strCodeSystemPath + strFileMask);

            if ((hSearchHandle=::FindFirstFile(strPath, &FD))==INVALID_HANDLE_VALUE) continue;
            else 
            {
                ::FindClose( hSearchHandle);

                if (pstrFoundFilename!=NULL)
                    *pstrFoundFilename = (i==0) ?  (m_strCodeUserPath + FD.cFileName)
                                                :(m_strCodeSystemPath + FD.cFileName);
                return nScriptType;
            }

            FindClose(hSearchHandle);
        }
        return SCRIPT_UNKNOWN;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

EScriptType CCodeManager::GetScriptType(CStringTableItem *stiName)
{
    CScript *pScript;

    m_mutexScripts.Lock();

    if (m_tpScripts.Lookup( LowerCase( stiName), pScript)) 
    {
        m_mutexScripts.Unlock();
        return pScript->m_eScriptType;
    }
    else 
    {
        m_mutexScripts.Unlock();
        return LookForScriptOnDisk(stiName);
    }
}

//////////////////////////////////////////////////////////////////////////////////////

bool CCodeManager::GetUnitTypeForCompiler(CStringTableItem *stiName, CPNamespace::CUnit *pCompilerUnit)
{
    CStringTableItem *pSTI = NULL;
    CIUnitType *pUnit;

    if ((pUnit = GetUnitType(stiName))==NULL) return false;

    // konverze do unity v kompilatoru
    POSITION pos;
    CIMember *pMember;
    CIMethod *pMethod;
    CPNamespace::CUnitMethod *pUMethod;
    int i;

    pCompilerUnit->m_stiName = pUnit->m_stiName->AddRef();
    if (pUnit->m_stiParentName != NULL) pCompilerUnit->m_stiParent = pUnit->m_stiParentName->AddRef();
    else pCompilerUnit->m_stiParent = NULL;

    pos = pUnit->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        pUnit->m_tpMembers.GetNextAssoc(pos, pSTI, pMember);
        pCompilerUnit->AddMember(pSTI, pMember->m_DataType);
    }

    pos = pUnit->m_tpMethods.GetStartPosition();
    while (pos != NULL)
    {
        pUnit->m_tpMethods.GetNextAssoc(pos, pSTI, pMethod);

        pUMethod = pCompilerUnit->AddMethod(pSTI, pMethod->m_ReturnType);

        for (i=0; i<pMethod->m_nArgCount; i++)
            pUMethod->AddArgument(pMethod->m_apArgAndLocalVars[i]);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////

bool CCodeManager::GetGlobalFunctionTypeForCompiler(CStringTableItem *stiName, CPNamespace::CGlobalFunction *pCompilerGlobFunc)
{
    CIGlobalFunction *pFunc;
    short i;

    if ((pFunc = GetGlobalFunction(stiName)) == NULL) return false;
    
    // konverze do globalni funkce v kompilatoru
    CString str;

    pCompilerGlobFunc->m_stiName = pFunc->m_stiName->AddRef();
    pCompilerGlobFunc->m_ReturnDataType = pFunc->m_ReturnType;

    for (i=0; i<pFunc->m_nArgCount; i++)
    {
        pCompilerGlobFunc->AddArgument(pFunc->m_apArgAndLocalVars[i]);
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////

bool CCodeManager::GetStructureTypeForCompiler(CStringTableItem *stiName, CPNamespace::CStruct *pCompilerStruct)
{
    CStringTableItem *pSTI = NULL;
    CIStructureType *pStruct;

    if ((pStruct = GetStructureType(stiName)) == NULL) return false;

    // konverze do struktury v kompilatoru
    POSITION pos;
    CIMember *pMember;
    CString str;

    pCompilerStruct->m_stiName = pStruct->m_stiName->AddRef();

    pos = pStruct->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        pStruct->m_tpMembers.GetNextAssoc(pos, pSTI, pMember);
        str=(CString)*pSTI;
        pCompilerStruct->AddMember(str, pMember->m_DataType);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadUnitByFilename(CString &strFilename, CIUnitType *pParentUnit,
                                                        bool bAddToLoadedScripts)
{
	ASSERT(strFilename != "");

    CFile file;
    CFileException *pFE=new CFileException();
    CScript *pScript;

    // otevreni souboru
    if (!file.Open(strFilename, CFile::modeRead | CFile::shareDenyWrite, pFE)) throw pFE;
    pFE->Delete();

    // vytvoreni noveho skriptu
    pScript = new CScript();
    pScript->m_eScriptType = SCRIPT_UNIT;
    pScript->m_pIUnitType = new CIUnitType();

    // naloadovani jednotky
    pScript->m_pIUnitType->Load(file, pParentUnit);

    // zadani jmena do CScript (dle naloadovaneho jmena)
    pScript->m_stiName = pScript->m_pIUnitType->m_stiName->AddRef();

    if (bAddToLoadedScripts)
    {
        m_mutexScripts.Lock();

        // assert, zda uz nahodou neni nekde naloadovanej
        ASSERT(!m_tpScripts.Lookup( LowerCase( pScript->m_stiName), pScript));

        // pridani do tabulky naloadovanych skriptu
        m_tpScripts.SetAt( LowerCase(pScript->m_stiName, true), pScript);       

        m_mutexScripts.Unlock();
    }

    file.Close();
    
    return pScript;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadGlobalFunctionByFilename(CString &strFilename, bool bAddToLoadedScripts)
{
	ASSERT(strFilename != "");

    CFile file;
    CFileException *pFE=new CFileException();
    CScript *pScript;

    // otevreni souboru
    if (!file.Open(strFilename, CFile::modeRead | CFile::shareDenyWrite, pFE)) throw pFE;
    pFE->Delete();

    // vytvoreni noveho skriptu
    pScript = new CScript();
    pScript->m_eScriptType = SCRIPT_FUNCTION;
    pScript->m_pGlobalFunc = new CIGlobalFunction();

    // naloadovani funkce
    pScript->m_pGlobalFunc->Load(file);

    // zadani jmena do CScript (dle naloadovaneho jmena)
    pScript->m_stiName = pScript->m_pGlobalFunc->m_stiName->AddRef();

    if (bAddToLoadedScripts)
    {
        m_mutexScripts.Lock();

        // assert, zda uz nahodou neni nekde naloadovanej
        ASSERT(!m_tpScripts.Lookup( LowerCase( pScript->m_stiName), pScript));

        // pridani do tabulky naloadovanych funkci
        m_tpScripts.SetAt( LowerCase( pScript->m_stiName, true), pScript);

        m_mutexScripts.Unlock();
    }
    
    file.Close();

    return pScript;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadStructureByFilename(CString &strFilename, bool bAddToLoadedScripts)
{
	ASSERT(strFilename != "");

    CFile file;
    CFileException *pFE=new CFileException();
    CScript *pScript = NULL;

    // otevreni souboru
    if (!file.Open(strFilename, CFile::modeRead | CFile::shareDenyWrite, pFE)) throw pFE;
    pFE->Delete();

    // vytvoreni noveho skriptu
    pScript = new CScript();
    pScript->m_eScriptType = SCRIPT_STRUCTURE;
    pScript->m_pStructType = new CIStructureType();

    // naloadovani struktury
    pScript->m_pStructType->Load(file);

    // zadani jmena do CScript (dle naloadovaneho jmena)
    pScript->m_stiName = pScript->m_pStructType->m_stiName->AddRef();

    if (bAddToLoadedScripts)
    {
        m_mutexScripts.Lock();

        // assert, zda uz nahodou neni nekde naloadovanej
        ASSERT(!m_tpScripts.Lookup( LowerCase( pScript->m_stiName), pScript));

        // pridani do tabulky naloadovanych skriptu
        m_tpScripts.SetAt( LowerCase( pScript->m_pStructType->m_stiName, true), pScript);       
        m_mutexScripts.Unlock();
    }

    file.Close();

    return pScript;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadUnit(CStringTableItem *stiName)
{
    CString strFilename;
    CScript *pParentScript;
    CUnitInheritanceInfo *pUIInfo = NULL;

    // neni uz nahodou naloadovany?
#ifdef _DEBUG
    CScript *pScript;
    m_mutexScripts.Lock();
    ASSERT(!m_tpScripts.Lookup( LowerCase( stiName), pScript));
    m_mutexScripts.Unlock();
#endif

// nalezeni souboru, kde je
    if (LookForScriptOnDisk(stiName, &strFilename, SCRIPT_UNIT) != SCRIPT_UNIT) return NULL;

// naloadovani predka
    // zjisteni, kdo je predek
    m_mutexUnitInheritanceInfo.Lock();
    VERIFY( m_tpUnitInheritanceInfo.Lookup( LowerCase( stiName), pUIInfo));
    m_mutexUnitInheritanceInfo.Unlock();
    
    ASSERT( pUIInfo->m_bDefined);
    
    if (pUIInfo->m_pParent == NULL) return NULL;

    m_mutexScripts.Lock();
    if (!m_tpScripts.Lookup( LowerCase( pUIInfo->m_pParent->m_stiUnitName), pParentScript))
    {
        pParentScript = LoadUnit(pUIInfo->m_pParent->m_stiUnitName);
        if (pParentScript == NULL) 
		{
	        m_mutexScripts.Unlock();
            throw new CStringException("Can't load parent unit '%s'.", 
                (CString)*pUIInfo->m_pParent->m_stiUnitName);
		}
    }

// naloadovani jednotky
    ASSERT( pParentScript!=NULL);
    CScript *pRetScript = LoadUnitByFilename(strFilename, pParentScript->m_pIUnitType, true);

    m_mutexScripts.Unlock();
	
	return pRetScript;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadGlobalFunction(CStringTableItem *stiName)
{
    CString strFilename;

    // neni uz nahodou naloadovany?
#ifdef _DEBUG
    CScript *pScript;
    m_mutexScripts.Lock();
    ASSERT(!m_tpScripts.Lookup( LowerCase( stiName), pScript));
    m_mutexScripts.Unlock();
#endif

    // nalezeni souboru, kde je
    if (LookForScriptOnDisk(stiName, &strFilename, SCRIPT_FUNCTION) != SCRIPT_FUNCTION) return false;
    
    // naloadovani
    return LoadGlobalFunctionByFilename(strFilename, true);
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::CScript* CCodeManager::LoadStructure(CStringTableItem *stiName)
{
    CString strFilename;

    // neni uz nahodou naloadovany?
#ifdef _DEBUG
    CScript *pScript;
    m_mutexScripts.Lock();
    ASSERT(!m_tpScripts.Lookup( LowerCase( stiName), pScript));
    m_mutexScripts.Unlock();
#endif

    // nalezeni souboru, kde je
    if (LookForScriptOnDisk(stiName, &strFilename, SCRIPT_STRUCTURE) != SCRIPT_STRUCTURE) return false;
    
    // naloadovani
    return LoadStructureByFilename(strFilename, true);
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DoUpdateUnit()
{
    CString  strNewFilename = m_strCodeUpdateFilename+"~";
    ECompareResult CompRes;
    CScript *pOldScript, *pNewScript, *pParentScript;
    CIChangedMethodEventsList *pChangedMethodEvents, *pPom;
    CUnitInheritanceInfo *pUIInfo;

// Je to, co updatuju, vubec naloadovany?
    m_mutexScripts.Lock();

    if ( !m_tpScripts.Lookup( LowerCase( m_stiCodeUpdateName), pOldScript))
    {
        // neni :-)
        m_mutexScripts.Unlock();
        this->MoveFile( strNewFilename, m_strCodeUpdateFilename);
        return;
    }

// NALOADOVANI NOVEHO KODU
    // zjisteni, kdo je predek
    m_mutexUnitInheritanceInfo.Lock();
    VERIFY( m_tpUnitInheritanceInfo.Lookup( LowerCase( m_stiCodeUpdateName), pUIInfo));
    m_mutexUnitInheritanceInfo.Unlock();

    ASSERT( pUIInfo->m_bDefined);
    
    if (pUIInfo->m_pParent == NULL) 
    {
        ASSERT(false);
        // TODO: co ted?
        m_mutexScripts.Unlock();
        throw new CStringException("Undefined parent unit of unit '%s'.", 
                m_stiCodeUpdateName);
    }

    // predek musi byt naloadovany, kdyz je jednotka naloadovana
    VERIFY( m_tpScripts.Lookup( LowerCase( pUIInfo->m_pParent->m_stiUnitName), pParentScript));

    // odemknout zamek nad skripty
    m_mutexScripts.Unlock();

    // naloadovani nove verze
    pNewScript = LoadUnitByFilename( strNewFilename, pParentScript->m_pIUnitType, false);

// POROVNANI
    CompRes = CompareUnits( pOldScript->m_pIUnitType, pNewScript->m_pIUnitType, pChangedMethodEvents);

	TRACE_INTERPRET1("-* Updating unit '%s' ... ", (CString)*pOldScript->m_pIUnitType->m_stiName);

    if (CompRes == NO_DIFFERENCES)
    {
		TRACE_INTERPRET0("no changes.\n");

        // uvolneni nove verze (je stejna jako stara)
        delete pNewScript->m_pIUnitType;
        delete pNewScript;

        // smazani ~ souboru
        this->DeleteFile( strNewFilename);

        return;

    }
    else if (CompRes & (DIFFERENT_CODE | DIFFERENT_CONSTRUCTOR | DIFFERENT_DESTRUCTOR | DIFFERENT_FLOATTABLE | DIFFERENT_STRINGTABLE))
    {
		TRACE_INTERPRET0("small update started.\n");
        // update
        DoUpdateUnitCode( pOldScript->m_pIUnitType, pNewScript->m_pIUnitType, pChangedMethodEvents, CompRes);
        
        // uvolneni spojaku
        while (pChangedMethodEvents != NULL)
        {
            pPom = pChangedMethodEvents;
            pChangedMethodEvents = pChangedMethodEvents->m_pNext;
            
            pPom->m_stiName->Release();
            // TODO: tady bude pool - CCodeManager::m_CSStringTableItemListPool.
            delete pPom;
        }

		TRACE_INTERPRET1("-* Updating unit '%s' ... small update done.\n", (CString)*pOldScript->m_pIUnitType->m_stiName);
        
    }
    else if (CompRes & DIFFERENT_ALL)
    {
		TRACE_INTERPRET0("big update started. \n");

        // update
        DoUpdateUnitAll( pOldScript->m_pIUnitType, pNewScript->m_pIUnitType);

		TRACE_INTERPRET1("-* Updating unit '%s' ... big update done.\n", (CString)*pOldScript->m_pIUnitType->m_stiName);
    }
    else ASSERT(false);

    // prekopirovani souboru
    this->MoveFile( strNewFilename, m_strCodeUpdateFilename);

    // uvolneni (pNewScript, protoze nove veci z pNewScript byly prestrkany do pOldScript); 
    delete pNewScript->m_pIUnitType;
    delete pNewScript;
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DoUpdateUnitCode( CIUnitType *pOldType, CIUnitType *pNewType, 
                                     CIChangedMethodEventsList *pChangedMethodEvents,
                                     ECompareResult CompRes)
{
    CIMethod *pMethodOld, *pMethodNew;
    CIEvent *pEventOld, *pEventNew;
    
    SIUpdateUnitCode sUpdateUnitCode;

    m_UpdateDone.ResetEvent();
    m_DoUpdate.ResetEvent();

    sUpdateUnitCode.m_pChangedMethodEvents = pChangedMethodEvents;
    sUpdateUnitCode.m_pIUnitType = pOldType;
    sUpdateUnitCode.m_pDoJob = &m_DoUpdate;
    sUpdateUnitCode.m_pJobDone = &m_UpdateDone;
    sUpdateUnitCode.m_bDestructorChanged = (CompRes & DIFFERENT_DESTRUCTOR) != 0;
    sUpdateUnitCode.m_bConstructorChanged = (CompRes & DIFFERENT_CONSTRUCTOR) != 0;

    // posle zpravu intepretu a ceka, az zabije vsechny procesy, kterych se to tyka
    InlayEvent( UPDATE_UNIT_CODE, (DWORD)&sUpdateUnitCode);
    m_UpdateDone.Lock();

    // zamknount zamek nad skripty
    m_mutexScripts.Lock();

    // zamknout m_pIUnitType, aby to nikdo zvenku nemohl cist, Interpret stoji
    m_pZCivilization->LockUnitData();

    // updatuje kod, interpret ceka, az bude hotovy
    while (pChangedMethodEvents != NULL)
    {
        switch (pChangedMethodEvents->m_eAction)
        {
        case CIChangedMethodEventsList::METHOD_CHANGED:
            VERIFY( pNewType->m_tpMethods.Lookup( pChangedMethodEvents->m_stiName, pMethodNew));
            VERIFY( pOldType->m_tpMethods.Lookup( pChangedMethodEvents->m_stiName, pMethodOld));
            pOldType->m_tpMethods.SetAt( pChangedMethodEvents->m_stiName, pMethodNew);
            pNewType->m_tpMethods.SetAt( pChangedMethodEvents->m_stiName, pMethodOld);
            pMethodNew->m_pIUnitType = pOldType;
            break;

        case CIChangedMethodEventsList::METHOD_DELETED:
            VERIFY( pOldType->m_tpMethods.Lookup( pChangedMethodEvents->m_stiName, pMethodOld));
            pNewType->m_tpMethods.SetAt( pChangedMethodEvents->m_stiName, pMethodOld);
            pOldType->m_tpMethods.RemoveKey( pChangedMethodEvents->m_stiName);
            break;

        case CIChangedMethodEventsList::METHOD_NEW:
            VERIFY( pNewType->m_tpMethods.Lookup( pChangedMethodEvents->m_stiName, pMethodNew));
            pOldType->m_tpMethods.SetAt( pChangedMethodEvents->m_stiName, pMethodNew);
            pNewType->m_tpMethods.RemoveKey( pChangedMethodEvents->m_stiName);
            pMethodNew->m_pIUnitType = pOldType;
            break;

        case CIChangedMethodEventsList::EVENT_CHANGED:
            VERIFY( pNewType->m_tpEvents.Lookup( pChangedMethodEvents->m_stiName, pEventNew));
            VERIFY( pOldType->m_tpEvents.Lookup( pChangedMethodEvents->m_stiName, pEventOld));
            pOldType->m_tpEvents.SetAt( pChangedMethodEvents->m_stiName, pEventNew);
            pNewType->m_tpEvents.SetAt( pChangedMethodEvents->m_stiName, pEventOld);
            pEventNew->m_pIUnitType = pOldType;
            break;

        case CIChangedMethodEventsList::EVENT_DELETED:
            VERIFY( pOldType->m_tpEvents.Lookup( pChangedMethodEvents->m_stiName, pEventOld));
            pNewType->m_tpEvents.SetAt( pChangedMethodEvents->m_stiName, pEventOld);
            pOldType->m_tpEvents.RemoveKey( pChangedMethodEvents->m_stiName);
            break;

        case CIChangedMethodEventsList::EVENT_NEW:
            VERIFY( pNewType->m_tpEvents.Lookup( pChangedMethodEvents->m_stiName, pEventNew));
            pOldType->m_tpEvents.SetAt( pChangedMethodEvents->m_stiName, pEventNew);
            pNewType->m_tpEvents.RemoveKey( pChangedMethodEvents->m_stiName);
            pEventNew->m_pIUnitType = pOldType;
            break;
        }
        pChangedMethodEvents = pChangedMethodEvents->m_pNext;
    }

    if (CompRes & DIFFERENT_STRINGTABLE)
    {
        pOldType->m_pCodeStringTable->UpdateFrom( pNewType->m_pCodeStringTable);
    }
    
    if (CompRes & DIFFERENT_FLOATTABLE)
    {
        pOldType->m_pFloatTable->UpdateFrom( pNewType->m_pFloatTable);
    }
    
    if (CompRes & DIFFERENT_CONSTRUCTOR)
    {  
        CIConstructor *pPom = pOldType->m_pConstructor;
        pOldType->m_pConstructor =  pNewType->m_pConstructor;
        pNewType->m_pConstructor = pPom;

        pOldType->m_pConstructor->m_pIUnitType = pOldType;
    }
    
    if (CompRes & DIFFERENT_DESTRUCTOR)
    {
        CIDestructor *pPom = pOldType->m_pDestructor;
        pOldType->m_pDestructor =  pNewType->m_pDestructor;
        pNewType->m_pDestructor = pPom;

        pOldType->m_pDestructor->m_pIUnitType = pOldType;
    }

    // odemknout m_pIUnitType
    m_pZCivilization->UnlockUnitData();

    // odemknount zamek nad skripty
    m_mutexScripts.Unlock();

    // vzbudi intepreta
    m_DoUpdate.SetEvent();

	// pocka az interpret dojede, aby mohl zrusit eventy
	m_UpdateDone.Lock();
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DoUpdateUnitAll( CIUnitType *pOldType, CIUnitType *pNewType)
{
    CString  strNewFilename = m_strCodeUpdateFilename+"~";

    // Zjistit zmenu offsetu, ktera mohla vzniknout pridanim/odebranim polozek jednotky
    int nOffsetDelta = pNewType->m_nHeapSize = pOldType->m_nHeapSize;

    // Vyrobit seznam potomku jednotky
    CIUnitTypeList *pChildren = GetUnitChildren( pOldType);

    // Poslat zpravu interpretu, at updatuje polozky vsech instanci jednotek
    SIUpdateUnitAll sUpdateUnitAll;

    m_UpdateDone.ResetEvent();
    m_DoUpdate.ResetEvent();

    sUpdateUnitAll.m_pOldUnitType = pOldType;
    sUpdateUnitAll.m_pNewUnitType = pNewType;
    sUpdateUnitAll.m_pChildren = pChildren;
    sUpdateUnitAll.m_nHeapOffsetDelta = nOffsetDelta;
    sUpdateUnitAll.m_pDoJob = &m_DoUpdate;
    sUpdateUnitAll.m_pJobDone = &m_UpdateDone;

// posle zpravu intepretu a ceka, az zabije vsechny procesy pouzivajici
    // danou jednotku, a az updatuje datove polozky vsech instanci a potomku teto jednotky.
    InlayEvent( UPDATE_UNIT_ALL, (DWORD)&sUpdateUnitAll);
    m_UpdateDone.Lock();

// ted je rada na CodeManageru, aby updatoval typ jednotky
    // (interpret ceka na DoUpdate);

    // zamknount zamek nad skripty
    m_mutexScripts.Lock();

    // zamknout m_pIUnitType, aby to nikdo zvenku nemohl cist, Interpret stoji
    m_pZCivilization->LockUnitData();

    // Zmenit (posunout) heap offsety vsem potomkum jednotky
    if (nOffsetDelta != 0)
    {
        CIUnitTypeList *pUTL = pChildren;
        while (pUTL != NULL)
        {
            pUTL->m_pIUnitType->ChangeHeapOffset( nOffsetDelta);
            pUTL = pUTL->m_pNext;
        }
    }

    // UpdateUnitFrom udela update typu jednotky v pOldScript.
    // !! pNewType bude vycucnuta, budou pretazeny vsechny polozky i codestringtable.
    // Takze pak bude obsahovat m_pNewType prazdnou strukturu, kterou je uz treba jen 
    // uvolnit.
    pOldType->UpdateUnitTypeFrom( pNewType);

    // odemknout m_pIUnitType
    m_pZCivilization->UnlockUnitData();

    // odemknount zamek nad skripty
    m_mutexScripts.Unlock();

    // vzbudi interpret
    m_DoUpdate.SetEvent();

	// pockat az se vzbudi, abysme mohli zrusit eventy
	m_UpdateDone.Lock();
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DoUpdateStruct()
{
    CString  strNewFilename = m_strCodeUpdateFilename+"~";

    CScript *pOldScript, *pNewScript;
    ECompareResult CompRes;

// Je to, co updatuju, vubec naloadovany?
    m_mutexScripts.Lock();

    if ( !m_tpScripts.Lookup( LowerCase( m_stiCodeUpdateName), pOldScript))
    {
        // neni :-)
        m_mutexScripts.Unlock();
        this->MoveFile( strNewFilename, m_strCodeUpdateFilename);

        return;
    }

    m_mutexScripts.Unlock();

// NALOADOVANI NOVEHO KODU
    // naloadovani nove verze
    pNewScript = LoadStructureByFilename( strNewFilename, false);

// POROVNANI
    ASSERT( pOldScript->m_eScriptType == SCRIPT_STRUCTURE);
    ASSERT( pNewScript->m_eScriptType == SCRIPT_STRUCTURE);

    CompRes  = CompareStructures( pOldScript->m_pStructType, pNewScript->m_pStructType);

	TRACE_INTERPRET1("-* Updating structure '%s' ... ", (CString)*pOldScript->m_pStructType->m_stiName);

    if (CompRes == NO_DIFFERENCES)
    {
		TRACE_INTERPRET0("no changes.\n");
        // uvolneni nove verze (stejna jako stara)
        delete pNewScript->m_pStructType;
        delete pNewScript;

        // smazani ~ souboru
        this->DeleteFile( strNewFilename);

        return;
    }

    ASSERT( CompRes == DIFFERENT_ALL);

    TRACE_INTERPRET0("update started\n");

    SIUpdateStruct sUpdateStruct;

    m_UpdateDone.ResetEvent();
    m_DoUpdate.ResetEvent();

    sUpdateStruct.m_pOldStructType = pOldScript->m_pStructType;
    sUpdateStruct.m_pNewStructType = pNewScript->m_pStructType;
    sUpdateStruct.m_pDoJob = &m_DoUpdate;
    sUpdateStruct.m_pJobDone = &m_UpdateDone;

    // posle zpravu intepretu a ceka, az updatuje vsechny instance struktur v interpretu
    // (udela jim nove datove polozky).
    InlayEvent( UPDATE_STRUCT, (DWORD)&sUpdateStruct);
    m_UpdateDone.Lock();

    // ted je rada na CodeManageru, aby updatoval typ struktury
    // (interpret ceka na DoJob);

    // UpdateStructFrom udela update struktury v pOldScript.
    // !! pNewScript->m_pStructType bude vycucnuta, budou pretazeny vsechny polozky i codestringtable.
    // Takze bude obsahovat pNewScript->m_pStructType prazdnou strukturu, kterou je uz treba jen 
    // uvolnit.
    pOldScript->m_pStructType->UpdateStructFrom( pNewScript->m_pStructType);

    // vzbudi interpret
    m_DoUpdate.SetEvent();

	TRACE_INTERPRET1("-* Updating structure '%s' ... update done.\n", (CString)*pOldScript->m_pStructType->m_stiName);
        
    // prekopirovani souboru
    this->MoveFile( strNewFilename, m_strCodeUpdateFilename);

    // uvolneni (pNewScript, protoze nove veci z pNewScript byly prestrkany do pOldScript); 
    delete pNewScript->m_pStructType;
    delete pNewScript;

	// pockat az se interpret vzbudi, abysme mohli zrusit eventy
	m_UpdateDone.Lock();
}

//////////////////////////////////////////////////////////////////////////////////////

#define _helper_XChangePointers( ptr, type)  \
    { \
        void *pPom; \
        pPom = pOldScript->m_pGlobalFunc->ptr;   \
        pOldScript->m_pGlobalFunc->ptr = pNewScript->m_pGlobalFunc->ptr;    \
        pNewScript->m_pGlobalFunc->ptr = (type*)pPom;   \
    }

#define _helper_XChangeValues( val, type)   \
    { \
        type pom; \
        pom = pOldScript->m_pGlobalFunc->val;   \
        pOldScript->m_pGlobalFunc->val = pNewScript->m_pGlobalFunc->val;    \
        pNewScript->m_pGlobalFunc->val = pom;   \
    }


void CCodeManager::DoUpdateFunction()
{
    CString  strNewFilename = m_strCodeUpdateFilename+"~";

    CScript *pOldScript, *pNewScript;
    ECompareResult CompRes;

// Je to, co updatuju, vubec naloadovany?
    m_mutexScripts.Lock();

    if ( !m_tpScripts.Lookup( LowerCase( m_stiCodeUpdateName), pOldScript))
    {
        // neni :-)
        m_mutexScripts.Unlock();
        this->MoveFile( strNewFilename, m_strCodeUpdateFilename);
        return;
    }

    m_mutexScripts.Unlock();

// NALOADOVANI NOVEHO KODU
    // naloadovani nove verze
    pNewScript = LoadGlobalFunctionByFilename( strNewFilename, false);

// POROVNANI
    ASSERT( pOldScript->m_eScriptType == SCRIPT_FUNCTION);
    ASSERT( pNewScript->m_eScriptType == SCRIPT_FUNCTION);

    CompRes  = CompareFunctions( pOldScript->m_pGlobalFunc, pNewScript->m_pGlobalFunc);

	TRACE_INTERPRET1("-* Updating global function '%s' ... ", (CString)*pOldScript->m_stiName);

    if (CompRes == NO_DIFFERENCES)
    {
		TRACE_INTERPRET0("no changes.\n");
        // uvolneni nove verze (stejna jako stara)
        delete pNewScript->m_pGlobalFunc;
        delete pNewScript;

        // smazani ~ souboru
        this->DeleteFile( strNewFilename);

        return;
    }

    ASSERT( CompRes == DIFFERENT_ALL);

    TRACE_INTERPRET0("update started\n");

    SIUpdateGlobalFunction sUpdateGlobalFunction;
    
    m_UpdateDone.ResetEvent();
    m_DoUpdate.ResetEvent();

    sUpdateGlobalFunction.m_stiGlobalFunctionName = pOldScript->m_stiName;
    sUpdateGlobalFunction.m_pDoJob = &m_DoUpdate;
    sUpdateGlobalFunction.m_pJobDone = &m_UpdateDone;

    // posle zpravu intepretu a ceka, az updatuje vsechny instance struktur v interpretu
    // (udela jim nove datove polozky).
    InlayEvent( UPDATE_GLOBAL_FUNCTION, (DWORD)&sUpdateGlobalFunction);
    m_UpdateDone.Lock();

// ted je rada na CodeManageru, aby updatoval globalni funkci
// (interpret ceka na DoJob);

    // tady se vymenuji obsahy pOldScript->m_pGlobalFunction a pNewScript->m_pGlobalFunction.
    // V pNewScript->m_pGlobalFunction je na konci 'nepouzitelna', jen se uvolni

    // navratovy typ
    pOldScript->m_pGlobalFunc->m_ReturnType = pNewScript->m_pGlobalFunc->m_ReturnType ;

    // argumenty a lokalni promenne
    _helper_XChangeValues( m_nArgCount, short);
    _helper_XChangeValues( m_nArgAndLocalVarCount, short);
    _helper_XChangePointers( m_apArgAndLocalVars, CIType);    
    
    // kod funkce
    _helper_XChangePointers( m_Code.m_pCode, INSTRUCTION);
    _helper_XChangeValues( m_Code.m_nLength, long);

    // tabulky
    _helper_XChangePointers( m_pCodeStringTable, CICodeStringTable);
    _helper_XChangePointers( m_pFloatTable, CFloatTable);

// vzbudi interpret
    m_DoUpdate.SetEvent();

	TRACE_INTERPRET1("-* Updating global function '%s' ... update done.\n", (CString)*pOldScript->m_stiName);
        
    // uvolneni (pNewScript, protoze nove veci z pNewScript byly prestrkany do pOldScript); 
    delete pNewScript->m_pStructType;
    delete pNewScript;

    // prekopirovani souboru
    this->MoveFile( strNewFilename, m_strCodeUpdateFilename);

	// pocka az interpret dojede, aby mohl zkusit eventy
	m_UpdateDone.Lock();
}

#undef _helper_XChangePointers
#undef _helper_XChangeValue

//////////////////////////////////////////////////////////////////////////////////////
// Porovnava dve jednotky, vraci v cem se lisi a v pripade DIFFERENT_CODE
// vraci spojak jmen metod a eventu, ve kterych se lisi. Spojak musis uvolnit,
// je to alokovano z CCodeManager::m_CSStringTableItemListPool.

#define _help_AddNameToList( name, action)   \
                    /* TODO: tady bude pool */    \
                    pPom = new CIChangedMethodEventsList;   \
                    pPom->m_stiName = name->AddRef();      \
                    pPom->m_eAction = CIChangedMethodEventsList:: action;     \
                    pPom->m_pNext = pMethodEvents;        \
                    pMethodEvents = pPom;       


CCodeManager::ECompareResult CCodeManager::CompareUnits( CIUnitType *pOldType, CIUnitType *pNewType,
                             CIChangedMethodEventsList* &pMethodEvents)
{
    int i, nCount;
    POSITION pos;
    CIMember *pMember1, *pMember2;
    CIMethod *pMethod1, *pMethod2;
    CIEvent *pEvent1, *pEvent2;
    CStringTableItem *stiName;
    CIChangedMethodEventsList *pPom;

    ECompareResult ret = NO_DIFFERENCES;

    pMethodEvents = NULL;

    // JMENO JEDNOTKY
    ASSERT( pOldType->m_stiName == pNewType->m_stiName);

    // RODIC
    if ( pOldType->m_stiParentName != pNewType->m_stiParentName) return DIFFERENT_ALL;
    ASSERT( pOldType->m_pParent == pNewType->m_pParent);

    // MEMBERS
    if ( pOldType->m_nMemberCount != pNewType->m_nMemberCount) return DIFFERENT_ALL;
    for (i=0; i<pOldType->m_nMemberCount; i++)
    {
        pos = pOldType->m_tpMembers.GetStartPosition();
        while (pos != NULL)
        {
            pOldType->m_tpMembers.GetNextAssoc(pos, stiName, pMember1);
            if (! pNewType->m_tpMembers.Lookup( stiName, pMember2)) return DIFFERENT_ALL;
            if ( !(*pMember1 == *pMember2)) return DIFFERENT_ALL;
        }
    }

    // STRINGTABLE
    if ( !(*pOldType->m_pCodeStringTable == *pNewType->m_pCodeStringTable)) ret = (ECompareResult)( ret | DIFFERENT_STRINGTABLE);

    // FLOATTABLE
    if ( !(*pOldType->m_pFloatTable == *pNewType->m_pFloatTable)) ret = (ECompareResult)( ret | DIFFERENT_FLOATTABLE);

    // KONSTRUKTOR
    if ( !(*pOldType->m_pConstructor == *pNewType->m_pConstructor)) ret = (ECompareResult)( ret |  DIFFERENT_CONSTRUCTOR);

    // DESTRUKTOR
    if ( !(*pOldType->m_pDestructor == *pNewType->m_pDestructor)) 
    {
        ret = (ECompareResult)( ret |  DIFFERENT_DESTRUCTOR);
    }

    // METHODS
    nCount = 0;
    pos = pOldType->m_tpMethods.GetStartPosition();
    while (pos != NULL)
    {
        pOldType->m_tpMethods.GetNextAssoc(pos, stiName, pMethod1);

        if ( !pNewType->m_tpMethods.Lookup( stiName, pMethod2))
        {
            // method deleted
            ret = (ECompareResult)( ret | DIFFERENT_CODE);
            _help_AddNameToList( pMethod1->m_stiName, METHOD_DELETED);
        }
        else
        {
             nCount++;
             if (!(*pMethod1 == *pMethod2))
             {
                 // method changed
                 ret = (ECompareResult)( ret | DIFFERENT_CODE);
                 _help_AddNameToList( pMethod1->m_stiName, METHOD_CHANGED);
             }
        }
    }
    
    if ( pNewType->m_nMethodCount != nCount) 
    // some new methods were added
    {
        ret = (ECompareResult)( ret | DIFFERENT_CODE);

        pos = pNewType->m_tpMethods.GetStartPosition();
        while (pos != NULL)
        {
            pNewType->m_tpMethods.GetNextAssoc( pos, stiName, pMethod2);
            if (!pOldType->m_tpMethods.Lookup( stiName, pMethod1))
            {
                _help_AddNameToList( pMethod2->m_stiName, METHOD_NEW);
            }
        }
    }

    // EVENTS
    nCount = 0;
    pos = pOldType->m_tpEvents.GetStartPosition();
    while (pos != NULL)
    {
        pOldType->m_tpEvents.GetNextAssoc(pos, stiName, pEvent1);

        if ( !pNewType->m_tpEvents.Lookup( stiName, pEvent2))
        {
            // Event deleted
            ret = (ECompareResult)( ret | DIFFERENT_CODE);
            _help_AddNameToList( pEvent1->m_stiName, EVENT_DELETED);
        }
        else
        {
             nCount++;
             if (!(*pEvent1 == *pEvent2))
             {
                 // Event changed
                 ret = (ECompareResult)( ret | DIFFERENT_CODE);
                 _help_AddNameToList( pEvent1->m_stiName, EVENT_CHANGED);
             }
        }
    }
    
    if ( pNewType->m_nEventCount != nCount) 
    // some new events were added
    {
        ret = (ECompareResult)( ret | DIFFERENT_CODE);

        pos = pNewType->m_tpEvents.GetStartPosition();
        while (pos != NULL)
        {
            pNewType->m_tpEvents.GetNextAssoc( pos, stiName, pEvent2);
            if (!pOldType->m_tpEvents.Lookup( stiName, pEvent1))
            {
                _help_AddNameToList( pEvent2->m_stiName, EVENT_NEW);
            }
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::ECompareResult CCodeManager::CompareStructures( CIStructureType *pOldType, CIStructureType *pNewType)
{
    int i;
    CIMember *pMember1, *pMember2;
    POSITION pos;
    CStringTableItem *stiName;

    // JMENO STRUKTURY
    ASSERT( pOldType->m_stiName == pNewType->m_stiName);

    // MEMBERS
    if ( pOldType->m_nMemberCount != pNewType->m_nMemberCount) return DIFFERENT_ALL;
    for (i=0; i<pOldType->m_nMemberCount; i++)
    {
        pos = pOldType->m_tpMembers.GetStartPosition();
        while (pos != NULL)
        {
            pOldType->m_tpMembers.GetNextAssoc(pos, stiName, pMember1);
            if (! pNewType->m_tpMembers.Lookup( stiName, pMember2)) return DIFFERENT_ALL;
            if ( !(*pMember1 == *pMember2)) return DIFFERENT_ALL;
        }
    }

    return NO_DIFFERENCES;
}

//////////////////////////////////////////////////////////////////////////////////////

CCodeManager::ECompareResult CCodeManager::CompareFunctions( CIGlobalFunction *pOldFunc, CIGlobalFunction *pNewFunc)
{
    // jmeno funkce
    ASSERT( pOldFunc->m_stiName == pNewFunc->m_stiName);

    // kod funkce
    if (!(pOldFunc->m_Code == pNewFunc->m_Code) ||
        // navratovy typ
        !(pOldFunc->m_ReturnType == pNewFunc->m_ReturnType) ||
        // pocet argumentu a lokalnich promennych
        pOldFunc->m_nArgCount != pNewFunc->m_nArgCount ||
        pOldFunc->m_nArgAndLocalVarCount != pNewFunc->m_nArgAndLocalVarCount) 
    {
        return DIFFERENT_ALL;
    }
    
    // typy argumentu (naalokovane pole delky m_nArgCount);
    int i;
    for (i=0; i<pOldFunc->m_nArgAndLocalVarCount; i++)
    {
        if (!(pOldFunc->m_apArgAndLocalVars[i] == pNewFunc->m_apArgAndLocalVars[i])) return DIFFERENT_ALL;
    }

    // tabulky
    if (!(*pOldFunc->m_pCodeStringTable == *pNewFunc->m_pCodeStringTable) ||
        !(*pOldFunc->m_pFloatTable == *pNewFunc->m_pFloatTable)) 
    {
        return DIFFERENT_ALL;
    }

    return NO_DIFFERENCES;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//	SAVE & LOAD
//
void CCodeManager::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // pointer na sebe
	storage << (DWORD)this;

	// notifier
	CNotifier::PersistentSave( storage);

    // civilizace
    storage << (DWORD)m_pZCivilization;

    // ulozeni jmen a typu skriptu, ktere jsou naloadovane
	POSITION pos;
	CStringTableItem *stiName;
	CScript *pScript;

	m_mutexScripts.Lock();

	pos = m_tpScripts.GetStartPosition();
	while ( pos != NULL)
	{
		m_tpScripts.GetNextAssoc( pos, stiName, pScript);

		storage << (DWORD)stiName << (DWORD)pScript->m_eScriptType;
		
		// tohle by se urcite dalo udelat tak, ze ulozim m_pIUnitType, protoze to je union, ale takhle je to bezpecnejsi
		switch ( pScript->m_eScriptType)
		{
		case SCRIPT_UNIT:		storage << (DWORD)pScript->m_pIUnitType;		break;
		case SCRIPT_FUNCTION:	storage << (DWORD)pScript->m_pGlobalFunc;	break;
		case SCRIPT_STRUCTURE:	storage << (DWORD)pScript->m_pStructType;	break;
		}
	}
	storage << (DWORD) NULL;

	m_mutexScripts.Unlock();

// Zkopirovat vsechny zdrojaky a binarky (uzivatelske)
    CDataArchive *pDataArchive;
    pDataArchive = storage.GetArchive();

    // vytvoreni adresare se jmenem civilizace
	ASSERT( m_pZCivilization != NULL);
    CString strCivName = m_pZCivilization->GetCivilizationName();
    pDataArchive->MakeDirectory( strCivName);

    pDataArchive->MakeDirectory( strCivName + "\\"DIRECTORY_SOURCE);
    pDataArchive->MakeDirectory( strCivName + "\\"DIRECTORY_BYTECODE);

    pDataArchive->AppendDir( strCivName + "\\"DIRECTORY_SOURCE, *CDataArchive::GetRootArchive(), 
                             m_strSourceUserPath);
    pDataArchive->AppendDir( strCivName + "\\"DIRECTORY_BYTECODE, *CDataArchive::GetRootArchive(), 
                             m_strCodeUserPath);
    
}    

//////////////////////////////////////////////////////////////////////////////////////
/*
void CCodeManager::CopyFileToArchive( CString &strSrcPath, CString &strDestArchivePath, 
                                      LPCTSTR lpszFilename, CDataArchive *pDataArchive,
                                      CFileException *pFE)
{
    CFile SrcFile;
    CArchiveFile DestFile;
    char pBuffer[1024];
    int nRead;

    if (!SrcFile.Open( strSrcPath + "\\" + lpszFilename, CFile::modeRead | CFile::shareDenyWrite, pFE)) throw pFE;
    DestFile = pDataArchive->CreateFile( strDestArchivePath + "\\" + lpszFilename, 
            CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);

    while ( (nRead = SrcFile.Read( pBuffer, 1024)) != 0)
    {
        DestFile.Write( pBuffer, nRead);
    }

    SrcFile.Close();
    

    if ( (::GetFileAttributes( strSrcPath + "\\" + lpszFilename) & FILE_ATTRIBUTE_READONLY) != 0)
    {
        DestFile.SetReadOnly( true);
    }

    DestFile.Close();

    return;
}

void CCodeManager::CopyFileFromArchive( CString &strSrcArchivePath, CString &strDestPath, 
                                        LPCTSTR lpszFilename, CDataArchive *pDataArchive,
                                        CFileException *pFE)
{
    CArchiveFile SrcFile;
    CFile DestFile;
    char pBuffer[1024];
    int nRead;

    if (!DestFile.Open( strDestPath + "\\" + lpszFilename, CFile::modeWrite | CFile::modeCreate, pFE)) throw pFE;
    SrcFile = pDataArchive->CreateFile( strSrcArchivePath + "\\" + lpszFilename, 
                                        CArchiveFile::modeRead | CArchiveFile::modeUncompressed | CFile::shareDenyWrite);

    while ( (nRead = SrcFile.Read( pBuffer, 1024)) != 0)
    {
        DestFile.Write( pBuffer, nRead);
    }

    DestFile.Close();

    if ( SrcFile.IsReadOnly())
    {
        ::SetFileAttributes(  strDestPath + "\\" + lpszFilename,
                  ::GetFileAttributes( strDestPath + "\\" + lpszFilename) | FILE_ATTRIBUTE_READONLY);
    }

    SrcFile.Close();

    return;
    ASSERT(false);
}
 */
//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::PersistentLoad( CPersistentStorage &storage, CString &strUserScriptsPath, 
                                   CString &strScriptSetPath, CString &strCivilizationName)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	CCodeManager *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// notifier
	CNotifier::PersistentLoad( storage);

    // civilizace
    storage >> (DWORD&)m_pZCivilization;

	// naloadovani jmen a typu skriptu
	CStringTableItem *stiName;
	CScript *pScript;

	m_ptpPersistentLoadScripts = new CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CScript*>;
	m_ptpPersistentLoadScripts->InitHashTable( SCRIPTS_HASHTABLE_SIZE);

	storage >> (DWORD&)stiName;
	while (stiName != NULL)
	{
		pScript = new CScript();
		pScript->m_stiName = stiName;
		
		storage >> (DWORD&)pScript->m_eScriptType;

		switch ( pScript->m_eScriptType)
		{
		case SCRIPT_UNIT:		storage >> (DWORD&)pScript->m_pIUnitType;	break;
		case SCRIPT_FUNCTION:	storage >> (DWORD&)pScript->m_pGlobalFunc;	break;
		case SCRIPT_STRUCTURE:	storage >> (DWORD&)pScript->m_pStructType;	break;
		}

		m_ptpPersistentLoadScripts->SetAt(stiName, pScript);

		storage >> (DWORD&)stiName;
	}

  

// Adresare:
    DWORD attr;

    // cesta k user bytekodum
    m_strCodeUserPath.Format("%s\\%s\\", strUserScriptsPath, DIRECTORY_BYTECODE);
    ::CreateDirectory( m_strCodeUserPath, NULL);
    if ((attr=::GetFileAttributes(m_strCodeUserPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strCodeUserPath);

    // cesta k user zdrojakum
    m_strSourceUserPath.Format("%s\\%s\\", strUserScriptsPath, DIRECTORY_SOURCE);
    ::CreateDirectory( m_strSourceUserPath, NULL);
    if ((attr=::GetFileAttributes(m_strSourceUserPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strSourceUserPath);

    // cesta k system bytekodum
    m_strCodeSystemPath.Format("%s\\%s\\", strScriptSetPath, DIRECTORY_BYTECODE);
    ::CreateDirectory( m_strCodeSystemPath, NULL);
    if ((attr=::GetFileAttributes(m_strCodeSystemPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strCodeSystemPath);

    // cesta k system zdrojakum
    m_strSourceSystemPath.Format("%s\\%s\\", strScriptSetPath, DIRECTORY_SOURCE);
    ::CreateDirectory( m_strSourceSystemPath, NULL);
    if ((attr=::GetFileAttributes(m_strSourceSystemPath)) == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		throw new CStringException("Cannot create directory %s.", m_strSourceSystemPath);

    // stejne adresare -> chyba
    if (m_strSourceUserPath==m_strSourceSystemPath) 
        throw new CStringException("The user and script set source directories cannot be the same.");
    if (m_strCodeUserPath==m_strCodeSystemPath) 
        throw new CStringException("The user and script set source directories cannot be the same.");

// Zkopirovat vsechny zdrojaky a binarky (uzivatelske) zpet
    
    CDataArchive *pDataArchive = storage.GetArchive();
    CDataArchive::GetRootArchive()->AppendDir( m_strSourceUserPath, *pDataArchive, 
                                               strCivilizationName + "\\"DIRECTORY_SOURCE);
    CDataArchive::GetRootArchive()->AppendDir( m_strCodeUserPath, *pDataArchive, 
                                               strCivilizationName + "\\"DIRECTORY_BYTECODE);
     
    /*
    CDataArchive *pDataArchive = storage.GetArchive();

    CString strSrcPath, strDestPath;
    CFileException *pFE = new CFileException();
    int i;

    CDataArchiveDirContents *pDirCont;
    CDataArchiveFileDirectoryItem *pInfo;

    for (i=0; i<2; i++)
    {
        if (i==0)
        {
            strDestPath = m_strSourceUserPath;
            strSrcPath = strCivilizationName + "\\"DIRECTORY_SOURCE;
        }
        else
        {
            strDestPath = m_strCodeUserPath;
            strSrcPath = strCivilizationName + "\\"DIRECTORY_BYTECODE;
        }

        pDirCont = pDataArchive->GetDirContents( strSrcPath);
        
        if (pDirCont->MoveFirst())
        {
            do
            { 
                pInfo = pDirCont->GetInfo();
                CopyFileFromArchive( strSrcPath, strDestPath, pInfo->GetName(), pDataArchive, pFE);

            } while ( pDirCont->MoveNext());
        }
    }

    pFE->Delete();
    pFE = NULL;
    */
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// notifier
	CNotifier::PersistentTranslatePointers( storage);

    // civilizace
    m_pZCivilization = (CZCivilization *)storage.TranslatePointer(m_pZCivilization);

	// naloadovani interface defaultni unity
	CreateDefaultUnit();

	// vytvoreni informace o dedicnosti jednotek
	CreateUnitInheritanceInfo();

    // naloadovani skriptu & zaregistrovani jejich pointeru
	m_mutexScripts.Lock();

	POSITION pos;
	CStringTableItem *stiName;
	CScript *pOldScript, *pNewScript;

	pos = m_ptpPersistentLoadScripts->GetStartPosition();

	while (pos != NULL)
	{
		m_ptpPersistentLoadScripts->GetNextAssoc( pos, stiName, pOldScript);

		// prelozit pointer
		pOldScript->m_stiName = (CStringTableItem*) storage.TranslatePointer( pOldScript->m_stiName);

		// naloadovat, pokud uz neni naloaodvany, & zaregistrovat pointer
		if (!m_tpScripts.Lookup( LowerCase( pOldScript->m_stiName), pNewScript))
		{
			switch (pOldScript->m_eScriptType)
			{
			case SCRIPT_UNIT:
				pNewScript = LoadUnit( pOldScript->m_stiName);
				break;
			case  SCRIPT_FUNCTION:
				pNewScript = LoadGlobalFunction( pOldScript->m_stiName);
				break;
			case SCRIPT_STRUCTURE:
				pNewScript = LoadStructure( pOldScript->m_stiName);
				break;
			default:
				ASSERT(false);
			}
		}

		if (pNewScript == NULL)
		{
    	    m_mutexScripts.Unlock();
			CString str;
			str.Format("Cannot load script '%s'", pOldScript->m_stiName);
            delete pOldScript;
			throw new CStringException(str);
		}

		switch (pOldScript->m_eScriptType)
		{
		case SCRIPT_UNIT:
			storage.RegisterPointer( pOldScript->m_pIUnitType, pNewScript->m_pIUnitType);
			break;
		case  SCRIPT_FUNCTION:
			storage.RegisterPointer( pOldScript->m_pGlobalFunc, pNewScript->m_pGlobalFunc);
			break;
		case SCRIPT_STRUCTURE:
			storage.RegisterPointer( pOldScript->m_pStructType, pNewScript->m_pStructType);
			break;
		default:
			ASSERT(false);
		}

		// pridat
		m_tpScripts.SetAt( LowerCase( pNewScript->m_stiName, true), pNewScript);

		// uvolnit puvodni
		delete pOldScript;
	}

	delete m_ptpPersistentLoadScripts;
	m_ptpPersistentLoadScripts = NULL;

	m_mutexScripts.Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::PersistentInit( CInterpret *pInterpret)
{
	// notifier
	CNotifier::PersistentInit();

    m_CodeManagerForCompiler.Init( this);

    m_bCreated = true;
}

// musi se volat PersistentInit s parametrem Interpret
void CCodeManager::PersistentInit() {  ASSERT(false); }

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::MoveFile( CString &strSrcFileName, CString &strDestFileName)
{
    this->DeleteFile( strDestFileName);
	VERIFY (::MoveFile(strSrcFileName, strDestFileName));
}

//////////////////////////////////////////////////////////////////////////////////////

void CCodeManager::DeleteFile( CString &strFileName)
{
    ::SetFileAttributes(strFileName, FILE_ATTRIBUTE_NORMAL);
    ::DeleteFile(strFileName);  
}

//////////////////////////////////////////////////////////////////////////////////////

CIUnitTypeList* CCodeManager::GetUnitChildren( CIUnitType *pSearchedUnitType)
{
    CIUnitTypeList *pChildren = NULL;

    CScript *pScript;
    CStringTableItem *stiName;
    POSITION pos;
    CIUnitType *pUnitType;

    m_mutexScripts.Lock();

    pos = m_tpScripts.GetStartPosition();
    while (pos != NULL)
    {
        m_tpScripts.GetNextAssoc( pos, stiName, pScript);
        if (pScript->m_eScriptType == SCRIPT_UNIT)
        {
            pUnitType = pScript->m_pIUnitType->m_pParent;
            while (pUnitType != NULL && pUnitType != CCodeManager::m_pDefaultUnitType)
            {
                if ( pUnitType == pSearchedUnitType) 
                {
                    pChildren = new CIUnitTypeList( pUnitType, pChildren);
                    break;
                }

                pUnitType = pUnitType->m_pParent;
            }
        }
    }

    m_mutexScripts.Unlock();

    return pChildren;
}

//////////////////////////////////////////////////////////////////////

void CCodeManager::ClearLocalCounters()        
{  
    m_pZCivilization->m_pInterpret->ClearLocalProfileCounter();
    ClearCounters( CIConstructor::ClearLocalProfileCounter); 
}

//////////////////////////////////////////////////////////////////////

void CCodeManager::ClearGlobalAndLocalCounters()   
{  
    m_pZCivilization->m_pInterpret->ClearGlobalAndLocalProfileCounter();
    ClearCounters( CIConstructor::ClearGlobalAndLocalProfileCounters);
}

//////////////////////////////////////////////////////////////////////

void CCodeManager::ClearCounters( void (CIConstructor::*pClearMethod)())
{
    m_mutexScripts.Lock();

    POSITION pos = m_tpScripts.GetStartPosition();
    CScript *pScript;
    CStringTableItem *stiName;
    while ( pos != NULL)
    {
        m_tpScripts.GetNextAssoc( pos, stiName, pScript);

        switch (pScript->m_eScriptType)
        {
        case SCRIPT_UNIT:
            {
                CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIMethod*> *ptpMethods = &pScript->m_pIUnitType->m_tpMethods;
                CIMethod *pMethod;
                POSITION pos2 = ptpMethods->GetStartPosition();
                while (pos2 != NULL)
                {
                    ptpMethods->GetNextAssoc( pos2, stiName, pMethod);
                    (pMethod->*pClearMethod)();
                }   
            }
            {
                CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIEvent*> *ptpEvents = &pScript->m_pIUnitType->m_tpEvents;
                CIEvent *pEvent;
                POSITION pos2 = ptpEvents->GetStartPosition();
                while (pos2 != NULL)
                {
                    ptpEvents->GetNextAssoc( pos2, stiName, pEvent);
                    (pEvent->*pClearMethod)();
                }   
            }
            break;

        case SCRIPT_FUNCTION:
            (pScript->m_pGlobalFunc->*pClearMethod)();
            break;
        }
    }

    m_mutexScripts.Unlock();
}

//////////////////////////////////////////////////////////////////////

void PrintCounters( CString &strResult, LONG nLocalCounter, LONG nGlobalCounter,
                    double nLocalInterpretCounter, double nGlobalInterpretCounter,
                    CStringTableItem *stiUnitName, CStringTableItem *stiFunctionName)
{
    CString strTmp;

    if ( stiUnitName != NULL) strResult += (CString)*stiUnitName + "::";

    strResult += (CString)*stiFunctionName + "/";
    
    if ( nLocalInterpretCounter == 0) strResult += "-/";
    else 
    {
        double nPercents = (double)((double)nLocalCounter / nLocalInterpretCounter);
        if ( nPercents != 0) 
        {
            strTmp.Format( "%.1f%%/", nPercents);
            strResult += strTmp;
        }
        else strResult += "-/";
    }

    if ( nGlobalInterpretCounter == 0) strResult += "-|";
    else 
    {
        double nPercents = (double)((double)nGlobalCounter / nGlobalInterpretCounter);
        if ( nPercents != 0) 
        {
            strTmp.Format( "%.1f%%|", nPercents);
            strResult += strTmp;
        }
        else strResult += "-|";
    }
}

//////////////////////////////////////////////////////////////////////

void CCodeManager::GetCounters( CString &strResult)
{
    strResult.Empty();

    double nLocalInterpretCounter = (double)m_pZCivilization->m_pInterpret->GetLocalProfileCounter() / 100;
    double  nGlobalInterpretCounter = (double)m_pZCivilization->m_pInterpret->GetGlobalProfileCounter() / 100;

    m_mutexScripts.Lock();

    POSITION pos = m_tpScripts.GetStartPosition();
    CScript *pScript;
    CStringTableItem *stiName;
    ULONG nLocalCounter, nGlobalCounter;
    CString strTmp;
    while ( pos != NULL)
    {
        m_tpScripts.GetNextAssoc( pos, stiName, pScript);

        switch (pScript->m_eScriptType)
        {
        case SCRIPT_UNIT:
          
            // defaultni jednotka -> nee
            if ( pScript->m_pIUnitType->m_pParent == NULL) continue;

            // constructor
            {
                CIConstructor *pConstructor = pScript->m_pIUnitType->m_pConstructor;
                nLocalCounter = pConstructor->GetLocalProfileCounter();
                nGlobalCounter = pConstructor->GetGlobalProfileCounter();

                if ( nLocalCounter + nGlobalCounter != 0) 
                {
                  PrintCounters( strResult, nLocalCounter, nGlobalCounter, nLocalInterpretCounter, 
                         nGlobalInterpretCounter, pScript->m_pIUnitType->m_stiName, pScript->m_pIUnitType->m_stiName); 
                }
            }
            // methods
            {
                CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIMethod*> *ptpMethods = &pScript->m_pIUnitType->m_tpMethods;
                CIMethod *pMethod;
                CStringTableItem *stiMethodName;
                POSITION pos2 = ptpMethods->GetStartPosition();
                while (pos2 != NULL)
                {
                    ptpMethods->GetNextAssoc( pos2, stiMethodName, pMethod);
                    nLocalCounter = pMethod->GetLocalProfileCounter();
                    nGlobalCounter = pMethod->GetGlobalProfileCounter();
                    if ( nLocalCounter + nGlobalCounter == 0) continue;

                    PrintCounters( strResult, nLocalCounter, nGlobalCounter, nLocalInterpretCounter, 
                           nGlobalInterpretCounter, pScript->m_pIUnitType->m_stiName, pMethod->m_stiName); 
                }   
            }
            // events
            {
                CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIEvent*> *ptpEvents = &pScript->m_pIUnitType->m_tpEvents;
                CIEvent *pEvent;
                CStringTableItem *stiEventName;
                POSITION pos2 = ptpEvents->GetStartPosition();
                while (pos2 != NULL)
                {
                    ptpEvents->GetNextAssoc( pos2, stiEventName, pEvent);
                    nLocalCounter = pEvent->GetLocalProfileCounter();
                    nGlobalCounter = pEvent->GetGlobalProfileCounter();
                    if ( nLocalCounter + nGlobalCounter == 0) continue;

                    PrintCounters( strResult, nLocalCounter, nGlobalCounter, nLocalInterpretCounter, 
                           nGlobalInterpretCounter, pScript->m_pIUnitType->m_stiName, pEvent->m_stiName); 

                }   
            }
            break;

        case SCRIPT_FUNCTION:
            nLocalCounter = pScript->m_pGlobalFunc->GetLocalProfileCounter();
            nGlobalCounter = pScript->m_pGlobalFunc->GetGlobalProfileCounter();
            if ( nLocalCounter + nGlobalCounter == 0) continue;
            
            PrintCounters( strResult, nLocalCounter, nGlobalCounter, nLocalInterpretCounter, 
                           nGlobalInterpretCounter, NULL, pScript->m_pGlobalFunc->m_stiName); 

            break;

        default:
            continue;
        }


    }

    m_mutexScripts.Unlock();
}

//////////////////////////////////////////////////////////////////////

void CCodeManager::LoadDefaultUnit()
{
	// tohle muze hodit vyjimku
	CArchiveFile file = g_cSFileManager.GetFile(DEFAULTUNIT_FILE);

	m_pLexDefaultUnit = new CLexDefaultUnit(&file);
	m_pHelperDefaultUnit = new CIUnitType();

	// TODO: Vyladit hodnoty podle realeho poctu polozek/metod/eventu.
	m_pHelperDefaultUnit->m_tpMembers.InitHashTable(10);
	m_pHelperDefaultUnit->m_tpMethods.InitHashTable(10);

	m_aDefaultUnitMethodArgs.SetSize(20,0);

	try
	{
		SDefUnitparse(NULL);
	}
	catch (CException *e)
	{
		delete m_pLexDefaultUnit;
		delete m_pHelperDefaultUnit;
		file.Close();
		throw e;
	}

	m_aDefaultUnitMethodArgs.SetSize(0,0);

	delete m_pLexDefaultUnit;

	// nastaveni staticke polozky
	ASSERT(CCodeManager::m_pDefaultUnitType == NULL);
	CCodeManager::m_pDefaultUnitType = m_pHelperDefaultUnit;
}

//////////////////////////////////////////////////////////////////////