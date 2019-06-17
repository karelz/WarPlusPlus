// LogFile.cpp: implementation of the CLogFile class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFile.h"

IMPLEMENT_DYNAMIC(CLogFile, CObserver)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// constructor
CLogFile::CLogFile() :
    m_bInitialized(FALSE),
    m_pConnections(NULL)
{
}

// destructor
CLogFile::~CLogFile()
{
    // call Delete() method
    Delete();
}


//////////////////////////////////////////////////////////////////////
// Create/Delete
//////////////////////////////////////////////////////////////////////

// create object of this class
// parameter should be a CFile opened for reading and writing
void CLogFile::Create(const CArchiveFile &LogFile)
{
    // object must not be initialized
    ASSERT(!m_bInitialized);
    
    // copy a CFile object
    m_LogFile = LogFile;

    // consider this object as initialized
    m_bInitialized = TRUE;
}

// final flush
void CLogFile::Delete()
{
    if (!m_bInitialized)
        return;

    // lock the object
    VERIFY(m_lockLogFile.Lock());
    
    // flush the content of logfile (probably nothing to do)
    m_LogFile.Flush();

#ifdef _DEBUG
    if (m_pConnections != NULL)
    {
        TRACE0("CLogFile::Delete() - there are still active network connections for auto-updates!\n");
    }
#endif

    while (m_pConnections != NULL)
    {
        // save the pointer to the item being deleted
        PSVirtualConnectionList pDelete = m_pConnections;

        // move the "first" pointer
        m_pConnections = m_pConnections->pNext;

        // delete saved item
        delete pDelete;
    }

    // unlock this object
    VERIFY(m_lockLogFile.Unlock());

    // consider this object as not initialized
    m_bInitialized = FALSE;
}


//////////////////////////////////////////////////////////////////////
// Operations on the log file
//////////////////////////////////////////////////////////////////////

void CLogFile::AddRecordFormat(LPCSTR pStringFormat, ...)
{
    va_list args;
    va_start(args, pStringFormat);
    AddRecordFormat(pStringFormat, args);
}

void CLogFile::AddRecordFormat(LPCSTR pString, va_list args)
{
    CString str;
    str.FormatV( pString, args);
    AddRecord( str);
}

void CLogFile::AddRecord(LPCSTR pString)
{
    ASSERT_VALID(this);

    // compute the length of string
    DWORD dwRecordSize = strlen(pString);

    // empty string or something...
    if (dwRecordSize == 0)
        return;
    
    // lock
    VERIFY(m_lockLogFile.Lock());

    // get the actual size of the logfile
    DWORD dwLogSize = m_LogFile.GetPosition();

    // prepare buffer (incl. trailing zero)
    LPSTR pBuffer = new CHAR[dwLogSize + 1];
    
    // seek to the beginning of the logfile
    m_LogFile.SeekToBegin();

    // read the whole file
    if (dwLogSize != 0) VERIFY(m_LogFile.Read(pBuffer, dwLogSize) == dwLogSize);

    // set the trailing zero
    pBuffer[dwLogSize] = '\0';

    // prepare temporary pointers for string compare
    LPCSTR pLog = pBuffer, pRec = pString;

    // result of comparison
    BOOL bIdentical = FALSE;
    
    while (*pLog != '\0')
    {
        while ((*pRec != '\0') && (*pLog != '\0') && (*pLog == *pRec))
        {
            pLog++;
            pRec++;
        }

        // are those strings identical?
        if ((*pRec == '\0') && ((*(pRec - 1) == '\n') || (*pLog == '\n')))
        {
            bIdentical = TRUE;
            break;
        }

        // reinitialize pointers
        pRec = pString;
        
        while ((*pLog != '\0') && (*pLog != '\n'))
            pLog++;

        if (*pLog != '\0')
            pLog++;
    }

    delete [] pBuffer;

    if (!bIdentical)
    {
        // record not found, add it...
        m_LogFile.Write(pString, dwRecordSize);

        // check the trailing \n
        if (pString[dwRecordSize - 1] != '\n')
            m_LogFile.Write("\n", 1);

        // flush the log file
        // m_LogFile.Flush();

        // Send it to network
        PSVirtualConnectionList pItem = m_pConnections;
        while(pItem != NULL){
          VERIFY(pItem->Connection.BeginSendCompoundBlock());
          VERIFY(pItem->Connection.SendBlock(pString, dwRecordSize));
          // check the trailing \n
          if (pString[dwRecordSize - 1] != '\n')
            VERIFY(pItem->Connection.SendBlock("\n", 1));

          VERIFY(pItem->Connection.EndSendCompoundBlock());

          pItem = pItem->pNext;
        }
    }
    
    // unlock
    VERIFY(m_lockLogFile.Unlock());
}


// clears the log file
void CLogFile::Clear()
{
    ASSERT_VALID(this);

    // lock
    VERIFY(m_lockLogFile.Lock());

    // seek to the beginning of the file
    m_LogFile.SeekToBegin();

    // delete the content of the file by setting the size to 0
    m_LogFile.SetLength(0);

    // unlock
    VERIFY(m_lockLogFile.Unlock());
}


// lists the actual content of logfile to CString
void CLogFile::List(const CVirtualConnection &VirtualConnection, BOOL bAutoUpdate)
{
    ASSERT_VALID(this);

    // lock
    VERIFY(m_lockLogFile.Lock());

    // get the actual size of the logfile
    DWORD dwSize = m_LogFile.GetLength();

    // prepare buffer (incl. trailing zero)
    BYTE *pBuffer = new BYTE[dwSize + 1];
    
    // seek to the beginning of the logfile
    m_LogFile.SeekToBegin();

    // read the whole file
    VERIFY(m_LogFile.Read(pBuffer, dwSize) == dwSize);

    // set the trailing zero
    pBuffer[dwSize] = '\0';

    // send data over the network
    VirtualConnection.SendBlock(pBuffer, dwSize + 1);

    // delete the buffer
    delete [] pBuffer;

    if (bAutoUpdate)
    {
        // add the connection to the list

        PSVirtualConnectionList *ppItem = &m_pConnections;

        while ((*ppItem != NULL) && ((*ppItem)->Connection != VirtualConnection))
            ppItem = &((*ppItem)->pNext);

        if (*ppItem == NULL)
        {
            // proceed
            *ppItem = new SVirtualConnectionList(VirtualConnection);
        }
        else
        {
            // paranoia
            ASSERT((*ppItem)->Connection == VirtualConnection);
        }
    }
    else
    {
        // remove the connection from the list

        PSVirtualConnectionList *ppItem = &m_pConnections;

        while ((*ppItem != NULL) && ((*ppItem)->Connection != VirtualConnection))
            ppItem = &((*ppItem)->pNext);

        if (*ppItem != NULL)
        {
            // proceed

            // paranoia
            ASSERT((*ppItem)->Connection == VirtualConnection);

            // save the pointer
            PSVirtualConnectionList pDelete = *ppItem;

            // remove item from the list
            *ppItem = (*ppItem)->pNext;

            // delete the item
            delete pDelete;
        }
    }
    
    // unlock
    VERIFY(m_lockLogFile.Unlock());
}


// stops automatic updates
BOOL CLogFile::StopAutoUpdate(const CVirtualConnection &VirtualConnection)
{
    ASSERT_VALID(this);

    // lock
    VERIFY(m_lockLogFile.Lock());

    // find the right virtual connection and delete it
    PSVirtualConnectionList *ppItem = &m_pConnections;

    // result variable
    BOOL bResult = FALSE;
    
    // find the connection
    while ((*ppItem != NULL) && ((*ppItem)->Connection != VirtualConnection))
        ppItem = &((*ppItem)->pNext);

    // have we found it?
    if (*ppItem != NULL)
    {
        // paranoia
        ASSERT((*ppItem)->Connection == VirtualConnection);

        // save the pointer to the deleted item
        PSVirtualConnectionList pDelete = *ppItem;

        // remove item from the list
        *ppItem = (*ppItem)->pNext;

        // delete saved item
        delete pDelete;

        // set the result to TRUE
        bResult = TRUE;
    }

    // unlock
    VERIFY(m_lockLogFile.Unlock());

    // return
    return bResult;
}


//////////////////////////////////////////////////////////////////////
// Debug methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// check content of object
void CLogFile::AssertValid() const
{
    CObserver::AssertValid();

    ASSERT(m_bInitialized);
}

// dump content of object
void CLogFile::Dump(CDumpContext &dc) const
{
    // inherited Dump
    CObserver::Dump(dc);

    // TODO: add more dumping here...
}

#endif  // _DEBUG
