/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Makra pro vytv��en� potomk� t��dy za b�hu programu 
 *          z ID a ze jm�na potomka
 * 
 ***********************************************************/

#ifndef __RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__
#define __RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__

#include <string.h>

// deklarace t��dy "ClassName", jej� potomky (deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_NAME_CREATION) bude mo�n� vytv��et za b�hu programu z ID 
//		potomka metodou static ClassName *CreateChildByID ( DWORD dwID ), deklarovan� jako 
//		public/protected/private dle um�st�n� makra v deklaraci t��dy "ClassName" a 
//		ze jm�na potomka metodou static ClassName *CreateChildByName ( LPCTSTR lpcszName ), 
//		deklarovan� jako public/protected/private dle um�st�n� makra v deklaraci t��dy 
//		"ClassName"
#define DECLARE_RUN_TIME_ID_NAME_STORAGE(ClassName) \
		static ClassName *CreateChildByID ( DWORD dwID );\
		static ClassName *CreateChildByName ( LPCTSTR lpcszName );\
	private:\
		struct SRTIDNSRecord \
		{\
			ClassName *(* pfnCreate) ();\
			LPCTSTR lpcszName;\
			DWORD dwID;\
		};\
		static struct SRTIDNSRecord m_aRTIDNSRecords[];

// implementace t��dy "ClassName" deklarovan� s makrem DECLARE_RUN_TIME_ID_NAME_STORAGE
#define IMPLEMENT_RUN_TIME_ID_NAME_STORAGE(ClassName) \
	ClassName *ClassName::CreateChildByID ( DWORD dwID ) \
	{\
		struct SRTIDNSRecord *pRecord = m_aRTIDNSRecords;\
		while ( pRecord->pfnCreate != NULL )\
		{\
			if ( pRecord->dwID == dwID )\
			{\
				return pRecord->pfnCreate ();\
			}\
			pRecord++;\
		}\
		TRACE1 ( "Error: can't find skill type with ID=%d\n", dwID );\
		return NULL;\
	}\
	ClassName *ClassName::CreateChildByName ( LPCTSTR lpcszName ) \
	{\
		struct SRTIDNSRecord *pRecord = m_aRTIDNSRecords;\
		while ( pRecord->pfnCreate != NULL )\
		{\
			if ( strcmp ( pRecord->lpcszName, lpcszName ) == 0 )\
			{\
				return pRecord->pfnCreate ();\
			}\
			pRecord++;\
		}\
		TRACE1 ( "Error: can't find skill type with name=%s\n", lpcszName );\
		return NULL;\
	}

// deklarace t��dy "ClassName" vytv��en� za b�hu programu z ID a ze jm�na ve t��d� 
//		"IDNameStorageClassName"
#define DECLARE_RUN_TIME_ID_NAME_CREATION(ClassName, IDNameStorageClassName) \
	public:\
		static IDNameStorageClassName *RTIDNCCreate ();\
		static DWORD m_dwRTIDNCID;\
		static LPCTSTR m_lpcszRTIDNCName;

// implementace t��dy "ClassName" deklarovan� s makrem DECLARE_RUN_TIME_ID_NAME_CREATION 
//		s ID potomka "dwID" a se jm�nem potomka "lpcszName" ve t��d� 
//		"IDNameStorageClassName"
#define IMPLEMENT_RUN_TIME_ID_NAME_CREATION(ClassName, IDNameStorageClassName, dwID, lpcszName) \
	IDNameStorageClassName *ClassName::RTIDNCCreate () { return new ClassName; }\
	DWORD ClassName::m_dwRTIDNCID = dwID;\
	LPCTSTR ClassName::m_lpcszRTIDNCName = lpcszName;

// za��tek tabulky potomk� t��dy "ClassName" deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_NAME_STORAGE
#define BEGIN_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE(ClassName) \
	ClassName::SRTIDNSRecord ClassName::m_aRTIDNSRecords[] = {

// z�znam t��dy "ClassName" deklarovan� s makrem RUN_TIME_ID_NAME_CREATION do tabulky 
//		deklarovan� s makrem BEGIN_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE
#define RUN_TIME_ID_NAME_CREATION_RECORD(ClassName) \
	{ &ClassName::RTIDNCCreate, ClassName::m_lpcszRTIDNCName, ClassName::m_dwRTIDNCID },

// ukon�en� tabulky potomk� t��dy "ClassName" deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_NAME_STORAGE
#define END_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE() { NULL, NULL, 0 } };

#endif //__RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__
