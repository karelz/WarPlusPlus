/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktn� t��da odlo�en�ho vol�n� procedury
 * 
 ***********************************************************/

#ifndef __SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__
#define __SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__

#include "AbstractDataClasses\RunTimeIDCreation.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace t��d

// t��da jednotky
class CSUnit;
// t��da mapy
class CSMap;

//////////////////////////////////////////////////////////////////////
// Makra pro deklaraci a implementaci odlo�en�ch vol�n� procedur
//////////////////////////////////////////////////////////////////////

// deklarace odlo�en�ho vol�n� procedury (DPC) "DPCName" (jm�no t��dy)
#define DECLARE_DPC(DPCClassName) \
	public:\
		virtual DWORD GetID ();\
		DECLARE_RUN_TIME_ID_CREATION ( DPCClassName, CSDeferredProcedureCall )

// implementace odlo�en�ho vol�n� procedury (DPC) "DPCName" s ID "dwID"
#define IMPLEMENT_DPC(DPCClassName, dwID) \
	DWORD DPCClassName::GetID () { return DPCClassName::m_dwRTIDCID; }\
	IMPLEMENT_RUN_TIME_ID_CREATION ( DPCClassName, CSDeferredProcedureCall, dwID )

//////////////////////////////////////////////////////////////////////
// Abstraktn� t��da odlo�en�ho vol�n� procedury
class CSDeferredProcedureCall : public CPersistentObject 
{
	friend class CSMap;

	DECLARE_DYNAMIC ( CSDeferredProcedureCall )

	// deklarace t��dy, jej� potomky lze vytv��et za b�hu programu z ID potomka
	DECLARE_RUN_TIME_ID_STORAGE ( CSDeferredProcedureCall )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	inline CSDeferredProcedureCall ( CSUnit *pUnit = NULL );
	// destruktor
	virtual ~CSDeferredProcedureCall ();

private:
// Ukl�d�n� dat (CPersistentObjectPlain metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

protected:
// Ukl�d�n� dat potomk� (analogie CPersistentObjectPlain metod)

	// ukl�d�n� dat
	virtual void PersistentSaveDPC ( CPersistentStorage &storage ) = 0;
	// nahr�v�n� pouze ulo�en�ch dat
	virtual void PersistentLoadDPC ( CPersistentStorage &storage ) = 0;
	// p�eklad ukazatel�
	virtual void PersistentTranslatePointersDPC ( CPersistentStorage &storage ) = 0;
	// inicializace nahran�ho objektu
	virtual void PersistentInitDPC () = 0;

public:
// Operace s DPC

	// vr�t� ID odlo�en�ho vol�n� procedury (implementuje makro IMPLEMENT_DPC)
	virtual DWORD GetID () = 0;
protected:
	// vr�t� p��znak, m�-li se jednotka zamykat pro z�pis/�ten�
	virtual BOOL NeedWriterLock () { return TRUE; };

	// provede odlo�en� vol�n� procedury (star� se o zneplatn�n� v�ech odkaz� na DPC)
	virtual void Run () = 0;

public:
	// vr�t� ukazatel na asociovanou jednotku
	CSUnit *GetUnit () { return m_pUnit; };
protected:
	// vr�t� index civilizace pr�v� prov�d�n�ho DPC
	static inline DWORD GetRunningDPCCivilizationIndex ();

// Data
private:
	// asociovan� jednotka
	CSUnit *m_pUnit;
};

#endif //__SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__
