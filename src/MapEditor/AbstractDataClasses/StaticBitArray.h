// StaticBitArray.h: implementation of the CStaticBitArray template.
//
//////////////////////////////////////////////////////////////////////

#ifndef __STATIC_BIT_ARRAY__HEADER_INCLUDED__
#define __STATIC_BIT_ARRAY__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// �ablona statick�ho bitov�ho pole (tj. jeho velikost je zn�m� v dob� 
//		kompilace).
template <class IndexType, DWORD nSize>
class CStaticBitArray
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CStaticBitArray () {};
	// destruktor
	~CStaticBitArray () {};

// Operace s bity

	// vr�t� hodnotu bitu "index" v poli bit�
	BOOL operator [] ( IndexType index ) const;
	// vr�t� hodnotu bitu "index" v poli bit�
	BOOL GetAt ( IndexType index ) const;
	// nastav� bit "index" (na hodnotu 1)
	void SetAt ( IndexType index );
	// vynuluje bit "index" (na hodnotu 0)
	void ClearAt ( IndexType index );
	// nastav� hodnotu bitu "index" v poli bit� na novou hodnotu "bNewValue"
	void SetAt ( IndexType index, BOOL bNewValue );
	// nastav� hodnoty v�ech bit� pole na hodnotu "bValue"
	void FillArray ( BOOL bValue = FALSE );

// Operace s cel�m polem bit�

	// vr�t� velikost pole bit� (v bytech)
	DWORD GetDataSize () { return ((nSize+7)/ 8); };
	// vr�t� ukazatel na pole bit� (za ��elem save/load pole)
	BYTE *GetData () { return m_pBitArray; };

// Data
private:
	// pole bit�
	BYTE m_pBitArray[(nSize+7)/8];
};

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vr�t� hodnotu bitu "index" v poli bit�
template <class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<IndexType,nSize>::operator [] ( IndexType index ) const 
{
	return ( m_pBitArray[index/8] & ( 0x01 << ( index & 0x07 ) ) ) ? TRUE : FALSE;
};

// vr�t� hodnotu bitu "index" v poli bit�
template <class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<IndexType,nSize>::GetAt ( IndexType index ) const 
{
	return ( m_pBitArray[index/8] & ( 0x01 << ( index & 0x07 ) ) ) ? TRUE : FALSE;
};

// nastav� bit "index" (na hodnotu 1)
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::SetAt ( IndexType index ) 
{
	// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
	m_pBitArray[index/8] |= (BYTE)( 0x01 << ( index & 0x07 ) );
};

// vynuluje bit "index" (na hodnotu 0)
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::ClearAt ( IndexType index ) 
{
	// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
	m_pBitArray[index/8] &= ~(BYTE)( 0x01 << ( index & 0x07 ) );
};

// nastav� hodnotu bitu "index" v poli bit� na novou hodnotu "bNewValue"
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::SetAt ( IndexType index, BOOL bNewValue ) 
{
	// zjist�, je-li nov� hodnota 1
	if ( bNewValue )
	{	// nov� hodnota bitu m� b�t 1
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
		m_pBitArray[index/8] |= (BYTE)( 0x01 << ( index & 0x07 ) );
	}
	else
	{	// nov� hodnota bitu m� b�t 0
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
		m_pBitArray[index/8] &= ~(BYTE)( 0x01 << ( index & 0x07 ) );
	}
};

// nastav� hodnoty v�ech bit� pole na hodnotu "bValue"
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::FillArray ( BOOL bValue ) 
{
	// vypln� pole bit� p��slu�nou hodnotou
	memset ( m_pBitArray, ( bValue ? 0xff : 0x00 ), (nSize+7)/8 );
};

#endif //__STATIC_BIT_ARRAY__HEADER_INCLUDED__
