
#include "stdafx.h"
#include "../Interpret.h"

#include "GameServer\Civilization\Src\ZUnit.h"

DEFINE_MULTITHREAD_POOL(CIRunEventData, 5);

CIRunEventData::CIRunEventData(CZUnit *pUnit, EEventType eEventType, CZUnitNotifyQueueItem *pNotifyOrCommand)  
{
    ASSERT( pUnit != NULL);
    ASSERT( pNotifyOrCommand != NULL);
	ASSERT( pNotifyOrCommand->m_stiEventName != NULL);

    m_pUnit = pUnit;
    m_eEventType = eEventType;

    ASSERT( eEventType == EVENT_NOTIFICATION || eEventType == EVENT_COMMAND);

    m_pNotifyOrCommand = pNotifyOrCommand->AddRef();
}

CIRunEventData::~CIRunEventData() 
{
    if (m_pNotifyOrCommand != NULL) m_pNotifyOrCommand = m_pNotifyOrCommand->Release();
}