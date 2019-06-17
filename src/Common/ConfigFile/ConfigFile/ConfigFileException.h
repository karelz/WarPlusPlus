/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Konfiguraèní soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileException reprezentující chybu
 *          konfiguraèního souboru
 * 
 ***********************************************************/

#ifndef __CONFIG_FILE_EXCEPTION__HEADER__
#define __CONFIG_FILE_EXCEPTION__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include "Archive\Archive\Archive.h"
#include "..\ConfigFileLex\ConfigFileLex.h"

// ************************************************************
// tøída výjimky (chyby) konfiguraèního souboru
class CConfigFileException : public CException
{
// Datové typy
public:
	// typ chyby v konfiguraèním souboru
	enum EError
	{
		// syntaktické chyby
		CFEEndOfSectionExpected,	// ukonèení sekce - promìnná, nová sekce nebo konec 
											//		souboru
		CFEEqualExpected,				// =
		CFEVariableValueExpected,	// hodnota promìnné
		CFESemicolonExpected,		// ;
//		CFESectionNameExpected,		// jméno sekce
		CFESectionEndExpected,		// ] (znak ukonèení sekce)
		CFESectionNameEndExpected,	// ukonèení jména sekce - ], @location]
		CFENewSectionExpected,		// nová sekce nebo konec souboru
		CFELastSyntaxError = CFENewSectionExpected,
											// konstanta poslední syntaktické chyby
		// semantické chyby
		CFEDuplicitSection,			// duplicitní definice sekce
		CFEFragmentedSection,		// èásti sekce již byly definovány v jiném souboru
		CFEDuplicitVariable,			// duplicitní definice promìnné v sekci
		CFELastSemanticsError = CFEDuplicitVariable,
											// konstanta poslední semantické chyby
		// chyby konverze
		CFEStringToInteger,			// nepovolená konverze øetìzec->èíslo
		CFEStringToReal,				// nepovolená konverze øetìzec->reálné èíslo
		CFEStringToBoolean,			// nepovolená konverze øetìzec->logická hodnota
		CFEIntegerToString,			// nepovolená konverze èíslo->øetìzec
		CFEIntegerToBoolean,			// nepovolená konverze èíslo->logická hodnota
		CFERealToString,				// nepovolená konverze reálné èíslo->øetìzec
		CFERealToInteger,				// nepovolená konverze reálné èíslo->èíslo
		CFERealToBoolean,				// nepovolená konverze reálné èíslo->logická hodnota
		CFEBooleanToString,			// nepovolená konverze logická hodnota->øetìzec
		CFEBooleanToInteger,			// nepovolená konverze logická hodnota->èíslo
		CFEBooleanToReal,				// nepovolená konverze logická hodnota->reálné èíslo
		CFELastConversionError = CFEBooleanToReal,
											// konstanta poslední konverzní chyby
		// run-time chyby
		CFEUnknownSection,			// požadavek na neexistující sekci
		CFEUnknownVariable,			// požadavek na neexistující promìnnou
		// kritické chyby
		CFELocationFile				// chyba konfiguraèního souboru podsekce
	};
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CConfigFileException ( CArchiveFile fileInput, CConfigFileLexToken cLastToken, 
		EError eError );
	// konstruktor
	CConfigFileException ( CArchiveFile fileInput, CConfigFileLexToken cLastToken, 
		CString strFileError );
	// konstruktor
	CConfigFileException ( CArchiveFile fileInput, EError eError, CString strText );
	// destruktor
	virtual ~CConfigFileException ();

// Správa výjimky

	// vrátí chybovou zprávu
	CString GetErrorMessage () const;
	// buffer "lpszError" délky "nMaxError" vyplní chybovou zprávou výjimky
	virtual BOOL GetErrorMessage ( LPTSTR lpszError, UINT nMaxError, 
		PUINT pnHelpContext = NULL );

// Data
private:
	// konfiguraèní soubor
	CArchiveFile m_fileInput;
	// poslední naètený token z konfiguraèního souboru
	CConfigFileLexToken m_cLastToken;
	// typ chyby v konfiguraèním souboru
	EError m_eError;
	// pomocný text chybové zprávy
	CString m_strText;
};

#endif //__CONFIG_FILE_EXCEPTION__HEADER__
