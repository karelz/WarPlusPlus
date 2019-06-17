/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret, CodeManager, Civilization, GameServer
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CStringTable a CStringTableItem,
 *          globalnich tabulek stringu.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CICommon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//
// Globalni stringtable
//
CStringTable g_StringTable;

CMultiThreadTypedMemoryPool<CStringTableItem> *CStringTable::m_pPool = NULL;
DWORD CStringTable::m_dwPoolReferenceCount = 0;
//////////////////////////////////////////////////////////////////////
// CStringTableItem
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

void CStringTableItem::Initialize(CString &strString)
{
    m_strString=strString;
    m_nRefCount=0;
}

CStringTableItem::operator LPCTSTR() const
{
    return (LPCTSTR)m_strString;
}

CStringTableItem::operator CString() const
{
    return m_strString;
}

CStringTableItem* CStringTableItem::AddRef()
{
	g_StringTable.m_StringTableLock.Lock();    
    m_nRefCount++;
	g_StringTable.m_StringTableLock.Unlock();    
    return this;
}

CStringTableItem* CStringTableItem::Release()
{
	g_StringTable.m_StringTableLock.Lock();    
	m_nRefCount--;

#ifdef _DEBUG
//	if (m_nRefCount < 0) TRACE("CStringTableItem - negative refcount: %d  \"%s\"\n", m_nRefCount, m_strString);
#endif;

#ifndef _DEBUG
	if (m_nRefCount == 0) g_StringTable.ReleaseItem(this);
#endif
	g_StringTable.m_StringTableLock.Unlock();    

    return NULL;
}

#ifdef _DEBUG
void CStringTableItem::Dump(CDumpContext& dc) const
{
    CObject::Dump(dc);
    dc << "m_nRefCount: " << m_nRefCount << "   String: " << m_strString;
}
#endif

//////////////////////////////////////////////////////////////////////
// CStringTable
//////////////////////////////////////////////////////////////////////

CStringTable::CStringTable()
    : m_lockPool()
{
	VERIFY(m_lockPool.Lock());
	if(m_pPool == NULL)
	{
		ASSERT(m_dwPoolReferenceCount == 0);
		m_pPool = new CMultiThreadTypedMemoryPool<CStringTableItem>(100);
	}
	m_dwPoolReferenceCount++;
	VERIFY(m_lockPool.Unlock());
	
	m_apSTITable.InitHashTable(HASHTABLE_SIZE);
	
	m_nItemCount = 0;
}

CStringTable::~CStringTable()
{
    RemoveAll();

	VERIFY(m_lockPool.Lock());
    ASSERT(m_pPool != NULL);
    m_dwPoolReferenceCount--;
    if(m_dwPoolReferenceCount == 0){
      delete m_pPool;
      m_pPool = NULL;
    }
	VERIFY(m_lockPool.Unlock());
}

CStringTableItem* CStringTable::AddItem(CString &strString, bool bAddRef/*=true*/)
{
    CStringTableItem *pSTI;

    m_StringTableLock.Lock();
    if (!m_apSTITable.Lookup(strString, pSTI)) 
	{
        // je to poolovane :-)
        pSTI = new CStringTableItem;

        pSTI->Initialize(strString);
        m_apSTITable.SetAt(strString, pSTI);

		m_nItemCount++;
    }
    
    if (bAddRef) pSTI->AddRef();

    m_StringTableLock.Unlock();    

    return pSTI;
}   

CStringTableItem* CStringTable::AddItem(LPCTSTR string, bool bAddRef/*=true*/)
{
	return AddItem(CString(string), bAddRef);
}

void CStringTable::ReleaseItem(CStringTableItem* pSTI)
{
	// the m_StringTableLock is already locked from CStringTableItem::ReleaseItem();

	if ( pSTI->m_nRefCount != 0)
	{
		_asm { int 3 }
	}

    m_apSTITable.RemoveKey(pSTI->m_strString);
	m_nItemCount--;

    delete pSTI;
}

void CStringTable::RemoveAll()
{
   POSITION pos;
   CString Str;
   CStringTableItem *pSTI;
   
   m_StringTableLock.Lock();
   
   pos = m_apSTITable.GetStartPosition();
   while (pos != NULL)
   {
	   m_apSTITable.GetNextAssoc( pos, Str, pSTI);
#ifdef _DEBUG
	   if ( pSTI->m_nRefCount != 0)
	   {
//		   TRACE("Unreferenced CStringTableItem: refcount = %d  \"%s\"\n", pSTI->m_nRefCount, pSTI->m_strString);
	   }
#endif
       delete pSTI;
   }
    m_apSTITable.RemoveAll();

    m_StringTableLock.Unlock();
}

//
// SAVE & LOAD
//
void CStringTable::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    m_StringTableLock.Lock();

	// pointer na sebe, pro jistotu
	storage << (DWORD)this;

	// polozky
	POSITION pos;
	CStringTableItem *stiItem;
	CString strString;

	storage << m_nItemCount;

	long nCount = 0;
	pos = m_apSTITable.GetStartPosition();
	while (pos != NULL)
	{
		m_apSTITable.GetNextAssoc( pos, strString, stiItem);
	    
        // save string table item
        storage << (DWORD)stiItem;
	    storage << stiItem->m_nRefCount;
	    storage << stiItem->m_strString.GetLength();
	    storage.Write( (LPCTSTR)stiItem->m_strString, stiItem->m_strString.GetLength());

        nCount++;
	}

	ASSERT( m_nItemCount == nCount);

    m_StringTableLock.Unlock();
}

void CStringTable::PersistentLoad( CPersistentStorage &storage)
{ 
    BRACE_BLOCK(storage);

    m_StringTableLock.Lock();

	// pointer na sebe, pro jistotu
	CStringTable *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// polozky
	CStringTableItem *stiOldItem, *stiItem;
    CString strString;
    long nRefCount, nSavedItemCount, i;
    int len;
    LPTSTR pBuffer;

	storage >> nSavedItemCount;
	for (i=0; i<nSavedItemCount; i++)
	{
	    storage >> (DWORD&)stiOldItem;
	    storage >> nRefCount;
	    storage >> len;

        pBuffer = strString.GetBufferSetLength(len);
        storage.Read( pBuffer, len);
        pBuffer[len]=0;
        strString.ReleaseBuffer(len);

        if (m_apSTITable.Lookup( strString, stiItem))
        // uz tam je
        {
            storage.RegisterPointer( stiOldItem, stiItem);
            stiItem->m_nRefCount = nRefCount - stiItem->m_nRefCount;
        }
        else
        // neni tam
        {
    		stiItem = new CStringTableItem;
            stiItem->Initialize(strString);
            stiItem->m_nRefCount = nRefCount;
		    m_apSTITable.SetAt( strString, stiItem);

            storage.RegisterPointer( stiOldItem, stiItem);

            m_nItemCount++;
        }
	}

    m_StringTableLock.Unlock();
}

void CStringTable::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// ve CStringTableItem neni treba nic prekladat -> empty
}

void CStringTable::PersistentInit()
{
	// ve CStringTableItem neni treba nic inicializovat -> empty
}
