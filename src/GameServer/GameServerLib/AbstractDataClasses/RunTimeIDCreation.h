/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Makra pro vytváøení potomkù tøídy za bìhu programu 
 *          z ID potomka
 * 
 ***********************************************************/

#ifndef __RUN_TIME_ID_CREATION__HEADER_INCLUDED__
#define __RUN_TIME_ID_CREATION__HEADER_INCLUDED__

// deklarace tøídy "ClassName", jejíž potomky (deklarované s makrem 
//		DECLARE_RUN_TIME_ID_CREATION) bude možné vytváøet za bìhu programu z ID 
//		potomka metodou static ClassName *CreateChildByID ( DWORD dwID ), deklarované jako 
//		public/protected/private dle umístìní makra v deklaraci tøídy "ClassName"
#define DECLARE_RUN_TIME_ID_STORAGE(ClassName) \
		static ClassName *CreateChildByID ( DWORD dwID );\
	private:\
		struct SRTIDSRecord \
		{\
			ClassName *(* pfnCreate) ();\
			DWORD dwID;\
		};\
		static struct SRTIDSRecord m_aRTIDSRecords[];

// implementace tøídy "ClassName" deklarované s makrem DECLARE_RUN_TIME_ID_STORAGE
#define IMPLEMENT_RUN_TIME_ID_STORAGE(ClassName) \
	ClassName *ClassName::CreateChildByID ( DWORD dwID ) \
	{\
		struct SRTIDSRecord *pRecord = m_aRTIDSRecords;\
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
	}

// deklarace tøídy "ClassName" vytváøené za bìhu programu z ID ve tøídì 
//		"IDStorageClassName"
#define DECLARE_RUN_TIME_ID_CREATION(ClassName, IDStorageClassName) \
	public:\
		static IDStorageClassName *RTIDCCreate ();\
		static DWORD m_dwRTIDCID;\

// implementace tøídy "ClassName" deklarované s makrem DECLARE_RUN_TIME_ID_CREATION 
//		s ID potomka "dwID" ve tøídì "IDStorageClassName"
#define IMPLEMENT_RUN_TIME_ID_CREATION(ClassName, IDStorageClassName, dwID) \
	IDStorageClassName *ClassName::RTIDCCreate () { return new ClassName; }\
	DWORD ClassName::m_dwRTIDCID = dwID;\

// zaèátek tabulky potomkù tøídy "ClassName" deklarované s makrem 
//		DECLARE_RUN_TIME_ID_STORAGE
#define BEGIN_RUN_TIME_ID_STORAGE_RECORD_TABLE(ClassName) \
	ClassName::SRTIDSRecord ClassName::m_aRTIDSRecords[] = {

// záznam tøídy "ClassName" deklarované s makrem RUN_TIME_ID_CREATION do tabulky 
//		deklarované s makrem BEGIN_RUN_TIME_ID_STORAGE_RECORD_TABLE
#define RUN_TIME_ID_CREATION_RECORD(ClassName) \
	{ &ClassName::RTIDCCreate, ClassName::m_dwRTIDCID },

// ukonèení tabulky potomkù tøídy "ClassName" deklarované s makrem 
//		DECLARE_RUN_TIME_ID_STORAGE
#define END_RUN_TIME_ID_STORAGE_RECORD_TABLE() { NULL, 0 } };

#endif //__RUN_TIME_ID_CREATION__HEADER_INCLUDED__
