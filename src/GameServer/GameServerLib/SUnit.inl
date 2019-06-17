/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT__INLINE_INCLUDED__
#define __SERVER_UNIT__INLINE_INCLUDED__

#include "SMap.h"
#include "SCivilization.h"

#include "Common\ServerClient\UnitInfoStructures.h"

//////////////////////////////////////////////////////////////////////
// Odkazy na instanci jednotky
//////////////////////////////////////////////////////////////////////

// zaznamená uschování odkazu na jednotku
inline void CSUnit::AddRef () 
{
	// zvýší poèet odkazù na jednotku
	DWORD dwReferenceCounter = ::InterlockedIncrement ( (long *)&m_dwReferenceCounter );
	ASSERT ( dwReferenceCounter != 0 );
}

//////////////////////////////////////////////////////////////////////
// Info struktury jednotky
//////////////////////////////////////////////////////////////////////

// vyplní brief info jednotky (mimo MainLoop thread musí být jednotka zamèena pro ètení)
inline void CSUnit::GetBriefInfo ( struct SUnitBriefInfo *pBriefInfo ) 
{
	ASSERT ( m_dwAppearanceID != 0 );

	// vyplní brief info jednotky
	pBriefInfo->dwID = m_dwID;
	pBriefInfo->dwPositionX = m_pointPosition.x;
	pBriefInfo->dwPositionY = m_pointPosition.y;
	pBriefInfo->dwVerticalPosition = m_dwVerticalPosition;
	pBriefInfo->dwViewRadius = m_dwViewRadius;
	pBriefInfo->nLives = m_nLives;
	pBriefInfo->nLivesMax = m_nLivesMax;
	pBriefInfo->nDirection = m_nDirection;
	pBriefInfo->dwAppearanceID = m_dwAppearanceID;
	pBriefInfo->m_nMode = GetMode ();
}

// vyplní check point info jednotky (mimo MainLoop thread musí být jednotka zamèena 
//		pro ètení)
inline void CSUnit::GetCheckPointInfo ( struct SUnitCheckPointInfo *pCheckPointInfo ) 
{
	// nechá vyplnit brief info jednotky
	GetBriefInfo ( pCheckPointInfo );
	// vyplní check point info jednotky
	pCheckPointInfo->dwFirstPositionX = m_cFirstCheckPoint.GetPosition ().x;
	pCheckPointInfo->dwFirstPositionY = m_cFirstCheckPoint.GetPosition ().y;
	pCheckPointInfo->dwFirstTime = m_cFirstCheckPoint.GetTime ();
	pCheckPointInfo->dwSecondPositionX = m_cSecondCheckPoint.GetPosition ().x;
	pCheckPointInfo->dwSecondPositionY = m_cSecondCheckPoint.GetPosition ().y;
	pCheckPointInfo->dwSecondTime = m_cSecondCheckPoint.GetTime ();
}

// vyplní start info jednotky (mimo MainLoop thread musí být jednotka zamèena pro ètení)
inline void CSUnit::GetStartInfo ( struct SUnitStartInfo *pStartInfo ) 
{
	ASSERT ( m_pUnitType != NULL );

	// nechá vyplnit check point info jednotky
	GetCheckPointInfo ( pStartInfo );
	// vyplní start info jednotky
	pStartInfo->dwUnitTypeID = m_pUnitType->GetID ();
	pStartInfo->dwCivilizationID = m_pUnitType->GetCivilization ()->GetCivilizationID ();
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o jednotce
//////////////////////////////////////////////////////////////////////

// vrátí ukazatel na civilizaci
inline CSCivilization *CSUnit::GetCivilization () 
{
	// vrátí ukazatel na civilizaci
	return g_cMap.GetCivilization ( m_dwCivilizationIndex );
};

//////////////////////////////////////////////////////////////////////
// Get/Set metody herních informací o jednotce
//////////////////////////////////////////////////////////////////////

// sníží poèet životù jednotky o "nLivesDecrement" (lze volat jen z MainLoop threadu, 
//		jednotka musí být zamèena pro zápis) (TRUE=jednotka je mrtvá)
inline BOOL CSUnit::DecreaseLives ( int nLivesDecrement ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// sníží poèet životù
	m_nLives -= nLivesDecrement;

	// nastaví pøíznak zmìny brief infa
	SetBriefInfoModified ();

	// zjistí, je-li jednotka mrtvá
	if ( m_nLives <= 0 )
	{	// jednotka je mrtvá
		// zjistí, jedná-li se o smrt jednotky
		if ( m_nLives + nLivesDecrement > 0 )
		{	// jedná se o smrt jednotky
			// obslouží smrt jednotky
			UnitDied ();
		}
		// **************************************************
		m_nLives = DEAD_UNIT_LIVES;
		// vrátí pøíznak mrtvé jednotky
		return TRUE;
	}

	// jednotka je živá
	return FALSE;
}

// vrátí MapSquare umístìní jednotky na mapì (mimo MainLoop thread musí být jednotka 
//		zamèena pro ètení) (NULL=jednotka není na mapì)
inline CSMapSquare *CSUnit::GetMapSquare () 
{
	// zjistí, je-li jednotka na mapì
	if ( g_cMap.IsMapPosition ( m_pointPosition ) )
	{	// jednotka je na mapì
		return g_cMap.GetMapSquareFromPosition ( m_pointPosition );
	}
	else
	{	// jednotka je mimo mapu
		return NULL;
	}
}

// vyplní naposledy vidìnou pozici "cPosition" jednotky civilizací "dwCivilizationIndex" 
//		(mimo MainLoop thread musí být jednotka zamèena pro ètení)
inline void CSUnit::GetLastSeenPosition ( DWORD dwCivilizationIndex, 
	CSPosition &cPosition ) 
{
	ASSERT ( dwCivilizationIndex < g_cMap.GetCivilizationCount () );

	// zjistí, je-li jednotka civilizací vidìna
	if ( m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
	{	// jednotka je civilizací vidìna
		// aktualizuje naposledy vidìnou pozici jednotky
		cPosition.x = m_pointPosition.x;
		cPosition.y = m_pointPosition.y;
		cPosition.z = m_dwVerticalPosition;
	}
	else
	{	// jednotka není civilizací vidìt
		// aktualizuje naposledy vidìnou pozici jednotky
		cPosition = m_aCivilizationLastSeenPosition[dwCivilizationIndex];
	}
}

// nastaví nasmìrování jednotky (volá SetBriefInfoModified, je-li potøeba)
inline void CSUnit::SetDirection ( BYTE nDirection ) 
{
	ASSERT ( nDirection <= Dir_Random );

	// zjistí, zmìnil-li se smìr pohybu
	if ( nDirection != m_nDirection )
	{	// zmìnil se smìr pohybu
		// zmìní smìr pohybu
		m_nDirection = nDirection;
		// nastaví pøíznak zmìny brief infa
		SetBriefInfoModified ();
	}
};

// nastaví nasmìrování jednotky na pozici "pointPosition" (volá SetBriefInfoModified, 
//		je-li potøeba)
inline void CSUnit::SetDirection ( CPointDW pointPosition ) 
{
	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( m_pointPosition ) );

	// zjistí vzdálenost prvního kontrolního bodu od pozice jednotky
	int nXDifference = pointPosition.x - m_pointPosition.x;
	int nYDifference = pointPosition.y - m_pointPosition.y;

	// aktualizuje smìr pohybu jednotky
	SetDirectionFromDifference ( nXDifference, nYDifference );
}

// nastaví nasmìrování jednotky smìrem o "nXDifference" a "nYDifference" (volá 
//		SetBriefInfoModified, je-li potøeba)
inline void CSUnit::SetDirectionFromDifference ( int nXDifference, int nYDifference ) 
{
	// aktualizuje smìr pohybu jednotky
	do
	{
		// smìr pohybu jednotky
		BYTE nDirection;

		// zjistí, pohybuje-li se jednotka ve smìru osy X
		if ( nXDifference != 0 )
		{	// jednotka se pohybuje ve smìru osy X
			// ukazatel na tabulku smìrù
			BYTE *pDirectionTable;

			// zjistí nasmìrování jednotky
			if ( nXDifference > 0 )
			{	// kladná polorovina osy X
				// nastaví tabulku smìrù
				pDirectionTable = m_aDirectionTableXPositive;
			}
			else
			{	// záporná polorovina osy X
				// nastaví tabulku smìrù
				pDirectionTable = m_aDirectionTableXNegative;
			}

			// smìr pohybu
			double dbDirection = (double)nYDifference / (double)nXDifference;
			// index smìru pohybu
			int nDirectionIndex;

			// zjistí index smìru pohybu
			if ( dbDirection >= 0.5 )
			{	// <1/2, nekoneèno>
				if ( dbDirection >= 2.0 )
				{	// <2, nekoneèno>
					nDirectionIndex = 0;
				}
				else
				{	// <1/2, 2)
					nDirectionIndex = 1;
				}
			}
			else
			{	// <-nekoneèno, 1/2)
				if ( dbDirection >= -0.5 )
				{	// <-1/2, 1/2)
					nDirectionIndex = 2;
				}
				else
				{	// <-nekoneèno, -1/2)
					if ( dbDirection >= -2.0 )
					{	// <-2, -1/2)
						nDirectionIndex = 3;
					}
					else
					{	// <-nekoneèno, -2)
						nDirectionIndex = 4;
					}
				}
			}

			// zjistí smìr pohybu z indexu smìru pohybu
			nDirection = pDirectionTable[nDirectionIndex];
		}
		else
		{	// jednotka se nepohybuje ve smìru osy X
			// zjistí smìr pohybu jednotky v ose Y
 			if ( nYDifference < 0 )
			{	// jednotka se pohybuje k severu
				nDirection = Dir_North;
			}
			else if ( nYDifference > 0 )
			{	// jednotka se pohybuje k jihu
				nDirection = Dir_South;
			}
			else
			{	// jednotka se nepohybuje
				// ukonèí aktualizaci smìru pohybu jednotky
				break;
			}
		}

		// zjistí, zmìnil-li se smìr pohybu
		if ( nDirection != m_nDirection )
		{	// zmìnil se smìr pohybu
			// zmìní smìr pohybu
			m_nDirection = nDirection;
			// nastaví pøíznak zmìny brief infa
			SetBriefInfoModified ();
		}
	} while ( 0 );
	// smìr pohybu jednotky byl aktualizován
}

// nastaví ID vzhledu jednotky (0=defaultní vzhled jednotky) (volá SetBriefInfoModified, 
//		je-li potøeba)
inline void CSUnit::SetAppearanceID ( DWORD dwAppearanceID ) 
{
	ASSERT ( m_pUnitType != NULL );

	// zjistí, jedná-li se o defaultní vzhled jednotky
	if ( dwAppearanceID == 0 )
	{	// jedná se o defaultní vzhled jednotky
		// nastaví defaultní vzhled jednotky
		dwAppearanceID = m_pUnitType->m_aAppearanceID[GetMode ()];
	}

	// zjistí, má-li se zmìnit ID vzhledu jednotky
	if ( dwAppearanceID != m_dwAppearanceID )
	{	// má se zmìnit ID vzhledu jednotky
		// zmìní ID vzhledu jednotky
		m_dwAppearanceID = dwAppearanceID;
		// nastaví pøíznak zmìny brief infa
		SetBriefInfoModified ();
	}
}

// nastaví ID defaultního vzhledu jednotky pro aktuální mód jednotky (volá 
//		SetBriefInfoModified, je-li potøeba)
inline void CSUnit::SetDefaultAppearanceID () 
{
	ASSERT ( m_pUnitType != NULL );

	// zjistí ID defaultního vzhledu jednotky v aktuálním modu jednotky
	DWORD dwAppearanceID = m_pUnitType->m_aAppearanceID[GetMode ()];
	ASSERT ( dwAppearanceID != 0 );

	// zjistí, má-li se zmìnit ID vzhledu jednotky
	if ( dwAppearanceID != m_dwAppearanceID )
	{	// má se zmìnit ID vzhledu jednotky
		// zmìní ID vzhledu jednotky
		m_dwAppearanceID = dwAppearanceID;
		// nastaví pøíznak zmìny brief infa
		SetBriefInfoModified ();
	}
};

#endif //__SERVER_UNIT__INLINE_INCLUDED__
