#ifndef _CSONEINSTANCE_INCLUDED
#define _CSONEINSTANCE_INCLUDED

class CSOneInstance : public CPersistentObject
{
public:
	// create & delete
	static void Create();
  // Called before any delete on the map occures
  static void PreDelete ();
  // Called after all the map objects are deleted
	static void Delete();

public:
	// save & load
	static void PersistentSave( CPersistentStorage &storage);
	static void PersistentLoad( CPersistentStorage &storage);
	static void PersistentTranslatePointers( CPersistentStorage &storage);
	static void PersistentInit();
};

#endif