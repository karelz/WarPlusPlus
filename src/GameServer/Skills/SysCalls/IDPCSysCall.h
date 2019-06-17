/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman �enka & Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� DPC s n�vratovou hodnotou
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
// Dop�edn� deklarace

// t��da DPC se syt�mov�m vol�n�m
class CSSysCallDPC;
// t��da datov� polo�ky z�sobn�ku
class CIDataStackItem;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� DPC s n�vratovou hodnotou.
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
// Zak�zan� metody vytv��en� objektu

	// vytvo�en� syst�mov�ho vol�n� - zak�z�no
	bool Create ( CIProcess *pProcess ) { ASSERT ( false ); return false; };

protected:
// Operace se syst�mov�m vol�n�m

	// vytvo�en� syst�mov�ho vol�n� DPC "pSysCallDPC" s n�vratovou hodnotou 
	//		"pReturnValue" a s volaj�c�m "pProcess" (zvy�uje po�et odkaz� na objekt)
	bool Create ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
		CSSysCallDPC *pSysCallDPC );

public:
	// ukon�en� syst�mov�ho vol�n� ze strany volan�ho (syst�mov� vol�n� DPC mus� b�t 
	//		zam�eno) (ukazatel na DPC d�le nen� platn�)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatele na DPC a n�vratovou hodnotu d�le 
	//		nejsou platn�)
	virtual void Cancel ( CIProcess *pProcess );

public:
	// zamkne syst�mov� vol�n� DPC
	BOOL Lock () { return m_mutexLock.Lock (); };
	// odemkne syst�mov� vol�n� DPC
	BOOL Unlock () { return m_mutexLock.Unlock (); };

public:
// Informace o syst�mov�m vol�n�

	// vr�t� p��znak platnosti syst�mov�ho vol�n� DPC
	BOOL IsValid () { return ( m_pSysCallDPC != NULL ); };
	// vr�t� ukazatel na n�vratovou hodnotu syst�mov�ho vol�n� (syst�mov� vol�n� DPC 
	//		mus� b�t zam�eno a mus� b�t platn�)
	CIDataStackItem *GetReturnValue () 
		{ ASSERT ( m_pReturnValue != NULL ); return m_pReturnValue; };

protected:
// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ( CSSysCallDPC *pSysCallDPC );

private:
// Zak�zan� metody ukl�d�n� dat

	// inicializace nahran�ho objektu
	void PersistentInit () { ASSERT ( FALSE ); };

// Data
private:

	// ukazatel na n�vratov� parametr
	CIDataStackItem *m_pReturnValue;
	// ukazatel na DPC
	CSSysCallDPC *m_pSysCallDPC;
	// z�mek syst�mov�ho vol�n�
	CMutex m_mutexLock;
};

#endif //__INTERPRET_DPC_SYSCALL__HEADER_INCLUDED__
