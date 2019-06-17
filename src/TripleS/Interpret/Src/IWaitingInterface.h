// IWaitingInterface.h: interface for the CIWaitingInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IWAITINGINTERFACE_H__04FDFD83_EC32_11D3_AFF2_004F49068BD6__INCLUDED_)
#define AFX_IWAITINGINTERFACE_H__04FDFD83_EC32_11D3_AFF2_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIWaitingInterface  :  public CNotifier
{
	DECLARE_DYNAMIC ( CIWaitingInterface )

public:
    // konstruktor a destruktor
    CIWaitingInterface()            {  m_nRefCount = 0; m_eWaitingType = ERROR_WAITING; }
    virtual ~CIWaitingInterface()   {  ASSERT( m_nRefCount == 0); }

    // zvyseni a snizeni counteru referenci
    CIWaitingInterface* AddRef();
    CIWaitingInterface* Release();

    // vytvoreni a zruseni objektu
    virtual bool Create();
    virtual void Delete();

    // Pridani procesu, ktery ceka, az bude prace dokoncena.
    // !!Muze byt volano pouze ze threadu intepreta!! (kvuli Connectu na spravny thread)
    void AddWaitingProcess( CIProcess *pProcess)
    {
        Connect( pProcess, WAITING_INTERFACE_NOTIFIER_ID);
    }

    // Ukonceni cekani, tohle vola cinnost, na kterou se ceka. 
    // Typicky to bude probouzet vsechny spici procesy
    // (ale nemusi, napr. cekani az se vyridi vsechny eventy, bude proces(y)
    // probouzet, az se vsechny dokonci, ale FinishWaiting vola 
    // kazda z eventa pri dokonceni).
    virtual void FinishWaiting( EInterpretStatus eStatus) = 0;

	// zruseni cekani jednoho procesu - vola intepret (process)
	virtual void Cancel( CIProcess *pProcess) { Disconnect( pProcess); }
	
protected:
    // reference counter
    long m_nRefCount;
public:
	// typ cekani
	typedef  enum { ERROR_WAITING, EVENT_WAITING_1ST, EVENT_WAITING_ALL, CONSTRUCTOR_WAITING, SYSCALL_WAITING } EWaitingType;
	EWaitingType m_eWaitingType;

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_IWAITINGINTERFACE_H__04FDFD83_EC32_11D3_AFF2_004F49068BD6__INCLUDED_)
