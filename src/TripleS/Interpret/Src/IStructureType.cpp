/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIStructureType uchovavajici
 *          informace o nejake strukture (o typu jejich polozek..).
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIStructureType::CIStructureType()
{
    m_stiName = NULL;
    m_nMemberCount = 0;
    m_bLoaded = false;

    m_pCodeStringTable = NULL;
}

CIStructureType::~CIStructureType()
{
    Clean();
}

#ifdef _DEBUG
void CIStructureType::AssertValid() const
{
    ASSERT(m_bLoaded);
}
#endif

void CIStructureType::Clean()
{
    POSITION pos;
    CIMember *pMember;
    CStringTableItem *pSTI;

    // vyprazdneni a uvolneni tabulky polozek
    pos = m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        m_tpMembers.GetNextAssoc(pos, pSTI, pMember);
        delete pMember;
        pSTI->Release();
    }
    m_tpMembers.RemoveAll();
    m_nMemberCount = 0;

    if (m_stiName != NULL) m_stiName = m_stiName->Release();

    // uvolneni stringtable
    if (m_pCodeStringTable != NULL)
    {
        m_pCodeStringTable->Clean();
        delete m_pCodeStringTable;
        m_pCodeStringTable = NULL;
    }
}

void CIStructureType::Load(CFile &file)
{
    char str[MAX_IDENTIFIER_LENGTH+1];
    short len, i;
    long heapSize;
    CIMember **apMembers = NULL;  // pole ukazatelu na membery

    ASSERT(!m_bLoaded);

	// sign
	file.Read(str, strlen(STRUCTFILE_FIRTSBYTES));
	str[strlen(STRUCTFILE_FIRTSBYTES)]=0;
	if (strcmp(str, STRUCTFILE_FIRTSBYTES)!=0) 
        throw new CStringException("%s: bad file sign (first %d bytes)", 
            file.GetFileName(), strlen(STRUCTFILE_FIRTSBYTES));

	// jmeno struktury
	file.Read(&len, sizeof(short));
	file.Read(str, len);
	str[len]=0;
	m_stiName=g_StringTable.AddItem(str);

	// pocet polozek ve strukture
	file.Read(&m_nMemberCount, sizeof(short));
    ASSERT(m_nMemberCount>=0);

    // nacteni polozek
    heapSize=0;
    if (m_nMemberCount != 0)
    {
        apMembers = (CIMember**) new char[sizeof(CIMember*)*m_nMemberCount];
        if (apMembers == NULL) throw new CStringException("Can't allocate %d bytes.", m_nMemberCount*sizeof(CIMember*));
    }
    
    for (i = 0; i < m_nMemberCount; i++)
    {
        // vytvoreni memberu
        apMembers[i] = new CIMember;

        // nacteni memberu
        apMembers[i]->Load( file, heapSize);

        // vypocet pffsetu dalsi polozky
        if (apMembers[i]->m_DataType.IsFloat()) heapSize += 2; // == sizeof(double)/sizeof(int)
        else heapSize += 1; // max sizeof(pointer, int, bool, ..)/sizeof(int), proste ze vseho ostatniho
        // sizeof(double)==8 => hezky zarovnane :-)
    }

    // celkova velikost pameti
    m_nHeapSize=heapSize;

    // nacteni stringtable
    m_pCodeStringTable = new CICodeStringTable;
    m_pCodeStringTable->Load(file);

// KONVERZE JMEN & pridani do hashovaci tabulky
    m_tpMembers.InitHashTable(m_nMemberCount);

    for (i=0; i<m_nMemberCount; i++)
    {
        // pridani do tabulky
        apMembers[i]->Translate( m_pCodeStringTable);
        m_tpMembers.SetAt( apMembers[i]->m_stiName->AddRef(), apMembers[i]);
    }    

    delete [] apMembers;

    m_bLoaded=true;
}

EImplementation CIStructureType::GetMember(CStringTableItem *stiName, CIMember* &pMember)
{
    if (m_tpMembers.Lookup(stiName, pMember)) return IMPL_SCRIPT;
    else return IMPL_NONE;
}

void CIStructureType::UpdateStructFrom(CIStructureType *pFrom)
{
    Clean();

    CStringTableItem *stiName;
    CIMember *pMember;

    POSITION pos = pFrom->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        pFrom->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
        m_tpMembers.SetAt( stiName, pMember);
        pFrom->m_tpMembers.SetAt( stiName, NULL);
    }

    m_nMemberCount = pFrom->m_nMemberCount;
    m_nHeapSize = pFrom->m_nHeapSize;
    m_pCodeStringTable = pFrom->m_pCodeStringTable;
    m_stiName = pFrom->m_stiName->AddRef();

    pFrom->m_pCodeStringTable = NULL; // aby se ji nepokousel uvolnit
    pFrom->m_nMemberCount = 0;
    pFrom->m_nHeapSize = 0;
}
