// MiniMapServer.cpp: implementation of the CMiniMapServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MiniMapServer.h"
#include "GameServer/GameServer/SMap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMiniMapServer::CMiniMapServer()
{
	m_dwDataSize=0;
	m_pData=NULL;

	m_dwMapDataSize=0;
	m_pMapData=NULL;

	m_dwPackedDataSize=0;
	m_pMapData=NULL;

	m_pMapDataSource=NULL;

	m_pRequest=NULL;
}

CMiniMapServer::~CMiniMapServer()
{
	ASSERT(m_pData==NULL && m_dwDataSize==0);
	ASSERT(m_pMapData==NULL && m_dwMapDataSize==0);
	ASSERT(m_pPackedData==NULL && m_dwPackedDataSize==0);
	ASSERT(m_pRequest==NULL);
}

void CMiniMapServer::Create()
{
	// Zatim zadny pozadavek
	m_pRequest=NULL;
	// A zadny buffer na jeho vyrizeni
	m_dwMapDataSize=0;	
	m_pMapData=NULL;

	m_pMapDataSource=NULL;

	// Alokace bufferu
	
	// Nejprve si spocteme jeho velikost - potrebujeme tolik, aby se tam vesla
	// minimapa o jiste maximalni velikosti (definovane v MiniMapStructures.h)
	// a jeste trochu extra mista pro spakovani teto minimapy
	m_dwDataSize=cMiniMapMaxWidth*cMiniMapMaxHeight+cMiniMapExtraStorageSize;
	// A naalokoujeme

    //  m_pData=(BYTE*)malloc(m_dwDataSize);
	m_pData=new BYTE[m_dwDataSize];
}

void CMiniMapServer::Delete()
{
	// Nesmi byt vyrizovan zadny pozadavek
	ASSERT(m_pRequest==NULL);
	ASSERT(m_dwMapDataSize==0 && m_pMapData==NULL);
	ASSERT(m_pData!=NULL && m_dwDataSize>0);

	m_dwDataSize=0;
	delete [] m_pData;
	m_pData=NULL;
}

void CMiniMapServer::StartRenderingUnits(CZMiniMapRequest *pRequest)
{
	ASSERT(m_pRequest==NULL);
	ASSERT(pRequest!=NULL);
	ASSERT(m_dwDataSize>0 && m_pData!=NULL);

	m_pRequest=pRequest;

	// Rekneme requestu, ze ho zaciname zpracovavat (aby nam nevysumel)
	m_pRequest->CalculationStart();

	// Zjistime si pozadovany obdelnik
	m_pRequest->GetClip(m_Clip);

	// Jestlize je tento obdelnik nesmyslny, splachujeme
	if(m_Clip.nWidth<=0 || m_Clip.nHeight<=0 || m_Clip.nZoom<=0) return;
	
	// Souradnice kraju
	int l, t, r, b;

	l=m_Clip.nLeft;
	t=m_Clip.nTop;
	r=m_Clip.nLeft+m_Clip.nWidth-1;
	b=m_Clip.nTop+m_Clip.nHeight-1;

	// Extra okraje
	int extra;

	extra=(+m_Clip.nWidth*cMiniMapExtraBorder)/100;

	l-=extra; t-=extra;
	r+=extra; b+=extra;

	// Kontrola na vyboceni do zaporu mimo mapu
	if(l<0) l=0;
	if(t<0) t=0;
	if(r<0) r=0;
	if(b<0) b=0;

	// Kontrola na velikost mapy
	CSizeDW size=g_cMap.GetMapSizeMapCell();

	if(l>=(int)size.cx) l=size.cx-1;
	if(t>=(int)size.cy) t=size.cy-1;
	if(r>=(int)size.cx) r=size.cx-1;
	if(b>=(int)size.cy) b=size.cy-1;

	// Prepocteni souradnic na zadany zoom
	ASSERT(m_Clip.nZoom==2 || m_Clip.nZoom==4 || m_Clip.nZoom==8 || m_Clip.nZoom==16);
	
	l/=m_Clip.nZoom; t/=m_Clip.nZoom;
	r/=m_Clip.nZoom; b/=m_Clip.nZoom;
		
	// Kontrola na velikost vznikleho obdelniku	
	if(r-l+1>cMiniMapMaxWidth) {
		// Vznikly obdelnik je prilis siroky
		extra=(r-l+1-cMiniMapMaxWidth+1)/2;
		l+=extra;
		r-=extra;
		ASSERT(r-l+1<=cMiniMapMaxWidth);
	}

	if(t-b+1>cMiniMapMaxHeight) {
		// Vznikly obdelnik je prilis vysoky
		extra=(r-l+1-cMiniMapMaxHeight+1)/2;
		t+=extra;
		b-=extra;
		ASSERT(t-b+1<=cMiniMapMaxHeight);
	}

	// Jeste kontrola, jestli se to nepomichalo
	ASSERT(0<=l && l<=r);
	ASSERT(0<=t && t<=b);
	
	// Nyni mame po obrovske namaze konecne obdelnik, ktery nam vyhovuje

	m_Clip.nLeft=l;
	m_Clip.nTop=t;
	m_Clip.nWidth=r-l+1;
	m_Clip.nHeight=b-t+1;

	// Takze mame informaci o velikosti pameti, kterou budeme potrebovat na minimapu
	m_dwMapDataSize=m_Clip.nWidth*m_Clip.nHeight;

	// Prepocitame jeho souradnice zpet na mapcelly a ulozime si je
	m_Clip.nLeft*=m_Clip.nZoom;
	m_Clip.nTop*=m_Clip.nZoom;
	m_Clip.nWidth*=m_Clip.nZoom;
	m_Clip.nHeight*=m_Clip.nZoom;

	// Nyni nastavime ukazatel na zacatek pameti, ktera bude obsahovat minimapu
	m_pMapData=m_pData+(m_dwDataSize-m_dwMapDataSize);

	// A tuto oblast pameti musime vynulovat (presneji receno, vyFFkovat)
	memset(m_pMapData, 0xFF, m_dwMapDataSize);
}

void CMiniMapServer::RenderUnit(DWORD dwX, DWORD dwY, int nCivilization, int nSize)
{
	ASSERT(m_dwDataSize>0 && m_pData!=NULL);

	// Jestlize je sledovany obdelnik nesmyslny, splachujeme
	if(m_Clip.nWidth<=0 || m_Clip.nHeight<=0 || m_Clip.nZoom<=0) return;
	
	ASSERT(m_dwMapDataSize>0 && m_pMapData!=NULL);
	ASSERT(m_pRequest!=NULL);
	ASSERT(nCivilization>=0 && nCivilization<255);

	// Vysledne souradnice v ramci minimapy
	int x, y;

	// A sirka radku minimapy (v pixelech)
	int scanline;

	scanline=m_Clip.nWidth/m_Clip.nZoom;	

	x=(int)dwX-m_Clip.nLeft;
	y=(int)dwY-m_Clip.nTop;

	// Jestlize je jednotka mimo sledovanou oblast, splachujeme tez
	if(x<0 || y<0 || x>=m_Clip.nWidth || y>=m_Clip.nHeight) return;
	
	x/=m_Clip.nZoom;
	y/=m_Clip.nZoom;

	if((nSize/m_Clip.nZoom)==0) {
		// Jestlize se jednotka promitne do jednopixeloveho ctverecku
		// Neni treba delat saskarny
        if(x+y*scanline<(int)m_dwMapDataSize)
    		m_pMapData[x+y*scanline]=(BYTE)nCivilization;
        if(x+1+y*scanline<(int)m_dwMapDataSize)
    		m_pMapData[x+1+y*scanline]=(BYTE)nCivilization;
        if(x+scanline+y*scanline<(int)m_dwMapDataSize)
    		m_pMapData[x+scanline+y*scanline]=(BYTE)nCivilization;
        if(x+scanline+1+y*scanline<(int)m_dwMapDataSize)
    		m_pMapData[x+scanline+1+y*scanline]=(BYTE)nCivilization;        
	} else {
		// Jinak je treba delat saskarny
		// Velikost jednotky v pixelech na mape a jeji souradnice
		int s, xs, ys, xe, ye;	

		s=nSize/m_Clip.nZoom;

		xs=x-(s/2); ys=y-(s/2);
		xe=xs+s; ye=ys+s;

        if(xs<0) xs=0;
        if(ys<0) ys=0;

		for(int i=xs; i<=xe; i++) {
			for(int j=ys; j<=ye; j++) {
                if(i+j*scanline<(int)m_dwMapDataSize) {
				    m_pMapData[i+j*scanline]=(BYTE)nCivilization;
                }
			}
		}
	}
}

void CMiniMapServer::StopRenderingUnits()
{
	ASSERT(m_dwDataSize>0 && m_pData!=NULL);

	// Jestlize je sledovany obdelnik nesmyslny, splachujeme
	if(m_Clip.nWidth<=0 || m_Clip.nHeight<=0 || m_Clip.nZoom<=0) {
		m_pRequest->CalculationStop();
		
		m_pRequest=NULL;
		m_dwMapDataSize=0;
		m_pMapData=NULL;

		return;
	}
	
	ASSERT(m_dwMapDataSize>0 && m_pMapData!=NULL);
	ASSERT(m_pRequest!=NULL);

	// Jinak nastava radostne pakovani
	
	// Nastavime ukazatel na misto pro pakovani
	m_pPackedData=m_pData;
	// Zatim nemame ani jeden spakovany bajt
	m_dwPackedDataSize=0;

	// Aktualni pozice v rozpakovanych datech
	m_pMapDataSource=m_pMapData;

	// Vytvorime si pakostroj
	CCompressEngine Engine;

	// Nastavime ho na pakovani (rychle)
	Engine.InitCompression(*this, 1);

	// Urcime, po kolika budeme zapisovat data z minimapy
	// Momentalne je to jedna scanline
	DWORD dwChunkSize=(m_Clip.nWidth/m_Clip.nZoom);

	// A nyni v cyklu, po radkach, strkame pakostroji minimapu		
	for(DWORD i=0; i<m_dwMapDataSize; i+=dwChunkSize) {
		// Vzdy strcime danou velikost bloku, pokud jsme na konci, tak ten zbytek
		Engine.Write(m_pMapDataSource, dwChunkSize<m_dwMapDataSize-i?dwChunkSize:m_dwMapDataSize-i);
		m_pMapDataSource+=dwChunkSize;
	}

	// Pakovani ukonceno
	Engine.DoneCompression();

	// Nyni na pozici m_pData mame m_dwPackedDataSize bajtu pakovane minimapy
	// Staci ji dat k dispozici CZMiniMapRequestu
	m_pRequest->SendMiniMap(m_Clip, m_pData, m_dwPackedDataSize, m_dwMapDataSize);

	// A nyni uz muze request jit k certu
	m_pRequest->CalculationStop();
	m_pRequest=NULL;

	// A vynulujeme informaci o pakovani a o mape
	m_dwPackedDataSize=0;
	m_pPackedData=NULL;
	
	m_dwMapDataSize=0;
	m_pMapData=NULL;

	m_pMapDataSource=NULL;
}

ECompressData CMiniMapServer::StoreData(void *lpBuf, DWORD dwSize)
{
	// Protoze pakovaci buffer je nastaven presne tam, kam pakujeme
	// nutne musi platit, ze ukazatel, ktery dostaneme, odpovida m_pPackedData
	ASSERT(lpBuf==m_pPackedData);

	// Tento ukazatel je treba updatnout a zvetsit velikost spotrebovane pameti
	m_pPackedData+=dwSize;
	m_dwPackedDataSize+=dwSize;

	return compressDataOK;
}

void CMiniMapServer::GetStoreBuffer(void *&pBuffer, DWORD &dwSize)
{
	ASSERT(m_pPackedData);
	ASSERT(m_pMapDataSource);

	// Bufer zacina na pozici m_pPackedData
	pBuffer=m_pPackedData;

	// A je tak velky, aby se neprekryl s vlastnimi daty mapy
	// (krome tech jiz zpracovanych)
	dwSize=m_pMapDataSource-m_pPackedData;

	// Kdyby tenhle assert neprosel, znamena to, ze spakovana data
	// rostou rychleji, nez jsou ubirana nepakovana. To je nesmirne patologicka situace
	// kterou lze resit zvetsenim konstanty cMiniMapExtraStorageSize v MiniMapStructures.h
	ASSERT(dwSize>0);
}

// Debugovani
#ifdef _DEBUG
void CMiniMapServer::AssertValid() const
{
	ASSERT(m_dwDataSize>0);
	ASSERT(m_pData!=NULL);

	m_Clip.AssertValid();
}

void CMiniMapServer::Dump(CDumpContext &dc) const
{
	dc << "CMiniMapServer\n";
}
#endif
