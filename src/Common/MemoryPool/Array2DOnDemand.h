/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Šablona dvourozmìrného pole na požádání
 * 
 ***********************************************************/

#ifndef __ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__
#define __ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__

#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Šablona tøídy dvourozmìrného pole na požádání
template<class Type> 
class CArray2DOnDemand 
{
// Datové typy
protected:
	// ukazatel na prvek pole
	typedef Type *PType;

// Metody
public:
	// konstruktor
	CArray2DOnDemand ( DWORD dwPooledBlockCount );
	// vytvoøí pole
	void Create ( int nSizeX, int nSizeY, int nBlockSize, Type nInitValue = 0 );
	// smaže všechny prvky pole
	void Clear ();
	// znièí pole
	void Delete ();
	// vrátí prvek pole "nIndexX", "nIndexY"
	inline Type &GetAt ( int nIndexX, int nIndexY );
	// vrátí prvek pole "pointIndex"
	inline Type &GetAt ( const CPoint &pointIndex ) 
		{ return GetAt ( pointIndex.x, pointIndex.y ); };
	// vrátí hodnotu prvku pole "nIndexX", "nIndexY"
	inline Type GetValueAt ( int nIndexX, int nIndexY );
	// vrátí hodnotu prvku pole "pointIndex"
	inline Type GetValueAt ( const CPoint &pointIndex ) 
		{ return GetValueAt ( pointIndex.x, pointIndex.y ); };
protected:
	// vytvoøí blok pole
	void CreateBlock ( int nBlockIndex );

// Data
private:
	// rozmìry pole
	int m_nSizeX;
	int m_nSizeY;
	// velikost bloku pole
	int m_nBlockSize;
	// rozmìry pole v blocích
	int m_nBlockSizeX;
	int m_nBlockSizeY;
	// inicializaèní hodnota pole
	Type m_nInitValue;

	// tabulka ukazatelù na bloky dat
	Type **m_aBlockTable;
	// memory pool blokù dat
	CMemoryPool m_cPool;

	// poèet použitých blokù dat
	int m_nUsedBlockCount;
	// tabulka indexù použitých blokù dat
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

// vytvoøí pole
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

	// vytvoøí tabulku blokù pole
	m_aBlockTable = new PType[m_nBlockSizeX * m_nBlockSizeY];
	// inicializuje tabulku blokù pole
	for ( int nIndex = m_nBlockSizeX * m_nBlockSizeY; nIndex-- > 0; )
	{
		// inicializuje ukazatel na blok pole
		m_aBlockTable[nIndex] = NULL;
	}

	// vytvoøí MemoryPool
	m_cPool.Create ( nBlockSize * nBlockSize * sizeof ( DWORD ) );

	// inicializuje poèet použitých blokù dat
	m_nUsedBlockCount = 0;
	// vytvoøí tabulku indexù použitých blokù dat
	m_aUsedBlockIndexTable = new int[m_nBlockSizeX * m_nBlockSizeY];
}

// smaže všechny prvky pole
template<class Type> 
void CArray2DOnDemand<Type>::Clear () 
{
	ASSERT ( ( m_nSizeX > 0 ) && ( m_nSizeY > 0 ) && ( m_nBlockSize > 0 ) && 
		( m_nBlockSizeX > 0 ) && ( m_nBlockSizeY > 0 ) && ( m_aBlockTable != NULL ) && 
		( m_aUsedBlockIndexTable != NULL ) );

	// znièí alokované bloky pole
	for ( int nIndex = m_nUsedBlockCount; nIndex-- > 0; )
	{
		// index použitého bloku dat
		int nBlockIndex = m_aUsedBlockIndexTable[nIndex];
		ASSERT ( nBlockIndex < m_nBlockSizeX * m_nBlockSizeY );

		ASSERT ( m_aBlockTable[nBlockIndex] != NULL );

		// znièí blok pole
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

	// aktualizuje poèet použitých blokù dat
	m_nUsedBlockCount = 0;
}

// znièí pole
template<class Type> 
void CArray2DOnDemand<Type>::Delete () 
{
	ASSERT ( ( m_nSizeX >= 0 ) && ( m_nSizeY >= 0 ) && ( m_nBlockSize >= 0 ) && 
		( m_nBlockSizeX >= 0 ) && ( m_nBlockSizeY >= 0 ) );

	// zjistí je-li alokována tabulka blokù dat
	if ( m_aBlockTable != NULL )
	{	// tabulka blokù dat je alokována
		// znièí alokované bloky dat
		for ( int nIndex = m_nBlockSizeX * m_nBlockSizeY; nIndex-- > 0; )
		{
			// zjistí, je-li blok dat alokován
			if ( m_aBlockTable[nIndex] != NULL )
			{	// blok dat je alokován
				// znièí blok dat
				m_cPool.Free ( m_aBlockTable[nIndex] );
			}
		}
		// znièí tabulku blokù dat
		delete [] m_aBlockTable;
		m_aBlockTable = NULL;
	}

	// zjistí, je-li alokována tabulka indexù použitých blokù dat
	if ( m_aUsedBlockIndexTable != NULL )
	{	//	tabulka indexù použitých blokù dat je alokována
		// znièí tabulka indexù použitých blokù dat
		delete [] m_aUsedBlockIndexTable;
		m_aUsedBlockIndexTable = NULL;
	}

	// zneškodní data objektu
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nBlockSize = 0;
	m_nBlockSizeX = 0;
	m_nBlockSizeY = 0;
	m_nInitValue = 0;

	// znièí MemoryPool
	m_cPool.Delete ();
}

// vrátí prvek pole "nIndexX", "nIndexY"
template<class Type> 
inline Type &CArray2DOnDemand<Type>::GetAt ( int nIndexX, int nIndexY ) 
{
	ASSERT ( ( nIndexX >= 0 ) && ( nIndexX < m_nSizeX ) && ( nIndexY >= 0 ) && 
		( nIndexY < m_nSizeY ) );

	// zjistí index bloku
	int nBlockIndexX = nIndexX / m_nBlockSize;
	int nX = nIndexX % m_nBlockSize;
	int nBlockIndexY = nIndexY / m_nBlockSize;
	int nY = nIndexY % m_nBlockSize;
	int nIndex = nY * m_nBlockSize + nX;
	int nBlockIndex = nBlockIndexY * m_nBlockSizeX + nBlockIndexX;

	// zjistí, exituje-li blok pole
	if ( m_aBlockTable[nBlockIndex] == NULL )
	{	// blok pole dosud neexistuje
		// vytvoøí blok pole
		CreateBlock ( nBlockIndex );
	}
	ASSERT ( m_aBlockTable[nBlockIndex] != NULL );

	// vrátí odkaz na prvek pole
	return m_aBlockTable[nBlockIndex][nIndex];
}

// vrátí hodnotu prvku pole "nIndexX", "nIndexY"
template<class Type> 
inline Type CArray2DOnDemand<Type>::GetValueAt ( int nIndexX, int nIndexY ) 
{
	ASSERT ( ( nIndexX >= 0 ) && ( nIndexX < m_nSizeX ) && ( nIndexY >= 0 ) && 
		( nIndexY < m_nSizeY ) );

	// zjistí index bloku
	int nBlockIndexX = nIndexX / m_nBlockSize;
	int nX = nIndexX % m_nBlockSize;
	int nBlockIndexY = nIndexY / m_nBlockSize;
	int nY = nIndexY % m_nBlockSize;
	int nIndex = nY * m_nBlockSize + nX;
	int nBlockIndex = nBlockIndexY * m_nBlockSizeX + nBlockIndexX;

	// vrátí hodnotu prvku pole
	return ( m_aBlockTable[nBlockIndex] == NULL ) ? m_nInitValue : 
		m_aBlockTable[nBlockIndex][nIndex];
}

// vytvoøí blok pole
template<class Type> 
void CArray2DOnDemand<Type>::CreateBlock ( int nBlockIndex ) 
{
	ASSERT ( m_aBlockTable[nBlockIndex] == NULL );
	ASSERT ( m_nUsedBlockCount < m_nBlockSizeX * m_nBlockSizeY );

	// vytvoøí blok prvkù pole
	Type *aBlock = m_aBlockTable[nBlockIndex] = (Type *)m_cPool.Allocate ();

	// pøidá záznam o použitém bloku dat
	m_aUsedBlockIndexTable[m_nUsedBlockCount++] = nBlockIndex;

	// inicializuje blok prvkù pole
	for ( int nIndex = m_nBlockSize * m_nBlockSize; nIndex-- > 0; )
	{
		// inicializuje prvek pole
		aBlock[nIndex] = m_nInitValue;
	}
}

#endif //__ARRAY_2D_ON_DEMAND__HEADER_INCLUDED__
