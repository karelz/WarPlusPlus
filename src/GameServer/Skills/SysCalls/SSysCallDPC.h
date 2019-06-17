/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Roman Ženka & Karel Zikmund
 * 
 *   Popis: Tøída DPC se systémovým voláním
 * 
 ***********************************************************/

#ifndef __SERVER_SYSCALL_DPC__HEADER_INCLUDED__
#define __SERVER_SYSCALL_DPC__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SDeferredProcedureCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída systémového volání DPC s návratovou hodnotou
class CIDPCSysCall;

// tøída procesu
class CIProcess;

// tøída datové položky na zásobníku
class CIDataStackItem;

//////////////////////////////////////////////////////////////////////
// Tøída DPC se systémovým voláním.
class CSSysCallDPC : public CSDeferredProcedureCall 
{
	friend class CIDPCSysCall;

	DECLARE_DYNAMIC ( CSSysCallDPC )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSSysCallDPC () { m_pDPCSysCall = NULL; };
	// konstruktor
	CSSysCallDPC ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
		CSUnit *pUnit = NULL );
	// destruktor
	virtual ~CSSysCallDPC (); 

protected:
// Ukládání dat potomkù (analogie CPersistentObjectPlain metod)

	// ukládání dat DPC
	virtual void PersistentSaveDPC ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat DPC
	virtual void PersistentLoadDPC ( CPersistentStorage &storage );
	// pøeklad ukazatelù DPC
	virtual void PersistentTranslatePointersDPC ( CPersistentStorage &storage );
	// inicializace nahraného DPC
	virtual void PersistentInitDPC ();

protected:
// Operace s DPC

	// provede odložené volání procedury (stará se o zneplatnìní všech odkazù na DPC)
	virtual void Run () = 0;

/* Dùležité zdìdìné metody:

	// vrátí pøíznak, má-li se jednotka zamykat pro zápis/ètení
	virtual BOOL NeedWriterLock () { return TRUE; };
*/

public:
	// vrátí ukazatel na systémové volání DPC s návratovou hodnotou
	CIDPCSysCall *GetDPCSysCall ();

// Data
protected:

	// ukazatel na systémové volání DPC s návratovou hodnotou
	CIDPCSysCall *m_pDPCSysCall;
};

#endif //__SERVER_SYSCALL_DPC__HEADER_INCLUDED__
