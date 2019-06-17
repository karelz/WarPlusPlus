// ZLogOutput.h: interface for the CZLogOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZLOGOUTPUT_H__74A17406_1DB4_11D4_84C0_004F4E0004AA__INCLUDED_)
#define AFX_ZLOGOUTPUT_H__74A17406_1DB4_11D4_84C0_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogFile\LogFile.h"

class CZLogOutput : public CObserver
{
  DECLARE_DYNAMIC(CZLogOutput);
  DECLARE_OBSERVER_MAP(CZLogOutput);

public:
  // constructor & destructor
	CZLogOutput();
  virtual ~CZLogOutput();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // creation
  void Create(CVirtualConnection VirtualConnection, CLogFile *pLogFile);
  virtual void Delete();

protected:
  void OnPacketAvailable();
  void OnVirtualConnectionClosed();
  void OnNetworkError();

private:
  // virtual connection
  CVirtualConnection m_VirtualConnection;
  // log file
  CLogFile *m_pLogFile;

  enum{ ID_VirtualConnection = 0x0100, };
};

#endif // !defined(AFX_ZLOGOUTPUT_H__74A17406_1DB4_11D4_84C0_004F4E0004AA__INCLUDED_)
