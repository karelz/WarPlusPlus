// ZMiniMapRequest.cpp: implementation of the CZMiniMapRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZMiniMapRequest.h"

CZMiniMapRequest::CZMiniMapRequest()
{
	// Po pouhem zavolani konstruktoru neni objekt pouzitelny
	m_nLeft=m_nTop=0;
	m_nWidth=m_nHeight=m_nZoom=-1;
	m_bCalculationInProgress=FALSE;
}

CZMiniMapRequest::~CZMiniMapRequest()
{
	// Pokud bylo zavolano Create, je tato situace mozna jen po patricnem 
	// uzavorkovani volanim Delete.
	ASSERT(m_nZoom==-1 && m_nWidth==-1 && m_nHeight==-1);
}

void CZMiniMapRequest::Create(CVirtualConnection VirtualConnection)
{
	m_VirtualConnection = VirtualConnection;
	
	// Nastavime data na pouzitelne, ale prazdne hodnoty
	m_nZoom=1;
	m_nLeft=m_nTop=0;
	m_nWidth=m_nHeight=0;
	m_bCalculationInProgress=FALSE;
}

void CZMiniMapRequest::Delete()
{
	// Ucinime objekt destruovatelnym bez rvani
	VERIFY(m_lockDataAccess.Lock());
	m_nZoom=m_nWidth=m_nHeight=-1;	
	VERIFY(m_lockDataAccess.Unlock());

	// V pripade, ze jsou data objektu zrovna zpracovavana
	while(m_bCalculationInProgress) {
		// Cekame na ukonceni zpracovani
		VERIFY(m_eventCalculationFinished.Lock());
	}
}

void CZMiniMapRequest::SetClip(int nLeft, int nTop, int nWidth, int nHeight, int nZoom)
{
	// Zamkneme data na pristup
	VERIFY(m_lockDataAccess.Lock());
	// Prekopirujeme udaje
	m_nLeft=nLeft; m_nTop=nTop; 
	m_nWidth=nWidth; m_nHeight=nHeight; 
	m_nZoom=nZoom;
	// Odemkneme
	VERIFY(m_lockDataAccess.Unlock());
}

void CZMiniMapRequest::GetClip(int &nLeft, int &nTop, int &nWidth, int &nHeight, int &nZoom)
{
	// Zamkneme data na pristup
	VERIFY(m_lockDataAccess.Lock());
	// Prekopirujeme udaje
	nLeft=m_nLeft; nTop=m_nTop; 
	nWidth=m_nWidth; nHeight=m_nHeight; 
	nZoom=m_nZoom;
	// Odemkneme
	VERIFY(m_lockDataAccess.Unlock());
}

void CZMiniMapRequest::CalculationStart()
{	
	ASSERT(!m_bCalculationInProgress);

	m_bCalculationInProgress=TRUE;	
}

void CZMiniMapRequest::CalculationStop()
{
	ASSERT(m_bCalculationInProgress);
	m_bCalculationInProgress=FALSE;
	VERIFY(m_eventCalculationFinished.SetEvent());
}

void CZMiniMapRequest::SendMiniMap(SMiniMapClip Clip, void *pData, DWORD dwSize, DWORD dwUnpackedSize)
{
	Clip.AssertValid();
	ASSERT(pData!=NULL);
	ASSERT(dwSize>0);	

	// Pokud zjistime, ze nas zoom je nastaven na parkovaci hodnotu,
	// znamena to, ze se konci cinnost a uz se zadna data neposlou
	if(m_nZoom==-1) {
		return;
	}

// Vypsani chybove hlasky
#define VC_ERR { TRACE("CZMiniMapRequest::SendMiniMap - selhalo odeslani bloku dat.\n"); }

	if(!m_VirtualConnection.BeginSendCompoundBlock(FALSE)) VC_ERR;
	if(!m_VirtualConnection.SendBlock(&Clip, sizeof(Clip), FALSE)) VC_ERR;
	if(!m_VirtualConnection.SendBlock(&dwUnpackedSize, sizeof(dwSize), FALSE)) VC_ERR;
	if(!m_VirtualConnection.SendBlock(&dwSize, sizeof(dwSize), FALSE)) VC_ERR;
	if(!m_VirtualConnection.SendBlock(pData, dwSize, FALSE)) VC_ERR;
	if(!m_VirtualConnection.EndSendCompoundBlock()) VC_ERR;

// Toto makro jiz nebude potreba
#undef VC_ERR

}

// Debugovani
#ifdef _DEBUG
void CZMiniMapRequest::AssertValid() const
{
	if(m_nZoom==-1) {
		TRACE("CZMiniMapRequestObject pravdepodobne neni ve stavu mezi volanim Create a Delete.\n");		
		ASSERT(FALSE);
	}
	ASSERT(m_nZoom==2 || m_nZoom==4 || m_nZoom==8 || m_nZoom==16);
	ASSERT(m_nWidth>=0 && m_nHeight>=0);

	m_lockDataAccess.AssertValid();
	m_eventCalculationFinished.AssertValid();
	m_VirtualConnection.AssertValid();
}
void CZMiniMapRequest::Dump(CDumpContext &dc) const
{
	dc << "CZMiniMapRequest: left=" << m_nLeft << ", top=" << m_nTop << ", ";
	dc << "width=" << m_nWidth << ", height=" << m_nHeight << "\n";
	if(m_bCalculationInProgress) {
		dc << " - calculation in progress.\n";
	}
}
#endif // Debugovani
