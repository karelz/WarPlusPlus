/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Roman �enka & Karel Zikmund
 * 
 *   Popis: T��da DPC se syst�mov�m vol�n�m
 * 
 ***********************************************************/

#ifndef __SERVER_SYSCALL_DPC__HEADER_INCLUDED__
#define __SERVER_SYSCALL_DPC__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SDeferredProcedureCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da syst�mov�ho vol�n� DPC s n�vratovou hodnotou
class CIDPCSysCall;

// t��da procesu
class CIProcess;

// t��da datov� polo�ky na z�sobn�ku
class CIDataStackItem;

//////////////////////////////////////////////////////////////////////
// T��da DPC se syst�mov�m vol�n�m.
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
// Ukl�d�n� dat potomk� (analogie CPersistentObjectPlain metod)

	// ukl�d�n� dat DPC
	virtual void PersistentSaveDPC ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat DPC
	virtual void PersistentLoadDPC ( CPersistentStorage &storage );
	// p�eklad ukazatel� DPC
	virtual void PersistentTranslatePointersDPC ( CPersistentStorage &storage );
	// inicializace nahran�ho DPC
	virtual void PersistentInitDPC ();

protected:
// Operace s DPC

	// provede odlo�en� vol�n� procedury (star� se o zneplatn�n� v�ech odkaz� na DPC)
	virtual void Run () = 0;

/* D�le�it� zd�d�n� metody:

	// vr�t� p��znak, m�-li se jednotka zamykat pro z�pis/�ten�
	virtual BOOL NeedWriterLock () { return TRUE; };
*/

public:
	// vr�t� ukazatel na syst�mov� vol�n� DPC s n�vratovou hodnotou
	CIDPCSysCall *GetDPCSysCall ();

// Data
protected:

	// ukazatel na syst�mov� vol�n� DPC s n�vratovou hodnotou
	CIDPCSysCall *m_pDPCSysCall;
};

#endif //__SERVER_SYSCALL_DPC__HEADER_INCLUDED__
