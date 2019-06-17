/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: V�t Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybov� skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__

#include "GameServer\GameServer\SSkillType.h"
#include "ISysCallMove.h"

/////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da cesty na serveru hry
class CSPath;

/////////////////////////////////////////////////////////////////////
// Typ pohybov� skilly (move skilla)
class CSMoveSkillType : public CSSkillType 
{
	friend class CISysCallMove;

	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSMoveSkillType )

// Datov� typy
public:
	// v�sledek metody Go a GoCloseTo
	enum EGoResult 
	{
		EGR_OK = 0,
		EGR_NoMapDestination = -1,
		EGR_PathNotExists = 1,
		EGR_PathBlocked = 2,
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSMoveSkillType ();
	// destruktor
	virtual ~CSMoveSkillType ();

protected:
// Inicializace a zni�en� dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		v�jimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// zni�� data typu skilly
	virtual void DeleteSkillType ();

// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)

	// ukl�d�n� dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// p�eklad ukazatel� potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu potomka
	virtual void PersistentInitSkillType ();

public:
// Get/Set metody serverov�ch informac� o typu skilly jednotky

	// vr�t� velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vr�t� masku skupin skill
	virtual DWORD GetGroupMask ();

protected:
// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je ji� zam�en� pro 
	//		z�pis, metoda m��e zamykat libovoln� po�et jin�ch jednotek pro z�pis/�ten� 
	//		(v�etn� jednotek ze sv�ho bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// zni�� skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	void ActivateSkill ( CSUnit *pUnit, CSPath *pPath, CISysCallMove *pSysCall, 
		CPointDW pointDestination, DWORD dwToleranceSquareLimit );
	// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	virtual void DeactivateSkill ( CSUnit *pUnit );

	// ukl�d�n� dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// nahr�v�n� pouze ulo�en�ch dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// p�eklad ukazatel� dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// inicializace nahran�ch dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit );

	// vypln� full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vyplni enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo );

public:
	// zjist�, m��e-li skillu volat nep��tel
	virtual BOOL CanBeCalledByEnemy () { return TRUE; };

private:
// Pomocn� metody skilly

	// ukon�� skillu s v�sledkem "eResult" (jednotka "pUnit" mus� b�t zam�ena pro z�pis)
	inline void Finish ( CSUnit *pUnit, enum EGoResult eResult );
	// vypln� kontroln� body jednotky (jednotka mus� b�t zam�ena pro z�pis) (FALSE=cesta je 
	//		pr�zdn�)
	BOOL FillCheckPoints ( CSUnit *pUnit );
	// p�iprav� data skilly na p�esun k prvn�mu kontroln�mu bodu jednotky (jednotka mus� 
	//		b�t zam�ena pro z�pis)
	void GoToFirstCheckPoint ( CSUnit *pUnit );

// Debuggovac� informace

#ifdef _DEBUG
	// zkontroluje ne�kodn� data skilly (TRUE=OK)
	BOOL CheckEmptySkillData ( CSUnit *pUnit );
#endif //_DEBUG

public:
// Interface vol�n� ze skript�

	// interface vol�n� ze skript�
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSMoveSkillType, 7 );
	DECLARE_SKILLCALL_METHOD2_WRITE_HARD ( SCI_Go, Int, int *, Int, int * );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT_EEC ( SCI_GetSpeed );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_GoPos, SystemObject, CISystemObject * );
	DECLARE_SKILLCALL_METHOD3_WRITE_HARD ( SCI_GoCloseTo, Int, int *, Int, int *, 
		Int, int * );
	DECLARE_SKILLCALL_METHOD2_WRITE_HARD ( SCI_GoCloseToPos, SystemObject, 
		CISystemObject *, Int, int * );

protected:
	// aktivuje skillu pro pohyb jednotky na pozici se sou�adnicemi "pX" a "pY" (jednotka 
	//		je zam�ena pro z�pis)
	ESyscallResult SCI_Go ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, 
		CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY );
	// vr�t� rychlost jednotky (jednotka je zam�ena pro �ten�)
	ESyscallResult SCI_GetSpeed ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn );
	// aktivuje skillu pro pohyb jednotky na pozici "pPosition" (jednotka je zam�ena 
	//		pro z�pis)
	ESyscallResult SCI_GoPos ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, 
		CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pPosition );
	// aktivuje skillu pro pohyb jednotky na pozici se sou�adnicemi "pX" a "pY", 
	//		s toleranc� "pTolerance" (jednotka je zam�ena pro z�pis)
	ESyscallResult SCI_GoCloseTo ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY, 
		int *pTolerance );
	// aktivuje skillu pro pohyb jednotky na pozici "pPosition" s toleranc� "pTolerance" 
	//		(jednotka je zam�ena pro z�pis)
	ESyscallResult SCI_GoCloseToPos ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
		CISystemObject *pPosition, int *pTolerance );

// Data
protected:

	// rychlost jednotky
	DWORD m_dwSpeed;
	// ID vzhledu jednotky p�i pohybu
	DWORD m_dwAppearanceID;
};

#endif //__SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__
