/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da typu jednotky na serveru hry. Obsahuje v�echny 
 *          informace o typu jednotky, kter� pot�ebuje GameServer. 
 *          Ka�d� civilizace m� svoji vlastn� instanci t�to t��dy.
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SUnitType.h"

#include "SMap.h"
#include "SSkillType.h"
#include "Common\Map\Map.h"
#include "GameServer\Civilization\SkillTypeName.h"

#include "SUnit.inl"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC ( CSUnitType, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSUnitType::CSUnitType () 
{
	// zne�kodn� data typu jednotky
	m_dwID = DWORD_MAX;
	m_pCivilization = NULL;

	m_dwSkillTypeCount = 0;
	m_aSkillTypeDescription = NULL;
	m_dwSkillDataSize = 0;

	m_pFirstUnit = NULL;

	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwVerticalPosition = 0;
	m_strUnitTypeName.Empty ();

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		m_aMoveWidth[nModeIndex] = 0;
		m_aAppearanceID[nModeIndex] = DWORD_MAX;
		m_aInvisibilityState[nModeIndex] = 0;
		m_aInvisibilityDetection[nModeIndex] = 0;
	}
	m_dwFlags = 0;
}

// destruktor
CSUnitType::~CSUnitType () 
{
	// zkontroluje ne�kodn� data typu jednotky
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�� typ jednotky ze souboru typu jednotky "cUnitTypeFile" civilizace 
//		"pCivilization"
void CSUnitType::Create ( CArchiveFile cUnitTypeFile, CSCivilization *pCivilization ) 
{
	// zkontroluje ne�kodn� data typu jednotky
	ASSERT ( CheckEmptyData () );

	// po�et vzhled� jednotky
	DWORD dwAppearanceCount;

	// aktualizuje ukazatel na civilizaci
	m_pCivilization = pCivilization;

// na�te hlavi�ku souboru s typem jednotky
	{
		SUnitTypeHeader sUnitTypeHeader;
		LOAD_ASSERT ( cUnitTypeFile.Read ( &sUnitTypeHeader, sizeof ( sUnitTypeHeader ) ) == sizeof ( sUnitTypeHeader ) );

		// zjist� ID typu jednotky
		m_dwID = sUnitTypeHeader.m_dwID;

		// zjist� jm�no typu jednotky
		m_strUnitTypeName = sUnitTypeHeader.m_pName;
		LOAD_ASSERT ( m_strUnitTypeName.GetLength () < sizeof ( sUnitTypeHeader.m_pName ) );
		LOAD_ASSERT ( !m_strUnitTypeName.IsEmpty () );

		// zjist� informace o typu jednotky
		m_nLivesMax = (int)sUnitTypeHeader.m_dwLifeMax;
		LOAD_ASSERT ( m_nLivesMax > 0 );
		m_dwViewRadius = sUnitTypeHeader.m_dwViewRadius;
		m_dwVerticalPosition = sUnitTypeHeader.m_dwZPos;
		LOAD_ASSERT ( m_dwVerticalPosition != 0 );
		m_dwFlags = sUnitTypeHeader.m_dwFlags;

		// projede m�dy typu jednotky
		for ( int nModeIndex = 8; nModeIndex-- > 0; )
		{
			// zjist� ���ku jednotky p�i pohybu
			m_aMoveWidth[nModeIndex] = sUnitTypeHeader.m_aMoveWidth[nModeIndex];
			// zjist� ID vzhledu jednotky
			m_aAppearanceID[nModeIndex] = sUnitTypeHeader.m_aDefaultAppearances[nModeIndex];
			// zjist� bitovou masku p��znak� neviditelnosti jednotky (tj. jak je 
			//		jednotka neviditeln�)
			m_aInvisibilityState[nModeIndex] = sUnitTypeHeader.m_aInvisibilityFlags[nModeIndex];
			// zjist� bitovou masku p��znak� detekce neviditelnosti ostatn�ch jednotek 
			//		(tj. kter� neviditelnosti jednotka vid�)
			m_aInvisibilityDetection[nModeIndex] = sUnitTypeHeader.m_aDetectionFlags[nModeIndex];
			// zjist� ukazatel na graf pro hled�n� cesty
			m_aFindPathGraph[nModeIndex] = (CSFindPathGraph *)sUnitTypeHeader.m_aPathGraphs[nModeIndex];
		}

		LOAD_ASSERT ( m_aAppearanceID[0] != 0 );
		// projede m�dy typu jednotky
		for ( nModeIndex = 8; nModeIndex-- > 1; )
		{
			// zjist�, je-li ID vzhledu jednotky platn�
			if ( m_aAppearanceID[nModeIndex] == 0 )
			{	// ID vzhledu jednotky nen� platn�
				// inicializuje ID vzhledu jednotky
				m_aAppearanceID[nModeIndex] = m_aAppearanceID[0];
			}
		}

		// zjist� po�et vzhled� typu jednotky
		dwAppearanceCount = sUnitTypeHeader.m_dwAppearancesNumber;

		// zjist� po�et typ� skill� typu jednotky
		m_dwSkillTypeCount = sUnitTypeHeader.m_dwSkillTypesCount;
		LOAD_ASSERT ( m_dwSkillTypeCount > 0 );
		LOAD_ASSERT ( m_dwSkillTypeCount < m_dwSkillTypeCount * sizeof ( SSkillTypeDescription ) );
	}

	// p�esko�� vzhledy typu jednotky
	cUnitTypeFile.Seek ( dwAppearanceCount * sizeof ( SAppearance ), CFile::current );

// na�te hlavi�ky typ� skill
	{
		SSkillTypeHeader sSkillTypeHeader;

		// vytvo�� pole typ� skill� typu jednotky
		m_aSkillTypeDescription = new SSkillTypeDescription[m_dwSkillTypeCount];

		// offset dat skilly v datech skill� jednotky
		DWORD dwSkillDataOffset = 0;
		// zjist� ukazatel na prvn� popis typu skilly
		SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

		// projede v�echny typy skill� typu jednotky
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++, pSkillTypeDescription++ )
		{
			// na�te hlavi�ku typu skilly
			LOAD_ASSERT ( cUnitTypeFile.Read ( &sSkillTypeHeader, sizeof ( sSkillTypeHeader ) ) == sizeof ( sSkillTypeHeader ) );

			// zjist� jm�no typu skilly
			CString strSkillTypeName = sSkillTypeHeader.m_pName;
			LOAD_ASSERT ( strSkillTypeName.GetLength () < sizeof ( sSkillTypeHeader.m_pName ) );
			LOAD_ASSERT ( !strSkillTypeName.IsEmpty () );

			// vytvo�� ulo�en� typ skilly podle jm�na
			CSSkillType *pSkillType = CSSkillType::Create ( strSkillTypeName, 
				dwSkillDataOffset, dwSkillTypeIndex, this );
			LOAD_ASSERT ( pSkillType != NULL );
			// aktualizuje offset dat dal�� skilly
			dwSkillDataOffset += pSkillType->GetSkillDataSize ();

			// ukazatel na pam� pro nahr�n� dat typu skilly
			BYTE *pSkillTypeData = NULL;

			try
			{
				// zjist�, m�-li typ skilly data pro nahr�n�
				if ( sSkillTypeHeader.m_dwDataSize > 0 )
				{	// typ skilly m� data pro nahr�n�
					// alokuje pam� pro nahr�n� dat typu skilly
					pSkillTypeData = new BYTE[sSkillTypeHeader.m_dwDataSize];

					// nahraje data typu skilly
					LOAD_ASSERT ( cUnitTypeFile.Read ( pSkillTypeData, sSkillTypeHeader.m_dwDataSize ) == sSkillTypeHeader.m_dwDataSize );
				}

				// inicializuje typ skilly
				pSkillType->Create ( pSkillTypeData, sSkillTypeHeader.m_dwDataSize );
			}
			catch ( CException * )
			{
				// zjist�, je-li alokov�na pam� nahran�ch dat typu skilly
				if ( pSkillTypeData != NULL )
				{	// pam� nahran�ch dat typu skilly je alokovan�
					// zni�� pam� nahran�ch dat typu skilly
					delete pSkillTypeData;
				}
				// zni�� typ skilly
				pSkillType->Delete();
				delete pSkillType;
				// pokra�uje ve zpracov�n� v�jimky
				throw;
			}

			// zjist�, jsou-li nahran� data typu skilly platn�
			if ( pSkillTypeData != NULL )
			{	// nahran� data typu skilly jsou platn�
				// zni�� pam� nahran�ch dat typu skilly
				delete [] pSkillTypeData;
			}

			// inicializuje popis typu skilly
			pSkillTypeDescription->dwID = pSkillType->GetID ();
			pSkillTypeDescription->pNameID = RegisterSkillTypeName ( pSkillType->GetName () );
			pSkillTypeDescription->nAllowedModesMask = sSkillTypeHeader.m_nAllowedModes;
			pSkillTypeDescription->bEnabled = sSkillTypeHeader.m_bEnabled;
			pSkillTypeDescription->pSkillType = pSkillType;
			pSkillTypeDescription->dwGroupMask = pSkillType->GetGroupMask ();
		}

		// aktualizuje velikost dat skilly
		m_dwSkillDataSize = dwSkillDataOffset;
		LOAD_ASSERT ( m_dwSkillDataSize > 0 );
	}

// ukon�� na��t�n� souboru typu jednotky

	// zkontroluje konec souboru
	LOAD_ASSERT ( cUnitTypeFile.GetPosition () == cUnitTypeFile.GetLength () );
}

// dokon�� vytvo�en� typu jednotky
void CSUnitType::PostCreate () 
{
	ASSERT ( m_dwSkillTypeCount != 0 );
	ASSERT ( m_aSkillTypeDescription != NULL );

	// ukazatel na popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede typy skill
	for ( DWORD dwSkillTypeIndex = m_dwSkillTypeCount; dwSkillTypeIndex-- > 0; 
		pSkillTypeDescription++ )
	{
		// dokon�� vytvo�en� typu skilly
		pSkillTypeDescription->pSkillType->PostCreateSkillType ();
	}
}

// zni�� data typu jednotky
void CSUnitType::Delete () 
{
	// zne�kodn� pole typ� skill�
	if ( m_aSkillTypeDescription != NULL )
	{
		ASSERT ( m_dwSkillTypeCount > 0 );
		// projede v�echny popisy typu skilly
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++ )
		{
			// zjist�, je-li ID jm�na typu skilly platn�
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pNameID != NULL )
			{	// ID jm�na typu skilly je platn�
				// zni�� ID jm�na typu skilly
				UnregisterSkillTypeName ( m_aSkillTypeDescription[dwSkillTypeIndex].pNameID );
			}
			// zjist�, je-li typ skilly platn�
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType != NULL )
			{	// typ skilly je platn�
				// zni�� data typu skilly
				m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType->Delete ();
				// zni�� typ skilly
				delete m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType;
			}
		}
		// zni�� pole popis� typ� skill
		delete [] m_aSkillTypeDescription;
		m_aSkillTypeDescription = NULL;
	}

	// zne�kodn� data typu jednotky
	SetEmptyData ();
}

// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
void CSUnitType::SetEmptyData () 
{
	m_dwID = DWORD_MAX;
	m_pCivilization = NULL;

	// zne�kodn� pole typ� skill�
	if ( m_aSkillTypeDescription != NULL )
	{
		ASSERT ( m_dwSkillTypeCount > 0 );
		// projede v�echny popisy typu skilly
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++ )
		{
			// zjist�, je-li typ skilly platn�
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType != NULL )
			{	// typ skilly je platn�
				// zni�� data typu skilly
				m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType->Delete ();
				// zni�� typ skilly
				delete m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType;
			}
		}
		// zni�� pole popis� typ� skill
		delete [] m_aSkillTypeDescription;
		m_aSkillTypeDescription = NULL;
	}
	m_dwSkillTypeCount = 0;
	m_dwSkillDataSize = 0;

	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwVerticalPosition = 0;
	m_strUnitTypeName.Empty ();

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		m_aMoveWidth[nModeIndex] = 0;
		m_aAppearanceID[nModeIndex] = DWORD_MAX;
		m_aInvisibilityState[nModeIndex] = 0;
		m_aInvisibilityDetection[nModeIndex] = 0;
	}
	m_dwFlags = 0;
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSUnitType stored

	DWORD m_dwID
	int m_nLivesMax
	DWORD m_dwViewRadius
	DWORD m_dwVerticalPosition
	int nUnitTypeNameSize					// mus� b�t > 0
		char aUnitTypeName[nUnitTypeNameSize]
	DWORD m_aMoveWidth[8]
	DWORD m_aAppearanceID[8]
	DWORD m_aInvisibilityState[8]
	DWORD m_aInvisibilityDetection[8]
	CSFindPathGraph *m_aFindPathGraph[8]
	DWORD m_dwFlags
	DWORD m_dwSkillTypeCount
	[m_dwSkillTypeCount]
		CSSkillType *pSkillType
		DWORD dwID
		CStringTableItem *pNameID
		BOOL bEnabled
		BYTE nAllowedModesMask
		CSSkillType stored
*/

// ukl�d�n� dat
void CSUnitType::PersistentSave ( CPersistentStorage &storage ) 
{
	// zkontroluje platnost dat typu jednotky
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// ulo�� ID typu jednotky
	storage << m_dwID;
	// ulo�� maxim�ln� po�et �ivot� jednotky
	storage << m_nLivesMax;
	// ulo�� polom�r viditelnosti jednotky
	storage << m_dwViewRadius;
	// ulo�� vertik�ln� um�st�n� jednotky
	storage << m_dwVerticalPosition;

	// zjist� d�lku jm�na typu jednotky
	int nUnitTypeNameSize = m_strUnitTypeName.GetLength ();
	ASSERT ( nUnitTypeNameSize > 0 );
	// ulo�� d�lku jm�na typu jednotky
	storage << nUnitTypeNameSize;
	// zjist�, je-li jm�no typu jednotky pr�zdn�
	// ulo�� jm�no typu jednotky
	storage.Write ( m_strUnitTypeName, nUnitTypeNameSize );

	// ulo�� ���ku jednotky
	ASSERT ( sizeof ( m_aMoveWidth ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aMoveWidth, sizeof ( m_aMoveWidth ) );

	// ulo�� ID vzhledu jednotky
	ASSERT ( sizeof ( m_aAppearanceID ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aAppearanceID, sizeof ( m_aAppearanceID ) );

	// ulo�� bitov� masky p��znak� neviditelnosti jednotky
	ASSERT ( sizeof ( m_aInvisibilityState ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aInvisibilityState, sizeof ( m_aInvisibilityState ) );

	// ulo�� bitov� masky p��znak� detekce neviditelnosti ostatn�ch jednotek
	ASSERT ( sizeof ( m_aInvisibilityDetection ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aInvisibilityDetection, sizeof ( m_aInvisibilityDetection ) );

	// ulo�� ukazatele na grafy pro hled�n� cesty
	ASSERT ( sizeof ( m_aFindPathGraph ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aFindPathGraph, sizeof ( m_aFindPathGraph ) );

	// ulo�� p��znaky typu jednotky
	storage << m_dwFlags;

// ulo�� typy skill� typu jednotky

	// ulo�� po�et typ� skill� typu jednotky
	storage << m_dwSkillTypeCount;

	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// ulo�� popis typu skilly
		storage << pSkillTypeDescription->pSkillType;
		storage << pSkillTypeDescription->dwID;
		storage << pSkillTypeDescription->pNameID;
		storage << pSkillTypeDescription->bEnabled;
		storage << pSkillTypeDescription->nAllowedModesMask;

		// ulo�� typ skilly
		pSkillTypeDescription->pSkillType->PersistentSave ( storage );
	}
}

// nahr�v�n� pouze ulo�en�ch dat
void CSUnitType::PersistentLoad ( CPersistentStorage &storage, CSUnitType *pOldUnitType ) 
{
	// zkontroluje ne�kodn� data typu jednotky
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// na�te ID typu jednotky
	storage >> m_dwID;
	// na�te maxim�ln� po�et �ivot� jednotky
	storage >> m_nLivesMax;
	LOAD_ASSERT ( m_nLivesMax > 0 );
	// na�te polom�r viditelnosti jednotky
	storage >> m_dwViewRadius;
	// na�te vertik�ln� um�st�n� jednotky
	storage >> m_dwVerticalPosition;
	LOAD_ASSERT ( m_dwVerticalPosition != 0 );

	// zjist� d�lku jm�na typu jednotky
	int nUnitTypeNameSize;
	// na�te d�lku jm�na typu jednotky
	storage >> nUnitTypeNameSize;
	LOAD_ASSERT ( nUnitTypeNameSize > 0 );
	// z�sk� ukazatel na na��tan� jm�no typu jednotky
	char *szUnitTypeName = m_strUnitTypeName.GetBufferSetLength ( nUnitTypeNameSize );
	// na�te jm�no typu jednotky
	storage.Read ( szUnitTypeName, nUnitTypeNameSize );
	// ukon�� na��t�n� jm�na typu jednotky
	m_strUnitTypeName.ReleaseBuffer ( nUnitTypeNameSize );

	// na�te ���ku jednotky
	ASSERT ( sizeof ( m_aMoveWidth ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aMoveWidth, sizeof ( m_aMoveWidth ) );

	// na�te ID vzhledu jednotky
	ASSERT ( sizeof ( m_aAppearanceID ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aAppearanceID, sizeof ( m_aAppearanceID ) );

	// zkontroluje ID vzhledu jednotky
	for ( int nModeIndex = 0; nModeIndex < 8; nModeIndex++ )
	{
		LOAD_ASSERT ( m_aAppearanceID[nModeIndex] != 0 );
	}

	// na�te bitov� masky p��znak� neviditelnosti jednotky
	ASSERT ( sizeof ( m_aInvisibilityState ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aInvisibilityState, sizeof ( m_aInvisibilityState ) );

	// na�te bitov� masky p��znak� detekce neviditelnosti ostatn�ch jednotek
	ASSERT ( sizeof ( m_aInvisibilityDetection ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aInvisibilityDetection, sizeof ( m_aInvisibilityDetection ) );

	// na�te ukazatele na grafy pro hled�n� cesty
	ASSERT ( sizeof ( m_aFindPathGraph ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aFindPathGraph, sizeof ( m_aFindPathGraph ) );

	// ulo�� p��znaky typu jednotky
	storage >> m_dwFlags;

// na�te typy skill� typu jednotky

	// na�te po�et typ� skill� typu jednotky
	storage >> m_dwSkillTypeCount;
	LOAD_ASSERT ( m_dwSkillTypeCount > 0 );
	LOAD_ASSERT ( m_dwSkillTypeCount < m_dwSkillTypeCount * sizeof ( SSkillTypeDescription ) );

	// vytvo�� pole typ� skill� typu jednotky
	m_aSkillTypeDescription = new SSkillTypeDescription[m_dwSkillTypeCount];

	// offset dat skilly v datech skill� jednotky
	DWORD dwSkillDataOffset = 0;
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// na�te popis typu skilly
		storage >> (void *&)pSkillTypeDescription->pSkillType;
		storage >> pSkillTypeDescription->dwID;
		storage >> (void *&)pSkillTypeDescription->pNameID;
		storage >> pSkillTypeDescription->bEnabled;
		storage >> pSkillTypeDescription->nAllowedModesMask;

		// zkontroluje jm�no typu skilly (ID jm�na typu skilly)
		LOAD_ASSERT ( pSkillTypeDescription->pNameID != NULL );

		// vytvo�� ulo�en� typ skilly
		CSSkillType *pSkillType = CSSkillType::Create ( pSkillTypeDescription->dwID, 
			dwSkillDataOffset, dwSkillTypeIndex, this );
		LOAD_ASSERT ( pSkillType != NULL );
		// aktualizuje offset dat dal�� skilly
		dwSkillDataOffset += pSkillType->GetSkillDataSize ();

		// zaregistruje ukazatel na typ skilly
		storage.RegisterPointer ( pSkillTypeDescription->pSkillType, pSkillType );

		try
		{
			// na�te typ skilly
			pSkillType->PersistentLoad ( storage );
		}
		catch ( CException * )
		{
			// zni�� typ skilly
			delete pSkillType;
			// pokra�uje ve zpracov�n� v�jimky
			throw;
		}

		// aktualizuje ukazatel na typ skilly
		pSkillTypeDescription->pSkillType = pSkillType;
	}

	// aktualizuje velikost dat skilly
	m_dwSkillDataSize = dwSkillDataOffset;
	LOAD_ASSERT ( m_dwSkillDataSize > 0 );
}

// p�eklad ukazatel�
void CSUnitType::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

    TRACE("*** 0x%x ***\n", this);

    // projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjist� ukazatel na jm�no typu skilly (ID jm�na typu skilly)
		pSkillTypeDescription->pNameID = (CStringTableItem *)storage.TranslatePointer ( 
			pSkillTypeDescription->pNameID );

		// p�elo�� ukazatele typu skilly
		pSkillTypeDescription->pSkillType->PersistentTranslatePointers ( storage );
	}

	// zjist� ukazatele na grafy pro hled�n� cesty
	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		// zjist� ukazatel na graf pro hled�n� cesty
		m_aFindPathGraph[nModeIndex] = (CSFindPathGraph *)storage.TranslatePointer ( 
			m_aFindPathGraph[nModeIndex] );
	}
}

// inicializace nahran�ho objektu
void CSUnitType::PersistentInit ( CSCivilization *pCivilization ) 
{
	// inicializuje ukazatel na civilizaci
	m_pCivilization = pCivilization;

	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// inicializuje masku skupin skilly
		pSkillTypeDescription->dwGroupMask = 
			pSkillTypeDescription->pSkillType->GetGroupMask ();
		// inicializuje typ skilly
		pSkillTypeDescription->pSkillType->PersistentInit ();
	}
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody serverov�ch informac� o typu jednotky
//////////////////////////////////////////////////////////////////////

// initializuje skilly jednotky "pUnit" (vol�no po vytvo�en� objektu jednotky)
void CSUnitType::InitializeSkills ( CSUnit *pUnit ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// inicializuje skillu jednotky
		pSkillTypeDescription->pSkillType->InitializeSkill ( pUnit );
	}
}

// zni�� skilly jednotky "pUnit" (vol�no p�ed zni�en�m objektu jednotky)
void CSUnitType::DestructSkills ( CSUnit *pUnit ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjist�, je-li skilla aktivn�
		if ( pSkillTypeDescription->pSkillType->IsSkillActive ( pUnit ) )
		{	// skilla je aktivn�
			// deaktivuje skillu jednotky
			pSkillTypeDescription->pSkillType->DeactivateSkill ( pUnit );
		}

		// zni�� skillu jednotky
		pSkillTypeDescription->pSkillType->DestructSkill ( pUnit );
	}
}

// deaktivuje skupiny skill "dwGroupMask" jednotky "pUnit" (jednotka mus� b�t zam�ena 
//		pro z�pis)
void CSUnitType::DeactivateSkillGroup ( CSUnit *pUnit, DWORD dwGroupMask ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede v�echny typy skill� typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjist�, jedn�-li se o skillu deaktivovan�ch skupin
		if ( pSkillTypeDescription->dwGroupMask & dwGroupMask )
		{	// jedn� se o skillu deaktivovan�ch skupin
			// zjist�, je-li skilla aktivn�
			if ( pSkillTypeDescription->pSkillType->IsSkillActive ( pUnit ) )
			{	// skilla je aktivn�
				// deaktivuje skillu jednotky
				pSkillTypeDescription->pSkillType->DeactivateSkill ( pUnit );
			}
		}
	}
}

// vr�t� dal�� typ skilly ze skupiny skill "dwGroupMask" po��naje typem skilly s indexem 
//		"dwIndex", kter� posune za nalezen� typ skilly (NULL=nenalezeno)
CSSkillType *CSUnitType::GetNextSkillType ( DWORD &dwIndex, DWORD dwGroupMask ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription + dwIndex;

	// projede v�echny typy skill� typu jednotky
	for ( ; dwIndex++ < m_dwSkillTypeCount; pSkillTypeDescription++ )
	{
		// zjist�, jedn�-li se o typ skilly z hledan� skupiny
		if ( pSkillTypeDescription->dwGroupMask & dwGroupMask )
		{	// jedn� se o typ skilly z hledan� skupiny
			// vr�t� nalezen� typ skilly
			return pSkillTypeDescription->pSkillType;
		}
	}
	// po�adovan� typ skilly nebyl nalezen
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami typu jednotky
//////////////////////////////////////////////////////////////////////

// vytvo�� jednotku na pozici "pointPosition", s velitelem "pCommander", skriptem 
//		"lpszScriptName", sm�rem "nDirection" a vertik�ln� pozic� "dwVerticalPosition" 
//		(0=implicitn� vertik�ln� pozice z typu jednotky) (typ jednotky mus� b�t zam�en pro 
//		z�pis, mus� b�t zam�eny MapSquary, lze volat jen z MainLoop threadu) (NULL=jednotku 
//		se nepoda�ilo vyrobit)
CSUnit *CSUnitType::CreateUnit ( CPointDW pointPosition, CZUnit *pCommander, 
	LPCTSTR lpszScriptName, BYTE nDirection, DWORD dwVerticalPosition ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// vytvo�� novou jednotku typu
	CSUnit *pUnit = new CSUnit;
	pUnit->PreCreate ( this );

	// inicializuje parametry jednotky
	pUnit->SetVerticalPosition ( ( dwVerticalPosition == 0 ) ? m_dwVerticalPosition : 
		dwVerticalPosition );

	// najde nejbli��� pozici um�st�n� jednotky
	CPointDW pointNearestPosition = g_cMap.GetNearestUnitPlace ( pUnit, pointPosition );

	// zjist�, je-li nalezeno m�sto pro jednotku na map�
	if ( !g_cMap.IsMapPosition ( pointNearestPosition ) )
	{	// m�sto pro jednotku na map� nebylo nalezeno
		// zni�� vytvo�enou jednotku
		pUnit->m_nLives = 0;
		pUnit->Delete ();
		delete pUnit;
		// vr�t� p��znak ne�sp�chu
		return NULL;
	}

	// um�st� jednotku na mapu
	VERIFY ( g_cMap.PlaceUnit ( pUnit, pointNearestPosition, TRUE ) );

	// vytvo�� jednotku na civilizaci
	pUnit->m_pZUnit = m_pCivilization->GetZCivilization ()->CreateUnit ( pUnit, pCommander, 
		lpszScriptName );
	ASSERT ( pUnit->m_pZUnit != NULL );

	// inicializuje parametry jednotky
	pUnit->SetDirection ( nDirection );
	// nastav� kontroln� body jednotky
	pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
	pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

	// zv��� po�et odkaz� na jednotku
	pUnit->AddRef ();
	// p�id� vytvo�enou jednotku do jednotek mapy
	g_cMap.AddUnit ( pUnit );

	// p�id� jednotku do seznamu jednotek typu jednotky
	AddUnit ( pUnit );

	// vr�t� ukazatel na jednotku
	return pUnit;
}

// p�id� jednotku "pUnit" do seznamu jednotek typu jednotky (typ jednotky mus� b�t 
//		zam�en pro z�pis)
void CSUnitType::AddUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// p�id� jednotku na za��tek seznamu jednotek
	pUnit->m_pNextUnit = m_pFirstUnit;
	m_pFirstUnit = pUnit->m_pNextUnit;
}

// odebere jednotku "pUnit" ze seznamu jednotek typu jednotky (typ jednotky mus� b�t 
//		zam�en pro z�pis) (nen�-li jednotka v seznamu jednotek, neprov�d� nic)
void CSUnitType::DeleteUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// ukazatel na odkaz na jednotku v seznamu
	CSUnit **ppUnit = &m_pFirstUnit;

	// projede seznam jednotek
	while ( *ppUnit != NULL )
	{
		// zjist�, jedn�-li se o hledanou jednotku
		if ( *ppUnit == pUnit )
		{	// jedn� se o hledanou jednotku
			// vypoj� jednotku ze seznamu jednotek typu jednotky
			*ppUnit = pUnit->m_pNextUnit;
			pUnit->m_pNextUnit = NULL;
			// ukon�� odeb�r�n� jednotky ze seznamu jednotek typu jednotky
			return;
		}

		// p�ejde na dal�� jednotku seznamu
		ppUnit = &(*ppUnit)->m_pNextUnit;
	}
}

//////////////////////////////////////////////////////////////////////
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje spr�vn� data objektu (TRUE=OK)
BOOL CSUnitType::CheckValidData () 
{
	ASSERT ( m_pCivilization != NULL );

	ASSERT ( m_dwSkillTypeCount > 0 );
	ASSERT ( m_aSkillTypeDescription != NULL );
	ASSERT ( m_dwSkillDataSize > 0 );

	ASSERT ( m_nLivesMax > 0 );
	ASSERT ( m_dwVerticalPosition != 0 );
	ASSERT ( !m_strUnitTypeName.IsEmpty () );

	return TRUE;
}

// zkontroluje ne�kodn� data objektu (TRUE=OK)
BOOL CSUnitType::CheckEmptyData () 
{
	ASSERT ( m_dwID == DWORD_MAX );
	ASSERT ( m_pCivilization == NULL );

	ASSERT ( m_dwSkillTypeCount == 0 );
	ASSERT ( m_aSkillTypeDescription == NULL );
	ASSERT ( m_dwSkillDataSize == 0 );

	ASSERT ( m_nLivesMax == 0 );
	ASSERT ( m_dwViewRadius == 0 );
	ASSERT ( m_dwVerticalPosition == 0 );
	ASSERT ( m_strUnitTypeName.IsEmpty () );

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		ASSERT ( m_aMoveWidth[nModeIndex] == 0 );
		ASSERT ( m_aAppearanceID[nModeIndex] == DWORD_MAX );
		ASSERT ( m_aInvisibilityState[nModeIndex] == 0 );
		ASSERT ( m_aInvisibilityDetection[nModeIndex] == 0 );
	}
	ASSERT ( m_dwFlags == 0 );

	return TRUE;
}

#endif //_DEBUG
