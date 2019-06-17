// ZScriptCompilerErrorOutput.cpp: implementation of the CZScriptCompilerErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZScriptCompilerErrorOutput.h"

#include "ZScriptErrorOutput.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZScriptCompilerErrorOutput::CZScriptCompilerErrorOutput()
{
  m_pErrorOutput = NULL;
}

CZScriptCompilerErrorOutput::~CZScriptCompilerErrorOutput()
{
  ASSERT(m_pErrorOutput == NULL);
}

void CZScriptCompilerErrorOutput::Open(CZScriptErrorOutput *pErrorOutput)
{
  m_pErrorOutput = pErrorOutput;
  m_bOpen = TRUE;

  // start the compound block
  VERIFY(m_pErrorOutput->m_VirtualConnection.BeginSendCompoundBlock());
}

void CZScriptCompilerErrorOutput::Close()
{
  if(m_bOpen){
    // close the compound block (this can return FALSE)
    // in fact it should return FALSE, cause it should be called previously from the Flush method
    m_pErrorOutput->m_VirtualConnection.EndSendCompoundBlock();

    m_bOpen = FALSE;
  }
  m_pErrorOutput = NULL;
}

void CZScriptCompilerErrorOutput::PutChars(const char *pBuffer, int nCount)
{
  // send the data to the virtual connection
  m_pErrorOutput->m_VirtualConnection.SendBlock(pBuffer, nCount);
}

void CZScriptCompilerErrorOutput::Flush()
{
  // just end the compound block
  VERIFY(m_pErrorOutput->m_VirtualConnection.EndSendCompoundBlock());

  m_bOpen = FALSE;
}