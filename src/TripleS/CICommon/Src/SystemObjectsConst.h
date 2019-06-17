/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Konstanty tykajici se systemovych objektu.
 * 
 ***********************************************************/

#ifndef _SYSTEMOBJECTSCONST_H
#define _SYSTEMOBJECTSCONST_H

//
// Defined system objects 
//

// Specialni objekty
#define SYSOB_STRING    "String"
#define SYSOB_SET       "Set"
#define SYSOB_ARRAY     "Array" 

// Enum

typedef enum {
    SO_NONE,
    SO_UNKNOWN,
    
    SO_STRING,
    SO_ARRAY,
    SO_SET,
    SO_POSITION,
    SO_MAP,
    SO_PATH,
    SO_CLIENT,
    SO_RESOURCES,
	/*SYSOB*/


    SO_SKILLINTERFACE,
    SO_EXAMPLESKILL,
    SO_MOVESKILL,
    SO_MAKESKILL,
    SO_BULLETATTACKSKILL,
    SO_BULLETDEFENSESKILL,
	SO_MINESKILL,
	SO_RESOURCESKILL,
	SO_STORAGESKILL,

	// pocet zaznamu v enumu - tato polozka MUSI byt posledni a v enumu se nesmí psat cisla!!!
	SYSTEM_OBJECT_COUNT
} ESystemObjectType;

// Tabulka prirazujici enumu stringy

extern char* g_SystemObjectNames[SYSTEM_OBJECT_COUNT];

ESystemObjectType GetSOTypeFromString(CString &strType);
char* GetStringFromSOType(ESystemObjectType nSOType);

#endif // _SYSTEMOBJECTSCONST_H
