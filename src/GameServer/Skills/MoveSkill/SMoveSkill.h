/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: V�t Karas & Karel Zikmund
 * 
 *   Popis: Pohybov� skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL__HEADER_INCLUDED__
#define __SERVER_MOVE_SKILL__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace t��d

// t��da cesty na map�
class CSPath;
// t��da syst�mov�ho vol�n�
class CISysCallMove;

/////////////////////////////////////////////////////////////////////
// Pohybov� skilla
struct SSMoveSkill 
{
	// rychlost jednotky (v MapCellech za 100 TimeSlic�)
	DWORD m_dwSpeed;

	// p�esn� pozice jednotky b�hem pohybu
	double m_dbXPosition;
	double m_dbYPosition;

	// p��r�stek pozice jednotky za jeden TimeSlice
	double m_dbXStep;
	double m_dbYStep;

	// doba �ek�n� (v TimeSlicech) p�ed nov�m pokusem pohnout jednotkou
	DWORD m_dwWaitCount;

	// cesta pohybu
	CSPath *m_pPath;

	// �ekaj�c� syst�mov� vol�n�
	CISysCallMove *m_pSysCall;

	// c�lov� pozice
	CPointDW m_pointDestination;
	// limit �tverce tolerance vzd�lenosti (�tverec tolerance vzd�lenosti zv�t�en� o 1)
	DWORD m_dwToleranceSquareLimit;

#ifdef _DEBUG
	// v�choz� pozice
	CPointDW m_pointStart;
#endif //_DEBUG
};

#endif //__SERVER_MOVE_SKILL__HEADER_INCLUDED__
