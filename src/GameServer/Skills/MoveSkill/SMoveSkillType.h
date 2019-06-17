/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Vít Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybové skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__

#include "GameServer\GameServer\SSkillType.h"
#include "ISysCallMove.h"

/////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída cesty na serveru hry
class CSPath;

/////////////////////////////////////////////////////////////////////
// Typ pohybové skilly (move skilla)
class CSMoveSkillType : public CSSkillType 
{
	friend class CISysCallMove;

	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSMoveSkillType )

// Datové typy
public:
	// výsledek metody Go a GoCloseTo
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
// Inicializace a znièení dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		výjimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// znièí data typu skilly
	virtual void DeleteSkillType ();

// Ukládání dat potomkù (analogie CPersistentObject metod)

	// ukládání dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// pøeklad ukazatelù potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahraného objektu potomka
	virtual void PersistentInitSkillType ();

public:
// Get/Set metody serverových informací o typu skilly jednotky

	// vrátí velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vrátí masku skupin skill
	virtual DWORD GetGroupMask ();

protected:
// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je již zamèená pro 
	//		zápis, metoda mùže zamykat libovolný poèet jiných jednotek pro zápis/ètení 
	//		(vèetnì jednotek ze svého bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// znièí skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	void ActivateSkill ( CSUnit *pUnit, CSPath *pPath, CISysCallMove *pSysCall, 
		CPointDW pointDestination, DWORD dwToleranceSquareLimit );
	// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	virtual void DeactivateSkill ( CSUnit *pUnit );

	// ukládání dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// nahrávání pouze uložených dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// pøeklad ukazatelù dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// inicializace nahraných dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit );

	// vyplní full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vyplni enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo );

public:
	// zjistí, mùže-li skillu volat nepøítel
	virtual BOOL CanBeCalledByEnemy () { return TRUE; };

private:
// Pomocné metody skilly

	// ukonèí skillu s výsledkem "eResult" (jednotka "pUnit" musí být zamèena pro zápis)
	inline void Finish ( CSUnit *pUnit, enum EGoResult eResult );
	// vyplní kontrolní body jednotky (jednotka musí být zamèena pro zápis) (FALSE=cesta je 
	//		prázdná)
	BOOL FillCheckPoints ( CSUnit *pUnit );
	// pøipraví data skilly na pøesun k prvnímu kontrolnímu bodu jednotky (jednotka musí 
	//		být zamèena pro zápis)
	void GoToFirstCheckPoint ( CSUnit *pUnit );

// Debuggovací informace

#ifdef _DEBUG
	// zkontroluje neškodná data skilly (TRUE=OK)
	BOOL CheckEmptySkillData ( CSUnit *pUnit );
#endif //_DEBUG

public:
// Interface volání ze skriptù

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSMoveSkillType, 7 );
	DECLARE_SKILLCALL_METHOD2_WRITE_HARD ( SCI_Go, Int, int *, Int, int * );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT_EEC ( SCI_GetSpeed );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_GoPos, SystemObject, CISystemObject * );
	DECLARE_SKILLCALL_METHOD3_WRITE_HARD ( SCI_GoCloseTo, Int, int *, Int, int *, 
		Int, int * );
	DECLARE_SKILLCALL_METHOD2_WRITE_HARD ( SCI_GoCloseToPos, SystemObject, 
		CISystemObject *, Int, int * );

protected:
	// aktivuje skillu pro pohyb jednotky na pozici se souøadnicemi "pX" a "pY" (jednotka 
	//		je zamèena pro zápis)
	ESyscallResult SCI_Go ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, 
		CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY );
	// vrátí rychlost jednotky (jednotka je zamèena pro ètení)
	ESyscallResult SCI_GetSpeed ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn );
	// aktivuje skillu pro pohyb jednotky na pozici "pPosition" (jednotka je zamèena 
	//		pro zápis)
	ESyscallResult SCI_GoPos ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, 
		CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pPosition );
	// aktivuje skillu pro pohyb jednotky na pozici se souøadnicemi "pX" a "pY", 
	//		s tolerancí "pTolerance" (jednotka je zamèena pro zápis)
	ESyscallResult SCI_GoCloseTo ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY, 
		int *pTolerance );
	// aktivuje skillu pro pohyb jednotky na pozici "pPosition" s tolerancí "pTolerance" 
	//		(jednotka je zamèena pro zápis)
	ESyscallResult SCI_GoCloseToPos ( CSUnit *pUnit, CIProcess *pProcess, 
		CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
		CISystemObject *pPosition, int *pTolerance );

// Data
protected:

	// rychlost jednotky
	DWORD m_dwSpeed;
	// ID vzhledu jednotky pøi pohybu
	DWORD m_dwAppearanceID;
};

#endif //__SERVER_MOVE_SKILL_TYPE__HEADER_INCLUDED__
