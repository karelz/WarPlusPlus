// IGlobalFunction.cpp: implementation of the CIGlobalFunction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIGlobalFunction::CIGlobalFunction()
{
    m_stiName = NULL;
    m_bLoaded = false;
    m_nArgCount = 0;
    m_apArgAndLocalVars = NULL;

    m_pCodeStringTable = NULL;
    m_pFloatTable = NULL;
}

CIGlobalFunction::~CIGlobalFunction()
{
    Clean();
}

#ifdef _DEBUG
void CIGlobalFunction::AssertValid() const
{
    CIMethod::AssertValid();
    ASSERT(m_bLoaded);
}
#endif

void CIGlobalFunction::Clean()
{
    if (m_nArgAndLocalVarCount!=0 && m_apArgAndLocalVars!=NULL) 
    {
        delete [] m_apArgAndLocalVars;
        m_apArgAndLocalVars = NULL;
    }

    if (m_stiName!=NULL) m_stiName = m_stiName->Release();

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

void CIGlobalFunction::Load(CFile &file)
{
    char str[MAX_IDENTIFIER_LENGTH+1];
    short len, i;

    ASSERT(!m_bLoaded);

	// sign
	file.Read(str, strlen(FUNCTIONFILE_FIRTSBYTES));
	str[strlen(FUNCTIONFILE_FIRTSBYTES)]=0;
	if (strcmp(str, FUNCTIONFILE_FIRTSBYTES)!=0) 
        throw new CStringException("%s: bad file sign (first %d bytes)", 
            file.GetFileName(), strlen(FUNCTIONFILE_FIRTSBYTES));

	// jmeno funkce
	file.Read(&len, sizeof(short));
	file.Read(str, len);
	str[len]=0;
    if (m_stiName!=NULL) m_stiName->Release();
	m_stiName=g_StringTable.AddItem(str);

    // navratovy typ
    m_ReturnType.Load(file);

    // pocet argumentu
	file.Read(&m_nArgCount, sizeof(short));
    ASSERT(m_nArgCount>=0);

    // pocet argumentu + lokalnich promennych
	file.Read(&m_nArgAndLocalVarCount, sizeof(short));
    ASSERT(m_nArgAndLocalVarCount>=0);

    // nacteni typu argumentu & lokalnich promennych
    if (m_nArgAndLocalVarCount != 0)
    {
        m_apArgAndLocalVars = new CIType[m_nArgAndLocalVarCount]; 
        for (i=0; i<m_nArgAndLocalVarCount; i++) 
        {
            m_apArgAndLocalVars[i].Load(file);
        }
    }

    // nacteni kodu
    m_Code.Load(file);

    // nacteni tabulek
    m_pCodeStringTable = new CICodeStringTable;
    m_pCodeStringTable->Load(file);

    m_pFloatTable = new CFloatTable;
    m_pFloatTable->Load(file);

// KONVERZE JMEN 
    m_ReturnType.Translate(m_pCodeStringTable);
    for (i=0; i<m_nArgAndLocalVarCount; i++) 
    {
        m_apArgAndLocalVars[i].Translate(m_pCodeStringTable);
    }

    m_bLoaded = true;
}

