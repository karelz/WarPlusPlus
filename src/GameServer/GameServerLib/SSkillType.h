/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída typu skilly jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_SKILL_TYPE__HEADER_INCLUDED__

#include "SUnit.h"

#include "AbstractDataClasses\RunTimeIDNameCreation.h"

#include "TripleS\Interpret\Src\InterpretEnums.h"
#include "TripleS\Interpret\Src\IBag.h"

#include "SSkillTypeMethods.h"

//////////////////////////////////////////////////////////////////////
// Makra pro deklaraci a implementaci typù skill
//////////////////////////////////////////////////////////////////////

// deklarace typu skilly "SkillTypeName" (jméno tøídy)
#define DECLARE_SKILL_TYPE(SkillTypeName) \
	public:\
		virtual DWORD SkillTypeName::GetID ();\
		virtual LPCTSTR SkillTypeName::GetName ();\
	private:\
		DECLARE_RUN_TIME_ID_NAME_CREATION ( SkillTypeName, CSSkillType )

// implementace typu skilly "SkillTypeName" s ID "dwID" a jménem "lpcszName"
#define IMPLEMENT_SKILL_TYPE(SkillTypeName, dwID, lpcszName) \
	DWORD SkillTypeName::GetID () { return SkillTypeName::m_dwRTIDNCID; }\
	LPCTSTR SkillTypeName::GetName () { return SkillTypeName::m_lpcszRTIDNCName; }\
	IMPLEMENT_RUN_TIME_ID_NAME_CREATION ( SkillTypeName, CSSkillType, dwID, lpcszName )

//////////////////////////////////////////////////////////////////////
// Makra pro trasování
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	extern BOOL g_bTraceSkillCalls;
	extern BOOL g_bTraceSkills;
	extern BOOL g_bTraceNotifications;

	#define TRACE_SKILLCALL if ( g_bTraceSkillCalls ) TRACE_NEXT 
	#define TRACE_SKILLCALL0(text) do { if ( g_bTraceSkillCalls ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_SKILLCALL1(text,p1) do { if ( g_bTraceSkillCalls ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_SKILLCALL2(text,p1,p2) do { if ( g_bTraceSkillCalls ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_SKILLCALL3(text,p1,p2,p3) do { if ( g_bTraceSkillCalls ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )

	#define TRACE_SKILL if ( g_bTraceSkills ) TRACE_NEXT 
	#define TRACE_SKILL0(text) do { if ( g_bTraceSkills ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_SKILL1(text,p1) do { if ( g_bTraceSkills ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_SKILL2(text,p1,p2) do { if ( g_bTraceSkills ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_SKILL3(text,p1,p2,p3) do { if ( g_bTraceSkills ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )

	#define TRACE_NOTIFICATION if ( g_bTraceNotifications ) TRACE_NEXT 
	#define TRACE_NOTIFICATION0(text) do { if ( g_bTraceNotifications ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_NOTIFICATION1(text,p1) do { if ( g_bTraceNotifications ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_NOTIFICATION2(text,p1,p2) do { if ( g_bTraceNotifications ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_NOTIFICATION3(text,p1,p2,p3) do { if ( g_bTraceNotifications ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
#else //!_DEBUG
	#define TRACE_SKILLCALL TRACE
	#define TRACE_SKILLCALL0 TRACE0
	#define TRACE_SKILLCALL1 TRACE1
	#define TRACE_SKILLCALL2 TRACE2
	#define TRACE_SKILLCALL3 TRACE3

	#define TRACE_SKILL TRACE
	#define TRACE_SKILL0 TRACE0
	#define TRACE_SKILL1 TRACE1
	#define TRACE_SKILL2 TRACE2
	#define TRACE_SKILL3 TRACE3

	#define TRACE_NOTIFICATION TRACE
	#define TRACE_NOTIFICATION0 TRACE0
	#define TRACE_NOTIFICATION1 TRACE1
	#define TRACE_NOTIFICATION2 TRACE2
	#define TRACE_NOTIFICATION3 TRACE3
#endif //!_DEBUG

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída typu jednotky
class CSUnitType;
// tøída procesu interpretu
class CIProcess;
// tøída systémového volání
class CISyscall;

//////////////////////////////////////////////////////////////////////
// Abstraktní tøída typu skilly jednotky
class CSSkillType : public CPersistentObject 
{
	// deklarace tøídy, jejíž potomky lze vytváøet za bìhu programu z ID nebo ze jména 
	//		potomka
	DECLARE_RUN_TIME_ID_NAME_STORAGE ( CSSkillType )

	DECLARE_DYNAMIC ( CSSkillType )

	friend class CSUnitType;
	friend class CSUnit;
	friend class CSMap;

// Datoté typy
public:
// Skupiny typù skill
	enum
	{
		Group_None = 0x0000,
		Group_Move = 0x0001,
		Group_Attack = 0x0002,
		Group_Defense = 0x0004,
		Group_Mine = 0x0008,
		Group_Resource = 0x0010,
		Group_Storage = 0x0020,
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSSkillType ();
	// destruktor
	virtual ~CSSkillType ();

	// vytvoøí typ skilly s ID "dwID" (NULL=neznámé ID) s offsetem dat skilly 
	//		"dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
	static CSSkillType *Create ( DWORD dwID, DWORD dwSkillDataOffset, DWORD dwIndex, 
		CSUnitType *pUnitType );
	// vytvoøí typ skilly se jménem "lpcszName" (NULL=neznámé jméno) s offsetem dat 
	//		skilly "dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
	static CSSkillType *Create ( LPCTSTR lpcszName, DWORD dwSkillDataOffset, 
		DWORD dwIndex, CSUnitType *pUnitType );

private:
// Inicializace a znièení dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		výjimky: CPersistentLoadException
	void Create ( void *pData, DWORD dwDataSize );
	// znièí data typu skilly
	void Delete ();
protected:
	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		výjimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize ) = 0;
	// inicializuje data typu skilly po vytvoøení typu skilly
	//		výjimky: CPersistentLoadException
	virtual void PostCreateSkillType () {};
	// znièí data typu skilly
	virtual void DeleteSkillType () = 0;

private:
// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

protected:
// Ukládání dat potomkù (analogie CPersistentObject metod)

	// ukládání dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage ) = 0;
	// nahrávání pouze uložených dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage ) = 0;
	// pøeklad ukazatelù potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) = 0;
	// inicializace nahraného objektu potomka
	virtual void PersistentInitSkillType () = 0;

public:
// Get/Set metody serverových informací o typu skilly jednotky

	// vrátí ID typu skilly (implementuje makro IMPLEMENT_SKILL_TYPE)
	virtual DWORD GetID () = 0;
	// vrátí jméno typu skilly (implementuje makro IMPLEMENT_SKILL_TYPE)
	virtual LPCTSTR GetName () = 0;
	// vrátí velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize () = 0;
	// vrátí masku skupin skill
	virtual DWORD GetGroupMask () = 0;

protected:
// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je již zamèená pro 
	//		zápis, metoda mùže zamykat libovolný poèet jiných jednotek pro zápis/ètení 
	//		(vèetnì jednotek ze svého bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit ) = 0;

	// informuje povolenou skillu o zmìnì módu jednotky "pUnit" z módu "nOldMode" 
	//		(jednotka je zamèena pro zápis)
	virtual void ModeChanged ( CSUnit *pUnit, BYTE nOldMode ) {};

	// inicializuje skillu jednotky "pUnit" (voláno po vytvoøení objektu jednotky)
	virtual void InitializeSkill ( CSUnit *pUnit ) = 0;
	// znièí skillu jednotky "pUnit" (voláno pøed znièením objektu jednotky)
	virtual void DestructSkill ( CSUnit *pUnit ) = 0;

	// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	void ActivateSkill ( CSUnit *pUnit ) 
		{ pUnit->ActivateSkill ( GetIndex () ); };
	// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	virtual void DeactivateSkill ( CSUnit *pUnit ) 
		{ pUnit->DeactivateSkill ( GetIndex () ); };

	// ukládání dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// nahrávání pouze uložených dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// pøeklad ukazatelù dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// inicializace nahraných dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit ) = 0;

	// vyplní full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo ) {};
	// vyplní enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) {};

public:
	// zjistí, mùže-li skillu volat nepøítel
	virtual BOOL CanBeCalledByEnemy () { return FALSE; };

protected:
	// zjistí, je-li typ skilly povolen (jednotka musí být zamèena alespoò pro read)
	BOOL IsEnabled ( CSUnit *pUnit );
	// zjistí, je-li skilla aktivní (jednotka musí být zamèena alespoò pro read)
	BOOL IsSkillActive ( CSUnit *pUnit );

public:
// Interface volání ze skriptù

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSSkillType, 2 );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_IsActive );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_GetName );

protected:
	// metoda volání ze skriptù pro urèení volání reakce pøedka
	ESyscallResult _SCI_M_SkillCallBaseClass ( CSUnit *pUnit, CIProcess *pProcess, 
		CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy ) { return SYSCALL_ERROR; };

	// vrátí pøíznak, je-li skilla aktivní
	ESyscallResult SCI_IsActive ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn );
	// vrátí jméno typu skilly
	ESyscallResult SCI_GetName ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn );

public:
// Serverové pomocné metody

	// vrátí ukazatel na typ jednotky
	CSUnitType *GetUnitType () { ASSERT ( m_pUnitType != NULL ); return m_pUnitType; };
	// vrátí ukazatel na data skilly jednotky "pUnit"
	void *GetSkillData ( CSUnit *pUnit ) 
		{ ASSERT ( m_dwSkillDataOffset != DWORD_MAX ); return (void *)( pUnit->GetSkillData () + m_dwSkillDataOffset ); };
private:
	// vrátí index v typu jednotky
	DWORD GetIndex () { ASSERT ( m_dwIndex != DWORD_MAX ); return m_dwIndex; };

// Data
private:
// Serverové informace o typu skilly

	// ukazatel na typ jednotky
	CSUnitType *m_pUnitType;
	// index v typu jednotky
	DWORD m_dwIndex;
	// offset dat skilly v datech skillù jednotky
	DWORD m_dwSkillDataOffset;
};

#endif //__SERVER_SKILL_TYPE__HEADER_INCLUDED__
