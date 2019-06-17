/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Typy skill
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SSkillType.h"

#include "GameServer\Skills\ExampleSkill\SExampleSkillType.h"
#include "GameServer\Skills\MoveSkill\SMoveSkillType.h"
#include "GameServer\Skills\AttackDefenseSkills\BulletAttackSkill\SBulletAttackSkillType.h"
#include "GameServer\Skills\AttackDefenseSkills\BulletDefenseSkill\SBulletDefenseSkillType.h"
#include "GameServer\Skills\MakeSkill\SMakeSkillType.h"
#include "GameServer\Skills\MineSkill\SMineSkillType.h"
#include "GameServer\Skills\ResourceSkill\SResourceSkillType.h"
#include "GameServer\Skills\StorageSkill\SStorageSkillType.h"

/////////////////////////////////////////////////////////////////////
// implementace typù skill
/////////////////////////////////////////////////////////////////////

BEGIN_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE ( CSSkillType )
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSExampleSkillType )        // 97
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSMoveSkillType )           // 0
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSBulletAttackSkillType )   // 1
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSBulletDefenseSkillType )  // 2
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSMakeSkillType )           // 3
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSMineSkillType )           // 4
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSResourceSkillType )			// 5
	RUN_TIME_ID_NAME_CREATION_RECORD ( CSStorageSkillType )			// 6
END_RUN_TIME_ID_NAME_STORAGE_RECORD_TABLE ()
