/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Statické bitové pole (compile-time velikost)
 * 
 ***********************************************************/

#ifndef __STATIC_BIT_ARRAY__HEADER_INCLUDED__
#define __STATIC_BIT_ARRAY__HEADER_INCLUDED__

#include <memory.h>

// poèet bitù datového typu "DataType"
#define BIT_COUNT(DataType) ( sizeof ( DataType ) * 8 )
// délka pole datového typu "DataType" s "nSize" bity (v datovém typu)
#define ARRAY_LENGTH(DataType,nSize) ( ( nSize + BIT_COUNT ( DataType ) - 1 ) / BIT_COUNT ( DataType ) )
// velikost pole datového typu "DataType" s "nSize" bity (v bytech)
#define ARRAY_SIZE(DataType,nSize) ( ARRAY_LENGTH ( DataType, nSize ) * sizeof ( DataType ) )
// bitová maska prvku pole
#define MEMBER_MASK(DataType) ( sizeof ( DataType ) * 8 - 1 )

//////////////////////////////////////////////////////////////////////
// Šablona statického bitového pole (tj. jeho velikost je známá v dobì 
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

	// vrátí hodnotu bitu "index" v poli bitù
	inline BOOL operator [] ( IndexType index ) const;
	// vrátí hodnotu bitu "index" v poli bitù
	inline BOOL GetAt ( IndexType index ) const;
	// nastaví bit "index" (na hodnotu 1)
	inline void SetAt ( IndexType index );
	// vynuluje bit "index" (na hodnotu 0)
	inline void ClearAt ( IndexType index );
	// nastaví hodnotu bitu "index" v poli bitù na novou hodnotu "bNewValue"
	inline void SetAt ( IndexType index, BOOL bNewValue );
	// nastaví hodnoty všech bitù pole na hodnotu "bValue"
	inline void FillArray ( BOOL bValue = FALSE );

// Operace s celým polem bitù

	// vrátí velikost pole bitù (v bytech)
	DWORD GetDataSize () const { return ARRAY_SIZE ( DataType, nSize ); };
	// vrátí ukazatel na pole bitù (za úèelem save/load pole)
	const DataType *GetData () const { return m_aBitArray; };
	// vrátí ukazatel na pole bitù (za úèelem save/load pole)
	DataType *GetData () { return m_aBitArray; };

// Data
private:
	// pole bitù
	DataType m_aBitArray[ARRAY_LENGTH(DataType,nSize)];
};

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vrátí hodnotu bitu "index" v poli bitù
template <class DataType, class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<DataType,IndexType,nSize>::operator [] ( IndexType index ) const 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	return ( m_aBitArray[index/BIT_COUNT(DataType)] & ( 0x01 << ( index & MEMBER_MASK(DataType) ) ) ) ? TRUE : FALSE;
};

// vrátí hodnotu bitu "index" v poli bitù
template <class DataType, class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<DataType,IndexType,nSize>::GetAt ( IndexType index ) const 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	return ( m_aBitArray[index/BIT_COUNT(DataType)] & ( 0x01 << ( index & MEMBER_MASK(DataType) ) ) ) ? TRUE : FALSE;
};

// nastaví bit "index" (na hodnotu 1)
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::SetAt ( IndexType index ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zachová hodnoty okolních bitù a požadovaný bit nastaví
	m_aBitArray[index/BIT_COUNT(DataType)] |= (DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
};

// vynuluje bit "index" (na hodnotu 0)
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::ClearAt ( IndexType index ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zachová hodnoty okolních bitù a požadovaný bit vynuluje
	m_aBitArray[index/BIT_COUNT(DataType)] &= ~(DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
};

// nastaví hodnotu bitu "index" v poli bitù na novou hodnotu "bNewValue"
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::SetAt ( IndexType index, BOOL bNewValue ) 
{
	ASSERT ( index >= 0 );
	ASSERT ( index < nSize );
	// zjistí, je-li nová hodnota 1
	if ( bNewValue )
	{	// nová hodnota bitu má být 1
		// zachová hodnoty okolních bitù a požadovaný bit nastaví
		m_aBitArray[index/BIT_COUNT(DataType)] |= (DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
	}
	else
	{	// nová hodnota bitu má být 0
		// zachová hodnoty okolních bitù a požadovaný bit vynuluje
		m_aBitArray[index/BIT_COUNT(DataType)] &= ~(DataType)( 0x01 << ( index & MEMBER_MASK(DataType) ) );
	}
};

// nastaví hodnoty všech bitù pole na hodnotu "bValue"
template <class DataType, class IndexType, DWORD nSize>
inline void CStaticBitArray<DataType,IndexType,nSize>::FillArray ( BOOL bValue ) 
{
	// vyplní pole bitù pøíslušnou hodnotou
	memset ( m_aBitArray, ( bValue ? ((DataType)-1) : 0 ), ARRAY_SIZE ( DataType, nSize ) );
};

#undef BIT_COUNT
#undef ARRAY_LENGTH
#undef ARRAY_SIZE
#undef MEMBER_MASK

#endif //__STATIC_BIT_ARRAY__HEADER_INCLUDED__
