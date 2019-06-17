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

#ifndef __CONFIG_FILE_LEX__HEADER__
#define __CONFIG_FILE_LEX__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include "Archive\Archive\Archive.h"

#ifndef yyFlexLexer
#define yyFlexLexer configfileFlexLexer
#include "FlexLexer.h"
#undef configfileFlexLexer
#endif

// ************************************************************
// t��da reprezentuj�c� token vracen� lexem
class CConfigFileLexToken
{
public:
	// typy token� vracen�ch lexem
	enum EConfigFileLexToken
	{
		CLTEqual					=  1,
		CLTString				=  2,
		CLTVariable				=  3,
		CLTInteger				=  4,
		CLTReal					=  5,
		CLTHexNumber			=  6,
		CLTSectionBegin		=  7,
		CLTSectionName			=  8,
		CLTSectionLocation	=  9,
		CLTSectionEnd			= 10,
		CLTSemicolon			= 11,
		CLTError					= 12,
		CLTTrue					= 13,
		CLTFalse					= 14,
		CLTEndOfFile			= 15
	};
public:
	// konstruktor
	CConfigFileLexToken ();
	// destruktor
	~CConfigFileLexToken () {};
public:
	// token vracen� lexem
	EConfigFileLexToken m_eToken;
	// �et�zec tokenu (�et�zec vrac� v�etn� uvozovek)
	CString m_strText;
	// offset za��tku tokenu
	DWORD m_dwOffset;
	// ��dek za��tku tokenu
	DWORD m_dwRow;
	// sloupec za��tku tokenu
	WORD m_wColumn;

	// vnit�n� funkce volan� lexem, nastav� aktu�ln� token
	void Set ( EConfigFileLexToken eToken, LPCTSTR lpszText, DWORD nOffset = 0, 
		DWORD nRow = 0, WORD nColumn = 0 );
};

// ************************************************************
// t��da reprezentuj�c� lex pracuj�c� s konfigura�n�m souborem
class CConfigFileLex  : public configfileFlexLexer
{
public:
	// konstruktor
	CConfigFileLex ();
	// destruktor
	~CConfigFileLex ();

	// za�ne pr�ci lexe s nov�m konfigura�n�m souboru
	//		v�jimky: CDataArchiveException
	bool Create ( CArchiveFile fileInput, DWORD dwOffset = 0, DWORD dwRow = 0, 
		WORD wColumn = 0 );
	// ukon�� pr�ci lexe s konfigura�n�m souborem
	void Delete ();

	// na�te dal�� token z konfigura�n�ho souboru
	void GetToken ( CConfigFileLexToken &cToken );

protected:
	// vol�no lexem, kdy� chce vstup
	virtual int LexerInput ( char *buf, int max_size );

	// funkce parseru
	virtual int yylex ();

protected:
	// ukazatel na konfigura�n� soubor
	CArchiveFile m_fileInput;
	// ukazatel na na�ten� token
	CConfigFileLexToken *m_pToken;
	// offset za��tku tokenu
	DWORD m_dwOffset;
	// ��dek za��tku tokenu
	DWORD m_dwRow;
	// sloupec za��tku tokenu
	WORD m_wColumn;
};

#endif //__CONFIG_FILE_LEX__HEADER__
