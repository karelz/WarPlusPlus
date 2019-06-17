/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Konfigura�n� soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileException reprezentuj�c� chybu
 *          konfigura�n�ho souboru
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
// t��da v�jimky (chyby) konfigura�n�ho souboru
class CConfigFileException : public CException
{
// Datov� typy
public:
	// typ chyby v konfigura�n�m souboru
	enum EError
	{
		// syntaktick� chyby
		CFEEndOfSectionExpected,	// ukon�en� sekce - prom�nn�, nov� sekce nebo konec 
											//		souboru
		CFEEqualExpected,				// =
		CFEVariableValueExpected,	// hodnota prom�nn�
		CFESemicolonExpected,		// ;
//		CFESectionNameExpected,		// jm�no sekce
		CFESectionEndExpected,		// ] (znak ukon�en� sekce)
		CFESectionNameEndExpected,	// ukon�en� jm�na sekce - ], @location]
		CFENewSectionExpected,		// nov� sekce nebo konec souboru
		CFELastSyntaxError = CFENewSectionExpected,
											// konstanta posledn� syntaktick� chyby
		// semantick� chyby
		CFEDuplicitSection,			// duplicitn� definice sekce
		CFEFragmentedSection,		// ��sti sekce ji� byly definov�ny v jin�m souboru
		CFEDuplicitVariable,			// duplicitn� definice prom�nn� v sekci
		CFELastSemanticsError = CFEDuplicitVariable,
											// konstanta posledn� semantick� chyby
		// chyby konverze
		CFEStringToInteger,			// nepovolen� konverze �et�zec->��slo
		CFEStringToReal,				// nepovolen� konverze �et�zec->re�ln� ��slo
		CFEStringToBoolean,			// nepovolen� konverze �et�zec->logick� hodnota
		CFEIntegerToString,			// nepovolen� konverze ��slo->�et�zec
		CFEIntegerToBoolean,			// nepovolen� konverze ��slo->logick� hodnota
		CFERealToString,				// nepovolen� konverze re�ln� ��slo->�et�zec
		CFERealToInteger,				// nepovolen� konverze re�ln� ��slo->��slo
		CFERealToBoolean,				// nepovolen� konverze re�ln� ��slo->logick� hodnota
		CFEBooleanToString,			// nepovolen� konverze logick� hodnota->�et�zec
		CFEBooleanToInteger,			// nepovolen� konverze logick� hodnota->��slo
		CFEBooleanToReal,				// nepovolen� konverze logick� hodnota->re�ln� ��slo
		CFELastConversionError = CFEBooleanToReal,
											// konstanta posledn� konverzn� chyby
		// run-time chyby
		CFEUnknownSection,			// po�adavek na neexistuj�c� sekci
		CFEUnknownVariable,			// po�adavek na neexistuj�c� prom�nnou
		// kritick� chyby
		CFELocationFile				// chyba konfigura�n�ho souboru podsekce
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

// Spr�va v�jimky

	// vr�t� chybovou zpr�vu
	CString GetErrorMessage () const;
	// buffer "lpszError" d�lky "nMaxError" vypln� chybovou zpr�vou v�jimky
	virtual BOOL GetErrorMessage ( LPTSTR lpszError, UINT nMaxError, 
		PUINT pnHelpContext = NULL );

// Data
private:
	// konfigura�n� soubor
	CArchiveFile m_fileInput;
	// posledn� na�ten� token z konfigura�n�ho souboru
	CConfigFileLexToken m_cLastToken;
	// typ chyby v konfigura�n�m souboru
	EError m_eError;
	// pomocn� text chybov� zpr�vy
	CString m_strText;
};

#endif //__CONFIG_FILE_EXCEPTION__HEADER__
