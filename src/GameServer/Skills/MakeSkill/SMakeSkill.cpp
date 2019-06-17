/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Skila pro vytvareni jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SMakeSkill.h"
#include "ISysCallMake.h"

///////////////////////////////////////////////////////////////////////////
// SSMakeSkillRecord

SSMakeSkillRecord *SSMakeSkillRecord::GetNext() 
{
    return (SSMakeSkillRecord*)(this+sizeof(SSMakeSkillRecord));
}

/* Format ulozeni dat SSMakeSkillRecord

  m_dwUnitType
  m_dwTimeToMake
  m_ResourcesNeeded

*/

DWORD SSMakeSkillRecord::GetDataSize() 
{
    return sizeof(SSMakeSkillRecord);
}

#ifdef _DEBUG

BOOL SSMakeSkillRecord::AssertValid() 
{
    ASSERT(m_nTimeToMake>=0);
    return TRUE;
}

#endif

///////////////////////////////////////////////////////////////////////////
// SSMakeSkill

SSMakeSkillRecord *SSMakeSkill::GetFirstRecord() 
{ 
    return(SSMakeSkillRecord*)(this+sizeof(SSMakeSkill)); 
}

/* Format ulozeni dat SSMakeSkill
    int m_nRecords;

    opakuj(m_nRecords krat) {
        SSMakeSkill record;        
    }
*/

DWORD SSMakeSkill::GetDataSize() {
    DWORD dwSize=0;

    dwSize+=sizeof(m_nRecords);

    SSMakeSkillRecord *pRecord=GetFirstRecord();

    for(int i=0; i<m_nRecords; i++) {
        dwSize+=pRecord->GetDataSize();
        
        pRecord=pRecord->GetNext();
    }

    return dwSize;
}

#ifdef _DEBUG

BOOL SSMakeSkill::AssertValid(DWORD dwSize)
{
    LOAD_ASSERT(dwSize>=sizeof(SSMakeSkill));
    LOAD_ASSERT(m_nRecords>=0 && m_nRecords<=1000000);

    DWORD dwChecked=sizeof(SSMakeSkill);
    
    SSMakeSkillRecord *pRecord=GetFirstRecord();

    for(int i=0; i<m_nRecords; i++) {
        dwChecked+=pRecord->GetDataSize();
        LOAD_ASSERT(dwChecked<=dwSize);
        pRecord=pRecord->GetNext();
    }

    LOAD_ASSERT(dwChecked==dwSize);

    return TRUE;
}

#endif

#ifdef _DEBUG

///////////////////////////////////////////////////////////////////////////
// SSMakeSkillUnit

BOOL SSMakeSkillUnit::AssertValid()
{
    ASSERT(m_nTimeRemaining>=0);
    ASSERT(m_pUnitType!=NULL);
    
    return TRUE;
}

#endif