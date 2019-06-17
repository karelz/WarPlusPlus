/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Statick� bitov� pole (compile-time velikost)
 * 
 ***********************************************************/

#ifndef __STATIC_BIT_ARRAY__HEADER_INCLUDED__
#define __STATIC_BIT_ARRAY__HEADER_INCLUDED__

#include <memory.h>

// po�et bit� datov�ho typu "DataType"
#define BIT_COUNT(DataType) ( sizeof ( DataType ) * 8 )
// d�lka pole datov�ho typu "DataType" s "nSize" bity (v datov�m typu)
#define ARRAY_LENGTH(DataType,nSize) ( ( nSize + BIT_COUNT ( DataType ) - 1 ) / BIT_COUNT ( DataType ) )
// velikost pole datov�ho typu "DataType" s "nSize" bity (v bytech)
#define ARRAY_SIZE(DataType,nSize) ( ARRAY_LENGTH ( DataType, nSize ) * sizeof ( DataType ) )
// bitov� maska prvku pole
#define MEMBER_MASK(DataType) ( sizeof ( DataType ) * 8 - 1 )

//////////////////////////////////////////////////////////////////////
// �ablona statick�ho bitov�ho pole (tj. jeho velikost je zn�m� v dob� 
//		kompilace).
template <class DataType, class IndexType, DWORD nSize>
class CStaticBitArray 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CStaticBitArray () { ASSERT ( nSize > 0 ); };
	// destruktor
	~CStaticBitArray () {};

// Operace s bity

	// vr�t� hodnotu bitu "index" v poli bit�
	inline BOOL operator [] ( IndexType index ) const;
	// vr�t� hodnotu bitu "index" v poli bit�
	inline BOOL GetAt ( IndexType index ) const;
	// nastav� bit "index" (na hodnotu 1)
	inline void SetAt ( IndexType index );
	// vynuluje bit "index" (na hodnotu 0)
	inline void ClearAt ( IndexType index );
	// nastav� hodnotu bitu "index" v poli bit� na novou hodnotu "bNewValue"
	inline void SetAt ( IndexType index, BOOL bNewValue );
	// nastav� hodnoty v�ech bit� pole na hodnotu "bValue"
	inline void FillArray ( BOOL bValue = FALSE );

// Operace s cel�m polem bit�

	// vr�t� velikost pole bit� (v bytech)
	DWORD GetDataSize () const { return ARRAY_SIZE ( DataType, nSize ); };
	// vr�t� ukazatel na pole bit� (za ��elem save/load pole)
	const DataType *GetData () const { return m_aBitArray; };
	// vr�t� ukazatel na pole bit� (za ��elem save/load pole)
	DataType *GetData () { return m_aBitArray; };

// Data
private:
	// pole bit�
	DataType m_aBitArray[ARRAY_LENGTH(DataType,nSize)];
};

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vr�t� hodnotu bitu "index" v poli bit�
template <class DataType, class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<DataType,IndexType,nSize>::operator [] ( IndexType index ) const 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	return ( m_aBitArray[index/BIT_COUNT(DataType)] & ( 0x01 << ( index & MEMBER_MASK(DataType) ) ) ) ? TRUE : FALSE;
};

// vr�t� hodnotu bitu "index" v poli bit�
template <class DataType, class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<DataType,IndexType,nSize>::GetAt ( IndexType index ) const 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	return ( m_aBitArray[index/BIT_COUNT(DataType)] & ( 0x01 << ( index & MEMBER_MASK(DataType) ) ) ) ? TRUE : FALSE;
};

// nastav� bit "index" (na hodnotu 1)
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::SetAt ( IndexType index ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
	m_aBitArray[index/BIT_COUNT(DataType)] |= (DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
};

// vynuluje bit "index" (na hodnotu 0)
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::ClearAt ( IndexType index ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
	m_aBitArray[index/BIT_COUNT(DataType)] &= ~(DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
};

// nastav� hodnotu bitu "index" v poli bit� na novou hodnotu "bNewValue"
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::SetAt ( IndexType index, BOOL bNewValue ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zjist�, je-li nov� hodnota 1
	if ( bNewValue )
	{	// nov� hodnota bitu m� b�t 1
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit nastav�
		m_aBitArray[index/BIT_COUNT(DataType)] |= (DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
	}
	else
	{	// nov� hodnota bitu m� b�t 0
		// zachov� hodnoty okoln�ch bit� a po�adovan� bit vynuluje
		m_aBitArray[index/BIT_COUNT(DataType)] &= ~(DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
	}
};

// nastav� hodnoty v�ech bit� pole na hodnotu "bValue"
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::FillArray ( BOOL bValue ) 
{
	// vypln� pole bit� p��slu�nou hodnotou
	memset ( m_aBitArray, ( bValue ? ((DataType)-1) : 0 ), ARRAY_SIZE ( DataType, nSize ) );
};

#undef BIT_COUNT
#undef ARRAY_LENGTH
#undef ARRAY_SIZE
#undef MEMBER_MASK

#endif //__STATIC_BIT_ARRAY__HEADER_INCLUDED__
