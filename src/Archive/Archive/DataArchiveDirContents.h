// DataArchiveDirContents.h: interface for the CDataArchiveDirContents class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEDIRCONTENTS_H__B9F8DE16_9343_11D3_BF6A_F011E1DF9D07__INCLUDED_)
#define AFX_DATAARCHIVEDIRCONTENTS_H__B9F8DE16_9343_11D3_BF6A_F011E1DF9D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataArchiveFileDirectoryItem.h"

class CDataArchiveDirContents : public CObject  
{
public:
	// Presun na prvni zaznam
	BOOL MoveFirst();

	// Presun na nasledujici zaznam
	BOOL MoveNext();

	// Vrati info o zaznamu
	CDataArchiveFileDirectoryItem *GetInfo();

	// Konstruktor
	CDataArchiveDirContents(int nItems, CDataArchiveFileDirectoryItem **pItems);
	
	// Destruktor
	virtual ~CDataArchiveDirContents();

private:
	// Pocet zaznamu
	int m_nItems;

	// Aktualni zaznam
	int m_nActual;

	// Pole zaznamu
	CDataArchiveFileDirectoryItem **m_pItems;
};

#endif // !defined(AFX_DATAARCHIVEDIRCONTENTS_H__B9F8DE16_9343_11D3_BF6A_F011E1DF9D07__INCLUDED_)
