// CTimestampedVirtualConnection.cpp: implementation of the CCTimestampedVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTimestampedVirtualConnection.h"
#include "Common/ServerClient/VirtualConnectionIDs.h"
#include "CServerUnitInfoReceiver.h"
#include "../GameClientNetwork.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ******** Hlasky o jednotkach se JENOM TRACUJI ********
//#define NO_UNITS

BEGIN_OBSERVER_MAP(CCTimestampedVirtualConnection, CObserver)
    BEGIN_NOTIFIER(ID_TimestampedVirtualConnection)
        EVENT(E_PACKETAVAILABLE)
            OnPacketAvailable((WORD) dwParam);
            return TRUE;
    END_NOTIFIER()
END_OBSERVER_MAP(CCTimestampedVirtualConnection, CObserver)

CCTimestampedVirtualConnection::CCTimestampedVirtualConnection()
{	
  m_bVirtualConnectionCreated = FALSE;
}

CCTimestampedVirtualConnection::CCTimestampedVirtualConnection(CVirtualConnection Connection, EVirtualConnectionID dwID, CCServerUnitInfoReceiver *pReceiver)
{	
	Create(Connection, dwID, pReceiver);
}

CCTimestampedVirtualConnection::~CCTimestampedVirtualConnection()
{	
}

void CCTimestampedVirtualConnection::Create(CVirtualConnection Connection, EVirtualConnectionID dwID, CCServerUnitInfoReceiver *pReceiver)
{	
	ASSERT(	dwID==VirtualConnection_ClientControl ||		   
			dwID==VirtualConnection_ClientBriefInfos ||
			dwID==VirtualConnection_ClientCheckPointInfos ||
			dwID==VirtualConnection_ClientFullInfos ||
			dwID==VirtualConnection_ClientEnemyFullInfos ||
			dwID==VirtualConnection_ClientPing);	

	m_VirtualConnection=Connection;
  m_bVirtualConnectionCreated = TRUE;
	m_dwID=dwID;
	m_pReceiver=pReceiver;
}

void CCTimestampedVirtualConnection::Delete()
{
  if(m_bVirtualConnectionCreated){
    m_VirtualConnection.DeleteVirtualConnection();
    m_bVirtualConnectionCreated = FALSE;
  }
  m_dwID = 0;
  m_pReceiver = NULL;
}

DWORD CCTimestampedVirtualConnection::GetTime() {
	return m_dwActualTime;
}

void CCTimestampedVirtualConnection::OnPacketAvailable(WORD wConnectionNumber)
{
    // Neni to zprava pro nas
	if(wConnectionNumber != m_VirtualConnection.GetVirtualConnectionNumber())
		return;

	// Zpracovani zpravy
	// Na zacatku compound bloku je vzdy udaj o case
    DWORD dwTime;
    DWORD size;
	BOOL bContinue = TRUE;

    // Zacneme prenos
	VERIFY(m_VirtualConnection.BeginReceiveCompoundBlock());

	// Nacteme cas
	size=sizeof(dwTime);
	VERIFY(m_VirtualConnection.ReceiveBlock((void *)&dwTime, size));
	// Aspon casovy udaj tam MUSI byt
	ASSERT(size==sizeof(dwTime));

	m_pReceiver->NotifyPacketArrived(dwTime);

	// Jedeme, dokud se nevyskoci uprostred
	while(bContinue) {
		switch(m_dwID) {
		case VirtualConnection_ClientControl:
			bContinue=ParseControl(dwTime);
			break;
		case VirtualConnection_ClientBriefInfos:
			bContinue=ParseBriefInfo(dwTime);
			break;
		case VirtualConnection_ClientCheckPointInfos:
			bContinue=ParseCheckPointInfo(dwTime);
			break;
		case VirtualConnection_ClientFullInfos:
			bContinue=ParseFullInfo(dwTime);
			break;
		case VirtualConnection_ClientEnemyFullInfos:
			bContinue=ParseEnemyFullInfo(dwTime);
			break;
		case VirtualConnection_ClientPing:
			bContinue=FALSE;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}

	// Skoncime prijem
	VERIFY(m_VirtualConnection.EndReceiveCompoundBlock());
}

BOOL CCTimestampedVirtualConnection::ParseControl(DWORD dwTime)
{
	UControlConnection uCC;
	BYTE buf[10240];

	DWORD size;

	size=sizeof(uCC.ccHeader);
	VERIFY(m_VirtualConnection.ReceiveBlock(&uCC.ccHeader, size));
	if(size==0) return FALSE;
	if(size!=sizeof(uCC.ccHeader)) {
		// Prisla porusena data
		ASSERT(FALSE);
		return FALSE;
	}

	// Hlavicka je zatim v poradku
	
	// Rozskocime se podle druhu zpravy
	switch(uCC.ccHeader.dwCCMessageType) {

////////////////////////////// PRO ULEHCENI PRACE //////////////////////////
#define READBLK(BLK, BLK2) \
		ASSERT(uCC.ccHeader.dwCCMessageDataLength==sizeof(uCC.BLK)-sizeof(uCC.ccHeader)); \
		size=sizeof(uCC.BLK)-sizeof(uCC.ccHeader); \
		VERIFY(m_VirtualConnection.ReceiveBlock(&(BLK2), size)); \
		ASSERT(size==sizeof(uCC.BLK)-sizeof(uCC.ccHeader)); \
////////////////////////////////////////////////////////////////////////////

	case CCM_StartInfo: // Start info o jednotce
		READBLK(ccStartInfo, (*(SUnitStartInfo*)(&uCC.ccStartInfo)));
		ParseStartInfo(uCC.ccStartInfo, dwTime);
		return TRUE;
		break;

	case CCM_StopInfo: // Stop info o jednotce
		READBLK(ccStopInfo, uCC.ccStopInfo.dwID);
		ParseStopInfo(uCC.ccStopInfo, dwTime);
		return TRUE;
		break;

	case CCM_StartFullInfo: // Start full info
		READBLK(ccStartFullInfo, (*(SUnitStartFullInfo*)(&uCC.ccStartFullInfo)));
		size=sizeof(buf);
		VERIFY(m_VirtualConnection.ReceiveBlock(&buf, size));
		ParseStartFullInfo(uCC.ccStartFullInfo, buf, size, dwTime);
		return FALSE;
		break;

	case CCM_StopFullInfo: // Stop full info
		READBLK(ccStopFullInfo, uCC.ccStopFullInfo.dwID);
		ParseStopFullInfo(uCC.ccStopFullInfo, dwTime);
		return TRUE;
		break;

	case CCM_StartEnemyFullInfo: // Start enemy full info
		READBLK(ccStartEnemyFullInfo, (*(SUnitStartEnemyFullInfo*)(&uCC.ccStartEnemyFullInfo)));
		size=sizeof(buf);
		VERIFY(m_VirtualConnection.ReceiveBlock(&buf, size));
		ParseStartEnemyFullInfo(uCC.ccStartEnemyFullInfo, buf, size, dwTime);
		return FALSE;
		break;

	case CCM_StopEnemyFullInfo: // Stop enemy full info
		READBLK(ccStopEnemyFullInfo, uCC.ccStopEnemyFullInfo.dwID);
		ParseStopEnemyFullInfo(uCC.ccStopEnemyFullInfo, dwTime);
		return TRUE;
		break;
	default:
		ASSERT(FALSE);
		break;
	}

#undef READBLK

	ASSERT(FALSE);
	return FALSE;
}

BOOL CCTimestampedVirtualConnection::ParseBriefInfo(DWORD dwTime)
{
	SUnitBriefInfo bi;
	CCUnit *pUnit;
	DWORD size;

	size=sizeof(bi);
	VERIFY(m_VirtualConnection.ReceiveBlock(&bi, size));
	if(size==0) return FALSE;
	if(size!=sizeof(bi)) {
		// Prisla porusena data
		ASSERT(FALSE);
		return FALSE;
	}
	// Vse je v poradku
#ifndef NO_UNITS
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(bi.dwID);
  if(pUnit) {
    ASSERT_VALID(pUnit);
		// Jednotka dokonce existuje
		pUnit->GetLock()->WriterLock();

		ApplyBriefInfo(pUnit, bi, dwTime);

    pUnit->InlayEvent ( CCUnit::E_BriefInfoChanged, (DWORD)pUnit );

		pUnit->GetLock()->WriterUnlock();
	}
  else{
    return TRUE;
  }

	pUnit->Release();
#else
	pUnit=NULL;
	TRACE("ParseBriefInfo for unit %d\n", bi.dwID);
#endif
	return TRUE;
}

BOOL CCTimestampedVirtualConnection::ParseCheckPointInfo(DWORD dwTime)
{
	SUnitCheckPointInfo cpi;
	CCUnit *pUnit;
	DWORD size;

	size=sizeof(cpi);
	VERIFY(m_VirtualConnection.ReceiveBlock(&cpi, size));
	if(size==0) return FALSE;
	if(size!=sizeof(cpi)) {
		// Prisla porusena data
		ASSERT(FALSE);
		return FALSE;
	}
	// Vse je v poradku

#ifndef NO_UNITS
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(cpi.dwID);
	if(pUnit) {
        ASSERT_VALID(pUnit);
		// Jednotka dokonce existuje
		pUnit->GetLock()->WriterLock();

		if(pUnit->m_dwTimeStampCheckPoint<=dwTime) {
			// Orazitkujeme
			pUnit->m_dwTimeStampCheckPoint=dwTime;

      ApplyCheckPointInfo(pUnit, cpi, dwTime);
    }

		pUnit->GetLock()->WriterUnlock();
	}
  else{
    return TRUE;
  }

	pUnit->Release();

#else
	pUnit=NULL;
	TRACE("ParseCheckPointInfo for unit %d\n", cpi.dwID);
#endif

	return TRUE;
}

BOOL CCTimestampedVirtualConnection::ParseFullInfo(DWORD dwTime)
{
	SUnitFullInfo fi;
	CCUnit *pUnit;
	BYTE buf[10240];
	DWORD size;

	size=sizeof(fi);
	VERIFY(m_VirtualConnection.ReceiveBlock(&fi, size));
	if(size==0) return FALSE;
	if(size!=sizeof(fi)) {
		// Prisla porusena data
		ASSERT(FALSE);
		return FALSE;
	}
	// Vse je v poradku
#ifndef NO_UNITS
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(fi.dwID);
	if(pUnit) {
        ASSERT_VALID(pUnit);
		// Jednotka dokonce existuje
		pUnit->GetLock()->WriterLock();

		if(pUnit->m_dwTimeStampFull<=dwTime) {
			// Orazitkujeme
			pUnit->m_dwTimeStampFull=dwTime;

			// A presuneme data
			if(pUnit->m_dwTimeStampBrief<=dwTime) {
				// Tahle cast je spolecna s brief infem
				pUnit->m_dwTimeStampBrief=dwTime; // Orazitkujeme taky brief info

				pUnit->m_dwLives=fi.nLives;
				pUnit->m_dwMaxLives=fi.nLivesMax;
// !!!! Not set the position - it will distruct all interpolation data
//				pUnit->SetPosition(fi.dwPositionX, fi.dwPositionY, fi.dwVerticalPosition, m_pReceiver->GetMap());
				pUnit->SetDirection ( fi.nDirection );
				pUnit->m_dwViewRadius=fi.dwViewRadius;
			}

			// Tahle cast je navic ve full infu
			pUnit->m_dwInvisibilityDetection=fi.dwInvisibilityDetection;
			pUnit->m_dwInvisibilityState=fi.dwInvisibilityState;
		}

		// A nyni nacteme zbytek syrovych dat - full info
		size=sizeof(buf);
		VERIFY(m_VirtualConnection.ReceiveBlock(&buf, size));

		if(pUnit->GetFullInfoFlag() && pUnit->m_dwTimeStampFull<=dwTime) {
			if(pUnit->m_pFullInfo) {
				delete pUnit->m_pFullInfo;
				if(size>0) {
					pUnit->m_pFullInfo=new BYTE[size];
					memcpy(pUnit->m_pFullInfo, &buf, size);
					pUnit->m_dwFullInfoLength = size;
				} else {
					pUnit->m_pFullInfo=NULL;
					pUnit->m_dwFullInfoLength = 0;
				}
				pUnit->InlayEvent ( CCUnit::E_FullInfoChanged, (DWORD)pUnit );
			}
		}

		pUnit->GetLock()->WriterUnlock();
	}
  else{
    return TRUE;
  }
	pUnit->Release();
#else
	buf[0]=0; pUnit=NULL;
	TRACE("ParseFullInfo for unit %d\n", fi.dwID);
#endif

	return FALSE;
}

BOOL CCTimestampedVirtualConnection::ParseEnemyFullInfo(DWORD dwTime)
{
	SUnitEnemyFullInfo efi;
	CCUnit *pUnit;
	BYTE buf[10240];
	DWORD size;
	
	size=sizeof(efi);
	VERIFY(m_VirtualConnection.ReceiveBlock(&efi, size));
	if(size==0) return FALSE;
	if(size!=sizeof(efi)) {
		// Prisla porusena data
		ASSERT(FALSE);
		return FALSE;
	}
	// Vse je v poradku
#ifndef NO_UNITS
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(efi.dwID);
	if(pUnit) {
		ASSERT_VALID(pUnit);
        // Jednotka dokonce existuje
		pUnit->GetLock()->WriterLock();
		
		if(pUnit->m_dwTimeStampFull<=dwTime) {
			// Orazitkujeme
			pUnit->m_dwTimeStampFull=dwTime;

			// A presuneme data
			if(pUnit->m_dwTimeStampBrief<=dwTime) {
				// Tahle cast je spolecna s brief infem
				pUnit->m_dwTimeStampBrief=dwTime; // Orazitkujeme taky brief info

				pUnit->m_dwLives=efi.nLives;
				pUnit->m_dwMaxLives=efi.nLivesMax;			
//				pUnit->SetPosition(efi.dwPositionX, efi.dwPositionY, efi.dwVerticalPosition, m_pReceiver->GetMap());
				pUnit->SetDirection ( efi.nDirection );
				pUnit->m_dwViewRadius=efi.dwViewRadius;
			}
			
			// Tahle cast je navic ve full infu
			pUnit->m_dwInvisibilityState=efi.dwInvisibilityState;
		}

		// A nyni nacteme zbytek syrovych dat - enemy full info
		size=sizeof(buf);
		VERIFY(m_VirtualConnection.ReceiveBlock(&buf, size));

		if(pUnit->GetFullInfoFlag() && pUnit->m_dwTimeStampFull<=dwTime) {
			if(pUnit->m_pFullInfo) {
				delete pUnit->m_pFullInfo;
				if(size>0) {
					pUnit->m_pFullInfo=new BYTE[size];
					memcpy(pUnit->m_pFullInfo, &buf, size);
					pUnit->m_dwFullInfoLength = size;
				} else {
					pUnit->m_pFullInfo=NULL;
					pUnit->m_dwFullInfoLength = 0;
				}
				pUnit->InlayEvent ( CCUnit::E_FullInfoChanged, (DWORD)pUnit );
			}
		}

		pUnit->GetLock()->WriterUnlock();
	}
  else{
    return TRUE;
  }

	pUnit->Release();
#else
	buf[0]=0; pUnit=NULL;
	TRACE("ParseEnemyFullInfo for unit %d\n", efi.dwID);
#endif

	return FALSE;
}

CCUnit * CCTimestampedVirtualConnection::CreateNewUnit(SUnitStartInfo &StartInfo, DWORD dwTime)
{
	CCUnit *pUnit=new CCUnit();

	pUnit->AddRef();

	VERIFY(pUnit->GetLock()->WriterLock());

	// Get our civilization
	CCCivilization *pCivilization = m_pReceiver->GetMap()->GetCivilization(StartInfo.dwCivilizationID);
	ASSERT_VALID(pCivilization);
	// Get our unit type
	CCUnitType *pUnitType = pCivilization->GetUnitType(StartInfo.dwUnitTypeID);
	ASSERT_VALID(pUnitType);

	pUnitType->CreateInstance(pUnit, m_pReceiver->GetUnitCache());

	pUnit->SetID(StartInfo.dwID);
	pUnit->m_dwLives=StartInfo.nLives;
	pUnit->m_dwMaxLives=StartInfo.nLivesMax;
	pUnit->m_dwViewRadius=StartInfo.dwViewRadius;
	pUnit->SetPosition(StartInfo.dwPositionX, StartInfo.dwPositionY, StartInfo.dwVerticalPosition, m_pReceiver->GetMap());
	pUnit->m_nWantedDirection = StartInfo.nDirection;
	  if ( pUnit->m_nWantedDirection == 8 )
	  {
		pUnit->m_nCurrentDirection = ( rand () * 8 ) / ( RAND_MAX + 1);
	  }
	  else
	  {
		pUnit->m_nCurrentDirection = pUnit->m_nWantedDirection;
	  }
    pUnit->SetAppearance(StartInfo.dwAppearanceID, pUnit->m_nCurrentDirection);
	pUnit->SetFullInfoFlag(FALSE);
	pUnit->m_dwTimeStampBrief=pUnit->m_dwTimeStampCheckPoint=pUnit->m_dwTimeStampFull=dwTime;

  SCCheckPoint sFirstCheckPoint, sSecondCheckPoint;
  sFirstCheckPoint.m_dwX = StartInfo.dwFirstPositionX;
  sFirstCheckPoint.m_dwY = StartInfo.dwFirstPositionY;
  sFirstCheckPoint.m_dwTime = StartInfo.dwFirstTime;
  sSecondCheckPoint.m_dwX = StartInfo.dwSecondPositionX;
  sSecondCheckPoint.m_dwY = StartInfo.dwSecondPositionY;
  sSecondCheckPoint.m_dwTime = StartInfo.dwSecondTime;
  pUnit->SetCheckPoints(&sFirstCheckPoint, &sSecondCheckPoint, m_pReceiver->GetMap());

	// add it to cache
	m_pReceiver->GetUnitCache()->AddUnit(pUnit);

    ASSERT_VALID(pUnit);
    
    pUnit->GetLock()->WriterUnlock();

	return pUnit;
}

void CCTimestampedVirtualConnection::ApplyBriefInfo(CCUnit *pUnit, SUnitBriefInfo &BriefInfo, DWORD dwTime)
{
    ASSERT_VALID(pUnit);
	if(pUnit->m_dwTimeStampBrief <= dwTime) {
		// Orazitkujeme
		pUnit->m_dwTimeStampBrief = dwTime;
  
		// A presuneme data
		pUnit->m_dwLives = BriefInfo.nLives;
		pUnit->m_dwMaxLives = BriefInfo.nLivesMax;
        pUnit->m_nMode = BriefInfo.m_nMode;
		pUnit->SetPosition(BriefInfo.dwPositionX, BriefInfo.dwPositionY, BriefInfo.dwVerticalPosition, m_pReceiver->GetMap());
		pUnit->SetAppearance(BriefInfo.dwAppearanceID, BriefInfo.nDirection);
		pUnit->m_dwViewRadius = BriefInfo.dwViewRadius;
	}
}

void CCTimestampedVirtualConnection::ApplyCheckPointInfo(CCUnit *pUnit, SUnitCheckPointInfo &CheckPointInfo, DWORD dwTime)
{
  ASSERT_VALID(pUnit);
  ApplyBriefInfo(pUnit, CheckPointInfo, dwTime);

  SCCheckPoint sFirstCheckPoint, sSecondCheckPoint;
  sFirstCheckPoint.m_dwX = CheckPointInfo.dwFirstPositionX;
  sFirstCheckPoint.m_dwY = CheckPointInfo.dwFirstPositionY;
  sFirstCheckPoint.m_dwTime = CheckPointInfo.dwFirstTime;
  sSecondCheckPoint.m_dwX = CheckPointInfo.dwSecondPositionX;
  sSecondCheckPoint.m_dwY = CheckPointInfo.dwSecondPositionY;
  sSecondCheckPoint.m_dwTime = CheckPointInfo.dwSecondTime;
  pUnit->SetCheckPoints(&sFirstCheckPoint, &sSecondCheckPoint, m_pReceiver->GetMap());
}

void CCTimestampedVirtualConnection::ParseStartInfo(SControlConnectionStartInfo &StartInfo, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StartInfo.dwID);
	if(pUnit != NULL) {
        ASSERT_VALID(pUnit);
		VERIFY(pUnit->GetLock()->WriterLock());
        ApplyCheckPointInfo(pUnit, StartInfo, dwTime);
		pUnit->GetLock()->WriterUnlock();
	} else {
		pUnit = CreateNewUnit(StartInfo, dwTime);
	}
	pUnit->Release();
#else
	TRACE("ParseStartInfo for unit %d\n", StartInfo.dwID);
#endif
}

void CCTimestampedVirtualConnection::ParseStopInfo(SControlConnectionStopInfo &StopInfo, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StopInfo.dwID);
	if(!pUnit) {
		// Jednotka neexistuje
	    return;
	} else {		
    ASSERT_VALID(pUnit);
    CCMap *pMap = m_pReceiver->GetMap();

    VERIFY(pUnit->GetLock()->WriterLock());

    // Remove the unit from the map
    pUnit->RemoveFromMap(pMap);
    // Send notification, that the unit has died (or disappeared)
    pUnit->InlayEvent ( CCUnit::E_UnitDisappeared, (DWORD)pUnit );

    pUnit->GetLock()->WriterUnlock();
	}

	pUnit->Release();
#else
	TRACE("ParseStopInfo for unit %d\n", StopInfo.dwID);
#endif
}

void CCTimestampedVirtualConnection::ParseStartFullInfo(SUnitStartFullInfo &StartFullInfo, BYTE *pAdditionalInfo, DWORD dwAdditionalInfoSize, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit = (CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StartFullInfo.dwID);
	if(!pUnit) {
		pUnit = CreateNewUnit(StartFullInfo, dwTime);
	}
    ASSERT_VALID(pUnit);
	pUnit->GetLock()->WriterLock();

	// Jednotka uz existuje
	pUnit->SetFullInfoFlag(TRUE);
	if(pUnit->m_dwTimeStampFull<=dwTime) {			
		pUnit->m_dwTimeStampFull=dwTime;
		pUnit->m_dwLives=StartFullInfo.nLives;
		pUnit->m_dwMaxLives=StartFullInfo.nLivesMax;
		pUnit->m_dwViewRadius=StartFullInfo.dwViewRadius;
//		pUnit->SetPosition(StartFullInfo.dwPositionX, StartFullInfo.dwPositionY, StartFullInfo.dwVerticalPosition, m_pReceiver->GetMap());
		pUnit->SetDirection ( StartFullInfo.nDirection );
		pUnit->m_dwInvisibilityDetection=StartFullInfo.dwInvisibilityDetection;
		pUnit->m_dwInvisibilityState=StartFullInfo.dwInvisibilityState;			

		if(pUnit->m_pFullInfo) delete pUnit->m_pFullInfo;

		pUnit->m_pFullInfo=new BYTE[dwAdditionalInfoSize];
		memcpy(pUnit->m_pFullInfo, pAdditionalInfo, dwAdditionalInfoSize);

		pUnit->InlayEvent ( CCUnit::E_FullInfoChanged, (DWORD)pUnit );
	}
	pUnit->GetLock()->WriterUnlock();

	pUnit->Release();
#else
	TRACE("ParseStartFullInfo for unit %d\n", StartFullInfo.dwID);
#endif
}

void CCTimestampedVirtualConnection::ParseStopFullInfo(SControlConnectionStopFullInfo &StopFullInfo, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StopFullInfo.dwID);
	if(!pUnit) {
		// Jednotka neexistuje
	    return;
	} else {		
        ASSERT_VALID(pUnit);
		pUnit->SetFullInfoFlag(FALSE);
	}	

	pUnit->Release();
#else
	TRACE("ParseStopFullInfo for unit %d\n", StopFullInfo.dwID);
#endif
}

void CCTimestampedVirtualConnection::ParseStartEnemyFullInfo(SUnitStartEnemyFullInfo &StartEnemyFullInfo, BYTE *pAdditionalInfo, DWORD dwAdditionalInfoSize, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit = (CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StartEnemyFullInfo.dwID);
	if(!pUnit) {
		pUnit = CreateNewUnit(StartEnemyFullInfo, dwTime);
	}
    ASSERT_VALID(pUnit);
	VERIFY(pUnit->GetLock()->WriterLock());

	// Jednotka uz existuje
	pUnit->SetFullInfoFlag(TRUE);
	if(pUnit->m_dwTimeStampFull<=dwTime) {			
		pUnit->m_dwTimeStampFull=dwTime;
		pUnit->m_dwLives=StartEnemyFullInfo.nLives;
		pUnit->m_dwMaxLives=StartEnemyFullInfo.nLivesMax;
		pUnit->m_dwViewRadius=StartEnemyFullInfo.dwViewRadius;
//		pUnit->SetPosition(StartEnemyFullInfo.dwPositionX, StartEnemyFullInfo.dwPositionY, StartEnemyFullInfo.dwVerticalPosition, m_pReceiver->GetMap());
		pUnit->SetDirection ( StartEnemyFullInfo.nDirection );
		pUnit->m_dwInvisibilityState=StartEnemyFullInfo.dwInvisibilityState;			

		if(pUnit->m_pFullInfo) delete pUnit->m_pFullInfo;

		pUnit->m_pFullInfo=new BYTE[dwAdditionalInfoSize];
		memcpy(pUnit->m_pFullInfo, pAdditionalInfo, dwAdditionalInfoSize);

		pUnit->InlayEvent ( CCUnit::E_FullInfoChanged, (DWORD)pUnit );
	}
	pUnit->GetLock()->WriterUnlock();

	pUnit->Release();
#else
	TRACE("ParseStartEnemyFullInfo for unit %d\n", StartEnemyFullInfo.dwID);
#endif
}

void CCTimestampedVirtualConnection::ParseStopEnemyFullInfo(SControlConnectionStopEnemyFullInfo &StopEnemyFullInfo, DWORD dwTime)
{
#ifndef NO_UNITS
	CCUnit *pUnit;
	pUnit=(CCUnit*)m_pReceiver->GetUnitCache()->GetUnit(StopEnemyFullInfo.dwID);
	if(!pUnit) {
		// Jednotka neexistuje
	    return;
	} else {		
        ASSERT_VALID(pUnit);
		pUnit->SetFullInfoFlag(FALSE);
	}	
	pUnit->Release();
#else
	TRACE("ParseStopEnemyFullInfo for unit %d\n", StopEnemyFullInfo.dwID);
#endif
}
