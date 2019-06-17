// ZScriptErrorOutput.h: interface for the CZScriptErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZSCRIPTERROROUTPUT_H__3AA53665_03D0_11D4_A8EC_00105ACA8325__INCLUDED_)
#define AFX_ZSCRIPTERROROUTPUT_H__3AA53665_03D0_11D4_A8EC_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZScriptCompilerErrorOutput.h"

// class for error output from compiler
class CZScriptErrorOutput : public CObserver  
{
  DECLARE_DYNAMIC(CZScriptErrorOutput);

public:
  // constructor & destructor
	CZScriptErrorOutput();
	virtual ~CZScriptErrorOutput();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creation
  void Create(CVirtualConnection VirtualConnection);
  // Delete
  void Delete();

  // opens the compiler error output
  CCompilerErrorOutput *Open();
  // closes the compiler error output
  void Close();

private:
  // virtual connection
  CVirtualConnection m_VirtualConnection;

  // the compiler error output itself
  CZScriptCompilerErrorOutput m_CompilerOutput;

  friend class CZScriptCompilerErrorOutput;
};

#endif // !defined(AFX_ZSCRIPTERROROUTPUT_H__3AA53665_03D0_11D4_A8EC_00105ACA8325__INCLUDED_)
