/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Makra pro vytváøení potomkù tøídy za bìhu programu 
 *          z ID a ze jména potomka
 * 
 ***********************************************************/

#ifndef __RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__
#define __RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__

#include <string.h>

// deklarace tøídy "ClassName", jejíž potomky (deklarované s makrem 
//		DECLARE_RUN_TIME_ID_NAME_CREATION) bude možné vytváøet za bìhu programu z ID 
//		potomka metodou static ClassName *CreateChildByID ( DWORD dwID ), deklarované jako 
//		public/protected/private dle umístìní makra v deklaraci tøídy "ClassName" a 
//		ze jména potomka metodou static ClassName *CreateChildByName ( LPCTSTR lpcszName ), 
//		deklarované jako public/protected/private dle umístìní makra v deklaraci tøídy 
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

// implementace tøídy "ClassName" deklarované s makrem DECLARE_RUN_TIME_ID_NAME_STORAGE
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

// deklarace tøídy "ClassName" vytváøené za bìhu programu z ID a ze jména ve tøídì 
//		"IDNameStorageClassName"
#define DECLARE_RUN_TIME_ID_NAME_CREATION(ClassName, IDNameStorageClassName) \
	public:\
		static IDNameStorageClassName *RTIDNCCreate ();\
		static DWORD m_dwRTIDNCID;\
		static LPCTSTR m_lpcszRTIDNCName;

// implementace tøídy "ClassName" deklarované s makrem DECLARE_RUN_TIME_ID_NAME_CREATION 
//		s ID potomka "dwID" a se jménem potomka "lpcszName" ve tøídì 
//		"IDNameStorageClassName"
#define IMPLEMENT_RUN_TIME_ID_NAME_CREATION(ClassName, IDNameStorageClassName, dwID, lpcszName) \
	IDNameStorageClassName *ClassName::RTIDNCCreate () { return new ClassName; }\
	DWORD ClassName::m_dwRTIDNCID = dwID;\
	LPCTSTR ClassName::m_lpcszRTIDNCName = lpcszName;

// zaèátek tabulky potomkù tøídy "ClassName" deklarované s makrem 
//		DECLARE_RUN_TIME_ID_NAME_STORAGE
#define BEGIN_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE(ClassName) \
	ClassName::SRTIDNSRecord ClassName::m_aRTIDNSRecords[] = {

// záznam tøídy "ClassName" deklarované s makrem RUN_TIME_ID_NAME_CREATION do tabulky 
//		deklarované s makrem BEGIN_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE
#define RUN_TIME_ID_NAME_CREATION_RECORD(ClassName) \
	{ &ClassName::RTIDNCCreate, ClassName::m_lpcszRTIDNCName, ClassName::m_dwRTIDNCID },

// ukonèení tabulky potomkù tøídy "ClassName" deklarované s makrem 
//		DECLARE_RUN_TIME_ID_NAME_STORAGE
#define END_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE() { NULL, NULL, 0 } };

#endif //__RUN_TIME_ID_NAME_CREATION__HEADER_INCLUDED__
