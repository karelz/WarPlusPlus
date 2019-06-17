// ZScriptEditor.h: interface for the CZScriptEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZSCRIPTEDITOR_H__A999A055_02F3_11D4_8482_004F4E0004AA__INCLUDED_)
#define AFX_ZSCRIPTEDITOR_H__A999A055_02F3_11D4_8482_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZScriptErrorOutput.h"

// The server end of the network connection for script editor on the client
// Upload, download scripts, Compile, Compile & Update ...
class CZScriptEditor : public CObserver  
{
  DECLARE_DYNAMIC(CZScriptEditor);
  DECLARE_OBSERVER_MAP(CZScriptEditor);

public:
  // Constructor & Destructor
	CZScriptEditor();
	virtual ~CZScriptEditor();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // Creates the object for existing virtual connection
  void Create(CVirtualConnection VirtualConnection, CCompiler *pCompiler, CCodeManager *pCodeManager);
  // Deletes the object
  void Delete();

  // Creates the error output virtual connection
  void CreateErrorOutput(CVirtualConnection VirtualConnection);

protected:
  // Reactions on the network events
  void OnPacketAvailable();

private:
  // the code manager
  CCodeManager *m_pCodeManager;
  // the compiler
  CCompiler *m_pCompiler;

  // the virtual connection to the client
  CVirtualConnection m_VirtualConnection;

  // the error output
  CZScriptErrorOutput m_ErrorOutput;

  // sends and error packet to he client
  void SendError(CException *e);

  enum{ ID_VirtualConnection = 1, };

};

#endif // !defined(AFX_ZSCRIPTEDITOR_H__A999A055_02F3_11D4_8482_004F4E0004AA__INCLUDED_)
