/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Konfiguraèní soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSectionInfo reprezentující informace
 *          o sekci konfiguraèního souboru
 * 
 ***********************************************************/

#include <stdlib.h>

#include "StdAfx.h"
#include "ConfigFile.h"

#include "ConfigFileException.h"

// ************************************************************
// konstanty

// offset v souboru oznaèující neexistující offset
const DWORD c_dwNotExistingOffset = 0xffffffff;

// ************************************************************
// globální funkce

// pøevede hexadecimální cifru 0-9,a-f,A-F na její èíselnou hodnotu
inline int GetHexadecimalDigitValue ( char cDigit ) 
{
	// zjistí, o jaký druh cifry se jedná
	if ( cDigit <= '9' )
	{	// jedná se o cifru 0-9
		ASSERT ( cDigit >= '0' );
		// vrátí hodnotu cifry 0-9
		return ( cDigit - '0' );
	}
	else
	{	// jedná se o cifry a-f,A-F
		if ( cDigit >= 'a' )
		{	// jedná se o cifru a-f
			ASSERT ( cDigit <= 'f' );
			// vrátí hodnotu cifry a-f
			return ( cDigit - 'a' + 10 );
		}
		else
		{	// se o cifru A-F
			ASSERT ( ( cDigit >= 'A' ) && ( cDigit <= 'F' ) );
			// vrátí hodnotu cifry A-F
			return ( cDigit - 'A' + 10 );
		}
	}
}

// ************************************************************
// tøída reprezentující informace o sekci konfiguraèního souboru
// ************************************************************

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// destruktor
CConfigFileSectionInfo::~CConfigFileSectionInfo () 
{
}

//////////////////////////////////////////////////////////////////////
// Incializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// naète z konfiguraèního souboru "pConfigFile" informace o sekci jménem "lpszName" 
//		z aktuální pozice v souboru a v "cLastToken" vrátí poslední naètený token, požívá 
//		lexikální analyzátor "cConfigFileLex"
//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::Read ( CConfigFile *pConfigFile, LPCSTR lpszName, 
	CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex ) 
{
	// nesmí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile == NULL );

	// uschová si informace o naèítané sekci
	m_pConfigFile = pConfigFile;
	m_strName = lpszName;
	m_dwOffset = pConfigFile->GetFile ().GetPosition ();

	// pøeète obsah sekce
	Read ( cLastToken, cConfigFileLex );
}

// pøeète z aktuální pozice v konfiguraèním souboru obsah sekce a v "cLastToken" vrátí 
//		poslední naètený token, požívá lexikální analyzátor "cConfigFileLex"
//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::Read ( CConfigFileLexToken &cLastToken, 
	CConfigFileLex &cConfigFileLex  ) 
{
// pomocné promìnné
	// pole jmen promìnných
	CStringArray aVariableNames;

	// inicializuje velikost tabulky øetìzcù jmen a hodnot promìnných
	m_nStringTableSize = 0;

	while ( 1 )
	{
		// naète z konfiguraèního souboru další token
		cConfigFileLex.GetToken ( cLastToken );

		// zjistí, následuje-li promìnná
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTVariable )
		{	// nenásleduje zaèátek promìnné
			// zjistí, zda byla sekce øádnì ukonèena
			if ( ( cLastToken.m_eToken == CConfigFileLexToken::CLTEndOfFile ) || 
				( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin ) )
			{	// sekce byla øádnì ukonèena
				// uschová si poèet promìnných v sekci
				m_nVariablesCount = aVariableNames.GetSize ();
				// vrátí poslední naètený token
				return;
			}
			else
			{	// sekce nebyla øádnì ukonèena
				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFEEndOfSectionExpected );
			}
		}
		// bylo pøeèteno jméno promìnné

		// zjistí, jedná-li se o duplicitní definici promìnné
		for ( int nIndex = aVariableNames.GetSize (); nIndex-- > 0; )
		{	// projede všechna jména promìnných
			// zjistí, jedná-li se o duplicitní definici promìnné
			if ( cLastToken.m_strText == aVariableNames[nIndex] )
			{	// jedná se o duplicitní definici promìnné
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEDuplicitVariable );
			}
		}

		// aktualizuje velikost tabulky øetìzcù jmen a hodnot promìnných 
		//		(zapoèítá i ukonèovací znak øetìzce)
		m_nStringTableSize += cLastToken.m_strText.GetLength () + 1;

		// pøidá jméno promìnné do pole jmen promìnných
		aVariableNames.Add ( cLastToken.m_strText );

		// naète z konfiguraèního souboru další token
		cConfigFileLex.GetToken ( cLastToken );

		// zjistí, následuje-li rovnítko
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTEqual )
		{	// nenásleduje støedník
			// hodí výjimku s informací o chybì konfiguraèního souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEEqualExpected );
		}
		// bylo pøeèteno rovnítko za jménem promìnné

		// naète z konfiguraèního souboru další token
		cConfigFileLex.GetToken ( cLastToken );

		// zjistí, následuje-li hodnota promìnné
		if ( ( cLastToken.m_eToken != CConfigFileLexToken::CLTString ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTInteger ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTReal ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTHexNumber ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTTrue ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTFalse ) )
		{	// nenásleduje hodnota promìnné
			// hodí výjimku s informací o chybì konfiguraèního souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEVariableValueExpected );
		}
		// byla pøeètena hodnota promìnné

		// zjistí, jedná-li se o øetìzcovou hodnotu promìnné
		if ( cLastToken.m_eToken == CConfigFileLexToken::CLTString )
		{	// jedná se o øetìzcovou promìnnou
			ASSERT ( cLastToken.m_strText.GetLength () > 0 );
			ASSERT ( cLastToken.m_strText[0] == '\"' );
			ASSERT ( cLastToken.m_strText[cLastToken.m_strText.GetLength () - 1] == '\"' );

			// zjistí, je-li hodnota promìnné prázdná
			if ( cLastToken.m_strText.GetLength () != 2 )
			{	// hodnota promìnné není prázdná
				// aktualizuje velikost tabulky øetìzcù jmen a hodnot promìnných 
				//		(nepoèítá uvozovky okolo øetìzce a zapoèítá i ukonèovací znak øetìzce)
				m_nStringTableSize += cLastToken.m_strText.GetLength () - 1;
			}
		}

		// naète z konfiguraèního souboru další token
		cConfigFileLex.GetToken ( cLastToken );

		// zjistí, následuje-li støedník
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTSemicolon )
		{	// nenásleduje støedník
			// hodí výjimku s informací o chybì konfiguraèního souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFESemicolonExpected );
		}
		// byl pøeèten celý záznam promìnné s hodnotou v sekci
	}
}

// znièí informace o sekci (vèetnì informací o podsekcích a vèetnì konfiguraèních 
//		souborù) (bSelfDelete - pøíznak volání destruktoru na sebe)
void CConfigFileSectionInfo::Delete ( BOOL bSelfDelete /*= FALSE*/ ) 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// nesmí existovat odkazy na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount == 0 );

	// ukazatel na informace o podsekci
	CConfigFileSectionInfo *pSubsectionInfo;

	// znièí všechny informace o podsekcích
	for ( int nIndex = m_aSubsections.GetSize (); nIndex > 0; )
	{	// projede všechny informace o podsekcích
		// získá ukazatel na informace o další podsekci
		pSubsectionInfo = (CConfigFileSectionInfo *)m_aSubsections[--nIndex];
		// znièí informace o podsekci
		pSubsectionInfo->Delete ( TRUE );
	}

	// uvolní pole informací o podsekcích
	m_aSubsections.RemoveAll ();

	// zjistí, má-li znièit sám sebe
	if ( bSelfDelete )
	{	// má znièit sám sebe
		// zjistí, je-li tato sekce hlavní sekcí konfiguraèního souboru
		if ( m_pConfigFile->GetRootSectionInfo () != this )
		{	// tato sekce není hlavní sekcí konfiguraèního souboru
			// znièí objekt informací o sekci
			delete this;
		}
		else
		{	// tato sekce je hlavní sekcí konfiguraèního souboru
			// znièí objekt konfiguraèního souboru
			delete m_pConfigFile;
		}
	}
}

// naète informace o podsekci této sekce z aktuální pozice (s prvním tokenem "cLastToken) 
//		v souboru a v "cLastToken" vrátí poslední naètený token, používá lexikální 
//		analyzátor "cConfigFileLex"
//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::ReadSubsection ( CConfigFileLexToken &cLastToken, 
	CConfigFileLex &cConfigFileLex ) 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );

	// musí být naèteno první jméno sekce
	ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionName );
	// byl pøeèten zaèátek nové sekce a nìkterá jména sekcí

	// ukazatel na informace o pùvodní podsekci
	CConfigFileSectionInfo *pSubsectionInfo = NULL;

	// zjistí, jedná-li se o již existující jméno podsekce
	int nIndex = m_aSubsections.GetSize ();
	while ( nIndex > 0 )
	{	// projede všechny informace o podsekcích
		// získá ukazatel na informace o další podsekci
		CConfigFileSectionInfo *pNextSubsectionInfo = 
			(CConfigFileSectionInfo *)m_aSubsections[nIndex-1];
		// zjistí, je-li to hledaná podsekce
		if ( pNextSubsectionInfo->m_strName == cLastToken.m_strText )
		{	// jedná se o hledanou podsekci
			// zjistí, jedná-li se o podsekci ze stejného konfiguraèního souboru
			if ( pNextSubsectionInfo->m_pConfigFile != m_pConfigFile )
			{	// jedná se o podsekci z jiného konfiguraèního souboru
				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFEFragmentedSection );
			}

			// inicializuje nalezenou podsekci
			pSubsectionInfo = pNextSubsectionInfo;
			break;
		}
		// nechá zkontrolovat informaci o další podsekci
		--nIndex;
	}

	// uschová si pùvodní naètený token
	CConfigFileLexToken cOriginalToken = cLastToken;

	// naète z konfiguraèního souboru další token
	cConfigFileLex.GetToken ( cLastToken );

	// zpracuje novì naètený token
	switch ( cLastToken.m_eToken )
	{
		// následuje jméno sekce
		case CConfigFileLexToken::CLTSectionName :
		{
			// zjistí, zda-li podsekce již existovala
			if ( pSubsectionInfo == NULL )
			{	// podsekce neexistovala
				// vytvoøí nový objekt informací o podsekci
				pSubsectionInfo = new CConfigFileSectionInfo ( m_pConfigFile );
				// inicializuje hodnoty novì vytvoøené podsekce
				pSubsectionInfo->m_pConfigFile = m_pConfigFile;
				pSubsectionInfo->m_strName = cOriginalToken.m_strText;
				pSubsectionInfo->m_dwOffset = c_dwNotExistingOffset;

				// uschová si novì vytvoøenou podsekci do seznamu podsekcí
				m_aSubsections.Add ( pSubsectionInfo );
			}

			// pokraèuje v naèítání podsekce v nové podsekci
			pSubsectionInfo->ReadSubsection ( cLastToken, cConfigFileLex );

			// ukonèí naèítání podsekce
			return;
		}
		// následuje konec sekce
		case CConfigFileLexToken::CLTSectionEnd :
			// zjistí, zda-li podsekce již existovala
			if ( pSubsectionInfo == NULL )
			{	// podsekce neexistovala
				// vytvoøí nový objekt informací o podsekci
				pSubsectionInfo = new CConfigFileSectionInfo ( m_pConfigFile );
				// inicializuje hodnoty novì vytvoøené podsekce
				pSubsectionInfo->m_pConfigFile = m_pConfigFile;
				pSubsectionInfo->m_strName = cOriginalToken.m_strText;
				pSubsectionInfo->m_dwOffset = m_pConfigFile->GetFile ().GetPosition ();

				// uschová si novì vytvoøenou podsekci do seznamu podsekcí
				m_aSubsections.Add ( pSubsectionInfo );
			}
			else
			{	// podsekce existovala
				// zjistí nejedná-li se o duplicitní definici sekce
				if ( pSubsectionInfo->m_dwOffset != c_dwNotExistingOffset )
				{	// jedná se o duplicitní definici sekce
					// hodí výjimku s informací o chybì konfiguraèního souboru
					throw new CConfigFileException ( m_pConfigFile->GetFile (), 
						cOriginalToken, CConfigFileException::CFEDuplicitSection );
				}
				// inicializuje hodnotu umístìní podsekce
				pSubsectionInfo->m_dwOffset = m_pConfigFile->GetFile ().GetPosition ();
			}

			// pøeète obsah podsekce
			pSubsectionInfo->Read ( cLastToken, cConfigFileLex );

			// ukonèí naèítání podsekce
			return;
		// následuje umístìní sekce v jiném souboru
		case CConfigFileLexToken::CLTSectionLocation :
		{
			// zjistí, zda-li podsekce již existovala
			if ( pSubsectionInfo != NULL )
			{	// podsekce existovala - jedná se o podsekci z jiného konfiguraèního souboru
				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), 
					cOriginalToken, CConfigFileException::CFEFragmentedSection );
			}

			// uschová si jméno umístìní sekce v jiném souboru
			CString strSectionLocation = cLastToken.m_strText;

			// naète z konfiguraèního souboru další token
			cConfigFileLex.GetToken ( cLastToken );

			// zjistí, následuje-li ukonèení sekce
			if ( cLastToken.m_eToken != CConfigFileLexToken::CLTSectionEnd )
			{	// nenásleduje ukonèení sekce
				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFESectionEndExpected );
			}
			// byla pøeètena celá definice sekce

			// naète z konfiguraèního souboru další token
			cConfigFileLex.GetToken ( cLastToken );

			// zjistí, následuje-li zaèátek nové sekce nebo konec souboru
			if ( ( cLastToken.m_eToken != CConfigFileLexToken::CLTSectionBegin ) && 
				( cLastToken.m_eToken != CConfigFileLexToken::CLTEndOfFile ) )
			{	// nenásleduje zaèátek nové sekce ani konec souboru
				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFENewSectionExpected );
			}
			// byla pøeètena celá definice sekce s umístìním v jiném souboru

		// pomocné promìnné
			// nový konfiguraèní soubor podsekce
			CConfigFile *pSubsectionConfigFile;
			// soubor s konfiguraèním souborem podsekce
			CArchiveFile fileSubsectionInput;

			// vytvoøí nový konfiguraèní soubor podsekce
			pSubsectionConfigFile = new CConfigFile;

			// uschová si hlavní sekci novì vytvoøeného konfiguraèního souboru do seznamu 
			//		podsekcí
			m_aSubsections.Add ( (void *)pSubsectionConfigFile->GetRootSectionInfo () );

			try
			{
				// pokusí se otevøít soubor s konfiguraèním souborem
				fileSubsectionInput = m_pConfigFile->GetFile ().GetDataArchive ().CreateFile ( 
					m_pConfigFile->GetFile ().GetFileDir () + _T("\\") + strSectionLocation, 
					CFile::modeRead | CFile::shareDenyWrite );
			}
			catch ( CDataArchiveException *pDataArchiveException )
			{
				// chybová zpráva výjimky souboru v archivu
				CString strFileError = pDataArchiveException->GetErrorMessage ();

				// znièí výjimku souboru v archivu
				pDataArchiveException->Delete ();

				// hodí výjimku s informací o chybì konfiguraèního souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					strFileError );
			}

			// nechá naèíst strukturu nového konfiguraèního souboru podsekce
			pSubsectionConfigFile->Create ( fileSubsectionInput, cOriginalToken.m_strText );

			// ukonèí naèítání podsekce
			return;
		}
		default :
			// hodí výjimku s informací o chybì konfiguraèního souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFESectionNameEndExpected );
	}
}

//////////////////////////////////////////////////////////////////////
// Operace se sekcí
//////////////////////////////////////////////////////////////////////

// vrátí jméno první podsekce "strName", které o tuto podsekci zkrátí
/*static*/ CString CConfigFileSectionInfo::GetSubsectionName ( CString &strName ) 
{
// pomocné promìnné
	// pozice zaèátku a konce jména podsekce
	int nSubsectionBegin, nSubsectionEnd;
	// délka jména sekce
	int nLength = strName.GetLength ();
	// aktuální pozice ve jménì podsekce
	int nPosition = 0;
	// aktuální znak jména
	char cChar;
	// jméno podsekce
	CString strSubsection;

	// pøeskoèí bílé znaky
	do
	{
		// zjistí, jedná-li se o konec jména sekce
		if ( nLength == nPosition )
		{	// jedná se o prázdné jméno sekce
			return CString ( _T("") );
		}

		// pøeète další znak ze jména sekce
		cChar = strName[nPosition++];
	} while ( ( cChar == ' ' ) || ( cChar == '\t' ) );

	// zaznamená jméno podsekce
	nSubsectionEnd = nSubsectionBegin = nPosition - 1;

	// zaznamená si jméno podsekce
	while ( cChar != '\\' )
	{
		// zjistí, jedná-li se o bílý znak
		if ( ( cChar != ' ' ) && ( cChar != '\t' ) )
		{	// nejedná se o bílý znak
			// aktualizuje konec jména podsekce
			nSubsectionEnd = nPosition;
		}

		// zjistí, jedná-li se o konec jména sekce
		if ( nLength == nPosition )
		{	// jedná se o konec jména sekce
			break;
		}
		// pøeète další znak ze jména sekce
		cChar = strName[nPosition++];
	}

	// vytvoøí jméno podsekce
	strSubsection = strName.Mid ( nSubsectionBegin, nSubsectionEnd - nSubsectionBegin );

	// zjistí, bylo-li zpracováno celé jméno sekce
	if ( nLength == nPosition )
	{	// bylo zpracováno celé jméno sekce
		// znièí jméno sekce
		strName.Empty ();
	}
	else
	{	// bylo zpracováno pouze jméno první podsekce
		// pøeskoèí jméno první podsekce
		strName = strName.Mid ( nPosition );
	}

	// vrátí jméno podsekce
	return strSubsection;
}

// vrátí sekci "lpszName" konfiguraèního souboru
CConfigFileSection CConfigFileSectionInfo::GetSection ( LPCSTR lpszName ) 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );

	// zjistí, má-li se vrátit root sekce
	if ( ( lpszName == NULL ) || ( *lpszName == 0x00 ) )
	{	// má se vrátit root sekce
		// vrátí root sekci
		return CConfigFileSection ( this );
	}

// pomocné promìnné
	// ukazatel na informace o aktuální podsekci
	CConfigFileSectionInfo *pSubsectionInfo = this;
	// øetìzec se jménem první podsekce
	CString strSubsectionName;
	// pracovní jméno sekce
	CString strName = lpszName;

	do
	{
		// zjistí jméno první podsekce
		strSubsectionName = GetSubsectionName ( strName );
		// zjistí, jedná-li se o platné jméno podsekce
		int nIndex = pSubsectionInfo->m_aSubsections.GetSize ();
		while ( nIndex > 0 )
		{	// projede všechny informace o podsekcích
			// získá ukazatel na informace o další podsekci
			CConfigFileSectionInfo *pNextSubsectionInfo = 
				(CConfigFileSectionInfo *)pSubsectionInfo->m_aSubsections[nIndex-1];
			// zjistí, je-li to hledaná podsekce
			if ( pNextSubsectionInfo->m_strName == strSubsectionName )
			{	// jedná se o hledanou podsekci
				// inicializuje nalezenou podsekci
				pSubsectionInfo = pNextSubsectionInfo;
				break;
			}
			// nechá zkontrolovat informaci o další podsekci
			--nIndex;
		}
		// zjistí, byla-li podsekce nalezena
		if ( nIndex == 0 )
		{	// podsekce nebyla nalezena
			// hodí výjimku s informací o chybì konfiguraèního souboru
			throw new CConfigFileException ( pSubsectionInfo->m_pConfigFile->GetFile (), 
				CConfigFileException::CFEUnknownSection, lpszName );
		}
	} while ( !strName.IsEmpty () );

	// nalezli jsme požadovanou podsekci
	return CConfigFileSection ( pSubsectionInfo );
}

// zvýší poèet odkazù na pole promìnných sekce
void CConfigFileSectionInfo::AddVariablesReference () 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// poèet odkazù na promìnné sekce musí být platný
	ASSERT ( m_dwVariablesReferenceCount >= 0 );

	// zjistí, jedná-li se o první odkaz na pole promìnných sekce
	if ( m_dwVariablesReferenceCount == 0 )
	{	// jedná se o první odkaz na pole promìnných

		// zjistí, jedná-li se o prázdnou sekci
		if ( m_nVariablesCount > 0 )
		{	// nejedná se o prázdnou sekci
			ASSERT ( m_nStringTableSize > 0 );

			// alokuje tabulku øetìzcù jmen a hodnot promìnných
			m_pStringTable = new char[m_nStringTableSize];
			// alokuje pole hodnot promìnných sekce
			m_pVariableValues = new struct SVariableValue[m_nVariablesCount];

		// pomocné promìnné
			// ukazatel na volné místo v tabulce øetìzcù jmen a hodnot promìnných
			char *pStringTablePosition = m_pStringTable;
			// poslední naètený token z konfiguraèního souboru
			CConfigFileLexToken cLastToken;
			// ukazatel na naèítanou hodnotu promìnné
			struct SVariableValue *pVariableValue;
			// lexikální analyzátor konfiguraèního souboru
			CConfigFileLex cConfigFileLex;

			// inicializuje lexikální analyzátor na promìnné sekce v konfiguraèním souboru
			cConfigFileLex.Create ( m_pConfigFile->GetFile (), m_dwOffset );

			// naète do pamìti všechny promìnné
			for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
			{
				// aktualizuje ukazatel na naèítanou hodnotu promìnné
				pVariableValue = m_pVariableValues + nVariableIndex;

				// naète z konfiguraèního souboru token jména promìnné
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTVariable );

				// zkopíruje jméno promìnné do tabulky øetìzcù
				strcpy ( pStringTablePosition, (LPCSTR)cLastToken.m_strText );
				// uschová si ukazatel na jméno promìnné v tabulce øetìzcù
				pVariableValue->m_lpszName = (LPCSTR)pStringTablePosition;

				// aktualizuje ukazatel na volné místo v tabulce øetìzcù
				pStringTablePosition += cLastToken.m_strText.GetLength () + 1;

				// naète z konfiguraèního souboru token rovnítka
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTEqual );

				// naète z konfiguraèního souboru token hodnoty promìnné
				cConfigFileLex.GetToken ( cLastToken );

				// zjistí, o jaký typ hodnoty promìnné se jedná
				switch ( cLastToken.m_eToken )
				{
				// øetìzec
					case CConfigFileLexToken::CLTString :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTString;

						ASSERT ( cLastToken.m_strText.GetLength () >= 2 );
						ASSERT ( cLastToken.m_strText[0] == '\"' );
						ASSERT ( cLastToken.m_strText[cLastToken.m_strText.GetLength () - 1] == '\"' );

					// pomocné promìnné
						// délka øetìzcové hodnoty promìnné
						int nLength = cLastToken.m_strText.GetLength () - 2;

						// zjistí, je-li øetìzec prázdný
						if ( nLength == 0 )
						{	// øetìzec je prázdný
							// posune ukazatel do tabulky øetìzcù na koncový znak øetìzce jména 
							//		promìnné øetìzce
							pStringTablePosition--;
						}

						// zkopíruje jméno promìnné do tabulky øetìzcù
						memcpy ( pStringTablePosition, ((const char *)cLastToken.m_strText) + 1, 
							nLength );

						// uschová si ukazatel na øetìzcovou hodnotu promìnné
						pVariableValue->m_lpszValue = (LPCSTR)pStringTablePosition;

						// posune ukazatel za poslední znak zkopírovaného øetìzce
						pStringTablePosition += nLength;
						// ukonèí øetìzec a aktualizuje ukazatel na volné místo v tabulce 
						//		øetìzcù
						*pStringTablePosition++ = 0x00;

						break;
					}
				// celé èíslo
					case CConfigFileLexToken::CLTInteger :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTInteger;

					// pomocné promìnné
						// naèítaná èíselná hodnota promìnné
						long nNumberValue = 0;
						// pøíznak naètení záporného znaménka
						BOOL bNumberNegative = FALSE;
						// zpracovávaná pozice v øetìzci èíselné hodnoty promìnné
						const char *lpszNumberPosition = (const char *)cLastToken.m_strText;

						ASSERT ( cLastToken.m_strText.GetLength () > 0 );

						// zjistí, zaèíná-li èíslo znaménkem
						switch ( *lpszNumberPosition )
						{
							case '-' :
								// nastaví pøíznak naètení záporného znaménka
								bNumberNegative = TRUE;
								// pøeskoèí znaménko
							case '+' :
								// pøeskoèí znaménko
								lpszNumberPosition++;
						}

						// naète vlastní èíselnou hodnotu
						while ( *lpszNumberPosition != 0x00 )
						{	// naète další cifru èísla
							ASSERT ( ( *lpszNumberPosition >= '0' ) && 
								( *lpszNumberPosition <= '9' ) );
							ASSERT ( (DWORD)nNumberValue <= 10 * ((DWORD)nNumberValue) + 
								( *lpszNumberPosition - '0' ) );

							// zapoèítá do výsledku další cifru èísla
							nNumberValue = 10 * nNumberValue + ( *lpszNumberPosition++ - '0' );
						}

						// zjistí, jedná-li se o záporné èíslo
						if ( bNumberNegative )
						{	// jedná se o záporné èíslo
							ASSERT ( nNumberValue >= 0 );
							// otoèí znaménko èísla
							nNumberValue = -nNumberValue;
						}

						// uschová si èíselnou hodnotu promìnné
						pVariableValue->m_nValue = nNumberValue;

						break;
					}
				// reálné èíslo
					case CConfigFileLexToken::CLTReal :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTReal;
#ifdef _DEBUG
					// pomocné promìnné
						// ukazatel za konec reálného èísla v øetìzci hodnoty promìnné
						char *lpszRealStringEnd;

						// nechá naèíst hodnotu promìnné
						pVariableValue->m_dbValue = strtod ( cLastToken.m_strText, 
							&lpszRealStringEnd );

						ASSERT ( *lpszRealStringEnd == 0x00 );
#else //!_DEBUG
						// nechá naèíst hodnotu promìnné
						pVariableValue->m_dbValue = strtod ( cLastToken.m_strText, NULL );
#endif //_DEBUG

						break;
					}
				// hexadecimální èíslo
					case CConfigFileLexToken::CLTHexNumber :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTInteger;

					// pomocné promìnné
						// naèítaná èíselná hodnota promìnné
						DWORD nNumberValue = 0;
						// zpracovávaná pozice v øetìzci èíselné hodnoty promìnné
						const char *lpszNumberPosition = ((const char *)cLastToken.m_strText) + 2;

						ASSERT ( cLastToken.m_strText.GetLength () > 2 );
						ASSERT ( cLastToken.m_strText[0] == '0' );
						ASSERT ( ( cLastToken.m_strText[1] == 'x' ) || 
							( cLastToken.m_strText[1] == 'X' ) );

						// naète vlastní èíselnou hodnotu
						while ( *lpszNumberPosition != 0x00 )
						{	// naète další cifru èísla
							ASSERT ( nNumberValue <= 16 * nNumberValue );

							// zapoèítá do výsledku další cifru èísla
							nNumberValue = 16 * nNumberValue + (DWORD)GetHexadecimalDigitValue ( 
								*lpszNumberPosition++ );
						}

						// uschová si èíselnou hodnotu promìnné
						pVariableValue->m_nValue = (long)nNumberValue;

						break;
					}
				// logická hodnota
					case CConfigFileLexToken::CLTTrue :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTBoolean;

						// uschová si hodnotu promìnní
						pVariableValue->m_bValue = TRUE;

						break;
					}
				// logická hodnota
					case CConfigFileLexToken::CLTFalse :
					{
						// uschová si typ hodnoty promìnné
						pVariableValue->m_eType = VVTBoolean;

						// uschová si hodnotu promìnní
						pVariableValue->m_bValue = FALSE;

						break;
					}
				// chybný typ hodnoty promìnné
					default:
						ASSERT ( FALSE );
				}

				// naète z konfiguraèního souboru token støedníku
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSemicolon );
			}
#ifdef _DEBUG
			// naète z konfiguraèního souboru token ukonèení sekce
			cConfigFileLex.GetToken ( cLastToken );

			ASSERT ( ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin ) || 
				( cLastToken.m_eToken == CConfigFileLexToken::CLTEndOfFile ) );
#endif //_DEBUG
		}
		else
		{	// jedná se o prázdnou sekci
			ASSERT ( m_nStringTableSize == 0 );
		}
	}

	// zvýší poèet odkazù na pole promìnných sekce
	m_dwVariablesReferenceCount++;
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vrátí pøíznak existence podsekce v sekci
BOOL CConfigFileSectionInfo::IsSubsection ( LPCSTR lpszSubsectionName ) const 
{
	// jméno podsekce nesmí být prázdné
	ASSERT ( ( lpszSubsectionName != NULL ) && ( *lpszSubsectionName != 0x00 ) );
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );

	// zjistí, jedná-li se o platné jméno podsekce
	int nIndex = m_aSubsections.GetSize ();
	while ( nIndex > 0 )
	{	// projede všechny informace o podsekcích
		// získá ukazatel na informace o další podsekci
		CConfigFileSectionInfo *pNextSubsectionInfo = 
			(CConfigFileSectionInfo *)m_aSubsections[nIndex-1];
		// zjistí, je-li to hledaná podsekce
		if ( pNextSubsectionInfo->m_strName == lpszSubsectionName )
		{	// jedná se o hledanou podsekci
			// vrátí pøíznak nalezení podsekce
			return TRUE;
		}
		// nechá zkontrolovat informaci o další podsekci
		--nIndex;
	}
	// nenalezli jsme požadovanou podsekci

	// vrátí pøíznak nenalezení podsekce
	return FALSE;
}

// vrátí pøíznak existence promìnné v sekci
BOOL CConfigFileSectionInfo::IsVariable ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// pokusí se najít požadovanou promìnnou
	for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
	{
		// zjistí, jedná-li se o hledanou promìnnou
		if ( strcmp ( lpszVariableName, m_pVariableValues[nVariableIndex].m_lpszName ) == 0 )
		{	// jedná se o hledanou promìnnou
			// vrátí pøíznak nalezení hodnoty promìnné
			return TRUE;
		}
	}
	// promìnná nebyla nalezena

	// vrátí pøíznak nenalezení hodnoty promìnné
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Ètení hodnot promìnných sekce
//////////////////////////////////////////////////////////////////////

// vrátí øetìzcovou hodnotu promìnné sekce
//    výjimky: CConfigFileException
LPCSTR CConfigFileSectionInfo::GetVariableStringValue ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocné promìnné
	// ukazatel na hodnotu promìnné
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjistí jedná-li se o øetìzcovou promìnnou
	if ( pVariableValue->m_eType == VVTString )
	{	// jedná se o øetìzcovou promìnnou
		// vrátí øetìzcovou hodnotu promìnné
		return pVariableValue->m_lpszValue;
	}
	// nejedná se o øetìzcovou promìnnou

// pomocné promìnné
	// chyba konverze typu promìnné
	enum CConfigFileException::EError eError;

	// zjistí chybu konverze typu promìnné
	switch ( pVariableValue->m_eType )
	{
		case VVTInteger :
			eError = CConfigFileException::CFEIntegerToString;
			break;
		case VVTReal :
			eError = CConfigFileException::CFERealToString;
			break;
		case VVTBoolean :
			eError = CConfigFileException::CFEBooleanToString;
			break;
		default :
			ASSERT ( FALSE );
	}

	// obslouží chybu - hodí výjimku s informací o chybì konfiguraèního souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vrátí èíselnou hodnotu promìnné sekce
//    výjimky: CConfigFileException
long CConfigFileSectionInfo::GetVariableIntegerValue ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocné promìnné
	// ukazatel na hodnotu promìnné
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjistí jedná-li se o èíselnou promìnnou
	if ( pVariableValue->m_eType == VVTInteger )
	{	// jedná se o èíselnou promìnnou
		// vrátí èíselnou hodnotu promìnné
		return pVariableValue->m_nValue;
	}
	// nejedná se o èíselnou promìnnou

// pomocné promìnné
	// chyba konverze typu promìnné
	enum CConfigFileException::EError eError;

	// zjistí chybu konverze typu promìnné
	switch ( pVariableValue->m_eType )
	{
		case VVTString :
			eError = CConfigFileException::CFEStringToInteger;
			break;
		case VVTReal :
			eError = CConfigFileException::CFERealToInteger;
			break;
		case VVTBoolean :
			eError = CConfigFileException::CFEBooleanToInteger;
			break;
		default :
			ASSERT ( FALSE );
	}

	// obslouží chybu - hodí výjimku s informací o chybì konfiguraèního souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vrátí reálnou hodnotu promìnné sekce
//    výjimky: CConfigFileException
double CConfigFileSectionInfo::GetVariableRealValue ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocné promìnné
	// ukazatel na hodnotu promìnné
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjistí jedná-li se o reálnou nebo èíselnou promìnnou
	switch ( pVariableValue->m_eType )
	{
		case VVTReal :
			// vrátí reálnou hodnotu promìnné
			return pVariableValue->m_dbValue;
		case VVTInteger :
			// vrátí reálnou hodnotu èíselné promìnné
			return (double)pVariableValue->m_nValue;
	}
	// nejedná se o reálnou ani èíselnou promìnnou

// pomocné promìnné
	// chyba konverze typu promìnné
	enum CConfigFileException::EError eError;

	// zjistí chybu konverze typu promìnné
	switch ( pVariableValue->m_eType )
	{
		case VVTString :
			eError = CConfigFileException::CFEStringToReal;
			break;
		case VVTBoolean :
			eError = CConfigFileException::CFEBooleanToReal;
			break;
		default :
			ASSERT ( FALSE );
	}

	// obslouží chybu - hodí výjimku s informací o chybì konfiguraèního souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vrátí logickou hodnotu promìnné sekce
//    výjimky: CConfigFileException
BOOL CConfigFileSectionInfo::GetVariableBooleanValue ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocné promìnné
	// ukazatel na hodnotu promìnné
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjistí jedná-li se o logickou promìnnou
	if ( pVariableValue->m_eType == VVTBoolean )
	{	// jedná se o logickou promìnnou
		// vrátí logickou hodnotu promìnné
		return pVariableValue->m_bValue;
	}
	// nejedná se o logickou promìnnou

// pomocné promìnné
	// chyba konverze typu promìnné
	enum CConfigFileException::EError eError;

	// zjistí chybu konverze typu promìnné
	switch ( pVariableValue->m_eType )
	{
		case VVTString :
			eError = CConfigFileException::CFEStringToBoolean;
			break;
		case VVTInteger :
			eError = CConfigFileException::CFEIntegerToBoolean;
			break;
		case VVTReal :
			eError = CConfigFileException::CFERealToBoolean;
			break;
		default :
			ASSERT ( FALSE );
	}

	// obslouží chybu - hodí výjimku s informací o chybì konfiguraèního souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vrátí index hodnoty promìnné v poli hodnot promìnných
//    výjimky: CConfigFileException
int CConfigFileSectionInfo::GetVariableIndex ( LPCSTR lpszVariableName ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// pokusí se najít požadovanou promìnnou
	for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
	{
		// zjistí, jedná-li se o hledanou promìnnou
		if ( strcmp ( lpszVariableName, m_pVariableValues[nVariableIndex].m_lpszName ) == 0 )
		{	// jedná se o hledanou promìnnou
			// vrátí index nalezené hodnoty promìnné
			return nVariableIndex;
		}
	}
	// promìnná nebyla nalezena

	// obslouží chybu - hodí výjimku s informací o chybì konfiguraèního souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), 
		CConfigFileException::CFEUnknownVariable, lpszVariableName );
}
