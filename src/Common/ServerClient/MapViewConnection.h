#ifndef SC_MAPVIEWCONNECTION_H_
#define SC_MAPVIEWCONNECTION_H_

enum EMapViewRequest{
  MapViewRequest_RunFunction = 1,  // Run global function (followed by SMapViewRequest_RunFunction)
  MapViewRequest_KillFunction = 2, // Kill currently running global function

  MapViewRequest_AskForPosition = 3,  // Result of the ask for question on position on the map
                                      // (followed by SMapViewRequest_AskForPosition)
  MapViewRequest_AskForUnit = 4,  // Result of the ask for question on unit
                                  // (followed by SMapViewRequest_AskForUnit)
  MapViewRequest_AskForPositionCanceled = 5, // the question was canceled
  MapViewRequest_AskForUnitCanceled = 6,  // the question was canceled

  MapViewRequest_ToolbarFileUpdate = 7, // Requests the toolbar file update (download) ( no more data )
};

enum EMapViewAnswer{
  MapViewAnswer_RunFunctionError = 1,  // There was an error running the function
  MapViewAnswer_FunctionActionDone = 2, // The function needs no more intreactive action
                                        // this is also reaction on KillFunction request
  MapViewAnswer_Message = 3,           // Display a message on client (followed by SMapViewAnswer_Message)

  MapViewAnswer_AskForPosition = 4,  // Ask the user for position on the map
                                     // Followed by SMapViewAnswer_AskForPosition
  MapViewAnswer_AskForUnit = 5,    // Ask the user for unit (followed by SMapViewAnswer_AskForUnit)

  MapViewAnswer_ToolbarFileUpdate = 6, // Sends the toolbar file (this will arrive during the initialization)
                                       // Followed by SMapViewAnswer_ToolbarFileUpdate
  MapViewAnswer_ToolbarUpdateComplete = 7, // The updating of the toolbar files has been done (no more data)
};

struct tagSMapViewRequest_RunFunction
{
  DWORD m_dwFunctionNameLength;  // Length of the function name (with \0)
  DWORD m_dwParamsCount;         // Number of additional parameters
  DWORD m_dwSelectedUnitsCount;  // Number of selected units
};
typedef struct tagSMapViewRequest_RunFunction SMapViewRequest_RunFunction;
// After this is the name of the function
// then unit IDs of selected units
// then array of SMapViewRequest_RunFunctionParam structures

struct tagSMapViewRequest_RunFunctionParam
{
  typedef enum{
    ParamType_None,
    ParamType_Integer,
    ParamType_Real,
    ParamType_String,
    ParamType_Bool,
    ParamType_Position,
    ParamType_Unit,
  } EParamType;

  EParamType m_eParamType; // type of the parameter
  union{  // value of the parameter
    int m_nValue;
    double m_dbValue;
    DWORD m_dwStringLength;  // This is filled when the type is ParamType_String
    BOOL m_bValue;
    struct Position{
      DWORD m_dwX;
      DWORD m_dwY;
      DWORD m_dwZ;
    } m_sPosition;
    DWORD m_dwSUnitID;
  };
};
typedef struct tagSMapViewRequest_RunFunctionParam SMapViewRequest_RunFunctionParam;
// If the param type is ParamType_String then here follows the string (with \0)

struct tagSMapViewAnswer_Message
{
  typedef enum{
    MsgType_User,
    MsgType_Info,
    MsgType_Warning,
    MsgType_Error,
    MsgType_Admin,
  } EMessageType;

  // type of the message
  EMessageType m_eMessageType;
  // X position - if 0x0F0000000 - no position specified
  DWORD m_dwXPos;
  // Y position - if 0x0F0000000 - no position specified
  DWORD m_dwYPos;
  // unit ID - if 0x0FFFFFFFF - no unit specified
  DWORD m_dwUnitID;

  // There should be specified only unit or position or none
  // never both unit and position

  // Length of the text of the message (with \0)
  DWORD m_dwTextLength;
};
typedef struct tagSMapViewAnswer_Message SMapViewAnswer_Message;
// Followed by the message text (with \0)

struct tagSMapViewAnswer_AskForPosition
{
  // Length of the question string (with \0)
  DWORD m_dwQuestionLength;
};
typedef struct tagSMapViewAnswer_AskForPosition SMapViewAnswer_AskForPosition;
// Followed by the question text (with \0)

struct tagSMapViewAnswer_AskForUnit
{
  // Length of the question string (with \0)
  DWORD m_dwQuestionLength;

  // Type of unit required
  typedef enum{
    UnitType_Any = 0,
    UnitType_My = 1,
    UnitType_Enemy = 2,
  } EUnitType;
  EUnitType m_eUnitType;
};
typedef struct tagSMapViewAnswer_AskForUnit SMapViewAnswer_AskForUnit;
// followed by the question text (with \0)

struct tagSMapViewRequest_AskForPosition
{
  // The answered position on the map in mapcells
  DWORD m_dwPositionX;
  DWORD m_dwPositionY;
};
typedef struct tagSMapViewRequest_AskForPosition SMapViewRequest_AskForPosition;

struct tagSMapViewRequest_AskForUnit
{
  // The answered unit ID
  DWORD m_dwUnitID;
};
typedef struct tagSMapViewRequest_AskForUnit SMapViewRequest_AskForUnit;

struct tagSMapViewAnswer_ToolbarFileUpdate
{
  // 0 - normal file - followed by the file itself
  // 1 - Directory - just use the Path to create the directory
  DWORD m_dwFlags;

  // Length of the path to the file ( within the Toolbars directory )
  // or to the directory which is to be created ( with NULL )
  DWORD m_dwPathLength;
  
  // Length of the file ( in case of the directory this will be 0 )
  DWORD m_dwFileLength;

  // The structure is followed by the path to the file (with NULL)
  // And then by the file itself
};
typedef struct tagSMapViewAnswer_ToolbarFileUpdate SMapViewAnswer_ToolbarFileUpdate;

#endif