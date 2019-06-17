
#include "stdafx.h"

#include "SOneInstance.h"

#include "GameServer\Civilization\ZCivilization.h"
#include "GameServer\Civilization\Src\ZUnit.h"

//////////////////////////////////////////////////////////////////////

void CSOneInstance::Create()
{
	// Helena - notifikace od civilizace
	CZCivilization::CreateStatic();

	// Helena - metody a polozky jednotky Unit
	CZUnit::CreateStatic();
}

//////////////////////////////////////////////////////////////////////

// Called before any delete on the map occures
void CSOneInstance::PreDelete ()
{
  CZCivilization::PreDeleteStatic ();
}

//////////////////////////////////////////////////////////////////////

void CSOneInstance::Delete()
{
	// Helena - notifikace od civilizace
	CZCivilization::DeleteStatic();

	// Helena - metody a polozky jednotky Unit
	CZUnit::DeleteStatic();
}

//////////////////////////////////////////////////////////////////////

void CSOneInstance::PersistentSave( CPersistentStorage &storage)
{
	BRACE_BLOCK ( storage );

	// Helena - notifikace od civilizace
	CZCivilization::PersistentSaveStatic( storage);

	// Helena - metody a polozky jednotky Unit
	CZUnit::PersistentSaveStatic( storage);
}

//////////////////////////////////////////////////////////////////////

void CSOneInstance::PersistentLoad( CPersistentStorage &storage)
{
	BRACE_BLOCK ( storage );

	// Helena - notifikace od civilizace
	CZCivilization::PersistentLoadStatic( storage);

	// Helena - metody a polozky jednotky Unit
	CZUnit::PersistentLoadStatic( storage);
}

//////////////////////////////////////////////////////////////////////

void CSOneInstance::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// Helena - notifikace od civilizace
	CZCivilization::PersistentTranslatePointersStatic( storage);

	// Helena - metody a polozky jednotky Unit
	CZUnit::PersistentTranslatePointersStatic( storage);
}

//////////////////////////////////////////////////////////////////////

void CSOneInstance::PersistentInit()
{
	// Helena - notifikace od civilizace
	CZCivilization::PersistentInitStatic();

	// Helena - metody a polozky jednotky Unit
	CZUnit::PersistentInitStatic();
}

//////////////////////////////////////////////////////////////////////
