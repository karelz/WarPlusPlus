/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøídy odložených volání procedur
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SDeferredProcedureCall.h"

#include "GameServer\Skills\SystemObjects\Map\SMapGetUnitsInAreaDPC.h"
#include "GameServer\Skills\AttackDefenseSkills\AbstractAttackSkill\SAttackableUnitsDPC.h"
#include "GameServer\Skills\AttackDefenseSkills\AbstractAttackSkill\SCanAttackUnitDPC.h"
#include "GameServer\Skills\MineSkill\SMineSetModeDPC.h"

/////////////////////////////////////////////////////////////////////
// Implementace odložených volání procedur
/////////////////////////////////////////////////////////////////////

BEGIN_RUN_TIME_ID_STORAGE_RECORD_TABLE ( CSDeferredProcedureCall )
	RUN_TIME_ID_CREATION_RECORD ( CSMapGetUnitsInAreaDPC )			// 0
	RUN_TIME_ID_CREATION_RECORD ( CSAttackableUnitsDPC )			// 1
	RUN_TIME_ID_CREATION_RECORD ( CSCanAttackUnitDPC )				// 2
	RUN_TIME_ID_CREATION_RECORD ( CSMineSetModeDPC )				// 3
END_RUN_TIME_ID_STORAGE_RECORD_TABLE ()
