/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Konstanty tykajici se systemovych objektu.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CICommon.h"

char* g_SystemObjectNames[SYSTEM_OBJECT_COUNT] = {
    "None",     // SO_NONE,  
    "Unknown",  // SO_UNKNOWN
                           
    "String",   // SO_STRING,
    "Array",    // SO_ARRAY, 
    "Set",      // SO_SET,   
    "Position", // SO_POSITION
    "Map",      // SO_MAP
    "Path",     // SO_PATH
    "Client",   // SO_CLIENT
    "Resources", // SO_RESOURCES
	/*SYSOB*/

              
    "_SkillInterface",  // SO_SKILLINTERFACE
    "SExampleSkill",    // SO_EXAMPLESKILL  
    "SMoveSkill",		// SO_MOVESKILL  
    "SMakeSkill",       // SO_MAKESKILL
    "SBulletAttackSkill",   // SO_BULLETATTACKSKILL
    "SBulletDefenseSkill",  // SO_BULLETDEFENSESKILL
	"SMineSkill",		// SO_MINESKILL,
	"SResourceSkill",	// SO_RESOURCESKILL,
	"SStorageSkill",	// SO_STORAGESKILL,

};

ESystemObjectType GetSOTypeFromString(CString &strType)
{
    int i=2;  // zaciname od SO_STRING
    
    while (i<SYSTEM_OBJECT_COUNT) 
    {
        if (strType == g_SystemObjectNames[i]) return (ESystemObjectType)i;
        else i++;
    }

    return SO_UNKNOWN;
}

char* GetStringFromSOType(ESystemObjectType nSOType)
{
    ASSERT(nSOType < SYSTEM_OBJECT_COUNT && nSOType>=0);
    return g_SystemObjectNames[nSOType];
}
    
