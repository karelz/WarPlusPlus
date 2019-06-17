// EditErrorOutput.cpp: implementation of the CEditErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameServerApp.h"
#include "EditErrorOutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEditErrorOutput::CEditErrorOutput()
{
  m_pEdit = NULL;
}

CEditErrorOutput::~CEditErrorOutput()
{

}

void CEditErrorOutput::SetEdit(CEdit *pEdit)
{
  m_pEdit = pEdit;

  if(m_pEdit == NULL)
    m_bOpen = false;
  else
    m_bOpen = true;
}

void CEditErrorOutput::PutChars(const char *pBuffer, int nCount)
{
  if(m_pEdit == NULL) return;

  CString str;
  char *pBuf = str.GetBuffer(nCount + 1);
  strncpy(pBuf, pBuffer, nCount);
  pBuf[nCount] = 0;
  str.ReleaseBuffer();

  int nPos = 0;
  while(nPos != -1){
    nPos = str.Find(0x0A, nPos + 1);
    if(nPos > 0){
      if(str[nPos - 1] != 0x0D){
        str.Insert(nPos, 0x0D);
      }
    }
  }

  m_pEdit->SetSel(m_pEdit->GetWindowTextLength(), m_pEdit->GetWindowTextLength(), FALSE);
  m_pEdit->ReplaceSel(str, FALSE);
  m_pEdit->UpdateWindow();
}

void CEditErrorOutput::Flush()
{
}