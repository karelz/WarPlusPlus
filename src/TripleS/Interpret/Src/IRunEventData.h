
#if !defined(AFX_IRUNEVENTDATA_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
#define AFX_IRUNEVENTDATA_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_

class CZUnit;
class CZUnitNotifyQueueItem;
class CZUnitCommandQueueItem;

#include "IBag.h"

class CIRunEventData
{
    DECLARE_MULTITHREAD_POOL(CIRunEventData);

public:
    CIRunEventData( CZUnit *pUnit, EEventType eEventType, CZUnitNotifyQueueItem *pNotifyOrCommand);
    ~CIRunEventData();

// polozky
    CZUnit* m_pUnit;
    EEventType m_eEventType;
    CZUnitNotifyQueueItem *m_pNotifyOrCommand;

};

#endif // AFX_IRUNEVENTDATA_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_