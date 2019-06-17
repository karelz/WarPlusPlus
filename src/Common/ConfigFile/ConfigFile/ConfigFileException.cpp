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

#include "StdAfx.h"
#include "ConfigFileException.h"

// ************************************************************
// konstanty

// jméno typu promìnné
enum EVariableTypeName
{
	VTNString	= 0,
	VTNInteger	= 1,
	VTNReal		= 2,
	VTNBoolean	= 3
};

// vrací jméno typu promìnné "eType"
inline const char *GetVariableTypeName ( enum EVariableTypeName eType ) 
{
	// tabulka jmen typù promìnných
	static LPCTSTR g_aVariableTypeNames[] = { _T("string"), _T("integer"), "real", "boolean" };

	// vrací jméno typu promìnné "eIndex"
	return g_aVariableTypeNames[(int)eType];
}

// ************************************************************
// tøída výjimky (chyby) konfiguraèního souboru
// ************************************************************

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, 
	CConfigFileLexToken cLastToken, EError eError ) 
{
	ASSERT ( fileInput.IsOpened () );

	// uschová si informace o chybì
	m_fileInput = fileInput;
	m_cLastToken = cLastToken;
	m_eError = eError;
}

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, 
	CConfigFileLexToken cLastToken, CString strFileError ) 
{
	ASSERT ( fileInput.IsOpened () );

	// uschová si informace o chybì
	m_fileInput = fileInput;
	m_cLastToken = cLastToken;
	m_eError = EError::CFELocationFile;
	m_strText = strFileError;
}

// konstruktor
CConfigFileException::CConfigFileException ( CArchiveFile fileInput, EError eError, 
	CString strText ) 
{
	// uschová si informace o chybì
	m_fileInput = fileInput;
	m_eError = eError;
	m_strText = strText;
}

// destruktor
CConfigFileException::~CConfigFileException () 
{
}

//////////////////////////////////////////////////////////////////////
// Správa výjimky
//////////////////////////////////////////////////////////////////////

// vrátí chybovou zprávu
CString CConfigFileException::GetErrorMessage () const 
{
	// umístìní posledního tokenu
	CString strTokenLocation;
	// text chyby konfiguraèního souboru
	CString strErrorText;
	// výsledná chybová zpráva
	CString strErrorMessage;

	// pøipraví text umístìní posledního tokenu
	strTokenLocation.Format ( _T(" (config file \"%s\" in archive \"%s\" at offset %d = " 
		"line %d, column %d)."), (const char *)m_fileInput.GetFilePath (), 
		(const char *)( m_fileInput.GetDataArchive ().GetArchivePath () ), 
		m_cLastToken.m_dwOffset, m_cLastToken.m_dwRow, m_cLastToken.m_wColumn );

	// zjistí, jedná-li se o lexikální chybu
	if ( m_cLastToken.m_eToken == CConfigFileLexToken::CLTError )
	{	// jedná se o lexikální chybu
		// pøipraví text lexikální chyby
		strErrorText.Format ( _T("Lexical error: Unknown token (character) \"%s\""), 
			(const char *)m_cLastToken.m_strText );
	}
	else if ( m_eError > CFELastSyntaxError )
	{	// jedná se o semantickou nebo kritickou chybu
		// zjistí, jedná-li se o semantickou chybu
		if ( m_eError > CFELastSemanticsError )
		{	// nejedná se o semantickou chybu
			// zjistí, jedná-li se o konverzní chybu
			if ( m_eError <= CFELastConversionError )
			{	// jedná se o konverzní chybu
			// pomocné promìnné
				// jméno typu promìnné
				enum EVariableTypeName eVariableTypeName;
				// jméno požadovaného typu promìnné
				enum EVariableTypeName eRequiredTypeName;

				// pøipraví jméno typu promìnné
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

				// pøipraví jméno požadovaného typu promìnné
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

				// pøipraví text chyby
				strErrorText = _T("Run-time error: Cannot convert variable \"") + m_strText + 
					_T("\" from ") + GetVariableTypeName ( eVariableTypeName ) + _T(" to ") + 
					GetVariableTypeName ( eRequiredTypeName ) + _T(" (config file \"") + 
					m_fileInput.GetFilePath () + _T("\" in archive \"") + 
					m_fileInput.GetDataArchive ().GetArchivePath () + _T("\").");
			}
			else
			{	// nejedná se o konverzní chybu
				// pøipraví text chyby
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
			// vrátí text chyby
			return strErrorText;
		}
		else
		{	// jedná se o semantickou chybu
			// pøípraví text semantické chyby
			strErrorText = _T("Semantics error: ");
			// pøípraví text chyby
			switch ( m_eError )
			{
				// duplicitní definice sekce
				case CFEDuplicitSection :
					strErrorText += _T("Duplicit section \"") + m_cLastToken.m_strText + 
						_T("\" definition");
					break;
				// èásti sekce již byly definovány v jiném souboru
				case CFEFragmentedSection :
					strErrorText += _T("Section \"") + m_cLastToken.m_strText + 
						_T("\" (or it's parts) already defined in another file");
					break;
				// duplicitní definice promìnné v sekci
				case CFEDuplicitVariable :
					strErrorText += _T("Duplicit variable \"") + m_cLastToken.m_strText + 
						_T("\" definition");
					break;
			}
		}
	}
	else
	{	// jedná se o syntaktickou chybu
		// pøipraví text syntaktické chyby
		strErrorText.Format ( _T("Syntax error: Unexpected ") );

		// jméno tokenu
		CString strTokenName;

		// pøipraví jméno tokenu
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

		// pøipojí k textu syntaktické chyby jméno tokenu
		strErrorText += strTokenName + _T(" - ");

		// pøipraví jméno oèekávaného tokenu
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

		// pøipojí k textu syntaktické chyby jméno oèekávaného tokenu
		strErrorText += strTokenName + _T(" expected");
	}

	// vrátí výslednou chybovou zprávu
	return strErrorText + strTokenLocation;
}

// buffer "lpszError" délky "nMaxError" vyplní chybovou zprávou výjimky
BOOL CConfigFileException::GetErrorMessage ( LPTSTR lpszError, UINT nMaxError, 
	PUINT pnHelpContext /*= NULL*/ ) 
{
	// vyplní požadovaný kontext helpu neškodnou hodnotou
	if ( pnHelpContext != NULL )
		*pnHelpContext = 0;

	// ukonèí nejdelší možnou chybovou zprávu v bufferu
	lpszError[nMaxError] = 0x00;

	// zjistí chybovou zprávu
	CString strErrorMessage = GetErrorMessage ();
	// zkopíruje do bufferu chybovou zprávu
	strcpy ( lpszError, (LPCSTR)( strErrorMessage.Left ( nMaxError - 1 ) ) );

	return TRUE;
}
