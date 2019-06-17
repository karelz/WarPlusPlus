/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Makra pro vytv��en� potomk� t��dy za b�hu programu 
 *          z ID potomka
 * 
 ***********************************************************/

#ifndef __RUN_TIME_ID_CREATION__HEADER_INCLUDED__
#define __RUN_TIME_ID_CREATION__HEADER_INCLUDED__

// deklarace t��dy "ClassName", jej� potomky (deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_CREATION) bude mo�n� vytv��et za b�hu programu z ID 
//		potomka metodou static ClassName *CreateChildByID ( DWORD dwID ), deklarovan� jako 
//		public/protected/private dle um�st�n� makra v deklaraci t��dy "ClassName"
#define DECLARE_RUN_TIME_ID_STORAGE(ClassName) \
		static ClassName *CreateChildByID ( DWORD dwID );\
	private:\
		struct SRTIDSRecord \
		{\
			ClassName *(* pfnCreate) ();\
			DWORD dwID;\
		};\
		static struct SRTIDSRecord m_aRTIDSRecords[];

// implementace t��dy "ClassName" deklarovan� s makrem DECLARE_RUN_TIME_ID_STORAGE
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

// deklarace t��dy "ClassName" vytv��en� za b�hu programu z ID ve t��d� 
//		"IDStorageClassName"
#define DECLARE_RUN_TIME_ID_CREATION(ClassName, IDStorageClassName) \
	public:\
		static IDStorageClassName *RTIDCCreate ();\
		static DWORD m_dwRTIDCID;\

// implementace t��dy "ClassName" deklarovan� s makrem DECLARE_RUN_TIME_ID_CREATION 
//		s ID potomka "dwID" ve t��d� "IDStorageClassName"
#define IMPLEMENT_RUN_TIME_ID_CREATION(ClassName, IDStorageClassName, dwID) \
	IDStorageClassName *ClassName::RTIDCCreate () { return new ClassName; }\
	DWORD ClassName::m_dwRTIDCID = dwID;\

// za��tek tabulky potomk� t��dy "ClassName" deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_STORAGE
#define BEGIN_RUN_TIME_ID_STORAGE_RECORD_TABLE(ClassName) \
	ClassName::SRTIDSRecord ClassName::m_aRTIDSRecords[] = {

// z�znam t��dy "ClassName" deklarovan� s makrem RUN_TIME_ID_CREATION do tabulky 
//		deklarovan� s makrem BEGIN_RUN_TIME_ID_STORAGE_RECORD_TABLE
#define RUN_TIME_ID_CREATION_RECORD(ClassName) \
	{ &ClassName::RTIDCCreate, ClassName::m_dwRTIDCID },

// ukon�en� tabulky potomk� t��dy "ClassName" deklarovan� s makrem 
//		DECLARE_RUN_TIME_ID_STORAGE
#define END_RUN_TIME_ID_STORAGE_RECORD_TABLE() { NULL, 0 } };

#endif //__RUN_TIME_ID_CREATION__HEADER_INCLUDED__
