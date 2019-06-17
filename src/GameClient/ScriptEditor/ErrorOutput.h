// ErrorOutput.h: interface for the CErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERROROUTPUT_H__472EBF93_0280_11D4_8481_004F4E0004AA__INCLUDED_)
#define AFX_ERROROUTPUT_H__472EBF93_0280_11D4_8481_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ErrorOutputEdit.h"

// the error output window
class CErrorOutput : public CFrameWindow  
{
  DECLARE_DYNAMIC(CErrorOutput);
  DECLARE_OBSERVER_MAP(CErrorOutput);

public:
// Constructor & destructor
	CErrorOutput();
	virtual ~CErrorOutput();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // Creates the window
  void Create(CScriptEditLayout *pScriptEditLayout, CFrameWindowLayout *pFrameLayout,
    CRect rcWindow, CWindow *pParent);

  // Deletes it
  virtual void Delete();

  // insert text to the error output window
  void AddText(CString &strText);

  // clears it
  void Clear();

  // jumps to the next error
  void NextError();
  // jumps to the prev error
  void PrevError();

  enum{
    E_SelectErrorLine = 0x0100,
  };

  CString GetErrorFileName(){ ASSERT_VALID(this); return m_strErrorFileName; }
  DWORD GetErrorFileLine(){ ASSERT_VALID(this); return m_dwErrorFileLine; }

protected:
  // reactions on network events
  void OnPacketAvailable();

  // reactions on our events
  void OnAddBuffer(char *pBuffer);

  // reactions on event from the edit window
  void OnLineSelected();

  enum{
    ID_VirtualConnection = 1,
    ID_Edit = 2,
    ID_Myself = 0x0FFFF,
  };

  enum{
    E_AddBuffer = 1,
  };

private:
  // the script edit control - the true error output
  CErrorOutputEdit m_ScriptEdit;

  // The virtual connection
  CVirtualConnection m_VirtualConnection;

  // parses the given line for error
  // returns TRUE if there was some error
  //   in that case it sets m_strErrorFileName and m_dwErrorFileLine
  BOOL ParseErrorLine(DWORD dwLine);

// Error finding
  // last line with some error
  DWORD m_dwLastErrorLine;

  // the last file with error
  CString m_strErrorFileName;
  // the last line in the file with error
  DWORD m_dwErrorFileLine;
};

#endif // !defined(AFX_ERROROUTPUT_H__472EBF93_0280_11D4_8481_004F4E0004AA__INCLUDED_)
