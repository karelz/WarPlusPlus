/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Roman Ženka & Karel Zikmund
 * 
 *   Popis: Tøída systémového volání DPC s návratovou hodnotou
 * 
 ***********************************************************/

#ifndef __INTERPRET_DPC_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_DPC_SYSCALL__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

#include "TripleS\Interpret\Src\ISyscall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída DPC se sytémovým voláním
class CSSysCallDPC;
// tøída datové položky zásobníku
class CIDataStackItem;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání DPC s návratovou hodnotou.
class CIDPCSysCall : public CISyscall 
{
	friend class CSSysCallDPC;

	DECLARE_DYNAMIC ( CIDPCSysCall )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CIDPCSysCall () { m_pReturnValue = NULL; m_pSysCallDPC = NULL; };
	// destruktor
	virtual ~CIDPCSysCall () {};

private:
// Zakázané metody vytváøení objektu

	// vytvoøení systémového volání - zakázáno
	bool Create ( CIProcess *pProcess ) { ASSERT ( false ); return false; };

protected:
// Operace se systémovým voláním

	// vytvoøení systémového volání DPC "pSysCallDPC" s návratovou hodnotou 
	//		"pReturnValue" a s volajícím "pProcess" (zvyšuje poèet odkazù na objekt)
	bool Create ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
		CSSysCallDPC *pSysCallDPC );

public:
	// ukonèení systémového volání ze strany volaného (systémové volání DPC musí být 
	//		zamèeno) (ukazatel na DPC dále není platný)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
protected:
	// zrušení systémového volání volajícím (ukazatele na DPC a návratovou hodnotu dále 
	//		nejsou platné)
	virtual void Cancel ( CIProcess *pProcess );

public:
	// zamkne systémové volání DPC
	BOOL Lock () { return m_mutexLock.Lock (); };
	// odemkne systémové volání DPC
	BOOL Unlock () { return m_mutexLock.Unlock (); };

public:
// Informace o systémovém volání

	// vrátí pøíznak platnosti systémového volání DPC
	BOOL IsValid () { return ( m_pSysCallDPC != NULL ); };
	// vrátí ukazatel na návratovou hodnotu systémového volání (systémové volání DPC 
	//		musí být zamèeno a musí být platné)
	CIDataStackItem *GetReturnValue () 
		{ ASSERT ( m_pReturnValue != NULL ); return m_pReturnValue; };

protected:
// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ( CSSysCallDPC *pSysCallDPC );

private:
// Zakázané metody ukládání dat

	// inicializace nahraného objektu
	void PersistentInit () { ASSERT ( FALSE ); };

// Data
private:

	// ukazatel na návratový parametr
	CIDataStackItem *m_pReturnValue;
	// ukazatel na DPC
	CSSysCallDPC *m_pSysCallDPC;
	// zámek systémového volání
	CMutex m_mutexLock;
};

#endif //__INTERPRET_DPC_SYSCALL__HEADER_INCLUDED__
