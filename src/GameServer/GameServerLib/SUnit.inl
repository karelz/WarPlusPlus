/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da jednotky
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

// zaznamen� uschov�n� odkazu na jednotku
inline void CSUnit::AddRef () 
{
	// zv��� po�et odkaz� na jednotku
	DWORD dwReferenceCounter = ::InterlockedIncrement ( (long *)&m_dwReferenceCounter );
	ASSERT ( dwReferenceCounter != 0 );
}

//////////////////////////////////////////////////////////////////////
// Info struktury jednotky
//////////////////////////////////////////////////////////////////////

// vypln� brief info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
inline void CSUnit::GetBriefInfo ( struct SUnitBriefInfo *pBriefInfo ) 
{
	ASSERT ( m_dwAppearanceID != 0 );

	// vypln� brief info jednotky
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

// vypln� check point info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena 
//		pro �ten�)
inline void CSUnit::GetCheckPointInfo ( struct SUnitCheckPointInfo *pCheckPointInfo ) 
{
	// nech� vyplnit brief info jednotky
	GetBriefInfo ( pCheckPointInfo );
	// vypln� check point info jednotky
	pCheckPointInfo->dwFirstPositionX = m_cFirstCheckPoint.GetPosition ().x;
	pCheckPointInfo->dwFirstPositionY = m_cFirstCheckPoint.GetPosition ().y;
	pCheckPointInfo->dwFirstTime = m_cFirstCheckPoint.GetTime ();
	pCheckPointInfo->dwSecondPositionX = m_cSecondCheckPoint.GetPosition ().x;
	pCheckPointInfo->dwSecondPositionY = m_cSecondCheckPoint.GetPosition ().y;
	pCheckPointInfo->dwSecondTime = m_cSecondCheckPoint.GetTime ();
}

// vypln� start info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
inline void CSUnit::GetStartInfo ( struct SUnitStartInfo *pStartInfo ) 
{
	ASSERT ( m_pUnitType != NULL );

	// nech� vyplnit check point info jednotky
	GetCheckPointInfo ( pStartInfo );
	// vypln� start info jednotky
	pStartInfo->dwUnitTypeID = m_pUnitType->GetID ();
	pStartInfo->dwCivilizationID = m_pUnitType->GetCivilization ()->GetCivilizationID ();
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody serverov�ch informac� o jednotce
//////////////////////////////////////////////////////////////////////

// vr�t� ukazatel na civilizaci
inline CSCivilization *CSUnit::GetCivilization () 
{
	// vr�t� ukazatel na civilizaci
	return g_cMap.GetCivilization ( m_dwCivilizationIndex );
};

//////////////////////////////////////////////////////////////////////
// Get/Set metody hern�ch informac� o jednotce
//////////////////////////////////////////////////////////////////////

// sn�� po�et �ivot� jednotky o "nLivesDecrement" (lze volat jen z MainLoop threadu, 
//		jednotka mus� b�t zam�ena pro z�pis) (TRUE=jednotka je mrtv�)
inline BOOL CSUnit::DecreaseLives ( int nLivesDecrement ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// sn�� po�et �ivot�
	m_nLives -= nLivesDecrement;

	// nastav� p��znak zm�ny brief infa
	SetBriefInfoModified ();

	// zjist�, je-li jednotka mrtv�
	if ( m_nLives <= 0 )
	{	// jednotka je mrtv�
		// zjist�, jedn�-li se o smrt jednotky
		if ( m_nLives + nLivesDecrement > 0 )
		{	// jedn� se o smrt jednotky
			// obslou�� smrt jednotky
			UnitDied ();
		}
		// **************************************************
		m_nLives = DEAD_UNIT_LIVES;
		// vr�t� p��znak mrtv� jednotky
		return TRUE;
	}

	// jednotka je �iv�
	return FALSE;
}

// vr�t� MapSquare um�st�n� jednotky na map� (mimo MainLoop thread mus� b�t jednotka 
//		zam�ena pro �ten�) (NULL=jednotka nen� na map�)
inline CSMapSquare *CSUnit::GetMapSquare () 
{
	// zjist�, je-li jednotka na map�
	if ( g_cMap.IsMapPosition ( m_pointPosition ) )
	{	// jednotka je na map�
		return g_cMap.GetMapSquareFromPosition ( m_pointPosition );
	}
	else
	{	// jednotka je mimo mapu
		return NULL;
	}
}

// vypln� naposledy vid�nou pozici "cPosition" jednotky civilizac� "dwCivilizationIndex" 
//		(mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
inline void CSUnit::GetLastSeenPosition ( DWORD dwCivilizationIndex, 
	CSPosition &cPosition ) 
{
	ASSERT ( dwCivilizationIndex < g_cMap.GetCivilizationCount () );

	// zjist�, je-li jednotka civilizac� vid�na
	if ( m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
	{	// jednotka je civilizac� vid�na
		// aktualizuje naposledy vid�nou pozici jednotky
		cPosition.x = m_pointPosition.x;
		cPosition.y = m_pointPosition.y;
		cPosition.z = m_dwVerticalPosition;
	}
	else
	{	// jednotka nen� civilizac� vid�t
		// aktualizuje naposledy vid�nou pozici jednotky
		cPosition = m_aCivilizationLastSeenPosition[dwCivilizationIndex];
	}
}

// nastav� nasm�rov�n� jednotky (vol� SetBriefInfoModified, je-li pot�eba)
inline void CSUnit::SetDirection ( BYTE nDirection ) 
{
	ASSERT ( nDirection <= Dir_Random );

	// zjist�, zm�nil-li se sm�r pohybu
	if ( nDirection != m_nDirection )
	{	// zm�nil se sm�r pohybu
		// zm�n� sm�r pohybu
		m_nDirection = nDirection;
		// nastav� p��znak zm�ny brief infa
		SetBriefInfoModified ();
	}
};

// nastav� nasm�rov�n� jednotky na pozici "pointPosition" (vol� SetBriefInfoModified, 
//		je-li pot�eba)
inline void CSUnit::SetDirection ( CPointDW pointPosition ) 
{
	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( m_pointPosition ) );

	// zjist� vzd�lenost prvn�ho kontroln�ho bodu od pozice jednotky
	int nXDifference = pointPosition.x - m_pointPosition.x;
	int nYDifference = pointPosition.y - m_pointPosition.y;

	// aktualizuje sm�r pohybu jednotky
	SetDirectionFromDifference ( nXDifference, nYDifference );
}

// nastav� nasm�rov�n� jednotky sm�rem o "nXDifference" a "nYDifference" (vol� 
//		SetBriefInfoModified, je-li pot�eba)
inline void CSUnit::SetDirectionFromDifference ( int nXDifference, int nYDifference ) 
{
	// aktualizuje sm�r pohybu jednotky
	do
	{
		// sm�r pohybu jednotky
		BYTE nDirection;

		// zjist�, pohybuje-li se jednotka ve sm�ru osy X
		if ( nXDifference != 0 )
		{	// jednotka se pohybuje ve sm�ru osy X
			// ukazatel na tabulku sm�r�
			BYTE *pDirectionTable;

			// zjist� nasm�rov�n� jednotky
			if ( nXDifference > 0 )
			{	// kladn� polorovina osy X
				// nastav� tabulku sm�r�
				pDirectionTable = m_aDirectionTableXPositive;
			}
			else
			{	// z�porn� polorovina osy X
				// nastav� tabulku sm�r�
				pDirectionTable = m_aDirectionTableXNegative;
			}

			// sm�r pohybu
			double dbDirection = (double)nYDifference / (double)nXDifference;
			// index sm�ru pohybu
			int nDirectionIndex;

			// zjist� index sm�ru pohybu
			if ( dbDirection >= 0.5 )
			{	// <1/2, nekone�no>
				if ( dbDirection >= 2.0 )
				{	// <2, nekone�no>
					nDirectionIndex = 0;
				}
				else
				{	// <1/2, 2)
					nDirectionIndex = 1;
				}
			}
			else
			{	// <-nekone�no, 1/2)
				if ( dbDirection >= -0.5 )
				{	// <-1/2, 1/2)
					nDirectionIndex = 2;
				}
				else
				{	// <-nekone�no, -1/2)
					if ( dbDirection >= -2.0 )
					{	// <-2, -1/2)
						nDirectionIndex = 3;
					}
					else
					{	// <-nekone�no, -2)
						nDirectionIndex = 4;
					}
				}
			}

			// zjist� sm�r pohybu z indexu sm�ru pohybu
			nDirection = pDirectionTable[nDirectionIndex];
		}
		else
		{	// jednotka se nepohybuje ve sm�ru osy X
			// zjist� sm�r pohybu jednotky v ose Y
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
				// ukon�� aktualizaci sm�ru pohybu jednotky
				break;
			}
		}

		// zjist�, zm�nil-li se sm�r pohybu
		if ( nDirection != m_nDirection )
		{	// zm�nil se sm�r pohybu
			// zm�n� sm�r pohybu
			m_nDirection = nDirection;
			// nastav� p��znak zm�ny brief infa
			SetBriefInfoModified ();
		}
	} while ( 0 );
	// sm�r pohybu jednotky byl aktualizov�n
}

// nastav� ID vzhledu jednotky (0=defaultn� vzhled jednotky) (vol� SetBriefInfoModified, 
//		je-li pot�eba)
inline void CSUnit::SetAppearanceID ( DWORD dwAppearanceID ) 
{
	ASSERT ( m_pUnitType != NULL );

	// zjist�, jedn�-li se o defaultn� vzhled jednotky
	if ( dwAppearanceID == 0 )
	{	// jedn� se o defaultn� vzhled jednotky
		// nastav� defaultn� vzhled jednotky
		dwAppearanceID = m_pUnitType->m_aAppearanceID[GetMode ()];
	}

	// zjist�, m�-li se zm�nit ID vzhledu jednotky
	if ( dwAppearanceID != m_dwAppearanceID )
	{	// m� se zm�nit ID vzhledu jednotky
		// zm�n� ID vzhledu jednotky
		m_dwAppearanceID = dwAppearanceID;
		// nastav� p��znak zm�ny brief infa
		SetBriefInfoModified ();
	}
}

// nastav� ID defaultn�ho vzhledu jednotky pro aktu�ln� m�d jednotky (vol� 
//		SetBriefInfoModified, je-li pot�eba)
inline void CSUnit::SetDefaultAppearanceID () 
{
	ASSERT ( m_pUnitType != NULL );

	// zjist� ID defaultn�ho vzhledu jednotky v aktu�ln�m modu jednotky
	DWORD dwAppearanceID = m_pUnitType->m_aAppearanceID[GetMode ()];
	ASSERT ( dwAppearanceID != 0 );

	// zjist�, m�-li se zm�nit ID vzhledu jednotky
	if ( dwAppearanceID != m_dwAppearanceID )
	{	// m� se zm�nit ID vzhledu jednotky
		// zm�n� ID vzhledu jednotky
		m_dwAppearanceID = dwAppearanceID;
		// nastav� p��znak zm�ny brief infa
		SetBriefInfoModified ();
	}
};

#endif //__SERVER_UNIT__INLINE_INCLUDED__
