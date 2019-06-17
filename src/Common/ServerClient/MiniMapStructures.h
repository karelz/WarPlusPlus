#ifndef __MINIMAP_STRUCTURES_H__
#define __MINIMAP_STRUCTURES_H__

// Definice zakladnich struktur a konstant pro fungovani minimapy

// Konstanty
enum EMiniMapConstants {
	cMiniMapMaxWidth=300, // Maximalni mozna sirka minimapy
	cMiniMapMaxHeight=300, // Maximalni mozna vyska minimapy
	cMiniMapExtraStorageSize=10240, // Velikost extra pameti pouzivane pri pakovani
	cMiniMapExtraBorder=30,	// Velikost pridavaneho okraje k pozadovane minimape (v procentech)
};

// Definice vyrezu minimapy
struct SMiniMapClip {
public:
	// Konstruktor
	SMiniMapClip() {
		Reset();
	}

	// Copy konstruktor
	SMiniMapClip(const SMiniMapClip& Clip) {
		*this=Clip;
	}
	
	// Operator =
	SMiniMapClip& operator=(const SMiniMapClip &Clip) {
		nLeft=Clip.nLeft;
		nTop=Clip.nTop;
		nWidth=Clip.nWidth;
		nHeight=Clip.nHeight;
		nZoom=Clip.nZoom;
		return *this;
	}

	// Operator ==
	BOOL operator==(SMiniMapClip &Clip) {
		// Dva vyrezy jsou stejne, pokud pokryvaji stejne mapcelly a maji stejny zoom
		// Pokud tyto dva vyrezy nepokryvaji ZADNE mapcelly, jsou stejne automaticky

		if(nWidth==0 && Clip.nWidth==0) return TRUE;
		if(nHeight==0 && Clip.nHeight==0) return TRUE;
		
		return nLeft==Clip.nLeft && nTop==Clip.nTop && 
			   nWidth==Clip.nWidth && nHeight==Clip.nHeight &&
			   nZoom==Clip.nZoom;
	}

	// Operator !=
	BOOL operator!=(SMiniMapClip &Clip) {
		return !(*this==Clip);
	}

	// Vytvoreni z dat
	SMiniMapClip(int nLeft, int nTop, int nWidth, int nHeight, int nZoom) {
		this->nLeft=nLeft;
		this->nTop=nTop;
		this->nWidth=nWidth;
		this->nHeight=nHeight;
		this->nZoom=nZoom;
	}
	
	// Je to povoleny vyrez?
	void AssertValid() const {
		ASSERT(nZoom==2 || nZoom==4 || nZoom==8 || nZoom==16);
		ASSERT(nWidth>=0 && nHeight>=0);
	}

	void Dump(CDumpContext &dc) const {
		dc << "pos: " << nLeft << ", " << nTop << " size: " << nWidth << ", " << nHeight << " zoom: " << nZoom;
	}

	// Vynulovani vyrezu
	void Reset() {
		nLeft=nTop=nWidth=nHeight=0;
		nZoom=2;
	}

	// Je vyrez prazdny? (neobsahuje zadne mapcelly)
	BOOL Empty() {
		return nWidth==0 || nHeight==0;
	}

	// Data
public:
	int nLeft, nTop;
	int nWidth, nHeight;
	
	// Zoom smi byt 2, 4, 8 a 16
	int nZoom;
};

#endif __MINIMAP_STRUCTURES_H__