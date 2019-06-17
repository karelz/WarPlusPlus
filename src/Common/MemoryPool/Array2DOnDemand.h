/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: �ablona dvourozm�rn�ho pole na po��d�n�
 * 
 ***********************************************************/

#ifndef __ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__
#define __ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__

#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// �ablona t��dy dvourozm�rn�ho pole na po��d�n�
template<class Type> 
class CArray2DOnDemand 
{
// Datov� typy
protected:
	// ukazatel na prvek pole
	typedef Type *PType;

// Metody
public:
	// konstruktor
	CArray2DOnDemand ( DWORD dwPooledBlockCount );
	// vytvo�� pole
	void Create ( int nSizeX, int nSizeY, int nBlockSize, Type nInitValue = 0 );
	// sma�e v�echny prvky pole
	void Clear ();
	// zni�� pole
	void Delete ();
	// vr�t� prvek pole "nIndexX", "nIndexY"
	inline Type &GetAt ( int nIndexX, int nIndexY );
	// vr�t� prvek pole "pointIndex"
	inline Type &GetAt ( const CPoint &pointIndex ) 
		{ return GetAt ( pointIndex.x, pointIndex.y ); };
	// vr�t� hodnotu prvku pole "nIndexX", "nIndexY"
	inline Type GetValueAt ( int nIndexX, int nIndexY );
	// vr�t� hodnotu prvku pole "pointIndex"
	inline Type GetValueAt ( const CPoint &pointIndex ) 
		{ return GetValueAt ( pointIndex.x, pointIndex.y ); };
protected:
	// vytvo�� blok pole
	void CreateBlock ( int nBlockIndex );

// Data
private:
	// rozm�ry pole
	int m_nSizeX;
	int m_nSizeY;
	// velikost bloku pole
	int m_nBlockSize;
	// rozm�ry pole v bloc�ch
	int m_nBlockSizeX;
	int m_nBlockSizeY;
	// inicializa�n� hodnota pole
	Type m_nInitValue;

	// tabulka ukazatel� na bloky dat
	Type **m_aBlockTable;
	// memory pool blok� dat
	CMemoryPool m_cPool;

	// po�et pou�it�ch blok� dat
	int m_nUsedBlockCount;
	// tabulka index� pou�it�ch blok� dat
	int *m_aUsedBlockIndexTable;
};

// konstruktor
template<class Type> 
CArray2DOnDemand<Type>::CArray2DOnDemand ( DWORD dwPooledBlockCount ) : 
	m_cPool ( dwPooledBlockCount ) 
{
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nBlockSize = 0;
	m_nBlockSizeX = 0;
	m_nBlockSizeY = 0;
	m_nInitValue = 0;

	m_aBlockTable = NULL;

	m_nUsedBlockCount = 0;
	m_aUsedBlockIndexTable = NULL;
}

// vytvo�� pole
template<class Type> 
void CArray2DOnDemand<Type>::Create ( int nSizeX, int nSizeY, int nBlockSize, 
	Type nInitValue ) 
{
	ASSERT ( ( m_nSizeX == 0 ) && ( m_nSizeY == 0 ) && ( m_nBlockSize == 0 ) && 
		( m_nBlockSizeX == 0 ) && ( m_nBlockSizeY == 0 ) && ( m_nInitValue == 0 ) && 
		( m_aBlockTable == NULL ) );
	ASSERT ( ( nSizeX > 0 ) && ( nSizeY > 0 ) && ( nBlockSize > 0 ) );

	// inicializuje hodnoty objektu
	m_nSizeX = nSizeX;
	m_nSizeY = nSizeY;
	m_nBlockSize = nBlockSize;
	m_nBlockSizeX = ( nSizeX + nBlockSize - 1 ) / nBlockSize;
	m_nBlockSizeY = ( nSizeY + nBlockSize - 1 ) / nBlockSize;
	m_nInitValue = nInitValue;

	// vytvo�� tabulku blok� pole
	m_aBlockTable = new PType[m_nBlockSizeX * m_nBlockSizeY];
	// inicializuje tabulku blok� pole
	for ( int nIndex = m_nBlockSizeX * m_nBlockSizeY; nIndex-- > 0; )
	{
		// inicializuje ukazatel na blok pole
		m_aBlockTable[nIndex] = NULL;
	}

	// vytvo�� MemoryPool
	m_cPool.Create ( nBlockSize * nBlockSize * sizeof ( DWORD ) );

	// inicializuje po�et pou�it�ch blok� dat
	m_nUsedBlockCount = 0;
	// vytvo�� tabulku index� pou�it�ch blok� dat
	m_aUsedBlockIndexTable = new int[m_nBlockSizeX * m_nBlockSizeY];
}

// sma�e v�echny prvky pole
template<class Type> 
void CArray2DOnDemand<Type>::Clear () 
{
	ASSERT ( ( m_nSizeX > 0 ) && ( m_nSizeY > 0 ) && ( m_nBlockSize > 0 ) && 
		( m_nBlockSizeX > 0 ) && ( m_nBlockSizeY > 0 ) && ( m_aBlockTable != NULL ) && 
		( m_aUsedBlockIndexTable != NULL ) );

	// zni�� alokovan� bloky pole
	for ( int nIndex = m_nUsedBlockCount; nIndex-- > 0; )
	{
		// index pou�it�ho bloku dat
		int nBlockIndex = m_aUsedBlockIndexTable[nIndex];
		ASSERT ( nBlockIndex < m_nBlockSizeX * m_nBlockSizeY );

		ASSERT ( m_aBlockTable[nBlockIndex] != NULL );

		// zni�� blok pole
		m_cPool.Free ( m_aBlockTable[nBlockIndex] );
		m_aBlockTable[nBlockIndex] = NULL;
	}

#ifdef _DEBUG
	// zkontroluje bloky dat
	for ( int nBlockIndex = m_nBlockSizeX * m_nBlockSizeY; nBlockIndex-- > 0; )
	{
		ASSERT ( m_aBlockTable[nBlockIndex] == NULL );
	}
#endif //_DEBUG

	// aktualizuje po�et pou�it�ch blok� dat
	m_nUsedBlockCount = 0;
}

// zni�� pole
template<class Type> 
void CArray2DOnDemand<Type>::Delete () 
{
	ASSERT ( ( m_nSizeX >= 0 ) && ( m_nSizeY >= 0 ) && ( m_nBlockSize >= 0 ) && 
		( m_nBlockSizeX >= 0 ) && ( m_nBlockSizeY >= 0 ) );

	// zjist� je-li alokov�na tabulka blok� dat
	if ( m_aBlockTable != NULL )
	{	// tabulka blok� dat je alokov�na
		// zni�� alokovan� bloky dat
		for ( int nIndex = m_nBlockSizeX * m_nBlockSizeY; nIndex-- > 0; )
		{
			// zjist�, je-li blok dat alokov�n
			if ( m_aBlockTable[nIndex] != NULL )
			{	// blok dat je alokov�n
				// zni�� blok dat
				m_cPool.Free ( m_aBlockTable[nIndex] );
			}
		}
		// zni�� tabulku blok� dat
		delete [] m_aBlockTable;
		m_aBlockTable = NULL;
	}

	// zjist�, je-li alokov�na tabulka index� pou�it�ch blok� dat
	if ( m_aUsedBlockIndexTable != NULL )
	{	//	tabulka index� pou�it�ch blok� dat je alokov�na
		// zni�� tabulka index� pou�it�ch blok� dat
		delete [] m_aUsedBlockIndexTable;
		m_aUsedBlockIndexTable = NULL;
	}

	// zne�kodn� data objektu
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nBlockSize = 0;
	m_nBlockSizeX = 0;
	m_nBlockSizeY = 0;
	m_nInitValue = 0;

	// zni�� MemoryPool
	m_cPool.Delete ();
}

// vr�t� prvek pole "nIndexX", "nIndexY"
template<class Type> 
inline Type &CArray2DOnDemand<Type>::GetAt ( int nIndexX, int nIndexY ) 
{
	ASSERT ( ( nIndexX >= 0 ) && ( nIndexX < m_nSizeX ) && ( nIndexY >= 0 ) && 
		( nIndexY < m_nSizeY ) );

	// zjist� index bloku
	int nBlockIndexX = nIndexX / m_nBlockSize;
	int nX = nIndexX % m_nBlockSize;
	int nBlockIndexY = nIndexY / m_nBlockSize;
	int nY = nIndexY % m_nBlockSize;
	int nIndex = nY * m_nBlockSize + nX;
	int nBlockIndex = nBlockIndexY * m_nBlockSizeX + nBlockIndexX;

	// zjist�, exituje-li blok pole
	if ( m_aBlockTable[nBlockIndex] == NULL )
	{	// blok pole dosud neexistuje
		// vytvo�� blok pole
		CreateBlock ( nBlockIndex );
	}
	ASSERT ( m_aBlockTable[nBlockIndex] != NULL );

	// vr�t� odkaz na prvek pole
	return m_aBlockTable[nBlockIndex][nIndex];
}

// vr�t� hodnotu prvku pole "nIndexX", "nIndexY"
template<class Type> 
inline Type CArray2DOnDemand<Type>::GetValueAt ( int nIndexX, int nIndexY ) 
{
	ASSERT ( ( nIndexX >= 0 ) && ( nIndexX < m_nSizeX ) && ( nIndexY >= 0 ) && 
		( nIndexY < m_nSizeY ) );

	// zjist� index bloku
	int nBlockIndexX = nIndexX / m_nBlockSize;
	int nX = nIndexX % m_nBlockSize;
	int nBlockIndexY = nIndexY / m_nBlockSize;
	int nY = nIndexY % m_nBlockSize;
	int nIndex = nY * m_nBlockSize + nX;
	int nBlockIndex = nBlockIndexY * m_nBlockSizeX + nBlockIndexX;

	// vr�t� hodnotu prvku pole
	return ( m_aBlockTable[nBlockIndex] == NULL ) ? m_nInitValue : 
		m_aBlockTable[nBlockIndex][nIndex];
}

// vytvo�� blok pole
template<class Type> 
void CArray2DOnDemand<Type>::CreateBlock ( int nBlockIndex ) 
{
	ASSERT ( m_aBlockTable[nBlockIndex] == NULL );
	ASSERT ( m_nUsedBlockCount < m_nBlockSizeX * m_nBlockSizeY );

	// vytvo�� blok prvk� pole
	Type *aBlock = m_aBlockTable[nBlockIndex] = (Type *)m_cPool.Allocate ();

	// p�id� z�znam o pou�it�m bloku dat
	m_aUsedBlockIndexTable[m_nUsedBlockCount++] = nBlockIndex;

	// inicializuje blok prvk� pole
	for ( int nIndex = m_nBlockSize * m_nBlockSize; nIndex-- > 0; )
	{
		// inicializuje prvek pole
		aBlock[nIndex] = m_nInitValue;
	}
}

#endif //__ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__
