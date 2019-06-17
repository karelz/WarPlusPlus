// CompressEngine.h: interface for the CCompressEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSENGINE_H__230D2B11_955E_11D3_BF6E_CD1501B48D07__INCLUDED_)
#define AFX_COMPRESSENGINE_H__230D2B11_955E_11D3_BF6E_CD1501B48D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "includes.h"

#include "../Compression/CompressDataSink.h"
#include "../Compression/CompressDataSource.h"

class CCompressEngine : public CObject  
{
public:
	enum ECompressEngineActivity;

public:
	// Konstruktor
	CCompressEngine();

	// Destruktor
	virtual ~CCompressEngine();

	// Vrati popis tohoto enginu
	CString GetID();

	// Vrati aktualni pozici ve streamu
	DWORD GetPosition() const;

	// Resetuje kompresi/dekompresi (predcasne ukonceni)
	void Reset();

// Dekomprese
	// Nastartovani dekomprese
	void InitDecompression(CCompressDataSource &DataSource);

	// Nacteni bloku dat pri dekompresi
	DWORD Read(void *lpBuf, DWORD dwSize);

	// Urceni celkove delky dat
	DWORD GetUncompressedSize() const;

	// Ukonceni dekomprese
	void DoneDecompression();

// Komprese
	// Nastartovani komprese
	void InitCompression(CCompressDataSink &DataSink, int nQuality=Z_DEFAULT_COMPRESSION);

	// Zapis bloku dat pri kompresi
	void Write(void *lpBuf, DWORD dwSize);

	// Ukonceni komprese
	void DoneCompression();

public:
	enum ECompressEngineActivity {
		inactiveEngine=0,
		compressingEngine=1,
		decompressingEngine=2
	};

	// Vraci defaultni jmeno archivu
	static CString GetDefaultArchiveExtension() { return m_strArchiveExtension; }

private:
	// Data pro ZLIB
	z_stream_s m_ZStream;

	// Co se deje
	ECompressEngineActivity m_eActivity;

	// Ukazatel na source
	CCompressDataSource *m_pSource;

	// Ukazatel na sink
	CCompressDataSink *m_pSink;

	// Ukazatel na buffer pro zapisovana data
	void *m_pBuffer;

	// Velikost bufferu pro zapisovana data
	DWORD m_dwBufferSize;

	// Pripona archivu (defaultni)
	static const char *m_strArchiveExtension;
};

#endif // !defined(AFX_COMPRESSENGINE_H__230D2B11_955E_11D3_BF6E_CD1501B48D07__INCLUDED_)
