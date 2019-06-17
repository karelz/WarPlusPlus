// DataArchiveFileDataList.cpp: implementation of the CDataArchiveFileDataList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFileDataList.h"
#include "DataArchiveException.h"
#include "Definitions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileDataList::CDataArchiveFileDataList()
{
	m_pFile=(CMappedFile*)NULL;
	m_dwPosition=0;
	m_dwGranularity=DATA_ARCHIVE_FILE_FILE_DATA_GRANULARITY;
}

CDataArchiveFileDataList::~CDataArchiveFileDataList()
{
	if(m_pFile) {
		Close();
		m_pFile=(CMappedFile*)NULL;
	}
}

void CDataArchiveFileDataList::Open(CMappedFile &File, DWORD dwOffset)
{
	m_pFile=&File;
	m_dwListOffset=dwOffset;
	m_dwPosition=0;

	// Nacteme si hlavicku
	m_pFile->Seek(dwOffset, CFile::begin);
	m_pFile->ForceRead(&m_DataList, sizeof(m_DataList));

	// Nacteme si prvni blok
	m_pFile->Seek(m_DataList.dwHeadOffset, CFile::begin);
	m_pFile->ForceRead(&m_DataItem, sizeof(m_DataItem));
	m_dwItemOffset=m_DataList.dwHeadOffset;
}

void CDataArchiveFileDataList::Close()
{
	// Neni treba nic extra delat
	m_pFile=(CMappedFile*)NULL;
	m_dwPosition=0;
}

DWORD CDataArchiveFileDataList::Create(CMappedFile &File, DWORD dwSize, BOOL bGonnaFill /* =FALSE */)
{
	SDataArchiveFileDataItem dtaItem;

	m_pFile=&File;
	m_dwPosition=0;

	// Vytvorime hlavicku
	m_pFile->Seek(0, CFile::end);
	m_dwListOffset=File.Position(); // Hlavicka zacina na konci souboru
	
	// Seznam bude mit jednu polozku
	m_DataList.nItems=1; 
	// Polozka bude umistena za hlavickou seznamu
	m_DataList.dwHeadOffset=m_DataList.dwTailOffset=m_dwListOffset+sizeof(m_DataList);
	if(!bGonnaFill) {
		// Data v listu zatim nejsou zadna, mame jen rezervovanu velikost
		m_DataList.dwLength=0;
	} else {
		// V zapeti budou data zapsana, muzeme delku dat nastavit
		m_DataList.dwLength=dwSize;
	}
	
	// Zapiseme hlavicku
	m_pFile->Write(&m_DataList, sizeof(m_DataList));

	// Tvorime datovou polozku
	m_pFile->Seek(0, CFile::end);
	dtaItem.dwDataOffset=File.Position()+sizeof(SDataArchiveFileDataItem); // Zacatek dat
	dtaItem.dwListOffset=0; // Jsme na zacatku listu
	dtaItem.dwDataSizeAllocated=dwSize; // Alokovana velikost je zadana
	dtaItem.dwNextOffset=0; // Zadny data item nenasleduje
	dtaItem.dwPrevOffset=0; // Zadny data item nepredchazi

	// Zapamatujeme si pozici polozky
	m_dwItemOffset=File.Position();
	// Zapiseme datovou polozku
	m_pFile->Write(&dtaItem, sizeof(SDataArchiveFileDataItem));
	// Skocime na konec
	m_pFile->Seek(0, CFile::end);
	// Prodlouzime soubor o data na konci
	m_pFile->SetLength(m_pFile->Length()+dwSize);

	// Zapamatujeme si DataItem
	m_DataItem=dtaItem;

	return m_dwListOffset;
}

UINT CDataArchiveFileDataList::Read(void *pData, UINT nCount)
{
	UINT nRead, alreadyRead;
	char *pDta=(char *)pData;
	ASSERT(m_pFile);

	// Seek na spravnou pozici
	Seek(m_dwPosition, CFile::begin);

	// Zatim jsme nic nenacetli
	alreadyRead=0;

	// Dokud nemame vsechno
	while(alreadyRead<nCount) {
		// Nacteme maximalne tolik, kolik zbyva do konce bloku, ...
		nRead=MIN3(m_DataItem.dwDataSizeAllocated-(m_dwPosition-m_DataItem.dwListOffset), 
				  nCount-alreadyRead, // ...maximalne kolik jeste chceme, ...
				  m_DataList.dwLength-m_dwPosition // ...maximalne kolik zbyva do konce souboru
				  ); 
		m_pFile->ForceRead(pDta, nRead);
		alreadyRead+=nRead;
		m_dwPosition+=nRead;
		pDta+=nRead;
		if(nRead==0) break;
		if(alreadyRead<nCount) NextItem();
	}
	return alreadyRead;
}

void CDataArchiveFileDataList::Write(void *pData, UINT nCount)
{
	UINT nWrite, alreadyWritten;
	DWORD dwLastOffset, dwDataSizeUsed;
	char *pDta=(char *)pData;
	BOOL bLengthIncreased=FALSE;

	ASSERT(m_pFile);

	// Seek na spravnou pozici
	Seek(m_dwPosition, CFile::begin);

	// Zatim jsme nic nezapsali
	alreadyWritten=0;

	// Dokud nejsme na konci seznamu (break vyskoci z cyklu)
	while(TRUE) {
		// Zapiseme jen tolik, kolik zbyva do konce bloku
		nWrite=min(m_DataItem.dwDataSizeAllocated-(m_dwPosition-m_DataItem.dwListOffset), nCount-alreadyWritten);
		if(nWrite>0) {
			m_pFile->Write(pDta, nWrite);
			alreadyWritten+=nWrite;
			m_dwPosition+=nWrite;
			pDta+=nWrite;
		}
		if(alreadyWritten<nCount) { 
			if(!NextItem()) break; // Skok z cyklu
		} else {
			ASSERT(alreadyWritten==nCount);
			break;
		}
	}

	if(alreadyWritten<nCount) { // Jsme na konci listu, je treba pridat dalsi zaznam
		SDataArchiveFileDataItem dtaItem;

	// Tvorime datovou polozku na konci listu
		m_pFile->Seek(0, CFile::end);
		dwLastOffset=m_pFile->Position();
		dtaItem.dwDataOffset=dwLastOffset+sizeof(SDataArchiveFileDataItem); // Zacatek dat
		dtaItem.dwListOffset=m_dwPosition; // Pozice v listu
		
		// Alokovana velikost je zbytek, zaokrouhleny na granularitu
		dtaItem.dwDataSizeAllocated=(1+((nCount-alreadyWritten)/m_dwGranularity))*m_dwGranularity; 
		
		dwDataSizeUsed=nCount-alreadyWritten; // Pouzivana velikost je presne zbytek
		dtaItem.dwNextOffset=0; // Zadny data item nenasleduje
		dtaItem.dwPrevOffset=m_dwItemOffset; // Predchazi aktualni item

		// Zapiseme datovou polozku
		m_pFile->Write(&dtaItem, sizeof(SDataArchiveFileDataItem));
		// Skocime na konec
		m_pFile->Seek(0, CFile::end);
		// Zapamatujeme si pozici
		DWORD oldPosition=m_pFile->Position();
		// Prodlouzime soubor o alokovana data
		m_pFile->SetLength(m_pFile->Length()+dtaItem.dwDataSizeAllocated);
		// A vratime se tam, kde jsme byli
		m_pFile->Seek(oldPosition, CFile::begin);
		// Zapiseme data na konec
		m_pFile->Write(pDta, dwDataSizeUsed);
		// Pozice se zmenila o to, co jsme zapsali
		m_dwPosition+=dwDataSizeUsed;

	// Zmena ukazatele na dalsi item v poslednim itemu listu
		m_pFile->Seek(m_dwItemOffset, CFile::begin);
		m_DataItem.dwNextOffset=dwLastOffset;
		m_pFile->Write(&m_DataItem, sizeof(m_DataItem));

		// Zmenime hlavicku listu
		m_DataList.dwTailOffset=dwLastOffset;
		m_DataList.nItems++;
		bLengthIncreased=TRUE;

		// Zapamatujeme si DataItem
		m_DataItem=dtaItem;
		m_dwItemOffset=dwLastOffset;
	}
	
	// Pokud se soubor prodlouzil je treba zmenit hlavicku listu 
	if(bLengthIncreased || m_dwPosition>m_DataList.dwLength) {
		m_pFile->Seek(m_dwListOffset, CFile::begin);
		m_DataList.dwLength=m_dwPosition; // Jsme na konci, delka==pozice
		m_pFile->Write(&m_DataList, sizeof(m_DataList));
	}
}

void CDataArchiveFileDataList::Empty() {
	Seek(0, CFile::begin);

	// Je treba zmenit hlavicku listu 
	m_dwPosition=0; // Jsme na zacatku
	m_pFile->Seek(m_dwListOffset, CFile::begin);
	m_DataList.dwLength=0; // Seznam je prazdny
	m_pFile->Write(&m_DataList, sizeof(m_DataList));
	m_pFile->Flush();
}

void CDataArchiveFileDataList::Flush() {
	ASSERT(m_pFile);
	m_pFile->Flush();
}

void CDataArchiveFileDataList::Seek(DWORD dwOffset, UINT nFrom)
{
	ASSERT(m_pFile);
	switch(nFrom) {
	case CFile::begin:
		break;
	case CFile::end:
		dwOffset=m_DataList.dwLength+dwOffset;
		break;
	case CFile::current:
		dwOffset+=m_dwPosition;
		break;
	}

	if(dwOffset <0 || dwOffset>m_DataList.dwLength) { // Seek mimo soubor
		throw new CDataArchiveException(m_pFile->Path(), CDataArchiveException::EDataArchiveExceptionTypes::badSeek, CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
	}

	while(m_DataItem.dwListOffset>dwOffset) { // Seekujeme dopredu
		if(!PrevItem()) { // Nejde to
			throw new CDataArchiveException(m_pFile->Path(), CDataArchiveException::EDataArchiveExceptionTypes::badSeek, CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		}
	}

	while(m_DataItem.dwListOffset+m_DataItem.dwDataSizeAllocated-1<dwOffset) { // Seekujeme dozadu
		// Jestlize seekujeme presne na konec souboru 
		// a jsme na poslednim bloku, ktery je uplne zaplnen, nedelame prechod na dalsi
		if(dwOffset==m_DataList.dwLength &&  // Na konec
			m_DataItem.dwListOffset+m_DataItem.dwDataSizeAllocated==dwOffset && // Uplne zaplnen
			m_DataItem.dwNextOffset==0) { // Posledni blok
			break;
		}
		if(!NextItem()) { // Nejde to
			throw new CDataArchiveException(m_pFile->Path(), CDataArchiveException::EDataArchiveExceptionTypes::badSeek, CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		}
	}

	// Seekneme se spravne v ramci souboru
	m_pFile->Seek(m_DataItem.dwDataOffset+dwOffset-m_DataItem.dwListOffset, CFile::begin);
	
	// Uz jsme na spravnem bloku
	m_dwPosition=dwOffset;
}

void CDataArchiveFileDataList::SetLength(DWORD dwLength) {
	ASSERT(m_pFile);
	if(m_DataList.dwLength<dwLength) {
		TRACE("Funkce SetLength neumi prodluzovat soubory v archivu (zatim)\n");
		ASSERT(FALSE);
	} else {
		m_DataList.dwLength=dwLength;
	}
}

void CDataArchiveFileDataList::SetRealLength(DWORD dwLength) {
	ASSERT(m_pFile);
	m_DataList.dwLength=dwLength;
}

DWORD CDataArchiveFileDataList::GetLength() {
	ASSERT(m_pFile);
	return m_DataList.dwLength;
}

DWORD CDataArchiveFileDataList::GetRealLength() {
	ASSERT(m_pFile);
	return m_DataList.dwLength;
}

DWORD CDataArchiveFileDataList::GetPosition() {
	ASSERT(m_pFile);
	return m_dwPosition;
}

BOOL CDataArchiveFileDataList::NextItem() 
{
	ASSERT(m_pFile);

	// Nacteme dalsi blok
	if(m_DataItem.dwNextOffset>0) { // Dalsi blok existuje
		m_dwItemOffset=m_DataItem.dwNextOffset;
		m_pFile->Seek(m_dwItemOffset, CFile::begin);
		m_pFile->ForceRead(&m_DataItem, sizeof(m_DataItem));
		return TRUE;
	} else { // Dalsi blok neni
		return FALSE;
	}
}

BOOL CDataArchiveFileDataList::PrevItem() 
{
	ASSERT(m_pFile);

	// Nacteme predchozi blok
	if(m_DataItem.dwPrevOffset>0) { // Predchozi blok existuje
		m_dwItemOffset=m_DataItem.dwPrevOffset;
		m_pFile->Seek(m_dwItemOffset, CFile::begin);
		m_pFile->ForceRead(&m_DataItem, sizeof(m_DataItem));
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CDataArchiveFileDataList::FirstItem()
{
	ASSERT(m_pFile);

	// Nacteme prvni blok
	if(m_DataList.dwHeadOffset>0) { // Prvni blok existuje
		m_dwItemOffset=m_DataList.dwHeadOffset;
		m_pFile->Seek(m_dwItemOffset, CFile::begin);
		m_pFile->ForceRead(&m_DataItem, sizeof(m_DataItem));
		return TRUE;
	} else {
		return FALSE;
	}
}