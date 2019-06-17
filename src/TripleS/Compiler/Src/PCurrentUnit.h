/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPCurrentUnit reprezentujici 
 *          jednotku (unit), ktera je prave prekladana (nebo je uz prelozena).
 * 
 ***********************************************************/

#if !defined(AFX_CURRENTUNIT_H__8C85D273_45D9_11D3_A4C2_00A0C970CB8E__INCLUDED_)
#define AFX_CURRENTUNIT_H__8C85D273_45D9_11D3_A4C2_00A0C970CB8E__INCLUDED_

class CPPreview;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPCurrentUnit 
// ==================
// Trida pro uchovavani informaci o prave prekladane jednotce.
class CPCurrentUnit 
{
public:
    // class CMethodEvent 
    // ==================
    // Spolecny predek pro tridy CMethod a CEvent.
	class CMethodEvent
	{
        // class CArgOrLocalVar
        // ====================
        // Trida pro uchovavani informaci o argumentech a lokalnich promennych
        // metod a reakci na zpravy
		class CArgOrLocalVar
		{
		public:
            // jmeno lokalni promenne/argumentu (neni treba ukladat => ve globalni stringtable)
            CStringTableItem *m_stiName;
            // datovy typ lokalni promenne/argumentu
			CType m_DataType;
		};

        // class CMethodEvent
	public:
		CMethodEvent(CPCodeStringTableItem *cstiName);
#ifdef _DEBUG
        CMethodEvent() {};
#endif
		virtual ~CMethodEvent();
	public:
        // nazev metody/eventy - ve stringtable jednotky
		CPCodeStringTableItem *m_cstiName;             
        // argumenty a lokalni promenne metody/eventy
		CArray<CArgOrLocalVar, CArgOrLocalVar&> m_aArgsAndLocalVars;

        // argumenty jsou v prvnich m_nArgCount polozkach v m_aArgsAndLocalVars
		int m_nArgCount; 

        // kod metody/eventy
		CPCode m_Code;

		virtual REG AddArgument(CStringTableItem *stiName, CType&DataType);
		virtual REG AddLocalVariable(CStringTableItem *stiName, CType&DataType);
		virtual bool IsArgOrLocalVar(CString &strName);
	};

    // class CMethod
    // =============
    // Trida uchovavajici informace o metode prekladane jednotky (dedi z CMethodEvent).
	class CMethod : public CMethodEvent
	{
	public:
		CMethod(CPCodeStringTableItem *cstiName, CType&ReturnDataType);
#ifdef _DEBUG
		CMethod() {}
#endif
	public:
        // navratovy typ metody
		CType m_ReturnDataType;

        // ulozeni metody na vystup
		void Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl);
	};

    // class CEvent
    // ============
    // Trida uchovavajici informace o reakci na zpravu 
    // prekladane jednotky (dedi z CMethodEvent).
	class CEvent : public CMethodEvent
	{
	public:
		CEvent(CPCodeStringTableItem *cstiName, bool bIsCommand);

#ifdef _DEBUG
		CEvent() {}
#endif

	public:
        // ulozeni reakci na zpravu na vystup
		void Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl);
	public:
		bool m_bIsCommand;
	};

    // class CMember
    // =============
    // Trida uchovavajici informace o polozce jednotky.
	class CMember
	{
	public:
		CMember(CPCodeStringTableItem *cstiName, CType&DataType);
#ifdef _DEBUG
		CMember() {}
#endif
	public:
        // nazev polozky - ve stringtable unity
		CPCodeStringTableItem *m_cstiName; 
        // datovy typ
		CType m_DataType;

        // ulozeni polozky na vystup
		void Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl);
	};

    // class CConstructor
    // ==================
    // Trida uchovavajici informace o konstruktoru jednotky (dedi z CMethodEvent).
	class CConstructor : public CMethodEvent
	{
	public:
		CConstructor() : CMethodEvent(NULL) {}

		// ulozeni konstruktoru na vystup
        void Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl);
	};

    // class CConstructor
    // ==================
    // Trida uchovavajici informace o destruktoru jednotky.
    typedef CConstructor CDestructor;

// class CPCurrentUnit
public:
	CPCurrentUnit(CString &strName);
	virtual ~CPCurrentUnit();

public:
    // nazev jednotky
	CString m_strName;
    // nazev predka (ze ktereho dedi)
	CString m_strParent;
    // typ predka, ze ktereho dedi
    CPNamespace::CUnit *m_pParent;
    // metody 
	CTypedPtrMap<CMapStringToPtr, CString, CMethod*> m_apMethods;
    // definovane reakce na zpravy
	CTypedPtrMap<CMapStringToPtr, CString, CEvent*> m_apEvents;
    // polozky
	CTypedPtrMap<CMapStringToPtr, CString, CMember*> m_apMembers;
    // konstruktor
	CConstructor m_Constructor;
    // destruktor
	CDestructor m_Destructor;

    // priznaky, zda jsou konstruktor a destruktor definovany
	bool m_bConstructorDefined, m_bDestructorDefined;
	
    // tabulka stringu, ktere se budou ukladat s kodem
	CPCodeStringTable m_StringTable;
    // tabulka realnych cisel
	CFloatTable m_FloatTable;

    // ukazatel na dalsi zkompilovanou jednotku ve spojaku kompileru
    CPCurrentUnit *m_pNext;
		
private:
    // velikosti hashovacich tabulek
	enum { MEMBERS_HASHTABLE_SIZE=13, METHODS_HASHTABLE_SIZE=21, EVENTS_HASHTABLE_SIZE=19 };

public:
    // pridani metody
	CMethod* AddMethod(CString &strName, CType&ReturnDataType);
    // dotaz na existenci metody - dle jmena
	bool HasMethodOnSelf(CString &strName);
    bool GetMethodFromParent(CString &strName, CPNamespace::CUnitMethod **pParentMethod, 
                             CStringTableItem **pstiParentName, CPNamespace *pNamespace);

    // pridani reakce na zpravu
	CEvent* AddEvent(CString &strName, bool bIsCommand);
    // dotaz na existenci reakce na zpravu
	bool HasEvent(CString &strName);

    // pridani polozky
	CMember* AddMember(CString &strName, CType&DataType); 
    // dotaz na existenci polozky
	bool HasMember(CString &strName, CPPreview *pPreview);
	// ziskani typu polozky
	bool GetMemberType(CString &strName, CType&MemberType, CPPreview *pPreview);

    // ulozeni jednotky na vystup
	void Save(CCompilerOutput *pOut);

    // debugovaci vypis obsahu
	void DebugWrite(CCompilerErrorOutput *pOut);
};

#endif // !defined(AFX_CURRENTUNIT_H__8C85D273_45D9_11D3_A4C2_00A0C970CB8E__INCLUDED_)
