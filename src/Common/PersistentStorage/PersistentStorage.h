// PersistentStorage.h: interface for the CPersistentStorage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTENTSTORAGE_H__75673153_E965_11D3_844B_004F4E0004AA__INCLUDED_)
#define AFX_PERSISTENTSTORAGE_H__75673153_E965_11D3_844B_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////
// Pokud je definovane toto makro, je pri ukladani ke kazde polozce
// ukladana jako tag jeji velikost, pouzivane pro asserteni

#ifdef _DEBUG
#define PERSISTENT_STORAGE_TAGGED
#endif

#ifdef PERSISTENT_STORAGE_TAGGED
#define WRITE_TAG(tag) { BYTE mTag=tag; Write("Tag :-)", 8); Write(&mTag, sizeof(mTag)); }
#define READ_TAG(tag) { BYTE mTag; char tagtext[8]; Read(tagtext, 8); CString str=tagtext; if(str!="Tag :-)") { TRACE("Nenasel jsem Tag :-)\n"); ASSERT(FALSE); } Read(&mTag, sizeof(mTag)); if(mTag!=tag) TRACE("Ocekavan typ %d, ale nahran typ %d\n", tag, mTag); ASSERT(mTag==tag); }
#else
#define WRITE_TAG(tag)
#define READ_TAG(tag)
#endif

////////////////////////////////////////////////////////////////////////

// class to store the savegame to archive

#include <afxtempl.h>

#include "Archive\Archive\Archive.h"

class CPersistentStorage : public CObject  
{
    DECLARE_DYNAMIC(CPersistentStorage);
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext &dc) const;
#endif
    
    // constructor
    CPersistentStorage();
    // destructor
    virtual ~CPersistentStorage();
    
    typedef enum { NONE, LOAD, SAVE} EStorageMode;
    
    // Opening the persistent storage (implemented in archive).
    // There must be full path in lpszFilename.
    bool Open( LPCTSTR lpszFilename, EStorageMode eMode);

    // Opening the persistent storage directly in given archive
    bool Open( CDataArchive archive, EStorageMode eMode);    

    // Close the storage.    
    void Close();
    
    // Called to write data to storage (during saving)
    // this should work like file write, it means you remeber the pointer to the file position
    // it means all writes are serialized (first go first, second go after it in the file and so on)
    virtual void Write(const void *pBuffer, DWORD dwLength);
    
    // Called to read data from storage (during load)
    // this should work similarly to Write (remebers the pointer)
    virtual void Read(void *pBuffer, DWORD dwLength);
    
    
    // Called by persistent objects in the Load faze to make a pair of pointers, the old one
    // and the new one - this should remember the pair in some table
    virtual void RegisterPointer(void *pOldPointer, void *pNewPointer);
    // Called by persistent objects to translate pointers
    // this should find the given pointer in the translation table and return the new pointer
    // also return NULL if the pointer was not found
    // it means also return NULL if you was given the NULL
    virtual void *TranslatePointer(void *pPointer);
    
    // This method returns the data archive the savegame is stored to or loaded from.  
    CDataArchive* GetArchive() { return &m_DataArchive; }
    
    CPersistentStorage& operator<<(BYTE value){ WRITE_TAG(1); Write(&value, sizeof(BYTE)); return *this; }
    CPersistentStorage& operator<<(LONG value){ WRITE_TAG(2); Write(&value, sizeof(LONG)); return *this; }
    CPersistentStorage& operator<<(DWORD value){ WRITE_TAG(3); Write(&value, sizeof(DWORD)); return *this; }
    CPersistentStorage& operator<<(WORD value){ WRITE_TAG(4); Write(&value, sizeof(WORD)); return *this; }
    CPersistentStorage& operator<<(int value){ WRITE_TAG(5); Write(&value, sizeof(int)); return *this; }
    CPersistentStorage& operator<<(float value){ WRITE_TAG(6); Write(&value, sizeof(float)); return *this; }
    CPersistentStorage& operator<<(double value){ WRITE_TAG(7); Write(&value, sizeof(double)); return *this; }
    CPersistentStorage& operator<<(void *value){ WRITE_TAG(8); Write(&value, sizeof(void *)); return *this; }
    CPersistentStorage& operator<<(CString str){ 
        WRITE_TAG(9); 
        DWORD dwLength=str.GetLength();
        Write(&dwLength, sizeof(dwLength));
        LPTSTR pBuf=str.GetBuffer(dwLength);
        Write(pBuf, dwLength);
        str.ReleaseBuffer(dwLength);
        return *this; 
    }
    CPersistentStorage& operator<<(bool value){ WRITE_TAG(10); Write(&value, sizeof(bool)); return *this; }
    
    CPersistentStorage& operator>>(BYTE &value){ READ_TAG(1); Read(&value, sizeof(BYTE)); return *this; }
    CPersistentStorage& operator>>(LONG &value){ READ_TAG(2); Read(&value, sizeof(LONG)); return *this; }
    CPersistentStorage& operator>>(DWORD &value){ READ_TAG(3); Read(&value, sizeof(DWORD)); return *this; }
    CPersistentStorage& operator>>(WORD &value){ READ_TAG(4); Read(&value, sizeof(WORD)); return *this; }
    CPersistentStorage& operator>>(int &value){ READ_TAG(5); Read(&value, sizeof(int)); return *this; }
    CPersistentStorage& operator>>(float &value){ READ_TAG(6); Read(&value, sizeof(float)); return *this; }
    CPersistentStorage& operator>>(double &value){ READ_TAG(7); Read(&value, sizeof(double)); return *this; }
    CPersistentStorage& operator>>(void *&value){ READ_TAG(8); Read(&value, sizeof(void *)); return *this; }
    CPersistentStorage& operator>>(CString &str){
        READ_TAG(9);
        DWORD dwLength;
        Read(&dwLength, sizeof(dwLength));
        LPTSTR pBuf=str.GetBufferSetLength(dwLength+1);
        Read(pBuf, dwLength);
        str.ReleaseBuffer(dwLength);
        return *this;       
    }
    CPersistentStorage& operator>>(bool &value){ READ_TAG(10); Read(&value, sizeof(bool)); return *this; }

    EStorageMode GetMode() { return m_eStorageMode; }
    
protected:
    // current archive
    CDataArchive m_DataArchive;
    //  the 'savegame' file in archive
    CArchiveFile m_ArchiveFile; 
    // storage mode
    EStorageMode m_eStorageMode;

    // hashovaci tabulka pro prevadeni pointeru
    CTypedPtrMap<CMapPtrToPtr, void*, void*> m_tpPointers;

    enum { HASH_TABLE_SIZE = 11013 };
};

#ifdef PERSISTENT_STORAGE_TAGGED

class CPersistentStorageAutoBracing
{
public:
    CPersistentStorageAutoBracing(CPersistentStorage &storage) {
        m_pStorage=&storage;
        if(storage.GetMode()==CPersistentStorage::SAVE) {
            g_dwBraceNumber++;
            storage.Write("OPEN((((", 9); 
            storage.Write(&g_dwBraceNumber, sizeof(g_dwBraceNumber));
        } else { 
            char maso[9]; DWORD dwBraceNumber;
            storage.Read(maso, 9); 
            storage.Read(&dwBraceNumber, sizeof(dwBraceNumber));
            if(strcmp(maso, "OPEN((((")!=0 || dwBraceNumber!=g_dwBraceNumber+1) ASSERT(FALSE);
            g_dwBraceNumber=dwBraceNumber;
        }
    }

    ~CPersistentStorageAutoBracing() {
        if(m_pStorage->GetMode()==CPersistentStorage::SAVE) {
            g_dwBraceNumber++;
            m_pStorage->Write("CLOSE)))", 9);
            m_pStorage->Write(&g_dwBraceNumber, sizeof(g_dwBraceNumber));
        } else { 
            char kafe[9]; DWORD dwBraceNumber;
            m_pStorage->Read(kafe, 9);
            m_pStorage->Read(&dwBraceNumber, sizeof(dwBraceNumber));
            if(strcmp(kafe, "CLOSE)))")!=0 || dwBraceNumber!=g_dwBraceNumber+1) ASSERT(FALSE);
            g_dwBraceNumber=dwBraceNumber;
        }
    }

public:
    static DWORD g_dwBraceNumber;

private:
    CPersistentStorage *m_pStorage;
};

#define BRACE_BLOCK(storage) CPersistentStorageAutoBracing temporaryAutoBracingObjectMadeByRoman(storage);
#else // !PERSISTENT_STORAGE_TAGGED
#define BRACE_BLOCK(storage)
#endif // PERSISTENT_STORAGE_TAGGED

#endif // !defined(AFX_PERSISTENTSTORAGE_H__75673153_E965_11D3_844B_004F4E0004AA__INCLUDED_)
