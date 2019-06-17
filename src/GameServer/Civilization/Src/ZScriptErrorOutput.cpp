// ZScriptErrorOutput.cpp: implementation of the CZScriptErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZScriptErrorOutput.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CZScriptErrorOutput, CObserver);


// Constructor
CZScriptErrorOutput::CZScriptErrorOutput()
{

}

// Destructor
CZScriptErrorOutput::~CZScriptErrorOutput()
{

}


// Debug functions
#ifdef _DEBUG
  
void CZScriptErrorOutput::AssertValid() const
{
  CObserver::AssertValid();
}

void CZScriptErrorOutput::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
}

#endif


// Creation
void CZScriptErrorOutput::Create(CVirtualConnection VirtualConnection)
{
  m_VirtualConnection = VirtualConnection;
  m_VirtualConnection.SetVirtualConnectionObserver(this, 1);
}

// Deletion
void CZScriptErrorOutput::Delete()
{
  m_VirtualConnection.DeleteVirtualConnection();
}

// opens the error output
CCompilerErrorOutput *CZScriptErrorOutput::Open()
{
  m_CompilerOutput.Open(this);

  return &m_CompilerOutput;
}

// closes the error output
void CZScriptErrorOutput::Close()
{
  m_CompilerOutput.Close();
}