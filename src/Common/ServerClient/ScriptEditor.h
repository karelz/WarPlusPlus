#ifndef SC_SCRIPTEDITOR_H_
#define SC_SCRIPTEDITOR_H_

typedef enum tagEScriptEditor_Commands{
  ScriptEditor_Command_ListFiles = 0x01,  // lists all files(sources) on the server
  ScriptEditor_Command_SaveFile = 0x02, // saves the script on the server
                                        // followed by the SScriptEditor_SaveFile structure

  ScriptEditor_Command_LoadFile = 0x03, // loads file from the server
                                        // followed by the SScriptEditor_LoadFile_Command structure

  ScriptEditor_Command_Compile = 0x04,  // compiles the file (and updates)
                                        // followed by the SScriptEditor_Compile_Command structure

  ScriptEditor_Command_MultiCompile = 0x05, // multi compile - compiles and updates all modified scripts
                                            // no more data should arrive

  ScriptEditor_Command_GetProfilingInfo = 0x06, // asks for profiling informations
                                                // No more data needed

  ScriptEditor_Command_Error = 0x0FFFF, // some error occured
} EScriptEditor_Commands;

typedef enum tagEScriptEditor_Answers{
  ScriptEditor_Answer_ListFiles = 0x01, // answer to file list command
                                        // followed by the SScriptEditor_LisFiles structure
                                        
  ScriptEditor_Answer_SaveFile_Success = 0x02, // the save file was successfuly completed

  ScriptEditor_Answer_LoadFile = 0x03, // answer to the load file command
                                       // followed by the SScriptEditor_LoadFile_Answer structure

  ScriptEditor_Answer_GetProfilingInfo = 0x04, // answer to the get profile info
                                               // followed by the SScriptEditor_GetProfilingInfo_Answer structure

} EScriptEditorAnswers;

typedef struct tagSScriptEditor_ListFiles{
  DWORD m_dwLocalLength; // length of the string (with NULL)
  DWORD m_dwGlobalLength; // length of the global string (with NULL)
  // and follows the local string (with NULL) and then the global string (with NULL)
} SScriptEditor_ListFiles;

typedef struct tagSScriptEditor_SaveFile{
  DWORD m_dwFileNameLength; // length of the file name (with NULL)
  DWORD m_dwFileLength; // length of the file (with NULL)
  // and follows the file name (with NULL)
  // and then the file (with NULL)
} SScriptEditor_SaveFile;

// types of script files for loading
typedef enum{
  ScriptEditor_LoadFile_UserFile = 0,
  ScriptEditor_LoadFile_SystemFile = 1,
} EScriptEditor_LoadFile_FileTypes;

// load file request
typedef struct tagSScriptEditor_LoadFile_Command{
  DWORD m_dwFileNameLength; // length of the file name (with NULL)
  EScriptEditor_LoadFile_FileTypes m_eFileTypes; // type of the script
  // followed by the file name (with NULL)
} SScriptEditor_LoadFile_Command;

typedef struct tagSScriptEditor_LoadFile_Answer{
  DWORD m_dwFileNameLength; // length of the file name (with NULL)
  DWORD m_dwFileLength; // length of the file (with NULL)
  // and follows the file name (with NULL)
  // and then the file (with NULL)
} SScriptEditor_LoadFile_Answer;

typedef struct tagSScriptEditor_Compile_Command{
  DWORD m_dwUpdate;         // if 0 - no update, elseway - update the script after the compile
  DWORD m_dwFileNameLength; // length of the file name (with NULL)
  // and follows the file name (with NULL) to compile (and/or) update
} SScriptEditor_Compile_Command;

typedef struct tagSScriptEditor_GetProfilingInfo_Answer{
  DWORD m_dwLength;    // Length of the profiling info string (with NULL)
  __int64 m_nLocalCounter;  // Local instruction counter
  __int64 m_nGlobalCounter; // Global instruction counter
  // and follows the string (with NULL)
} SScriptEditor_GetProfilingInfo_Answer;

#endif