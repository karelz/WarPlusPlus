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

#include "StdAfx.h"
#include "ConfigFileException.h"

// ************************************************************
// konstanty

// jm�no typu prom�nn�
enum EVariableTypeName
{
	VTNString	= 0,
	VTNInteger	= 1,
	VTNReal		= 2,
	VTNBoolean	= 3
};

// vrac� jm�no typu prom�nn� "eType"
inline const char *GetVariableTypeName ( enum EVariableTypeName eType ) 
{
	// tabulka jmen typ� prom�nn�ch
	static LPCTSTR g_aVariableTypeNames[] = { _T("string"), _T("integer"), "real", "boolean" };

	// vrac� jm�no typu prom�nn� "eIndex"
	return g_aVariableTypeNames[(int)eType];
}

// ************************************************************
// t��da v�jimky (chyby) konfigura�n�ho souboru
// ************************************************************

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, 
	CConfigFileLexToken cLastToken, EError eError ) 
{
	ASSERT ( fileInput.IsOpened () );

	// uschov� si informace o chyb�
	m_fileInput = fileInput;
	m_cLastToken = cLastToken;
	m_eError = eError;
}

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, 
	CConfigFileLexToken cLastToken, CString strFileError ) 
{
	ASSERT ( fileInput.IsOpened () );

	// uschov� si informace o chyb�
	m_fileInput = fileInput;
	m_cLastToken = cLastToken;
	m_eError = EError::CFELocationFile;
	m_strText = strFileError;
}

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, EError eError, 
	CString strText ) 
{
	// uschov� si informace o chyb�
	m_fileInput = fileInput;
	m_eError = eError;
	m_strText = strText;
}

// destruktor
CConfigFileException::~CConfigFileException () 
{
}

//////////////////////////////////////////////////////////////////////
// Spr�va v�jimky
//////////////////////////////////////////////////////////////////////

// vr�t� chybovou zpr�vu
CString CConfigFileException::GetErrorMessage () const 
{
	// um�st�n� posledn�ho tokenu
	CString strTokenLocation;
	// text chyby konfigura�n�ho souboru
	CString strErrorText;
	// v�sledn� chybov� zpr�va
	CString strErrorMessage;

	// p�iprav� text um�st�n� posledn�ho tokenu
	strTokenLocation.Format ( _T(" (config file \"%s\" in archive \"%s\" at offset %d = " 
		"line %d, column %d)."), (const char *)m_fileInput.GetFilePath (), 
		(const char *)( m_fileInput.GetDataArchive ().GetArchivePath () ), 
		m_cLastToken.m_dwOffset, m_cLastToken.m_dwRow, m_cLastToken.m_wColumn );

	// zjist�, jedn�-li se o lexik�ln� chybu
	if ( m_cLastToken.m_eToken == CConfigFileLexToken::CLTError )
	{	// jedn� se o lexik�ln� chybu
		// p�iprav� text lexik�ln� chyby
		strErrorText.Format ( _T("Lexical error: Unknown token (character) \"%s\""), 
			(const char *)m_cLastToken.m_strText );
	}
	else if ( m_eError > CFELastSyntaxError )
	{	// jedn� se o semantickou nebo kritickou chybu
		// zjist�, jedn�-li se o semantickou chybu
		if ( m_eError > CFELastSemanticsError )
		{	// nejedn� se o semantickou chybu
			// zjist�, jedn�-li se o konverzn� chybu
			if ( m_eError <= CFELastConversionError )
			{	// jedn� se o konverzn� chybu
			// pomocn� prom�nn�
				// jm�no typu prom�nn�
				enum EVariableTypeName eVariableTypeName;
				// jm�no po�adovan�ho typu prom�nn�
				enum EVariableTypeName eRequiredTypeName;

				// p�iprav� jm�no typu prom�nn�
				switch ( m_eError )
				{
					case CFEStringToInteger :
					case CFEStringToReal :
					case CFEStringToBoolean :
						eVariableTypeName = VTNString;
						break;
					case CFEIntegerToString :
					case CFEIntegerToBoolean :
						eVariableTypeName = VTNInteger;
						break;
					case CFERealToString :
					case CFERealToInteger :
					case CFERealToBoolean :
						eVariableTypeName = VTNReal;
						break;
					case CFEBooleanToString :
					case CFEBooleanToInteger :
					case CFEBooleanToReal :
						eVariableTypeName = VTNBoolean;
						break;
					default :
						ASSERT ( FALSE );
				}

				// p�iprav� jm�no po�adovan�ho typu prom�nn�
				switch ( m_eError )
				{
					case CFERealToString :
					case CFEIntegerToString :
					case CFEBooleanToString :
						eRequiredTypeName = VTNString;
						break;
					case CFERealToInteger :
					case CFEStringToInteger :
					case CFEBooleanToInteger :
						eRequiredTypeName = VTNInteger;
						break;
					case CFEStringToReal :
					case CFEBooleanToReal :
						eRequiredTypeName = VTNReal;
						break;
					case CFEStringToBoolean :
					case CFEIntegerToBoolean :
					case CFERealToBoolean :
						eRequiredTypeName = VTNBoolean;
						break;
					default :
						ASSERT ( FALSE );
				}

				// p�iprav� text chyby
				strErrorText = _T("Run-time error: Cannot convert variable \"") + m_strText + 
					_T("\" from ") + GetVariableTypeName ( eVariableTypeName ) + _T(" to ") + 
					GetVariableTypeName ( eRequiredTypeName ) + _T(" (config file \"") + 
					m_fileInput.GetFilePath () + _T("\" in archive \"") + 
					m_fileInput.GetDataArchive ().GetArchivePath () + _T("\").");
			}
			else
			{	// nejedn� se o konverzn� chybu
				// p�iprav� text chyby
				switch ( m_eError )
				{
					case CFEUnknownVariable :
						strErrorText = _T("Run-time error: Unknown variable \"") + m_strText + 
							_T("\" (config file \"") + m_fileInput.GetFilePath () + _T("\" in archive \"") + 
							m_fileInput.GetDataArchive ().GetArchivePath () + _T("\").");
						break;
					case CFEUnknownSection :
						strErrorText = _T("Run-time error: Unknown section \"") + m_strText + 
							_T("\" (config file \"") + m_fileInput.GetFilePath () + _T("\" in archive \"") 
							+ m_fileInput.GetDataArchive ().GetArchivePath () + _T("\").");
						break;
					case CFELocationFile :
						strErrorText = _T("Critical error: Section location file error ") + 
							strTokenLocation + _T("\nDetails: ") + m_strText;
						break;
				}
			}
			// vr�t� text chyby
			return strErrorText;
		}
		else
		{	// jedn� se o semantickou chybu
			// p��prav� text semantick� chyby
			strErrorText = _T("Semantics error: ");
			// p��prav� text chyby
			switch ( m_eError )
			{
				// duplicitn� definice sekce
				case CFEDuplicitSection :
					strErrorText += _T("Duplicit section \"") + m_cLastToken.m_strText + 
						_T("\" definition");
					break;
				// ��sti sekce ji� byly definov�ny v jin�m souboru
				case CFEFragmentedSection :
					strErrorText += _T("Section \"") + m_cLastToken.m_strText + 
						_T("\" (or it's parts) already defined in another file");
					break;
				// duplicitn� definice prom�nn� v sekci
				case CFEDuplicitVariable :
					strErrorText += _T("Duplicit variable \"") + m_cLastToken.m_strText + 
						_T("\" definition");
					break;
			}
		}
	}
	else
	{	// jedn� se o syntaktickou chybu
		// p�iprav� text syntaktick� chyby
		strErrorText.Format ( _T("Syntax error: Unexpected ") );

		// jm�no tokenu
		CString strTokenName;

		// p�iprav� jm�no tokenu
		switch ( m_cLastToken.m_eToken )
		{
			case CConfigFileLexToken::CLTEndOfFile :
				strTokenName = _T("end of file");
				break;
			case CConfigFileLexToken::CLTVariable :
				strTokenName = _T("variable name");
				break;
			case CConfigFileLexToken::CLTEqual :
				strTokenName = _T("equal character");
				break;
			case CConfigFileLexToken::CLTString :
			case CConfigFileLexToken::CLTInteger :
			case CConfigFileLexToken::CLTReal :
			case CConfigFileLexToken::CLTHexNumber :
				strTokenName = _T("variable value");
				break;
			case CConfigFileLexToken::CLTSemicolon :
				strTokenName = _T("semicolon");
				break;
			case CConfigFileLexToken::CLTSectionBegin :
				strTokenName = _T("begin of section name");
				break;
			case CConfigFileLexToken::CLTSectionEnd :
				strTokenName = _T("end of section name");
				break;
			case CConfigFileLexToken::CLTSectionName :
				strTokenName = _T("section name");
				break;
			case CConfigFileLexToken::CLTSectionLocation :
				strTokenName = _T("section name file location");
				break;
		}

		// p�ipoj� k textu syntaktick� chyby jm�no tokenu
		strErrorText += strTokenName + _T(" - ");

		// p�iprav� jm�no o�ek�van�ho tokenu
		switch ( m_eError )
		{
			case CFEEndOfSectionExpected :
				strTokenName = _T("variable name or new section");
				break;
			case CFEEqualExpected :
				strTokenName = _T("equal character");
				break;
			case CFEVariableValueExpected :
				strTokenName = _T("variable value");
				break;
			case CFESemicolonExpected :
				strTokenName = _T("semicolon");
				break;
	// ************
	// ************
	// ************
	// ************
/*			case CFESectionNameExpected :
				strTokenName = _T("section name");
				break;*/
			case CFESectionEndExpected :
				strTokenName = _T("section end (after section location)");
				break;
			case CFESectionNameEndExpected :
				strTokenName = _T("section name end or section location");
				break;
			case CFENewSectionExpected :
				strTokenName = _T("new section (after definition of section with location)");
				break;
		}

		// p�ipoj� k textu syntaktick� chyby jm�no o�ek�van�ho tokenu
		strErrorText += strTokenName + _T(" expected");
	}

	// vr�t� v�slednou chybovou zpr�vu
	return strErrorText + strTokenLocation;
}

// buffer "lpszError" d�lky "nMaxError" vypln� chybovou zpr�vou v�jimky
BOOL CConfigFileException::GetErrorMessage ( LPTSTR lpszError, UINT nMaxError, 
	PUINT pnHelpContext /*= NULL*/ ) 
{
	// vypln� po�adovan� kontext helpu ne�kodnou hodnotou
	if ( pnHelpContext != NULL )
		*pnHelpContext = 0;

	// ukon�� nejdel�� mo�nou chybovou zpr�vu v bufferu
	lpszError[nMaxError] = 0x00;

	// zjist� chybovou zpr�vu
	CString strErrorMessage = GetErrorMessage ();
	// zkop�ruje do bufferu chybovou zpr�vu
	strcpy ( lpszError, (LPCSTR)( strErrorMessage.Left ( nMaxError - 1 ) ) );

	return TRUE;
}
