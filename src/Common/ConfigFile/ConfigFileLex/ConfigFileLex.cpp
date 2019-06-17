/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: �ten� konfigura�n�ch soubor�
 *   Autor: Helena Kupkov�
 * 
 *   Popis: Objekt CConfigFileLex implementuj�c� rozhran� k lexem 
 *          vygenerovan�mu automatu
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

// vnit�n� funkce volan� lexem, nastav� aktu�ln� token
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

// za�ne pr�ci lexe s nov�m konfigura�n�m souboru
//		v�jimky: CDataArchiveException
bool CConfigFileLex::Create ( CArchiveFile fileInput, DWORD dwOffset /*= 0*/, 
	DWORD dwRow /*= 0*/, WORD wColumn /*= 0*/ ) 
{
	// nesm� b�t otev�en ��dn� konfigura�n� soubor
//	ASSERT ( !fileInput.IsOpen () );

	m_fileInput = fileInput;
	m_dwOffset = dwOffset;
	m_dwRow = dwRow;
	m_wColumn = wColumn;

	// nastav� pozici v konfigura�n�m souboru na po�adovan� offset
	m_fileInput.Seek ( dwOffset, CFile::begin );

	return true;
}

// ukon�� pr�ci lexe s konfigura�n�m souborem
void CConfigFileLex::Delete () 
{
	// mus� b�t otev�en n�jak� konfigura�n� soubor
//	ASSERT ( fileInput.IsOpen () );
}

// na�te dal�� token z konfigura�n�ho souboru
void CConfigFileLex::GetToken ( CConfigFileLexToken &cToken ) 
{
	// mus� b�t otev�en n�jak� konfigura�n� soubor
//	ASSERT ( fileInput.IsOpen () );

	// uschov� si odkaz na strukturu tokenu, do kter�ho m� na��st nov� token
	m_pToken = &cToken;

	// nech� na��st nov� token z konfigura�n�ho souboru
	this->yylex ();
}

// vol�no lexem, kdy� chce vstup
int CConfigFileLex::LexerInput ( char *buf, int max_size )
{
	return m_fileInput.Read ( buf, 1 );
}
