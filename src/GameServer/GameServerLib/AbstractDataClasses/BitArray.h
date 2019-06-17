/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Bitov� pole
 * 
 ***********************************************************/

#ifndef __BIT_ARRAY__HEADER_INCLUDED__
#define __BIT_ARRAY__HEADER_INCLUDED__

#include <afxwin.h>

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// T��da bitov�ho pole pevn� d�lky (tj. jeho velikost je zn�m� p�i 
//		vytv��en� instance objektu)
class CBitArray 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CBitArray ();
	// destruktor
	~CBitArray ();

// Inicializace a zni�en� dat objektu

	// vytvo�� bitov� pole velikosti "dwSize"
	//		v�jimky: CMemoryException
	void Create ( DWORD dwSize );
	// inicializuje bitov� pole velikosti "dwSize" na hodnotu "bValue"
	//		v�jimky: CMemoryException
	void Create ( DWORD dwSize, BOOL bValue );
	// zni�� bitov� pole
	void Delete ();
	// vr�t� p��znak zni�en� dat
	BOOL IsDeleted () { return ( m_pBitArray == NULL ); };

	// inicializuje bitov� pole "pBitArray" velikosti "dwSize"
#ifdef _DEBUG
	void Assign ( BYTE *pBitArray, DWORD dwSize );
#else
	void Assign ( BYTE *pBitArray );
#endif
	// zni�� bitov� pole, ale nema�e vlastn� data (pro vn�j�� alokaci pam�ti)
	void DeleteAssigned ();

// Operace s bity

	// vr�t� hodnotu bitu "dwIndex" v poli bit�
	BOOL operator [] ( DWORD dwIndex ) const;
	// vr�t� hodnotu bitu "dwIndex" v poli bit�
	BOOL GetAt ( DWORD dwIndex ) const;
	// nastav� bit "dwIndex" (na hodnotu 1)
	void SetAt ( DWORD dwIndex );
	// vynuluje bit "dwIndex" (na hodnotu 0)
	void ClearAt ( DWORD dwIndex );
	// nastav� hodnotu bitu "dwIndex" v poli bit� na novou hodnotu "bNewValue"
	void SetAt ( DWORD dwIndex, BOOL bNewValue );

// Operace s cel�m polem bit�

	// vr�t� ukazatel na pole bit� (za ��elem save/load pole)
	const BYTE *GetData () const { return m_pBitArray; };

// Data
private:
	// ukazatel na pole bit�
	BYTE *m_pBitArray;

#ifdef _DEBUG
	// velikost pole bit�
	DWORD m_dwSize;
	// p��znak vytvo�en� bitov�ho pole metodou "Assign"
	BOOL m_bAssigned;
#endif //_DEBUG
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CBitArray::CBitArray () 
{
	// zne�kodn� data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

// destruktor
inline CBitArray::~CBitArray ()
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�� bitov� pole velikosti "dwSize"
//		v�jimky: CMemoryException
inline void CBitArray::Create ( DWORD dwSize ) 
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// alokuje pot�ebnou pam� pro bitov� pole
	m_pBitArray = new BYTE[(dwSize+7)/8];

#ifdef _DEBUG
	// inicializuje velikost alokovan�ho pole
	m_dwSize = dwSize;
	// nastav� p��znak alokace bitov�ho pole metodou "Assign"
	m_bAssigned = FALSE;
#endif //_DEBUG
}

// inicializuje bitov� pole velikosti "dwSize" na hodnotu "bValue"
//		v�jimky: CMemoryException
inline void CBitArray::Create ( DWORD dwSize, BOOL bValue ) 
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// alokuje pot�ebnou pam� pro bitov� pole
	m_pBitArray = new BYTE[(dwSize+7)/8];

#ifdef _DEBUG
	// inicializuje velikost alokovan�ho pole
	m_dwSize = dwSize;
	// nastav� p��znak alokace bitov�ho pole metodou "Assign"
	m_bAssigned = FALSE;
#endif //_DEBUG

	// vypln� pole bit� p��slu�nou hodnotou
	memset ( m_pBitArray, ( bValue ? 0xff : 0x00 ), (dwSize+7)/8 );
}

// zni�� bitov� pole
inline void CBitArray::Delete () 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( m_dwSize > 0 );
	ASSERT ( !m_bAssigned );

	// zni�� alokovanou pam�
	delete []m_pBitArray;

	// zne�kodn� data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

// inicializuje bitov� pole "pBitArray" velikosti "dwSize"
#ifdef _DEBUG
inline void CBitArray::Assign ( BYTE *pBitArray, DWORD dwSize ) 
#else
inline void CBitArray::Assign ( BYTE *pBitArray ) 
#endif
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// uschov� si ukazatel na bitov� pole
	m_pBitArray = pBitArray;

#ifdef _DEBUG
	// inicializuje velikost alokovan�ho pole
	m_dwSize = dwSize;
	// nastav� p��znak alokace bitov�ho pole metodou "Assign"
	m_bAssigned = TRUE;
#endif //_DEBUG
}

// zni�� bitov� pole, ale nema�e vlastn� data (pro vn�j�� alokaci pam�ti)
inline void CBitArray::DeleteAssigned () 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( m_dwSize > 0 );
	ASSERT ( m_bAssigned );

	// zne�kodn� data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vr�t� hodnotu bitu "dwIndex" v poli bit�
inline BOOL CBitArray::operator [] ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// vr�t� hodnotu bitu "dwIndex" v poli bit�
inline BOOL CBitArray::GetAt ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// nastav� bit "dwIndex" (na hodnotu 1)
inline void CBitArray::SetAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
	m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// vynuluje bit "dwIndex" (na hodnotu 0)
inline void CBitArray::ClearAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
	m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// nastav� hodnotu bitu "dwIndex" v poli bit� na novou hodnotu "bNewValue"
inline void CBitArray::SetAt ( DWORD dwIndex, BOOL bNewValue ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zjist�, je-li nov� hodnota 1
	if ( bNewValue )
	{	// nov� hodnota bitu m� b�t 1
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
		m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
	else
	{	// nov� hodnota bitu m� b�t 0
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
		m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
}

#endif //__BIT_ARRAY__HEADER_INCLUDED__
