// LogFile.h: interface for the CLogFile class
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGFILE__HEADER_INCLUDED_)
#define _LOGFILE__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// define fictive class CLock
#ifdef _DEBUG
#define CLock CSemaphore
#else
#define CLock CCriticalSection
#endif

class CLogFile : public CObserver 
{
    DECLARE_DYNAMIC(CLogFile)

public:
    // default constructor
	CLogFile();
    // destructor
	virtual ~CLogFile();

    // create object of this class
    // parameter should be a CFile opened for reading and writing
    virtual void Create(const CArchiveFile &LogFile);
    // final flush
    virtual void Delete();

    // adds a record to the logfile
    virtual void AddRecord(LPCSTR pStringFormat);

    // adds a record to the logfile - formated record
    virtual void AddRecordFormat(LPCSTR pString, ...);

    // adds a record to the logfile - formated record II
    virtual void AddRecordFormat(LPCSTR pString, va_list args);

    // clears the log file
    virtual void Clear();

    // lists the actual content of logfile to CString
    virtual void List(const CVirtualConnection &VirtualConnection, BOOL bAutoUpdate = FALSE);
    // stops automatic updates
    virtual BOOL StopAutoUpdate(const CVirtualConnection &VirtualConnection);

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

protected:
    // Create() was called
    BOOL m_bInitialized;

    // lock for the class (all methods must be serialized)
    CLock m_lockLogFile;

    // logfile object
    CArchiveFile m_LogFile;
    
    // structure for link list of virtual connections
    struct tagSVirtualConnectionList
    {
        tagSVirtualConnectionList *pNext;
        CVirtualConnection Connection;

        // default constructor
        tagSVirtualConnectionList() : pNext(NULL) { }
        // constructor with parameter
        tagSVirtualConnectionList(const CVirtualConnection &VirtualConnection) : pNext(NULL), Connection(VirtualConnection) { }
    };
    typedef tagSVirtualConnectionList SVirtualConnectionList, *PSVirtualConnectionList;

    // virtual connections for auto-updates
    PSVirtualConnectionList m_pConnections;
};

// undefine CLock
#undef CLock

#endif  // !defined(_LOGFILE__HEADER_INCLUDED_)
