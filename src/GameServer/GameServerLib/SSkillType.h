/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da typu skilly jednotky
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
// Makra pro deklaraci a implementaci typ� skill
//////////////////////////////////////////////////////////////////////

// deklarace typu skilly "SkillTypeName" (jm�no t��dy)
#define DECLARE_SKILL_TYPE(SkillTypeName) \
	public:\
		virtual DWORD SkillTypeName::GetID ();\
		virtual LPCTSTR SkillTypeName::GetName ();\
	private:\
		DECLARE_RUN_TIME_ID_NAME_CREATION ( SkillTypeName, CSSkillType )

// implementace typu skilly "SkillTypeName" s ID "dwID" a jm�nem "lpcszName"
#define IMPLEMENT_SKILL_TYPE(SkillTypeName, dwID, lpcszName) \
	DWORD SkillTypeName::GetID () { return SkillTypeName::m_dwRTIDNCID; }\
	LPCTSTR SkillTypeName::GetName () { return SkillTypeName::m_lpcszRTIDNCName; }\
	IMPLEMENT_RUN_TIME_ID_NAME_CREATION ( SkillTypeName, CSSkillType, dwID, lpcszName )

//////////////////////////////////////////////////////////////////////
// Makra pro trasov�n�
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
// Dop�edn� deklarace

// t��da typu jednotky
class CSUnitType;
// t��da procesu interpretu
class CIProcess;
// t��da syst�mov�ho vol�n�
class CISyscall;

//////////////////////////////////////////////////////////////////////
// Abstraktn� t��da typu skilly jednotky
class CSSkillType : public CPersistentObject 
{
	// deklarace t��dy, jej� potomky lze vytv��et za b�hu programu z ID nebo ze jm�na 
	//		potomka
	DECLARE_RUN_TIME_ID_NAME_STORAGE ( CSSkillType )

	DECLARE_DYNAMIC ( CSSkillType )

	friend class CSUnitType;
	friend class CSUnit;
	friend class CSMap;

// Datot� typy
public:
// Skupiny typ� skill
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

	// vytvo�� typ skilly s ID "dwID" (NULL=nezn�m� ID) s offsetem dat skilly 
	//		"dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
	static CSSkillType *Create ( DWORD dwID, DWORD dwSkillDataOffset, DWORD dwIndex, 
		CSUnitType *pUnitType );
	// vytvo�� typ skilly se jm�nem "lpcszName" (NULL=nezn�m� jm�no) s offsetem dat 
	//		skilly "dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
	static CSSkillType *Create ( LPCTSTR lpcszName, DWORD dwSkillDataOffset, 
		DWORD dwIndex, CSUnitType *pUnitType );

private:
// Inicializace a zni�en� dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		v�jimky: CPersistentLoadException
	void Create ( void *pData, DWORD dwDataSize );
	// zni�� data typu skilly
	void Delete ();
protected:
	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		v�jimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize ) = 0;
	// inicializuje data typu skilly po vytvo�en� typu skilly
	//		v�jimky: CPersistentLoadException
	virtual void PostCreateSkillType () {};
	// zni�� data typu skilly
	virtual void DeleteSkillType () = 0;

private:
// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

protected:
// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)

	// ukl�d�n� dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage ) = 0;
	// nahr�v�n� pouze ulo�en�ch dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage ) = 0;
	// p�eklad ukazatel� potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) = 0;
	// inicializace nahran�ho objektu potomka
	virtual void PersistentInitSkillType () = 0;

public:
// Get/Set metody serverov�ch informac� o typu skilly jednotky

	// vr�t� ID typu skilly (implementuje makro IMPLEMENT_SKILL_TYPE)
	virtual DWORD GetID () = 0;
	// vr�t� jm�no typu skilly (implementuje makro IMPLEMENT_SKILL_TYPE)
	virtual LPCTSTR GetName () = 0;
	// vr�t� velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize () = 0;
	// vr�t� masku skupin skill
	virtual DWORD GetGroupMask () = 0;

protected:
// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je ji� zam�en� pro 
	//		z�pis, metoda m��e zamykat libovoln� po�et jin�ch jednotek pro z�pis/�ten� 
	//		(v�etn� jednotek ze sv�ho bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit ) = 0;

	// informuje povolenou skillu o zm�n� m�du jednotky "pUnit" z m�du "nOldMode" 
	//		(jednotka je zam�ena pro z�pis)
	virtual void ModeChanged ( CSUnit *pUnit, BYTE nOldMode ) {};

	// inicializuje skillu jednotky "pUnit" (vol�no po vytvo�en� objektu jednotky)
	virtual void InitializeSkill ( CSUnit *pUnit ) = 0;
	// zni�� skillu jednotky "pUnit" (vol�no p�ed zni�en�m objektu jednotky)
	virtual void DestructSkill ( CSUnit *pUnit ) = 0;

	// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	void ActivateSkill ( CSUnit *pUnit ) 
		{ pUnit->ActivateSkill ( GetIndex () ); };
	// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	virtual void DeactivateSkill ( CSUnit *pUnit ) 
		{ pUnit->DeactivateSkill ( GetIndex () ); };

	// ukl�d�n� dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// nahr�v�n� pouze ulo�en�ch dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// p�eklad ukazatel� dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) = 0;
	// inicializace nahran�ch dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit ) = 0;

	// vypln� full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo ) {};
	// vypln� enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) {};

public:
	// zjist�, m��e-li skillu volat nep��tel
	virtual BOOL CanBeCalledByEnemy () { return FALSE; };

protected:
	// zjist�, je-li typ skilly povolen (jednotka mus� b�t zam�ena alespo� pro read)
	BOOL IsEnabled ( CSUnit *pUnit );
	// zjist�, je-li skilla aktivn� (jednotka mus� b�t zam�ena alespo� pro read)
	BOOL IsSkillActive ( CSUnit *pUnit );

public:
// Interface vol�n� ze skript�

	// interface vol�n� ze skript�
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSSkillType, 2 );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_IsActive );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_GetName );

protected:
	// metoda vol�n� ze skript� pro ur�en� vol�n� reakce p�edka
	ESyscallResult _SCI_M_SkillCallBaseClass ( CSUnit *pUnit, CIProcess *pProcess, 
		CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy ) { return SYSCALL_ERROR; };

	// vr�t� p��znak, je-li skilla aktivn�
	ESyscallResult SCI_IsActive ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn );
	// vr�t� jm�no typu skilly
	ESyscallResult SCI_GetName ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn );

public:
// Serverov� pomocn� metody

	// vr�t� ukazatel na typ jednotky
	CSUnitType *GetUnitType () { ASSERT ( m_pUnitType != NULL ); return m_pUnitType; };
	// vr�t� ukazatel na data skilly jednotky "pUnit"
	void *GetSkillData ( CSUnit *pUnit ) 
		{ ASSERT ( m_dwSkillDataOffset != DWORD_MAX ); return (void *)( pUnit->GetSkillData () + m_dwSkillDataOffset ); };
private:
	// vr�t� index v typu jednotky
	DWORD GetIndex () { ASSERT ( m_dwIndex != DWORD_MAX ); return m_dwIndex; };

// Data
private:
// Serverov� informace o typu skilly

	// ukazatel na typ jednotky
	CSUnitType *m_pUnitType;
	// index v typu jednotky
	DWORD m_dwIndex;
	// offset dat skilly v datech skill� jednotky
	DWORD m_dwSkillDataOffset;
};

#endif //__SERVER_SKILL_TYPE__HEADER_INCLUDED__
