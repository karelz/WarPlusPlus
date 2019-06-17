#ifndef SC_UNITHIERARCHYCONNECTION_H_
#define SC_UNITHIERARCHYCONNECTION_H_

enum EUnitHierarchyRequest{
  UnitHierarchyRequest_GetWhole = 1,  // Get whole unit hierarchy
  UnitHierarchyRequest_ChangeScript = 2, // Change script for set of units
                                         // Followed be SUnitHierarchyRequest_ChangeScript
  UnitHierarchyRequest_ChangeName = 3,  // Change name for set of units
                                        // Followed be SUnitHierarchyRequest_ChangeName
};

enum EUnitHierarchyAnswer{
  UnitHierarchyAnswer_GetWhole = 1,  // Replies withi whole unit hierarchy
                                     // Followed by SUnitHierarchyAnswer_GetWhole structure
  UnitHierarchyAnswer_ChangeScript = 2, // Reply to script change request
                                        // Followed by SUnitHierarchyAnswer_ChangeScript
  UnitHierarchyAnswer_ChangeName = 3, // Reply to name change request
                                      // Followed by SUnitHierarchyAnswer_ChangeName
};

struct SUnitHierarchyAnswer_GetWhole{
  DWORD m_dwUnitCount;   // Number of all units in following list
                         // By now only two values are possible - 0 -> no units, 1 - >1 units
};
// Followed by unit records
// First is the GeneralCommander
// And then follows its children, every one of them is followed by
// its children and so on
struct SUnitHierarchyAnswer_UnitRecord{
  DWORD m_dwZUnitID;  // ID of the virtual unit (this field is always valid)

  DWORD m_dwSUnitID;  // ID of the underlying fysical unit (if no - 0x0FFFFFFFF)
  DWORD m_dwSUnitTypeID;  // Unit type ID for the fysical unit

  DWORD m_dwZCommanderID; // ID of the virtual unit commander for this unit
                          // General commander has 0
  DWORD m_dwInferiorCount; // Number of inferior units

  DWORD m_dwScriptNameLength; // Name of the script - length (with \0)
  DWORD m_dwUnitNameLength;  // Name of the unit - length (with \0)
};
// Followed by script name with \0
// Followed by unit name with \0

struct SUnitHierarchyRequest_ChangeScript{
  DWORD m_dwScriptNameLength;  // length of the script name with \0
  DWORD m_dwUnitCount;     // number of units to change
};
// Followed by script name with \0
// Followed by array of DWORDs - IDs of ZUnits

struct SUnitHierarchyRequest_ChangeName{
  DWORD m_dwNameLength;  // length of the name with \0
  DWORD m_dwUnitCount;     // number of units to change
};
// Followed by name with \0
// Followed by array of DWORDs - IDs of ZUnits
  
struct SUnitHierarchyAnswer_ChangeScript{
  DWORD m_dwUnitCount;  // Number of unit records following 
};
// Followed by SUnitHierarchyAnswer_ChangeScript_UnitRecord unit records

struct SUnitHierarchyAnswer_ChangeName{
  DWORD m_dwUnitCount;  // Number of unit records following 
};
// Followed by SUnitHierarchyAnswer_ChangeScript_UnitRecord unit records

struct SUnitHierarchyAnswer_ChangeScript_UnitRecord{
  DWORD m_dwZUnitID;  // ID of unit

  DWORD m_dwResult;   // The result
                      // 0 - success, 1 - error, 2 - already dead
};

#endif