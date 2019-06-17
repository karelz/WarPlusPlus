/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Civilizace na serveru hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Klient civilizace na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "ZClientUnitInfoSender.h"
#include "../../ZCivilization.h"
#include "Common/ServerClient/VirtualConnectionIDs.h"
#include "GameServer/GameServer/SMap.h"

//////////////////////////////////////////////////////////////////////
// Makra pro trasování
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	extern BOOL g_bTraceClientRequests;

	#define TRACE_CLIENT_REQUEST if ( g_bTraceClientRequests ) TRACE_NEXT 
	#define TRACE_CLIENT_REQUEST0(text) do { if ( g_bTraceClientRequests ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_CLIENT_REQUEST1(text,p1) do { if ( g_bTraceClientRequests ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_CLIENT_REQUEST2(text,p1,p2) do { if ( g_bTraceClientRequests ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_CLIENT_REQUEST3(text,p1,p2,p3) do { if ( g_bTraceClientRequests ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
#else //!_DEBUG
	#define TRACE_CLIENT_REQUEST TRACE
	#define TRACE_CLIENT_REQUEST0 TRACE0
	#define TRACE_CLIENT_REQUEST1 TRACE1
	#define TRACE_CLIENT_REQUEST2 TRACE2
	#define TRACE_CLIENT_REQUEST3 TRACE3
#endif //!_DEBUG

void CZClientUnitInfoSender::Create(CZCivilization *pZCivilization)
{
	m_dwNumConnections=0;
	m_dwTime=g_cMap.GetTimeSlice();
	m_bFinishing=FALSE;

	m_pZCivilization=pZCivilization;

	m_RequestConnection.SetClientUnitInfoSender(this);	

	m_pRectList=NULL;	

    m_bRegistered=FALSE;

#ifdef _DEBUG
	m_nFullInfoMode=0;
#endif // _DEBUG
}

void CZClientUnitInfoSender::Delete()
{
	if(!m_bFinishing) {
		// Ukonci to nejak            	
        m_bFinishing=TRUE;

        if(m_bRegistered) {
            g_cMap.UnregisterClient(this);
            m_bRegistered=FALSE;
        }
        m_MiniMapRequest.Delete();
        m_MiniMapConnection.DeleteVirtualConnection();
        m_ResourceConnection.DeleteVirtualConnection();

        m_RequestConnection.Delete(); 
        m_ControlConnection.Delete();
        m_BriefInfoConnection.Delete();
        m_CheckPointInfoConnection.Delete();
        m_FullInfoConnection.Delete();
        m_EnemyFullInfoConnection.Delete();
        m_PingConnection.Delete();

		// Zrusime seznam obdelniku
		if(m_pRectList!=NULL) {
			CZWatchedRectangle *pActual=m_pRectList->Next();
			while(m_pRectList!=pActual) {
				CZWatchedRectangle *pNext;
				pNext=pActual->Next();
				delete pActual;
				pActual=pNext;
			}
			
			delete pActual;
			m_pRectList=NULL;
		}
   }
}

CZClientUnitInfoSender::CZClientUnitInfoSender()
{
	Create((CZCivilization*)NULL);
}

CZClientUnitInfoSender::CZClientUnitInfoSender(CZCivilization *pZCivilization)
{	
	Create(pZCivilization);
}

CZClientUnitInfoSender::~CZClientUnitInfoSender()
{
	ASSERT(m_bFinishing == TRUE);
}

// Makro pro zjednoduseny zapis nasledujici funkce
#define CONNECTION_ADD(connection) \
	connection.SetConnection(Connection); \
	m_dwNumConnections++; \
	break;

void CZClientUnitInfoSender::AddConnection(CVirtualConnection Connection, DWORD dwVirtualConnectionID, CZCivilization *pCivilization)
{
	// Bud civku jeste nezname, nebo uz musi byt porad stejna
	ASSERT(!m_pZCivilization || m_pZCivilization==pCivilization); 
	// A jeste nemame plny pocet spojeni
	ASSERT(m_dwNumConnections<TotalConnections);

	if(m_dwNumConnections==0) {
		// Zaciname
	}

	m_pZCivilization=pCivilization;

	switch(dwVirtualConnectionID) {
		case VirtualConnection_ClientControl:
			CONNECTION_ADD(m_ControlConnection);

		case VirtualConnection_ClientRequests:
			CONNECTION_ADD(m_RequestConnection);

		case VirtualConnection_ClientBriefInfos:
			CONNECTION_ADD(m_BriefInfoConnection);

		case VirtualConnection_ClientCheckPointInfos:
			CONNECTION_ADD(m_CheckPointInfoConnection);

		case VirtualConnection_ClientFullInfos:
			CONNECTION_ADD(m_FullInfoConnection);

		case VirtualConnection_ClientEnemyFullInfos:
			CONNECTION_ADD(m_EnemyFullInfoConnection);

		case VirtualConnection_ClientPing:
			CONNECTION_ADD(m_PingConnection);

	    case VirtualConnection_Resources:
			m_ResourceConnection = Connection;
			m_dwNumConnections++;		
			break;

		case VirtualConnection_MiniMap:
			m_MiniMapConnection = Connection;
			m_MiniMapRequest.Create(m_MiniMapConnection);
			m_dwNumConnections++;
			break;
		
		default:
			TRACE0("CZClientUnitInfoSender: Neznamy typ spojeni\n");
			ASSERT(FALSE);
	}

	if(m_dwNumConnections==TotalConnections) {
		// Spojeni jsou vsechna hotova
		// muze zacit zabava

        // Zaregistrujeme se u mapy		
#ifndef PROJEKTAPP
        g_cMap.RegisterClient(this);
        m_bRegistered=TRUE;
#endif
	}
}

#undef CONNECTION_ADD

DWORD CZClientUnitInfoSender::GetCivilizationIndex() 
{
	return m_pZCivilization->GetSCivilization ()->GetCivilizationIndex ();
}

void CZClientUnitInfoSender::BriefInfo(const SUnitBriefInfo *pBriefInfo) 
{
    if(m_bFinishing) return;
	VERIFY(m_BriefInfoConnection.SendBlock(pBriefInfo, sizeof(SUnitBriefInfo)));
}

void CZClientUnitInfoSender::CheckPointInfo(const SUnitCheckPointInfo *pCheckPointInfo)
{
    if(m_bFinishing) return;
	VERIFY(m_CheckPointInfoConnection.SendBlock(pCheckPointInfo, sizeof(SUnitCheckPointInfo)));
}

// Makro pro zjednoduseni metod, provadi totez se vsemi *vystupnimi* spojenimi
// ktere prenasi data po velkych blocich na kazdy timeslice
// (tedy ne EnemyFullInfo, FullInfo a PingInfo
#define FOR_EACH_BLOCKED_CONNECTION(code) \
	m_ControlConnection.code; \
	m_BriefInfoConnection.code; \
	m_CheckPointInfoConnection.code; \

// Makro pro zjednoduseni metod, provadi totez se vsemi *vystupnimi* spojenimi
#define FOR_EACH_CONNECTION(code) \
	m_ControlConnection.code; \
	m_BriefInfoConnection.code; \
	m_CheckPointInfoConnection.code; \
	m_FullInfoConnection.code; \
	m_EnemyFullInfoConnection.code; \
	m_PingConnection.code;

void CZClientUnitInfoSender::StartSendingInfo()
{
	if(m_bFinishing) return;
    m_dwTime=g_cMap.GetTimeSlice();
	
    FOR_EACH_CONNECTION(SetTime(m_dwTime));

	/* Pingneme uzivatele */
	VERIFY(m_PingConnection.BeginSendCompoundBlock());
	VERIFY(m_PingConnection.SendBlock(&m_dwTime, sizeof(m_dwTime)));
	VERIFY(m_PingConnection.EndSendCompoundBlock());
}

void CZClientUnitInfoSender::StopSendingInfo()
{    
	FOR_EACH_BLOCKED_CONNECTION(EndSendCompoundBlock());
}

void CZClientUnitInfoSender::StartSendingFullInfo(const SUnitFullInfo *pFullInfo)
{   
    if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==0);
	m_nFullInfoMode=1;
#endif // _DEBUG

    m_FullInfoConnection.SetTime(m_dwTime);
	VERIFY(m_FullInfoConnection.BeginSendCompoundBlock());
	VERIFY(m_FullInfoConnection.SendBlock(pFullInfo, sizeof(SUnitFullInfo)));

}
void CZClientUnitInfoSender::SendingFullInfo(const BYTE *pData, int nSize)
{
    if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==1);
#endif // _DEBUG

	VERIFY(m_FullInfoConnection.SendBlock(pData, nSize));
}
void CZClientUnitInfoSender::StopSendingFullInfo()
{	
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==1);
	m_nFullInfoMode=0;
#endif // _DEBUG
    VERIFY(m_FullInfoConnection.EndSendCompoundBlock());
}

void CZClientUnitInfoSender::StartSendingEnemyFullInfo(const SUnitEnemyFullInfo *pEnemyFullInfo)
{
    if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==0);
	m_nFullInfoMode=2;
#endif // _DEBUG
	m_EnemyFullInfoConnection.SetTime(m_dwTime);
	VERIFY(m_EnemyFullInfoConnection.BeginSendCompoundBlock());
	VERIFY(m_EnemyFullInfoConnection.SendBlock(pEnemyFullInfo, sizeof(SUnitEnemyFullInfo)));
}
void CZClientUnitInfoSender::SendingEnemyFullInfo(const BYTE *pData, int nSize)
{
	if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==2);
#endif // _DEBUG
    m_EnemyFullInfoConnection.SendBlock(pData, nSize);
}
void CZClientUnitInfoSender::StopSendingEnemyFullInfo()
{
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==2);
	m_nFullInfoMode=0;
#endif // _DEBUG
	m_EnemyFullInfoConnection.EndSendCompoundBlock();
}

void CZClientUnitInfoSender::StartSendingStartFullInfo(const SUnitStartFullInfo *pStartFullInfo)
{
	if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==0);
	m_nFullInfoMode=3;
#endif // _DEBUG

    SControlConnectionHeaderFormat hdr;
	
	m_ControlConnection.SetTime(m_dwTime);
	m_ControlConnection.BeginSendCompoundBlock();

	hdr.dwCCMessageType=CCM_StartFullInfo;
	hdr.dwCCMessageDataLength=sizeof(SUnitStartFullInfo);
	m_ControlConnection.SendBlock(&hdr, sizeof(hdr));
	m_ControlConnection.SendBlock(pStartFullInfo, sizeof(SUnitStartFullInfo));
}
void CZClientUnitInfoSender::SendingStartFullInfo(const BYTE *pData, int nSize)
{
	if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==3);
#endif // _DEBUG

    m_ControlConnection.SendBlock(pData, nSize);
}
void CZClientUnitInfoSender::StopSendingStartFullInfo()
{
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==3);
	m_nFullInfoMode=0;
#endif // _DEBUG
    m_ControlConnection.EndSendCompoundBlock();
}

void CZClientUnitInfoSender::StartSendingStartEnemyFullInfo(const SUnitStartEnemyFullInfo *pStartEnemyFullInfo)
{
	if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==0);
	m_nFullInfoMode=4;
#endif // _DEBUG

    SControlConnectionHeaderFormat hdr;

	m_ControlConnection.SetTime(m_dwTime);
	m_ControlConnection.BeginSendCompoundBlock();
	
	hdr.dwCCMessageType=CCM_StartEnemyFullInfo;
	hdr.dwCCMessageDataLength=sizeof(SUnitStartEnemyFullInfo);
	m_ControlConnection.SendBlock(&hdr, sizeof(hdr));
	m_ControlConnection.SendBlock(pStartEnemyFullInfo, sizeof(SUnitStartEnemyFullInfo));
}
void CZClientUnitInfoSender::SendingStartEnemyFullInfo(const BYTE *pData, int nSize)
{
    if(m_bFinishing) return;
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==4);
#endif // _DEBUG

	m_ControlConnection.SendBlock(pData, nSize);
}
void CZClientUnitInfoSender::StopSendingStartEnemyFullInfo()
{
#ifdef _DEBUG
	ASSERT(m_nFullInfoMode==4);
	m_nFullInfoMode=0;
#endif // _DEBUG
    m_ControlConnection.EndSendCompoundBlock();
}

void CZClientUnitInfoSender::StartInfo(const SUnitStartInfo *pStartInfo)
{	
	if(m_bFinishing) return;
    ASSERT(pStartInfo->dwAppearanceID != 0);
    SControlConnectionStartInfo si;
	
	m_ControlConnection.SetTime(m_dwTime);

    si.dwCCMessageType=CCM_StartInfo;
	si.dwCCMessageDataLength=sizeof(si)-sizeof(SControlConnectionHeaderFormat);
	
	// ************* PRASARNA ****************
	// * Opravit!!!
	memcpy((SUnitStartInfo *)&si, pStartInfo, sizeof(SUnitStartInfo));

	m_ControlConnection.SendBlock(&si, sizeof(si));
}

void CZClientUnitInfoSender::StopInfo(DWORD dwID)
{
	if(m_bFinishing) return;
    SControlConnectionStopInfo si;

	m_ControlConnection.SetTime(m_dwTime);
	
	si.dwCCMessageType=CCM_StopInfo;
	si.dwCCMessageDataLength=sizeof(si)-sizeof(SControlConnectionHeaderFormat);
	si.dwID=dwID;

	m_ControlConnection.SendBlock(&si, sizeof(si));
}

void CZClientUnitInfoSender::StopFullInfo(DWORD dwID)
{
    if(m_bFinishing) return;
    SControlConnectionStopFullInfo sfi;

	m_ControlConnection.SetTime(m_dwTime);
	
	sfi.dwCCMessageType=CCM_StopFullInfo;
	sfi.dwCCMessageDataLength=sizeof(sfi)-sizeof(SControlConnectionHeaderFormat);
	sfi.dwID=dwID;

	m_ControlConnection.SendBlock(&sfi, sizeof(sfi));
}

void CZClientUnitInfoSender::StopEnemyFullInfo(DWORD dwID)
{
	if(m_bFinishing) return;
    SControlConnectionStopEnemyFullInfo sefi;

	m_ControlConnection.SetTime(m_dwTime);
	
	sefi.dwCCMessageType=CCM_StopEnemyFullInfo;
	sefi.dwCCMessageDataLength=sizeof(sefi)-sizeof(SControlConnectionHeaderFormat);
	sefi.dwID=dwID;
	
	m_ControlConnection.SendBlock(&sefi, sizeof(sefi));
}

void CZClientUnitInfoSender::RequestArrived(DWORD dwType, DWORD dwSize, void *pData)
{
	if(m_bFinishing) return;
   CZWatchedRectangle *pRect;
	SUserRequestStartWatchingRectangle *pSWR;
	SUserRequestStopWatchingRectangle *pEWR;
	SUserRequestStartFullInfo *pSFI;
	SUserRequestStopFullInfo *pEFI;
	SUserRequestRectangleMoved *pSRM;
	SUserRequestMiniMap *pMM;

	switch(dwType) {
		// User zacal koukat na obdelnik
		case UR_StartWatchingRectangle:
			ASSERT(dwSize==sizeof(SUserRequestStartWatchingRectangle));
			
			pSWR=(SUserRequestStartWatchingRectangle*)pData;
			ASSERT(pSWR);
			
			TRACE_CLIENT_REQUEST("++ Client Request: StartWatchingRectangle #%d (%d, %d, %d, %d)\n", pSWR->dwID, pSWR->dwLeft, pSWR->dwTop, pSWR->dwWidth, pSWR->dwHeight);
			
			pRect=new CZWatchedRectangle(this, pSWR->dwID);
			pRect->SetDimensions(pSWR->dwLeft, pSWR->dwTop, pSWR->dwWidth, pSWR->dwHeight);

			if(!m_pRectList) {				
				// Prazdny seznam
				m_pRectList=pRect;
			} else {
				// Nesmi tam jiz byt
				ASSERT(!m_pRectList->FindRectangle(pSWR->dwID));
				// Jestlize neni v seznamu, pridame ho
				pRect->Append(m_pRectList);
			}
			// Jestlize nekoncime, rekneme mape
			if(!m_bFinishing) pRect->NotifyMap();

			break;

		// User prestal koukat na obdelnik
		case UR_StopWatchingRectangle:
			ASSERT(dwSize==sizeof(SUserRequestStopWatchingRectangle));
			// Seznam musi existovat
			ASSERT(m_pRectList); 
			
			pEWR=(SUserRequestStopWatchingRectangle*)pData;
			ASSERT(pEWR);
			
			pRect=m_pRectList->FindRectangle(pEWR->dwID);
			ASSERT(pRect); // Rectangle musi byt nalezitelny

			TRACE_CLIENT_REQUEST1("++ Client Request: StopWatchingRectangle #%d\n", pEWR->dwID);

			if(pRect->Next()!=pRect) {
				// Neni to samotna hlava
				pRect->Disconnect();
			} else {
				// Cely seznam je jen hlava
				m_pRectList=NULL;
			}
			// Jestlize nekoncime, rekneme mape
			if(!m_bFinishing) { 
				pRect->Empty();
				pRect->NotifyMap();
			}

			delete pRect;
			break;

		// Obdelnik se pohnul
		case UR_RectangleMoved:
			ASSERT(dwSize==sizeof(SUserRequestRectangleMoved));
			// Musi existovat seznam
			ASSERT(m_pRectList);
			
			pSRM=(SUserRequestRectangleMoved*)pData;
			ASSERT(pSRM);
			
			pRect=m_pRectList->FindRectangle(pSRM->dwID);
			ASSERT(pRect);

			TRACE_CLIENT_REQUEST("++ Client Request: RectangleMoved #%d (%d, %d, %d, %d)\n", pSRM->dwID, pSRM->dwLeft, pSRM->dwTop, pSRM->dwWidth, pSRM->dwHeight);

			pRect->SetDimensions(pSRM->dwLeft, pSRM->dwTop, pSRM->dwWidth, pSRM->dwHeight);
			// Jestlize nekoncime, rekneme mape
			if(!m_bFinishing) pRect->NotifyMap();
			break;

		// Uzivatel zacal sledovat full info
		case UR_StartFullInfo:
			ASSERT(dwSize==sizeof(SUserRequestStartFullInfo));
			
			pSFI=(SUserRequestStartFullInfo*)pData;
			ASSERT(pSFI);

#ifndef PROJEKTAPP
            g_cMap.StartWatchingUnit(this, pSFI->dwID);
#endif
			break;

		// Uzivatel prestal sledovat full info
		case UR_StopFullInfo:
			ASSERT(dwSize==sizeof(SUserRequestStopFullInfo));

			pEFI=(SUserRequestStopFullInfo*)pData;
			ASSERT(pEFI);

#ifndef PROJEKTAPP
         g_cMap.StopWatchingUnit(this, pEFI->dwID);
#endif
         break;

		case UR_CloseComms:
			ASSERT(dwSize==0);
			if(!m_bFinishing) {
				m_bFinishing=TRUE;
#ifndef PROJEKTAPP
                if(m_bRegistered) {
				    g_cMap.UnregisterClient(this);
                    m_bRegistered=FALSE;
                }
#endif
			}
			break;

		case UR_MiniMap:
			ASSERT(dwSize==sizeof(SUserRequestMiniMap));

			pMM=(SUserRequestMiniMap*)pData;
			ASSERT(pMM);
			m_MiniMapRequest.SetClip(*pMM);
			break;
		
		// Spatny request
		default:
			TRACE0("Wrong request number.\n");
			ASSERT(FALSE);
			break;
	}
}

void CZClientUnitInfoSender::SendResourceInfo ( int (* pResources)[RESOURCE_COUNT] )
{
	VERIFY(m_ResourceConnection.BeginSendCompoundBlock());

	m_ResourceConnection.SendBlock(pResources, sizeof(int) * RESOURCE_COUNT);

	VERIFY(m_ResourceConnection.EndSendCompoundBlock());
}

CMiniMapServer CZClientUnitInfoSender::m_MiniMapServer;