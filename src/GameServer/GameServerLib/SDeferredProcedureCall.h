/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktní tøída odloženého volání procedury
 * 
 ***********************************************************/

#ifndef __SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__
#define __SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__

#include "AbstractDataClasses\RunTimeIDCreation.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída jednotky
class CSUnit;
// tøída mapy
class CSMap;

//////////////////////////////////////////////////////////////////////
// Makra pro deklaraci a implementaci odložených volání procedur
//////////////////////////////////////////////////////////////////////

// deklarace odloženého volání procedury (DPC) "DPCName" (jméno tøídy)
#define DECLARE_DPC(DPCClassName) \
	public:\
		virtual DWORD GetID ();\
		DECLARE_RUN_TIME_ID_CREATION ( DPCClassName, CSDeferredProcedureCall )

// implementace odloženého volání procedury (DPC) "DPCName" s ID "dwID"
#define IMPLEMENT_DPC(DPCClassName, dwID) \
	DWORD DPCClassName::GetID () { return DPCClassName::m_dwRTIDCID; }\
	IMPLEMENT_RUN_TIME_ID_CREATION ( DPCClassName, CSDeferredProcedureCall, dwID )

//////////////////////////////////////////////////////////////////////
// Abstraktní tøída odloženého volání procedury
class CSDeferredProcedureCall : public CPersistentObject 
{
	friend class CSMap;

	DECLARE_DYNAMIC ( CSDeferredProcedureCall )

	// deklarace tøídy, jejíž potomky lze vytváøet za bìhu programu z ID potomka
	DECLARE_RUN_TIME_ID_STORAGE ( CSDeferredProcedureCall )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	inline CSDeferredProcedureCall ( CSUnit *pUnit = NULL );
	// destruktor
	virtual ~CSDeferredProcedureCall ();

private:
// Ukládání dat (CPersistentObjectPlain metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

protected:
// Ukládání dat potomkù (analogie CPersistentObjectPlain metod)

	// ukládání dat
	virtual void PersistentSaveDPC ( CPersistentStorage &storage ) = 0;
	// nahrávání pouze uložených dat
	virtual void PersistentLoadDPC ( CPersistentStorage &storage ) = 0;
	// pøeklad ukazatelù
	virtual void PersistentTranslatePointersDPC ( CPersistentStorage &storage ) = 0;
	// inicializace nahraného objektu
	virtual void PersistentInitDPC () = 0;

public:
// Operace s DPC

	// vrátí ID odloženého volání procedury (implementuje makro IMPLEMENT_DPC)
	virtual DWORD GetID () = 0;
protected:
	// vrátí pøíznak, má-li se jednotka zamykat pro zápis/ètení
	virtual BOOL NeedWriterLock () { return TRUE; };

	// provede odložené volání procedury (stará se o zneplatnìní všech odkazù na DPC)
	virtual void Run () = 0;

public:
	// vrátí ukazatel na asociovanou jednotku
	CSUnit *GetUnit () { return m_pUnit; };
protected:
	// vrátí index civilizace právì provádìného DPC
	static inline DWORD GetRunningDPCCivilizationIndex ();

// Data
private:
	// asociovaná jednotka
	CSUnit *m_pUnit;
};

#endif //__SERVER_DEFERRED_PROCEDURE_CALL__HEADER_INCLUDED__
