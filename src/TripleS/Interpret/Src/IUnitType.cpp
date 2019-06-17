/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIUnitType uchovavajici informace
 *          o jednom typu jednotky.
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

DEFINE_MULTITHREAD_POOL( CIUnitTypeList, 10);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIUnitType::CIUnitType()
{
    m_stiName = NULL;
    m_stiParentName = NULL;
    m_pParent = NULL;

    m_nMemberCount = 0;
    m_nHeapSize = 0;
    m_nMethodCount = 0;
    m_nEventCount = 0;

    m_pConstructor = NULL;
    m_pDestructor = NULL;

    m_pCodeStringTable = NULL;
    m_pFloatTable = NULL;
        
    m_bLoaded = false;
}

CIUnitType::~CIUnitType()
{
    Clean();
}

#ifdef _DEBUG
void CIUnitType::AssertValid() const
{
    CIStructureType::AssertValid();
    ASSERT(m_bLoaded);
}
#endif

void CIUnitType::Clean()
{
    POSITION pos;
    CStringTableItem *pSTI = NULL;
    CIMember *pMember = NULL;
    CIMethod *pMethod = NULL;
    CIEvent *pEvent = NULL;

    if (m_stiName != NULL) m_stiName = m_stiName->Release();
    if (m_stiParentName != NULL) m_stiParentName = m_stiParentName->Release();

    m_pParent = NULL;

    //konstruktor
    if (m_pConstructor != NULL) 
    {
        delete m_pConstructor;
        m_pConstructor = NULL;
    }

    // destruktor
    if (m_pDestructor != NULL) 
    {
        delete m_pDestructor;
        m_pDestructor = NULL;
    }

    // polozky
    pos = m_tpMembers.GetStartPosition();
    while (pos!=NULL)
    {
        m_tpMembers.GetNextAssoc(pos, pSTI, pMember);
        pSTI->Release();
        delete pMember;
    }
    m_tpMembers.RemoveAll();
    m_nMemberCount = 0;

    // metody
    pos = m_tpMethods.GetStartPosition();
    while (pos!=NULL)
    {
        m_tpMethods.GetNextAssoc(pos, pSTI, pMethod);
        pSTI->Release();
        delete pMethod;
    }
    m_tpMethods.RemoveAll();
    m_nMethodCount = 0;

    // eventy
    pos = m_tpEvents.GetStartPosition();
    while (pos!=NULL)
    {
        m_tpEvents.GetNextAssoc(pos, pSTI, pEvent);
        pSTI->Release();
        delete pEvent;
    }
    m_tpEvents.RemoveAll();
    m_nEventCount = 0;

    // tabulky
    if (m_pCodeStringTable != NULL)
    {
        m_pCodeStringTable->Clean();
        delete m_pCodeStringTable;
        m_pCodeStringTable = NULL;
    }

    if (m_pFloatTable != NULL)
    {
        m_pFloatTable->Clean();
        delete m_pFloatTable;
        m_pFloatTable = NULL;
    }
}

void CIUnitType::Load(CFile &file, CIUnitType *pParentUnit)
{
    char str[MAX_IDENTIFIER_LENGTH+1];
    short len, i;
    long heapSize;
    CIMember **apMembers = NULL;  // temporary pole ukazatelu na membery
    CIMethod **apMethods = NULL;  // temporary pole ukazatelu na metody
    CIEvent **apEvents = NULL;  // temporary pole ukazatelu na eventy

    ASSERT(!m_bLoaded);
    
	// sign
	file.Read(str, strlen(UNITFILE_FIRTSBYTES));
	str[strlen(UNITFILE_FIRTSBYTES)]=0;
	if (strcmp(str, UNITFILE_FIRTSBYTES)!=0) 
        throw new CStringException("%s: bad file sign (first %d bytes)", 
            file.GetFileName(), strlen(UNITFILE_FIRTSBYTES));

	// jmeno unity
	file.Read(&len, sizeof(short));
	file.Read(str, len);
	str[len]=0;
	m_stiName=g_StringTable.AddItem(str);

	// jmeno predka
	file.Read(&len, sizeof(short));
	file.Read(str, len);
	str[len]=0;
	m_stiParentName=g_StringTable.AddItem(str);

    // ukazatel na typ predka
    m_pParent = pParentUnit;

	// pocet polozek v unite
	file.Read(&m_nMemberCount, sizeof(short));
    ASSERT(m_nMemberCount>=0);

// nacteni polozek
    if (m_pParent!=NULL) heapSize=m_pParent->m_nHeapSize;
    else heapSize = 0;

    // temporary alokace
    if (m_nMemberCount != 0)
    {
        apMembers = (CIMember**) new char[sizeof(CIMember*)*m_nMemberCount];
        if (apMembers == NULL) throw new CStringException("Can't allocate %d bytes.", m_nMemberCount*sizeof(CIMember*));
    }
    
    for (i = 0; i < m_nMemberCount; i++)
    {
        // vytvoreni memberu
        apMembers[i] = new CIMember;

        // load memberu
        apMembers[i]->Load(file, heapSize);
        
        // vypocet offsetu dalsi polozky
        if (apMembers[i]->m_DataType.IsFloat()) heapSize += sizeof(double)/sizeof(int);
        else heapSize += 1; // max sizeof(pointer, int, bool, ..), proste ze vseho ostatniho
        // sizeof(double)==8 => hezky zarovnane :-)
    }

    // celkova velikost pameti
    m_nHeapSize=heapSize;

// nacteni konstruktoru
    m_pConstructor = new CIConstructor();
    m_pConstructor->Load(file, this);
    if (m_pConstructor->m_Code.m_nLength==0)
    {
        delete m_pConstructor;
        m_pConstructor = NULL;
    }

// nacteni destruktoru
    m_pDestructor = new CIDestructor();
    m_pDestructor->Load(file, this);
    if (m_pDestructor->m_Code.m_nLength==0)
    {
        delete m_pDestructor;
        m_pDestructor = NULL;
    }

// nacteni metod
    // pocet metod
    file.Read(&m_nMethodCount, sizeof(short));
    ASSERT(m_nMethodCount >= 0);

    // temporary alokace
    if (m_nMethodCount != 0)
    {
        apMethods = (CIMethod**) new char[sizeof(CIMethod*)*m_nMethodCount];
        if (apMethods == NULL) throw new CStringException("Can't allocate %d bytes.", m_nMethodCount*sizeof(CIMethod*));
    }

    for (i = 0; i < m_nMethodCount; i++)
    {
        // vytvoreni methody
        apMethods[i] = new CIMethod;

        // nacteni metody
        apMethods[i]->Load(file, this);
    }

// nacteni eventu
    // pocet eventu
    file.Read(&m_nEventCount, sizeof(short));
    ASSERT(m_nEventCount >= 0);

    // temporary alokace
    if (m_nEventCount != 0)
    {
        apEvents = (CIEvent**) new char[sizeof(CIEvent*)*m_nEventCount];
        if (apEvents == NULL) throw new CStringException("Can't allocate %d bytes.", m_nEventCount*sizeof(CIEvent*));
    }

    for (i = 0; i < m_nEventCount; i++)
    {
        // vytvoreni Eventy
        apEvents[i] = new CIEvent;

        // nacteni metody
        apEvents[i]->Load(file, this);
    }

// nacteni tabulek
    m_pCodeStringTable = new CICodeStringTable;
    m_pCodeStringTable->Load(file);

    m_pFloatTable = new CFloatTable;
    m_pFloatTable->Load(file);

// KONVERZE JMEN & pridani do hashovacich tabulek

    // polozky
    if (m_nMemberCount>0) m_tpMembers.InitHashTable(m_nMemberCount);
    for (i=0; i<m_nMemberCount; i++)
    {
        apMembers[i]->Translate( m_pCodeStringTable);
        m_tpMembers.SetAt( apMembers[i]->m_stiName->AddRef(), apMembers[i]);
    }    
    delete [] apMembers;

    // konstruktor
    if (m_pConstructor != NULL) m_pConstructor->Translate(m_pCodeStringTable);

    // destruktor
    if (m_pDestructor != NULL) m_pDestructor->Translate(m_pCodeStringTable);

    // metody
    if (m_nMethodCount>0) m_tpMethods.InitHashTable(m_nMethodCount);
    for (i=0; i<m_nMethodCount; i++)
    {
        apMethods[i]->Translate(m_pCodeStringTable);
        m_tpMethods.SetAt( apMethods[i]->m_stiName->AddRef(), apMethods[i]);
    }
    delete [] apMethods;

    // eventy
    if (m_nEventCount>0) m_tpEvents.InitHashTable(m_nEventCount);
    for (i=0; i<m_nEventCount; i++)
    {
        apEvents[i]->Translate(m_pCodeStringTable);
        m_tpEvents.SetAt( apEvents[i]->m_stiName->AddRef(), apEvents[i]);
    }
    delete [] apEvents;


    m_bLoaded=true;
}


EImplementation CIUnitType::GetEvent(CStringTableItem *stiName, CIEvent* &pEvent)
{
    // je to defaultni jednotka
    if (m_stiParentName == NULL) 
    {
        if (m_tpEvents.Lookup(stiName, pEvent)) return IMPL_CPLUSPLUS;
        else return IMPL_NONE;
    }
    // je to skriptova jednotka
    else 
    {
        if (m_tpEvents.Lookup(stiName, pEvent)) return IMPL_SCRIPT;
	    else
        {
            ASSERT(m_pParent != NULL);
            return m_pParent->GetEvent(stiName, pEvent);
        }
    }
}

EImplementation CIUnitType::GetMethod(CStringTableItem *stiName, CIMethod* &pMethod)
{
    // je to defaultni jednotka
    if (m_stiParentName == NULL)
    {
        if (m_tpMethods.Lookup(stiName, pMethod)) return IMPL_CPLUSPLUS;
        else return IMPL_NONE;
    }
    // je to skriptova jednotka
    else
    {
        if (m_tpMethods.Lookup(stiName, pMethod)) return IMPL_SCRIPT;
        else
        {
            ASSERT(m_pParent != NULL);
            return m_pParent->GetMethod(stiName, pMethod);
        }
    }
}

EImplementation CIUnitType::GetMember(CStringTableItem *stiName, CIMember* &pMember)
{
    // je to defaultni jednotka
    if (m_stiParentName == NULL)
    {
        if (m_tpMembers.Lookup(stiName, pMember)) return IMPL_CPLUSPLUS;
        else return IMPL_NONE;
    }
    // je to skriptova jednotka
    else
    {
        if (m_tpMembers.Lookup(stiName, pMember)) return IMPL_SCRIPT;
        else
        {
            ASSERT(m_pParent != NULL);
            return m_pParent->GetMember(stiName, pMember);
        }
    }
}

void CIUnitType::UpdateUnitTypeFrom(CIUnitType *pFrom)
{
    Clean();

    CStringTableItem *stiName;
    CIMember *pMember;
    CIEvent *pEvent;
    CIMethod *pMethod;
    POSITION pos;

    // jmeno
    ASSERT( pFrom->m_stiName != NULL);
    m_stiName = pFrom->m_stiName->AddRef();

    // predek
    ASSERT( pFrom->m_stiParentName != NULL);
    m_stiParentName = pFrom->m_stiParentName->AddRef();
    m_pParent = pFrom->m_pParent;

    // members
    pos = pFrom->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        pFrom->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
        m_tpMembers.SetAt( stiName, pMember);
        pFrom->m_tpMembers.SetAt( stiName, NULL);
    }

    m_nMemberCount = pFrom->m_nMemberCount;
    pFrom->m_nMemberCount = 0;

    m_nHeapSize = pFrom->m_nHeapSize;
    pFrom->m_nHeapSize = 0;

    // eventy
    pos = pFrom->m_tpEvents.GetStartPosition();
    while (pos != NULL)
    {
         pFrom->m_tpEvents.GetNextAssoc( pos, stiName, pEvent);
         m_tpEvents.SetAt( stiName, pEvent);
         pEvent->m_pIUnitType = this;
         pFrom->m_tpEvents.SetAt(stiName, NULL);
    }
    m_nEventCount = pFrom->m_nEventCount;

    // metody
    pos = pFrom->m_tpMethods.GetStartPosition();
    while (pos != NULL)
    {
         pFrom->m_tpMethods.GetNextAssoc( pos, stiName, pMethod);
         m_tpMethods.SetAt( stiName, pMethod);
         pMethod->m_pIUnitType = this;
         pFrom->m_tpMethods.SetAt(stiName, NULL);
    }
    m_nMethodCount = pFrom->m_nMethodCount;

    // konstruktor & destruktor
    m_pConstructor = pFrom->m_pConstructor;
    m_pConstructor->m_pIUnitType = this;
    pFrom->m_pConstructor = NULL;

    m_pDestructor = pFrom->m_pDestructor;
    m_pDestructor->m_pIUnitType = this;
    pFrom->m_pDestructor = NULL;

    // tabulky
    m_pCodeStringTable = pFrom->m_pCodeStringTable;
    pFrom->m_pCodeStringTable = NULL;

    m_pFloatTable = pFrom->m_pFloatTable;
    pFrom->m_pFloatTable = NULL;
}

void CIUnitType::ChangeHeapOffset( int nOffsetDelta)
{
    POSITION pos;
    CIMember *pMember;
    CStringTableItem *stiName;

    pos = m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        m_tpMembers.GetNextAssoc( pos, stiName, pMember);
        pMember->m_nHeapOffset += nOffsetDelta;
    }

    m_nHeapSize += nOffsetDelta;
}