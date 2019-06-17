/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Bitové pole
 * 
 ***********************************************************/

#ifndef __BIT_ARRAY__HEADER_INCLUDED__
#define __BIT_ARRAY__HEADER_INCLUDED__

#include <afxwin.h>

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Tøída bitového pole pevné délky (tj. jeho velikost je známá pøi 
//		vytváøení instance objektu)
class CBitArray 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CBitArray ();
	// destruktor
	~CBitArray ();

// Inicializace a znièení dat objektu

	// vytvoøí bitové pole velikosti "dwSize"
	//		výjimky: CMemoryException
	void Create ( DWORD dwSize );
	// inicializuje bitové pole velikosti "dwSize" na hodnotu "bValue"
	//		výjimky: CMemoryException
	void Create ( DWORD dwSize, BOOL bValue );
	// znièí bitové pole
	void Delete ();
	// vrátí pøíznak znièení dat
	BOOL IsDeleted () { return ( m_pBitArray == NULL ); };

	// inicializuje bitové pole "pBitArray" velikosti "dwSize"
#ifdef _DEBUG
	void Assign ( BYTE *pBitArray, DWORD dwSize );
#else
	void Assign ( BYTE *pBitArray );
#endif
	// znièí bitové pole, ale nemaže vlastní data (pro vnìjší alokaci pamìti)
	void DeleteAssigned ();

// Operace s bity

	// vrátí hodnotu bitu "dwIndex" v poli bitù
	BOOL operator [] ( DWORD dwIndex ) const;
	// vrátí hodnotu bitu "dwIndex" v poli bitù
	BOOL GetAt ( DWORD dwIndex ) const;
	// nastaví bit "dwIndex" (na hodnotu 1)
	void SetAt ( DWORD dwIndex );
	// vynuluje bit "dwIndex" (na hodnotu 0)
	void ClearAt ( DWORD dwIndex );
	// nastaví hodnotu bitu "dwIndex" v poli bitù na novou hodnotu "bNewValue"
	void SetAt ( DWORD dwIndex, BOOL bNewValue );

// Operace s celým polem bitù

	// vrátí ukazatel na pole bitù (za úèelem save/load pole)
	const BYTE *GetData () const { return m_pBitArray; };

// Data
private:
	// ukazatel na pole bitù
	BYTE *m_pBitArray;

#ifdef _DEBUG
	// velikost pole bitù
	DWORD m_dwSize;
	// pøíznak vytvoøení bitového pole metodou "Assign"
	BOOL m_bAssigned;
#endif //_DEBUG
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CBitArray::CBitArray () 
{
	// zneškodní data objektu
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
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøí bitové pole velikosti "dwSize"
//		výjimky: CMemoryException
inline void CBitArray::Create ( DWORD dwSize ) 
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// alokuje potøebnou pamì pro bitové pole
	m_pBitArray = new BYTE[(dwSize+7)/8];

#ifdef _DEBUG
	// inicializuje velikost alokovaného pole
	m_dwSize = dwSize;
	// nastaví pøíznak alokace bitového pole metodou "Assign"
	m_bAssigned = FALSE;
#endif //_DEBUG
}

// inicializuje bitové pole velikosti "dwSize" na hodnotu "bValue"
//		výjimky: CMemoryException
inline void CBitArray::Create ( DWORD dwSize, BOOL bValue ) 
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// alokuje potøebnou pamì pro bitové pole
	m_pBitArray = new BYTE[(dwSize+7)/8];

#ifdef _DEBUG
	// inicializuje velikost alokovaného pole
	m_dwSize = dwSize;
	// nastaví pøíznak alokace bitového pole metodou "Assign"
	m_bAssigned = FALSE;
#endif //_DEBUG

	// vyplní pole bitù pøíslušnou hodnotou
	memset ( m_pBitArray, ( bValue ? 0xff : 0x00 ), (dwSize+7)/8 );
}

// znièí bitové pole
inline void CBitArray::Delete () 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( m_dwSize > 0 );
	ASSERT ( !m_bAssigned );

	// znièí alokovanou pamì
	delete []m_pBitArray;

	// zneškodní data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

// inicializuje bitové pole "pBitArray" velikosti "dwSize"
#ifdef _DEBUG
inline void CBitArray::Assign ( BYTE *pBitArray, DWORD dwSize ) 
#else
inline void CBitArray::Assign ( BYTE *pBitArray ) 
#endif
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// uschová si ukazatel na bitové pole
	m_pBitArray = pBitArray;

#ifdef _DEBUG
	// inicializuje velikost alokovaného pole
	m_dwSize = dwSize;
	// nastaví pøíznak alokace bitového pole metodou "Assign"
	m_bAssigned = TRUE;
#endif //_DEBUG
}

// znièí bitové pole, ale nemaže vlastní data (pro vnìjší alokaci pamìti)
inline void CBitArray::DeleteAssigned () 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( m_dwSize > 0 );
	ASSERT ( m_bAssigned );

	// zneškodní data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vrátí hodnotu bitu "dwIndex" v poli bitù
inline BOOL CBitArray::operator [] ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// vrátí hodnotu bitu "dwIndex" v poli bitù
inline BOOL CBitArray::GetAt ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// nastaví bit "dwIndex" (na hodnotu 1)
inline void CBitArray::SetAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachová hodnoty okolních bitù a požadovaný bit nastaví
	m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// vynuluje bit "dwIndex" (na hodnotu 0)
inline void CBitArray::ClearAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachová hodnoty okolních bitù a požadovaný bit vynuluje
	m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// nastaví hodnotu bitu "dwIndex" v poli bitù na novou hodnotu "bNewValue"
inline void CBitArray::SetAt ( DWORD dwIndex, BOOL bNewValue ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zjistí, je-li nová hodnota 1
	if ( bNewValue )
	{	// nová hodnota bitu má být 1
		// zachová hodnoty okolních bitù a požadovaný bit nastaví
		m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
	else
	{	// nová hodnota bitu má být 0
		// zachová hodnoty okolních bitù a požadovaný bit vynuluje
		m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
}

#endif //__BIT_ARRAY__HEADER_INCLUDED__
