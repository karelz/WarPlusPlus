/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Roman Zenka
 * 
 *   Popis: Tøída odloženého volání procedury GetUnitsInArea 
 *          na systémovém objektu mapy
 * 
 ***********************************************************/

#ifndef __SERVER_SO_MAP_GetUnitsInArea_DPC__HEADER_INCLUDED__
#define __SERVER_SO_MAP_GetUnitsInArea_DPC__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\SSysCallDPC.h"

#include "GameServer\GameServer\GeneralClasses\GeneralClasses.h"

//////////////////////////////////////////////////////////////////////
// Tøída odloženého volání procedury GetUnitsInArea na systémovém objektu mapy
class CSMapGetUnitsInAreaDPC : public CSSysCallDPC 
{
	DECLARE_DYNAMIC ( CSMapGetUnitsInAreaDPC )
	DECLARE_DPC ( CSMapGetUnitsInAreaDPC )

	// Ktere jednotky vlastne chceme ziskat?
	enum ESelector
	{
		SelectorNotInitialized = -1,	// Neinicializovany selektor
		AllUnitsInArea = 0,				// Vsechny jednotky
		MyUnitsInArea = 1,				// Moje jednotky
		EnemyUnitsInArea = 2,			// Nepratelske jednotky
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSMapGetUnitsInAreaDPC ();
public:
	// konstruktor
	CSMapGetUnitsInAreaDPC ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
		CPointDW pointCenter, DWORD dwRadius, enum ESelector eSelector );
protected:
	// destruktor
	virtual ~CSMapGetUnitsInAreaDPC ();

// Ukládání dat potomkù (analogie CPersistentObjectPlain metod)
protected:

	// ukládání dat
	virtual void PersistentSaveDPC ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	virtual void PersistentLoadDPC ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	virtual void PersistentTranslatePointersDPC ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	virtual void PersistentInitDPC ();

// Operace s DPC

	// provede odložené volání procedury (stará se o zneplatnìní všech odkazù na DPC)
	virtual void Run ();

// Data pro DPC
protected:
	// souradnice stredu oblasti
	CPointDW m_pointCenter;
	// polomer oblasti
	DWORD m_dwRadius;
	// ktere jednotky vlastne chceme?
	enum ESelector m_eSelector;

// Debugovaci metody
public:
#ifdef _DEBUG
	void AssertValid();
#endif  // _DEBUG
};

#endif //__SERVER_SO_MAP_GetUnitsInArea_DPC__HEADER_INCLUDED__
