#ifndef _SKILLTYPENAME_H
#define _SKILLTYPENAME_H

#include "TripleS\CICommon\Src\StringTable.h"

inline CStringTableItem* RegisterSkillTypeName( LPCTSTR lpcszName)
{
    return g_StringTable.AddItem( lpcszName);
}

inline void UnregisterSkillTypeName( CStringTableItem *m_stiSkillTypeName)
{
    m_stiSkillTypeName->Release();
}

#endif // _SKILLTYPENAME_H

