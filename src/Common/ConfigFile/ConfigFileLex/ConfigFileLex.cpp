/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Ètení konfiguraèních souborù
 *   Autor: Helena Kupková
 * 
 *   Popis: Objekt CConfigFileLex implementující rozhraní k lexem 
 *          vygenerovanému automatu
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

#include "ConfigFileLex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// class CConfigFileLexToken
//////////////////////////////////////////////////////////////////////

// konstrutor
CConfigFileLexToken::CConfigFileLexToken () 
{
	m_eToken = (EConfigFileLexToken)-1;
	m_strText.Empty ();
	m_dwOffset = -1;
	m_dwRow = -1;
	m_wColumn = -1;
}

// vnitøní funkce volaná lexem, nastaví aktuální token
void CConfigFileLexToken::Set ( EConfigFileLexToken eToken, LPCTSTR lpszText, 
	DWORD dwOffset /*= 0*/, DWORD dwRow /*= 0*/, WORD wColumn /*= 0*/ ) 
{
	m_eToken = eToken;
	m_strText = lpszText;
	m_dwOffset = dwOffset;
	m_dwRow = dwRow;
	m_wColumn = wColumn;
}

//////////////////////////////////////////////////////////////////////
// class CConfigFileLex
//////////////////////////////////////////////////////////////////////
                                              
// konstruktor
CConfigFileLex::CConfigFileLex () 
{
}

// destruktor
CConfigFileLex::~CConfigFileLex () 
{
}

// zaène práci lexe s novým konfiguraèním souboru
//		výjimky: CDataArchiveException
bool CConfigFileLex::Create ( CArchiveFile fileInput, DWORD dwOffset /*= 0*/, 
	DWORD dwRow /*= 0*/, WORD wColumn /*= 0*/ ) 
{
	// nesmí být otevøen žádný konfiguraèní soubor
//	ASSERT ( !fileInput.IsOpen () );

	m_fileInput = fileInput;
	m_dwOffset = dwOffset;
	m_dwRow = dwRow;
	m_wColumn = wColumn;

	// nastaví pozici v konfiguraèním souboru na požadovaný offset
	m_fileInput.Seek ( dwOffset, CFile::begin );

	return true;
}

// ukonèí práci lexe s konfiguraèním souborem
void CConfigFileLex::Delete () 
{
	// musí být otevøen nìjaký konfiguraèní soubor
//	ASSERT ( fileInput.IsOpen () );
}

// naète další token z konfiguraèního souboru
void CConfigFileLex::GetToken ( CConfigFileLexToken &cToken ) 
{
	// musí být otevøen nìjaký konfiguraèní soubor
//	ASSERT ( fileInput.IsOpen () );

	// uschová si odkaz na strukturu tokenu, do kterého má naèíst nový token
	m_pToken = &cToken;

	// nechá naèíst nový token z konfiguraèního souboru
	this->yylex ();
}

// voláno lexem, když chce vstup
int CConfigFileLex::LexerInput ( char *buf, int max_size )
{
	return m_fileInput.Read ( buf, 1 );
}
