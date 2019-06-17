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
// tøída reprezentující token vracený lexem
class CConfigFileLexToken
{
public:
	// typy tokenù vracených lexem
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
	// token vracený lexem
	EConfigFileLexToken m_eToken;
	// øetìzec tokenu (øetìzec vrací vèetnì uvozovek)
	CString m_strText;
	// offset zaèátku tokenu
	DWORD m_dwOffset;
	// øádek zaèátku tokenu
	DWORD m_dwRow;
	// sloupec zaèátku tokenu
	WORD m_wColumn;

	// vnitøní funkce volaná lexem, nastaví aktuální token
	void Set ( EConfigFileLexToken eToken, LPCTSTR lpszText, DWORD nOffset = 0, 
		DWORD nRow = 0, WORD nColumn = 0 );
};

// ************************************************************
// tøída reprezentující lex pracující s konfiguraèním souborem
class CConfigFileLex  : public configfileFlexLexer
{
public:
	// konstruktor
	CConfigFileLex ();
	// destruktor
	~CConfigFileLex ();

	// zaène práci lexe s novým konfiguraèním souboru
	//		výjimky: CDataArchiveException
	bool Create ( CArchiveFile fileInput, DWORD dwOffset = 0, DWORD dwRow = 0, 
		WORD wColumn = 0 );
	// ukonèí práci lexe s konfiguraèním souborem
	void Delete ();

	// naète další token z konfiguraèního souboru
	void GetToken ( CConfigFileLexToken &cToken );

protected:
	// voláno lexem, když chce vstup
	virtual int LexerInput ( char *buf, int max_size );

	// funkce parseru
	virtual int yylex ();

protected:
	// ukazatel na konfiguraèní soubor
	CArchiveFile m_fileInput;
	// ukazatel na naètený token
	CConfigFileLexToken *m_pToken;
	// offset zaèátku tokenu
	DWORD m_dwOffset;
	// øádek zaèátku tokenu
	DWORD m_dwRow;
	// sloupec zaèátku tokenu
	WORD m_wColumn;
};

#endif //__CONFIG_FILE_LEX__HEADER__
