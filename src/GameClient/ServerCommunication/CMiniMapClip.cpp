// CMiniMapClip.cpp: implementation of the CCMiniMapClip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CMiniMapClip.h"
#include "Common/ServerClient/VirtualConnectionIDs.h"
#include "CRequestVirtualConnection.h"
#include "Archive/Compression/CompressException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_OBSERVER_MAP(CCMiniMapClip, CNotifier)
    BEGIN_NOTIFIER(ID_MiniMapVirtualConnection)
        EVENT(E_PACKETAVAILABLE)
            OnPacketAvailable((WORD) dwParam);
            return TRUE;
    END_NOTIFIER()
END_OBSERVER_MAP(CCMiniMapClip, CNotifier)

//////////////

CCMiniMapClip::CCMiniMapClip()
{
	m_DownloadedClip.Reset();
	m_RequestedClip.Reset();

	m_dwMaxPackedDataSize=0;
	m_pPackedData=NULL;
	
	m_pRequestConnection=NULL;
	m_pMiniMapConnection=NULL;

	m_dwPackedDataSize=0;
	m_pPackedDataPosition=NULL;
	m_dwPackedDataPosition=0;
	m_dwUncompressedDataSize=0;

    m_bClearToSendRequest=TRUE;
    m_bRequestNotSent=FALSE;

	m_bDataLocked=FALSE;
}

CCMiniMapClip::~CCMiniMapClip()
{
	// Destrukci povolime jenom po radnem zavolani Delete (a nebo vubec bez volani Create)
	ASSERT(m_dwMaxPackedDataSize==0 && m_pPackedData==NULL);
	ASSERT(m_pRequestConnection==NULL && m_pMiniMapConnection==NULL);
	ASSERT(m_pPackedDataPosition==NULL && m_dwPackedDataSize==0);
	ASSERT(m_bDataLocked==FALSE);
}

void CCMiniMapClip::Create(CVirtualConnection *pMiniMapConnection, CCRequestVirtualConnection *pRequestConnection)
{
	ASSERT(pRequestConnection);
	ASSERT(pMiniMapConnection);

	m_pRequestConnection=pRequestConnection;
	m_pMiniMapConnection=pMiniMapConnection;

	// Max. velikost pakovanych dat
	m_dwMaxPackedDataSize=cPackedMiniMapSize;

	// Alokujeme pro ne pamet
	m_pPackedData=new BYTE[m_dwMaxPackedDataSize];

	// Zatim zadna data, neni odkud co cist
	m_pPackedDataPosition=NULL;
	m_dwPackedDataPosition=0;
	m_dwPackedDataSize=0;
	m_dwUncompressedDataSize=0;

	m_DataSource.Create(this);

	m_bDataLocked=FALSE;
}

void CCMiniMapClip::Delete()
{
	// Momentalne nesmime byt v procesu cteni dat
	ASSERT(m_pPackedDataPosition==NULL);

	// Zapomeneme jiz nepotrebne spojeni
	m_pRequestConnection=NULL;
	m_pMiniMapConnection=NULL;

	// Zapomeneme obdelniky
	m_DownloadedClip.Reset();
	m_RequestedClip.Reset();	


	// Jestlize jsme po Create
    if(m_dwMaxPackedDataSize>0 && m_pPackedData!=NULL) {
	    // Dealokace dat
	    m_dwMaxPackedDataSize=0;
	    delete [] m_pPackedData;
	    m_pPackedData=NULL;
	    m_dwPackedDataSize=0;
    } else {
        // Nejsme po Craete
        ASSERT(m_dwMaxPackedDataSize==0 && m_pPackedData==NULL);
    }

	// Musi byt odemceno
	ASSERT(m_bDataLocked==FALSE);

	m_DataSource.Delete();
}

void CCMiniMapClip::RequestClip(SMiniMapClip Clip)
{
#ifdef _DEBUG
	Clip.AssertValid();
#endif // _DEBUG

	// Otestujeme, jsme-li po create
	ASSERT(m_pRequestConnection!=NULL);
	ASSERT(m_pMiniMapConnection!=NULL);

	// Jestlize je pozadovano neco noveho
	if(m_RequestedClip!=Clip) {
		// Zapamatujeme si to
		m_RequestedClip=Clip;
        // Jestlize mame povoleno odesilat zadosti
        if(m_bClearToSendRequest) {
            // Pozadame o to server
		    m_pRequestConnection->SendMiniMapClip(m_RequestedClip);
            // A protoze jsme uz poslali pozadavek, shodime si povoleni
            m_bClearToSendRequest=FALSE;
            // A nastavime, ze se pozadavek podarilo odeslat
            m_bRequestNotSent=FALSE;
        } else {
            // Pozadavek nebyl odeslan na server
            // To je treba si zapamatovat
            m_bRequestNotSent=TRUE;
        }
	}
}

void CCMiniMapClip::NotifyDataArrived(SMiniMapClip Clip) {
  InlayEvent(E_ClipChanged, 0);
}

// V nekterych funkcich je treba zamykat data jenom kvuli assertum
// - k tomu slouzi nasledujici makra
#ifdef _DEBUG
#define ASSERT_LOCK VERIFY(m_lockPackedData.Lock());
#else
#define ASSERT_LOCK
#endif // _DEBUG


#ifdef _DEBUG
#define ASSERT_UNLOCK VERIFY(m_lockPackedData.Unlock());
#else
#define ASSERT_UNLOCK
#endif // _DEBUG

void CCMiniMapClip::LockClip()
{
	ASSERT_LOCK;

	ASSERT(!m_bDataLocked);	
    m_bDataLocked=TRUE;
    VERIFY(m_lockDataLocked.Lock());


	ASSERT_UNLOCK;
}

void CCMiniMapClip::UnlockClip()
{
	ASSERT_LOCK;

	ASSERT(m_pPackedDataPosition==NULL && m_dwPackedDataPosition==0);
	ASSERT(m_bDataLocked);

	m_bDataLocked=FALSE;
    VERIFY(m_lockDataLocked.Unlock());

	ASSERT_UNLOCK;
}

SMiniMapClip CCMiniMapClip::GetClip()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	SMiniMapClip Clip(m_DownloadedClip);

	ASSERT_UNLOCK

	return Clip;
}

BOOL CCMiniMapClip::IsCorrectZoom()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	
	BOOL bResult=m_DownloadedClip.nZoom==m_RequestedClip.nZoom;

	ASSERT_UNLOCK

	return bResult;
}

DWORD CCMiniMapClip::GetScanLineSize()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	
	DWORD dwScanLine;

	if(!m_DownloadedClip.Empty()) {
		// Vyrez je neprazdny
		dwScanLine=m_DownloadedClip.nWidth/m_DownloadedClip.nZoom;
	} else {
		// Vyrez je prazdny, velikost scanline je tedy 0
		dwScanLine=0;
	}

	ASSERT_UNLOCK

	return dwScanLine;
}

void CCMiniMapClip::StartDataReading()
{
	ASSERT_LOCK
	
	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);
	ASSERT(m_pPackedDataPosition==NULL);
	
	m_pPackedDataPosition=m_pPackedData;
	m_dwPackedDataPosition=0;

	try {
		m_Engine.InitDecompression(m_DataSource);
	} catch(CCompressException *pException) {
		ASSERT_UNLOCK
		throw pException;
	}

	ASSERT_UNLOCK
}

void CCMiniMapClip::CancelDataReading()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);	
	if(m_pPackedDataPosition==NULL) {
		ASSERT_UNLOCK
		return;
	}

	try {
		m_Engine.DoneDecompression();
	} catch(CCompressException *pException) {
		pException->Delete();
	}

	m_pPackedDataPosition=NULL;
	m_dwPackedDataPosition=0;

	ASSERT_UNLOCK
}

BOOL CCMiniMapClip::ReadData(BYTE *pData, DWORD &dwSize)
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);
	ASSERT(m_pPackedDataPosition!=NULL);

	try {
		dwSize=m_Engine.Read(pData, dwSize);

		if(dwSize==0) {
			// Konec cteni
			m_Engine.DoneDecompression();
			m_pPackedDataPosition=NULL;
			m_dwPackedDataPosition=0;			
		}
		ASSERT_UNLOCK
		return dwSize>0;
	} catch(CCompressException *pException) {
		
		ASSERT_UNLOCK

		throw pException;
	}
}

BOOL CCMiniMapClip::IsEOF()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);	

	try {
		if(m_pPackedDataPosition==NULL) {
			ASSERT_UNLOCK
			return TRUE;
		}

		BOOL bReturn=m_Engine.GetPosition()<m_dwUncompressedDataSize;
		
		ASSERT_UNLOCK

		return bReturn;
	} catch(CCompressException *pException) {		
		pException->Delete();
		
		ASSERT_UNLOCK
		
		return TRUE;
	}
}

int CCMiniMapClip::GetX()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);
	ASSERT(m_pPackedDataPosition!=NULL);

	DWORD dwX;

	if(!m_DownloadedClip.Empty()) {
		// Vyrez je neprazdny
		dwX=m_Engine.GetPosition() % (m_DownloadedClip.nWidth/m_DownloadedClip.nZoom);
	} else {
		// Vyrez je prazdny, pozice je tudiz 0
		dwX=0;
	}

	ASSERT_UNLOCK
	
	return dwX;
}

int CCMiniMapClip::GetY()
{
	ASSERT_LOCK

	ASSERT(m_bDataLocked);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);
	ASSERT(m_pPackedDataPosition!=NULL);

	DWORD dwY;

	if(!m_DownloadedClip.Empty()) {
		// Vyrez je neprazdny
		dwY=m_Engine.GetPosition() / (m_DownloadedClip.nWidth/m_DownloadedClip.nZoom);
	} else {
		// Vyrez je prazdny, pozice je tudiz 0
		dwY=0;
	}

	ASSERT_UNLOCK
	
	return dwY;
}

int CCMiniMapClip::GetMapX()
{
	return (GetX()*m_DownloadedClip.nZoom)+m_DownloadedClip.nLeft;
}

int CCMiniMapClip::GetMapY()
{
	return (GetY()*m_DownloadedClip.nZoom)+m_DownloadedClip.nTop;
}

void CCMiniMapClip::OnPacketAvailable(WORD wConnectionNumber)
{
	// Kdyby nahodou prisla zprava, a my byli jeste pred create (nebo po delete)
	// tak se ta zprava nas NEMUZE tykat
	if(m_pMiniMapConnection==NULL) return;

	ASSERT(m_pRequestConnection!=NULL);
	ASSERT(m_dwMaxPackedDataSize>0);
	ASSERT(m_pPackedData!=NULL);
	
    // Neni to zprava pro nas
	if(wConnectionNumber != m_pMiniMapConnection->GetVirtualConnectionNumber())
		return;
	
    // Prisla minimapa, takze muzeme chrlit dalsi pozadavky
    m_bClearToSendRequest=TRUE;
    // Ale jestlize mame nejaky v zaloze
    if(m_bRequestNotSent) {
	    m_pRequestConnection->SendMiniMapClip(m_RequestedClip);
        m_bRequestNotSent=FALSE;
    }

	VERIFY(m_pMiniMapConnection->BeginReceiveCompoundBlock());

	// Nejprve je zaslan zpracovany vyrez
	SMiniMapClip Clip;
	DWORD dwSize=sizeof(Clip);
	
	VERIFY(m_pMiniMapConnection->ReceiveBlock(&Clip, dwSize));
	ASSERT(dwSize==sizeof(Clip));

#ifdef _DEBUG
	Clip.AssertValid();
#endif

	// Potom velikost rozpakovanych dat
	DWORD dwUncompressedDataSize;
	dwSize=sizeof(dwUncompressedDataSize);
	
	VERIFY(m_pMiniMapConnection->ReceiveBlock(&dwUncompressedDataSize, dwSize));
	ASSERT(dwSize==sizeof(dwUncompressedDataSize));

	// A potom velikost spakovanych dat
	DWORD dwPackedDataSize;
	dwSize=sizeof(dwPackedDataSize);
	
	VERIFY(m_pMiniMapConnection->ReceiveBlock(&dwPackedDataSize, dwSize));
	ASSERT(dwSize==sizeof(dwPackedDataSize));

	ASSERT(dwPackedDataSize>0);

	// Zamkneme si pristup k datum
	VERIFY(m_lockDataLocked.Lock());

	// Pokud prislo moc dat
	if(dwPackedDataSize>m_dwMaxPackedDataSize) {	
		// smula smula, nova se zapomenou		
		// (na to nemusime byt zamceni
		VERIFY(m_lockDataLocked.Unlock());

		BYTE bThrowAway[1024];
		DWORD dwThrowAwaySize;

		dwThrowAwaySize=dwPackedDataSize;

		while(dwThrowAwaySize>0) {
			DWORD dwRead;
			dwRead=(dwThrowAwaySize<sizeof(bThrowAway))?dwThrowAwaySize:sizeof(bThrowAway);
			VERIFY(m_pMiniMapConnection->ReceiveBlock(bThrowAway, dwRead));
			dwThrowAwaySize-=dwRead;

			if(dwRead==0) break;
		}

		VERIFY(m_pMiniMapConnection->EndReceiveCompoundBlock());

	} else {
		// Kopirujeme ziskana data
		m_DownloadedClip=Clip;
		m_dwUncompressedDataSize=dwUncompressedDataSize;
		m_dwPackedDataSize=dwPackedDataSize;
		
		// Nacteme blok spakovane minimapy
		VERIFY(m_pMiniMapConnection->ReceiveBlock(m_pPackedData, dwPackedDataSize));
		ASSERT(dwPackedDataSize==m_dwPackedDataSize);
		VERIFY(m_pMiniMapConnection->EndReceiveCompoundBlock());
		
		// Nakonec odemkneme
		VERIFY(m_lockDataLocked.Unlock());

		// A rekneme to funkci
		NotifyDataArrived(Clip);
	}
}

#undef ASSERT_LOCK
#undef ASSERT_UNLOCK

// Debug functions
#ifdef _DEBUG

void CCMiniMapClip::AssertValid() const
{
	ASSERT(m_pRequestConnection!=NULL && m_pMiniMapConnection!=NULL);
	ASSERT(m_pPackedData!=NULL && m_dwMaxPackedDataSize>0);	
}

void CCMiniMapClip::Dump(CDumpContext &dc) const
{
	dc << "CCMiniMapClip: buffer size: " << m_dwMaxPackedDataSize << " used: " << m_dwPackedDataSize;
	
	if(m_pPackedDataPosition!=NULL && m_pPackedData!=NULL) {
		dc << " reading at: " << (m_pPackedDataPosition-m_pPackedData);
	}
	
	dc << "\n - requested clip ";
	m_RequestedClip.Dump(dc);
	
	dc << "\n - downloaded clip ";
	m_DownloadedClip.Dump(dc);
}

#endif

ECompressData CCMiniMapClipDataSource::GiveData(void *&lpBuf, DWORD &dwSize)	
{
	ASSERT(m_pClip);
	ASSERT(m_pClip->m_pPackedData!=NULL);
	ASSERT(m_pClip->m_bDataLocked);
	ASSERT(m_pClip->m_pPackedDataPosition!=NULL);	
	
	// Kolik zbyva dat
	dwSize=m_pClip->m_dwPackedDataSize-m_pClip->m_dwPackedDataPosition;
	
	// Vratime data
	lpBuf=m_pClip->m_pPackedDataPosition;

	// A updatneme si sve informace
	m_pClip->m_pPackedDataPosition+=dwSize;
	m_pClip->m_dwPackedDataPosition+=dwSize;

	return compressDataEOF;
}

DWORD CCMiniMapClipDataSource::GetUncompressedSize()
{
	ASSERT(m_pClip);
	return m_pClip->m_dwUncompressedDataSize;
}

