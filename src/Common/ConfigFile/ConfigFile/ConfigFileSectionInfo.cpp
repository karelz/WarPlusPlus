/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Konfigura�n� soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSectionInfo reprezentuj�c� informace
 *          o sekci konfigura�n�ho souboru
 * 
 ***********************************************************/

#include <stdlib.h>

#include "StdAfx.h"
#include "ConfigFile.h"

#include "ConfigFileException.h"

// ************************************************************
// konstanty

// offset v souboru ozna�uj�c� neexistuj�c� offset
const DWORD c_dwNotExistingOffset = 0xffffffff;

// ************************************************************
// glob�ln� funkce

// p�evede hexadecim�ln� cifru 0-9,a-f,A-F na jej� ��selnou hodnotu
inline int GetHexadecimalDigitValue ( char cDigit ) 
{
	// zjist�, o jak� druh cifry se jedn�
	if ( cDigit <= '9' )
	{	// jedn� se o cifru 0-9
		ASSERT ( cDigit >= '0' );
		// vr�t� hodnotu cifry 0-9
		return ( cDigit - '0' );
	}
	else
	{	// jedn� se o cifry a-f,A-F
		if ( cDigit >= 'a' )
		{	// jedn� se o cifru a-f
			ASSERT ( cDigit <= 'f' );
			// vr�t� hodnotu cifry a-f
			return ( cDigit - 'a' + 10 );
		}
		else
		{	// se o cifru A-F
			ASSERT ( ( cDigit >= 'A' ) && ( cDigit <= 'F' ) );
			// vr�t� hodnotu cifry A-F
			return ( cDigit - 'A' + 10 );
		}
	}
}

// ************************************************************
// t��da reprezentuj�c� informace o sekci konfigura�n�ho souboru
// ************************************************************

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// destruktor
CConfigFileSectionInfo::~CConfigFileSectionInfo () 
{
}

//////////////////////////////////////////////////////////////////////
// Incializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// na�te z konfigura�n�ho souboru "pConfigFile" informace o sekci jm�nem "lpszName" 
//		z aktu�ln� pozice v souboru a v "cLastToken" vr�t� posledn� na�ten� token, po��v� 
//		lexik�ln� analyz�tor "cConfigFileLex"
//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::Read ( CConfigFile *pConfigFile, LPCSTR lpszName, 
	CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex ) 
{
	// nesm� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile == NULL );

	// uschov� si informace o na��tan� sekci
	m_pConfigFile = pConfigFile;
	m_strName = lpszName;
	m_dwOffset = pConfigFile->GetFile ().GetPosition ();

	// p�e�te obsah sekce
	Read ( cLastToken, cConfigFileLex );
}

// p�e�te z aktu�ln� pozice v konfigura�n�m souboru obsah sekce a v "cLastToken" vr�t� 
//		posledn� na�ten� token, po��v� lexik�ln� analyz�tor "cConfigFileLex"
//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::Read ( CConfigFileLexToken &cLastToken, 
	CConfigFileLex &cConfigFileLex  ) 
{
// pomocn� prom�nn�
	// pole jmen prom�nn�ch
	CStringArray aVariableNames;

	// inicializuje velikost tabulky �et�zc� jmen a hodnot prom�nn�ch
	m_nStringTableSize = 0;

	while ( 1 )
	{
		// na�te z konfigura�n�ho souboru dal�� token
		cConfigFileLex.GetToken ( cLastToken );

		// zjist�, n�sleduje-li prom�nn�
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTVariable )
		{	// nen�sleduje za��tek prom�nn�
			// zjist�, zda byla sekce ��dn� ukon�ena
			if ( ( cLastToken.m_eToken == CConfigFileLexToken::CLTEndOfFile ) || 
				( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin ) )
			{	// sekce byla ��dn� ukon�ena
				// uschov� si po�et prom�nn�ch v sekci
				m_nVariablesCount = aVariableNames.GetSize ();
				// vr�t� posledn� na�ten� token
				return;
			}
			else
			{	// sekce nebyla ��dn� ukon�ena
				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFEEndOfSectionExpected );
			}
		}
		// bylo p�e�teno jm�no prom�nn�

		// zjist�, jedn�-li se o duplicitn� definici prom�nn�
		for ( int nIndex = aVariableNames.GetSize (); nIndex-- > 0; )
		{	// projede v�echna jm�na prom�nn�ch
			// zjist�, jedn�-li se o duplicitn� definici prom�nn�
			if ( cLastToken.m_strText == aVariableNames[nIndex] )
			{	// jedn� se o duplicitn� definici prom�nn�
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEDuplicitVariable );
			}
		}

		// aktualizuje velikost tabulky �et�zc� jmen a hodnot prom�nn�ch 
		//		(zapo��t� i ukon�ovac� znak �et�zce)
		m_nStringTableSize += cLastToken.m_strText.GetLength () + 1;

		// p�id� jm�no prom�nn� do pole jmen prom�nn�ch
		aVariableNames.Add ( cLastToken.m_strText );

		// na�te z konfigura�n�ho souboru dal�� token
		cConfigFileLex.GetToken ( cLastToken );

		// zjist�, n�sleduje-li rovn�tko
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTEqual )
		{	// nen�sleduje st�edn�k
			// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEEqualExpected );
		}
		// bylo p�e�teno rovn�tko za jm�nem prom�nn�

		// na�te z konfigura�n�ho souboru dal�� token
		cConfigFileLex.GetToken ( cLastToken );

		// zjist�, n�sleduje-li hodnota prom�nn�
		if ( ( cLastToken.m_eToken != CConfigFileLexToken::CLTString ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTInteger ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTReal ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTHexNumber ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTTrue ) && 
			( cLastToken.m_eToken != CConfigFileLexToken::CLTFalse ) )
		{	// nen�sleduje hodnota prom�nn�
			// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFEVariableValueExpected );
		}
		// byla p�e�tena hodnota prom�nn�

		// zjist�, jedn�-li se o �et�zcovou hodnotu prom�nn�
		if ( cLastToken.m_eToken == CConfigFileLexToken::CLTString )
		{	// jedn� se o �et�zcovou prom�nnou
			ASSERT ( cLastToken.m_strText.GetLength () > 0 );
			ASSERT ( cLastToken.m_strText[0] == '\"' );
			ASSERT ( cLastToken.m_strText[cLastToken.m_strText.GetLength () - 1] == '\"' );

			// zjist�, je-li hodnota prom�nn� pr�zdn�
			if ( cLastToken.m_strText.GetLength () != 2 )
			{	// hodnota prom�nn� nen� pr�zdn�
				// aktualizuje velikost tabulky �et�zc� jmen a hodnot prom�nn�ch 
				//		(nepo��t� uvozovky okolo �et�zce a zapo��t� i ukon�ovac� znak �et�zce)
				m_nStringTableSize += cLastToken.m_strText.GetLength () - 1;
			}
		}

		// na�te z konfigura�n�ho souboru dal�� token
		cConfigFileLex.GetToken ( cLastToken );

		// zjist�, n�sleduje-li st�edn�k
		if ( cLastToken.m_eToken != CConfigFileLexToken::CLTSemicolon )
		{	// nen�sleduje st�edn�k
			// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFESemicolonExpected );
		}
		// byl p�e�ten cel� z�znam prom�nn� s hodnotou v sekci
	}
}

// zni�� informace o sekci (v�etn� informac� o podsekc�ch a v�etn� konfigura�n�ch 
//		soubor�) (bSelfDelete - p��znak vol�n� destruktoru na sebe)
void CConfigFileSectionInfo::Delete ( BOOL bSelfDelete /*= FALSE*/ ) 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// nesm� existovat odkazy na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount == 0 );

	// ukazatel na informace o podsekci
	CConfigFileSectionInfo *pSubsectionInfo;

	// zni�� v�echny informace o podsekc�ch
	for ( int nIndex = m_aSubsections.GetSize (); nIndex > 0; )
	{	// projede v�echny informace o podsekc�ch
		// z�sk� ukazatel na informace o dal�� podsekci
		pSubsectionInfo = (CConfigFileSectionInfo *)m_aSubsections[--nIndex];
		// zni�� informace o podsekci
		pSubsectionInfo->Delete ( TRUE );
	}

	// uvoln� pole informac� o podsekc�ch
	m_aSubsections.RemoveAll ();

	// zjist�, m�-li zni�it s�m sebe
	if ( bSelfDelete )
	{	// m� zni�it s�m sebe
		// zjist�, je-li tato sekce hlavn� sekc� konfigura�n�ho souboru
		if ( m_pConfigFile->GetRootSectionInfo () != this )
		{	// tato sekce nen� hlavn� sekc� konfigura�n�ho souboru
			// zni�� objekt informac� o sekci
			delete this;
		}
		else
		{	// tato sekce je hlavn� sekc� konfigura�n�ho souboru
			// zni�� objekt konfigura�n�ho souboru
			delete m_pConfigFile;
		}
	}
}

// na�te informace o podsekci t�to sekce z aktu�ln� pozice (s prvn�m tokenem "cLastToken) 
//		v souboru a v "cLastToken" vr�t� posledn� na�ten� token, pou��v� lexik�ln� 
//		analyz�tor "cConfigFileLex"
//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
void CConfigFileSectionInfo::ReadSubsection ( CConfigFileLexToken &cLastToken, 
	CConfigFileLex &cConfigFileLex ) 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );

	// mus� b�t na�teno prvn� jm�no sekce
	ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionName );
	// byl p�e�ten za��tek nov� sekce a n�kter� jm�na sekc�

	// ukazatel na informace o p�vodn� podsekci
	CConfigFileSectionInfo *pSubsectionInfo = NULL;

	// zjist�, jedn�-li se o ji� existuj�c� jm�no podsekce
	int nIndex = m_aSubsections.GetSize ();
	while ( nIndex > 0 )
	{	// projede v�echny informace o podsekc�ch
		// z�sk� ukazatel na informace o dal�� podsekci
		CConfigFileSectionInfo *pNextSubsectionInfo = 
			(CConfigFileSectionInfo *)m_aSubsections[nIndex-1];
		// zjist�, je-li to hledan� podsekce
		if ( pNextSubsectionInfo->m_strName == cLastToken.m_strText )
		{	// jedn� se o hledanou podsekci
			// zjist�, jedn�-li se o podsekci ze stejn�ho konfigura�n�ho souboru
			if ( pNextSubsectionInfo->m_pConfigFile != m_pConfigFile )
			{	// jedn� se o podsekci z jin�ho konfigura�n�ho souboru
				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFEFragmentedSection );
			}

			// inicializuje nalezenou podsekci
			pSubsectionInfo = pNextSubsectionInfo;
			break;
		}
		// nech� zkontrolovat informaci o dal�� podsekci
		--nIndex;
	}

	// uschov� si p�vodn� na�ten� token
	CConfigFileLexToken cOriginalToken = cLastToken;

	// na�te z konfigura�n�ho souboru dal�� token
	cConfigFileLex.GetToken ( cLastToken );

	// zpracuje nov� na�ten� token
	switch ( cLastToken.m_eToken )
	{
		// n�sleduje jm�no sekce
		case CConfigFileLexToken::CLTSectionName :
		{
			// zjist�, zda-li podsekce ji� existovala
			if ( pSubsectionInfo == NULL )
			{	// podsekce neexistovala
				// vytvo�� nov� objekt informac� o podsekci
				pSubsectionInfo = new CConfigFileSectionInfo ( m_pConfigFile );
				// inicializuje hodnoty nov� vytvo�en� podsekce
				pSubsectionInfo->m_pConfigFile = m_pConfigFile;
				pSubsectionInfo->m_strName = cOriginalToken.m_strText;
				pSubsectionInfo->m_dwOffset = c_dwNotExistingOffset;

				// uschov� si nov� vytvo�enou podsekci do seznamu podsekc�
				m_aSubsections.Add ( pSubsectionInfo );
			}

			// pokra�uje v na��t�n� podsekce v nov� podsekci
			pSubsectionInfo->ReadSubsection ( cLastToken, cConfigFileLex );

			// ukon�� na��t�n� podsekce
			return;
		}
		// n�sleduje konec sekce
		case CConfigFileLexToken::CLTSectionEnd :
			// zjist�, zda-li podsekce ji� existovala
			if ( pSubsectionInfo == NULL )
			{	// podsekce neexistovala
				// vytvo�� nov� objekt informac� o podsekci
				pSubsectionInfo = new CConfigFileSectionInfo ( m_pConfigFile );
				// inicializuje hodnoty nov� vytvo�en� podsekce
				pSubsectionInfo->m_pConfigFile = m_pConfigFile;
				pSubsectionInfo->m_strName = cOriginalToken.m_strText;
				pSubsectionInfo->m_dwOffset = m_pConfigFile->GetFile ().GetPosition ();

				// uschov� si nov� vytvo�enou podsekci do seznamu podsekc�
				m_aSubsections.Add ( pSubsectionInfo );
			}
			else
			{	// podsekce existovala
				// zjist� nejedn�-li se o duplicitn� definici sekce
				if ( pSubsectionInfo->m_dwOffset != c_dwNotExistingOffset )
				{	// jedn� se o duplicitn� definici sekce
					// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
					throw new CConfigFileException ( m_pConfigFile->GetFile (), 
						cOriginalToken, CConfigFileException::CFEDuplicitSection );
				}
				// inicializuje hodnotu um�st�n� podsekce
				pSubsectionInfo->m_dwOffset = m_pConfigFile->GetFile ().GetPosition ();
			}

			// p�e�te obsah podsekce
			pSubsectionInfo->Read ( cLastToken, cConfigFileLex );

			// ukon�� na��t�n� podsekce
			return;
		// n�sleduje um�st�n� sekce v jin�m souboru
		case CConfigFileLexToken::CLTSectionLocation :
		{
			// zjist�, zda-li podsekce ji� existovala
			if ( pSubsectionInfo != NULL )
			{	// podsekce existovala - jedn� se o podsekci z jin�ho konfigura�n�ho souboru
				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), 
					cOriginalToken, CConfigFileException::CFEFragmentedSection );
			}

			// uschov� si jm�no um�st�n� sekce v jin�m souboru
			CString strSectionLocation = cLastToken.m_strText;

			// na�te z konfigura�n�ho souboru dal�� token
			cConfigFileLex.GetToken ( cLastToken );

			// zjist�, n�sleduje-li ukon�en� sekce
			if ( cLastToken.m_eToken != CConfigFileLexToken::CLTSectionEnd )
			{	// nen�sleduje ukon�en� sekce
				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFESectionEndExpected );
			}
			// byla p�e�tena cel� definice sekce

			// na�te z konfigura�n�ho souboru dal�� token
			cConfigFileLex.GetToken ( cLastToken );

			// zjist�, n�sleduje-li za��tek nov� sekce nebo konec souboru
			if ( ( cLastToken.m_eToken != CConfigFileLexToken::CLTSectionBegin ) && 
				( cLastToken.m_eToken != CConfigFileLexToken::CLTEndOfFile ) )
			{	// nen�sleduje za��tek nov� sekce ani konec souboru
				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					CConfigFileException::CFENewSectionExpected );
			}
			// byla p�e�tena cel� definice sekce s um�st�n�m v jin�m souboru

		// pomocn� prom�nn�
			// nov� konfigura�n� soubor podsekce
			CConfigFile *pSubsectionConfigFile;
			// soubor s konfigura�n�m souborem podsekce
			CArchiveFile fileSubsectionInput;

			// vytvo�� nov� konfigura�n� soubor podsekce
			pSubsectionConfigFile = new CConfigFile;

			// uschov� si hlavn� sekci nov� vytvo�en�ho konfigura�n�ho souboru do seznamu 
			//		podsekc�
			m_aSubsections.Add ( (void *)pSubsectionConfigFile->GetRootSectionInfo () );

			try
			{
				// pokus� se otev��t soubor s konfigura�n�m souborem
				fileSubsectionInput = m_pConfigFile->GetFile ().GetDataArchive ().CreateFile ( 
					m_pConfigFile->GetFile ().GetFileDir () + _T("\\") + strSectionLocation, 
					CFile::modeRead | CFile::shareDenyWrite );
			}
			catch ( CDataArchiveException *pDataArchiveException )
			{
				// chybov� zpr�va v�jimky souboru v archivu
				CString strFileError = pDataArchiveException->GetErrorMessage ();

				// zni�� v�jimku souboru v archivu
				pDataArchiveException->Delete ();

				// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
				throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
					strFileError );
			}

			// nech� na��st strukturu nov�ho konfigura�n�ho souboru podsekce
			pSubsectionConfigFile->Create ( fileSubsectionInput, cOriginalToken.m_strText );

			// ukon�� na��t�n� podsekce
			return;
		}
		default :
			// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
			throw new CConfigFileException ( m_pConfigFile->GetFile (), cLastToken, 
				CConfigFileException::CFESectionNameEndExpected );
	}
}

//////////////////////////////////////////////////////////////////////
// Operace se sekc�
//////////////////////////////////////////////////////////////////////

// vr�t� jm�no prvn� podsekce "strName", kter� o tuto podsekci zkr�t�
/*static*/ CString CConfigFileSectionInfo::GetSubsectionName ( CString &strName ) 
{
// pomocn� prom�nn�
	// pozice za��tku a konce jm�na podsekce
	int nSubsectionBegin, nSubsectionEnd;
	// d�lka jm�na sekce
	int nLength = strName.GetLength ();
	// aktu�ln� pozice ve jm�n� podsekce
	int nPosition = 0;
	// aktu�ln� znak jm�na
	char cChar;
	// jm�no podsekce
	CString strSubsection;

	// p�esko�� b�l� znaky
	do
	{
		// zjist�, jedn�-li se o konec jm�na sekce
		if ( nLength == nPosition )
		{	// jedn� se o pr�zdn� jm�no sekce
			return CString ( _T("") );
		}

		// p�e�te dal�� znak ze jm�na sekce
		cChar = strName[nPosition++];
	} while ( ( cChar == ' ' ) || ( cChar == '\t' ) );

	// zaznamen� jm�no podsekce
	nSubsectionEnd = nSubsectionBegin = nPosition - 1;

	// zaznamen� si jm�no podsekce
	while ( cChar != '\\' )
	{
		// zjist�, jedn�-li se o b�l� znak
		if ( ( cChar != ' ' ) && ( cChar != '\t' ) )
		{	// nejedn� se o b�l� znak
			// aktualizuje konec jm�na podsekce
			nSubsectionEnd = nPosition;
		}

		// zjist�, jedn�-li se o konec jm�na sekce
		if ( nLength == nPosition )
		{	// jedn� se o konec jm�na sekce
			break;
		}
		// p�e�te dal�� znak ze jm�na sekce
		cChar = strName[nPosition++];
	}

	// vytvo�� jm�no podsekce
	strSubsection = strName.Mid ( nSubsectionBegin, nSubsectionEnd - nSubsectionBegin );

	// zjist�, bylo-li zpracov�no cel� jm�no sekce
	if ( nLength == nPosition )
	{	// bylo zpracov�no cel� jm�no sekce
		// zni�� jm�no sekce
		strName.Empty ();
	}
	else
	{	// bylo zpracov�no pouze jm�no prvn� podsekce
		// p�esko�� jm�no prvn� podsekce
		strName = strName.Mid ( nPosition );
	}

	// vr�t� jm�no podsekce
	return strSubsection;
}

// vr�t� sekci "lpszName" konfigura�n�ho souboru
CConfigFileSection CConfigFileSectionInfo::GetSection ( LPCSTR lpszName ) 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );

	// zjist�, m�-li se vr�tit root sekce
	if ( ( lpszName == NULL ) || ( *lpszName == 0x00 ) )
	{	// m� se vr�tit root sekce
		// vr�t� root sekci
		return CConfigFileSection ( this );
	}

// pomocn� prom�nn�
	// ukazatel na informace o aktu�ln� podsekci
	CConfigFileSectionInfo *pSubsectionInfo = this;
	// �et�zec se jm�nem prvn� podsekce
	CString strSubsectionName;
	// pracovn� jm�no sekce
	CString strName = lpszName;

	do
	{
		// zjist� jm�no prvn� podsekce
		strSubsectionName = GetSubsectionName ( strName );
		// zjist�, jedn�-li se o platn� jm�no podsekce
		int nIndex = pSubsectionInfo->m_aSubsections.GetSize ();
		while ( nIndex > 0 )
		{	// projede v�echny informace o podsekc�ch
			// z�sk� ukazatel na informace o dal�� podsekci
			CConfigFileSectionInfo *pNextSubsectionInfo = 
				(CConfigFileSectionInfo *)pSubsectionInfo->m_aSubsections[nIndex-1];
			// zjist�, je-li to hledan� podsekce
			if ( pNextSubsectionInfo->m_strName == strSubsectionName )
			{	// jedn� se o hledanou podsekci
				// inicializuje nalezenou podsekci
				pSubsectionInfo = pNextSubsectionInfo;
				break;
			}
			// nech� zkontrolovat informaci o dal�� podsekci
			--nIndex;
		}
		// zjist�, byla-li podsekce nalezena
		if ( nIndex == 0 )
		{	// podsekce nebyla nalezena
			// hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
			throw new CConfigFileException ( pSubsectionInfo->m_pConfigFile->GetFile (), 
				CConfigFileException::CFEUnknownSection, lpszName );
		}
	} while ( !strName.IsEmpty () );

	// nalezli jsme po�adovanou podsekci
	return CConfigFileSection ( pSubsectionInfo );
}

// zv��� po�et odkaz� na pole prom�nn�ch sekce
void CConfigFileSectionInfo::AddVariablesReference () 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// po�et odkaz� na prom�nn� sekce mus� b�t platn�
	ASSERT ( m_dwVariablesReferenceCount >= 0 );

	// zjist�, jedn�-li se o prvn� odkaz na pole prom�nn�ch sekce
	if ( m_dwVariablesReferenceCount == 0 )
	{	// jedn� se o prvn� odkaz na pole prom�nn�ch

		// zjist�, jedn�-li se o pr�zdnou sekci
		if ( m_nVariablesCount > 0 )
		{	// nejedn� se o pr�zdnou sekci
			ASSERT ( m_nStringTableSize > 0 );

			// alokuje tabulku �et�zc� jmen a hodnot prom�nn�ch
			m_pStringTable = new char[m_nStringTableSize];
			// alokuje pole hodnot prom�nn�ch sekce
			m_pVariableValues = new struct SVariableValue[m_nVariablesCount];

		// pomocn� prom�nn�
			// ukazatel na voln� m�sto v tabulce �et�zc� jmen a hodnot prom�nn�ch
			char *pStringTablePosition = m_pStringTable;
			// posledn� na�ten� token z konfigura�n�ho souboru
			CConfigFileLexToken cLastToken;
			// ukazatel na na��tanou hodnotu prom�nn�
			struct SVariableValue *pVariableValue;
			// lexik�ln� analyz�tor konfigura�n�ho souboru
			CConfigFileLex cConfigFileLex;

			// inicializuje lexik�ln� analyz�tor na prom�nn� sekce v konfigura�n�m souboru
			cConfigFileLex.Create ( m_pConfigFile->GetFile (), m_dwOffset );

			// na�te do pam�ti v�echny prom�nn�
			for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
			{
				// aktualizuje ukazatel na na��tanou hodnotu prom�nn�
				pVariableValue = m_pVariableValues + nVariableIndex;

				// na�te z konfigura�n�ho souboru token jm�na prom�nn�
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTVariable );

				// zkop�ruje jm�no prom�nn� do tabulky �et�zc�
				strcpy ( pStringTablePosition, (LPCSTR)cLastToken.m_strText );
				// uschov� si ukazatel na jm�no prom�nn� v tabulce �et�zc�
				pVariableValue->m_lpszName = (LPCSTR)pStringTablePosition;

				// aktualizuje ukazatel na voln� m�sto v tabulce �et�zc�
				pStringTablePosition += cLastToken.m_strText.GetLength () + 1;

				// na�te z konfigura�n�ho souboru token rovn�tka
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTEqual );

				// na�te z konfigura�n�ho souboru token hodnoty prom�nn�
				cConfigFileLex.GetToken ( cLastToken );

				// zjist�, o jak� typ hodnoty prom�nn� se jedn�
				switch ( cLastToken.m_eToken )
				{
				// �et�zec
					case CConfigFileLexToken::CLTString :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTString;

						ASSERT ( cLastToken.m_strText.GetLength () >= 2 );
						ASSERT ( cLastToken.m_strText[0] == '\"' );
						ASSERT ( cLastToken.m_strText[cLastToken.m_strText.GetLength () - 1] == '\"' );

					// pomocn� prom�nn�
						// d�lka �et�zcov� hodnoty prom�nn�
						int nLength = cLastToken.m_strText.GetLength () - 2;

						// zjist�, je-li �et�zec pr�zdn�
						if ( nLength == 0 )
						{	// �et�zec je pr�zdn�
							// posune ukazatel do tabulky �et�zc� na koncov� znak �et�zce jm�na 
							//		prom�nn� �et�zce
							pStringTablePosition--;
						}

						// zkop�ruje jm�no prom�nn� do tabulky �et�zc�
						memcpy ( pStringTablePosition, ((const char *)cLastToken.m_strText) + 1, 
							nLength );

						// uschov� si ukazatel na �et�zcovou hodnotu prom�nn�
						pVariableValue->m_lpszValue = (LPCSTR)pStringTablePosition;

						// posune ukazatel za posledn� znak zkop�rovan�ho �et�zce
						pStringTablePosition += nLength;
						// ukon�� �et�zec a aktualizuje ukazatel na voln� m�sto v tabulce 
						//		�et�zc�
						*pStringTablePosition++ = 0x00;

						break;
					}
				// cel� ��slo
					case CConfigFileLexToken::CLTInteger :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTInteger;

					// pomocn� prom�nn�
						// na��tan� ��seln� hodnota prom�nn�
						long nNumberValue = 0;
						// p��znak na�ten� z�porn�ho znam�nka
						BOOL bNumberNegative = FALSE;
						// zpracov�van� pozice v �et�zci ��seln� hodnoty prom�nn�
						const char *lpszNumberPosition = (const char *)cLastToken.m_strText;

						ASSERT ( cLastToken.m_strText.GetLength () > 0 );

						// zjist�, za��n�-li ��slo znam�nkem
						switch ( *lpszNumberPosition )
						{
							case '-' :
								// nastav� p��znak na�ten� z�porn�ho znam�nka
								bNumberNegative = TRUE;
								// p�esko�� znam�nko
							case '+' :
								// p�esko�� znam�nko
								lpszNumberPosition++;
						}

						// na�te vlastn� ��selnou hodnotu
						while ( *lpszNumberPosition != 0x00 )
						{	// na�te dal�� cifru ��sla
							ASSERT ( ( *lpszNumberPosition >= '0' ) && 
								( *lpszNumberPosition <= '9' ) );
							ASSERT ( (DWORD)nNumberValue <= 10 * ((DWORD)nNumberValue) + 
								( *lpszNumberPosition - '0' ) );

							// zapo��t� do v�sledku dal�� cifru ��sla
							nNumberValue = 10 * nNumberValue + ( *lpszNumberPosition++ - '0' );
						}

						// zjist�, jedn�-li se o z�porn� ��slo
						if ( bNumberNegative )
						{	// jedn� se o z�porn� ��slo
							ASSERT ( nNumberValue >= 0 );
							// oto�� znam�nko ��sla
							nNumberValue = -nNumberValue;
						}

						// uschov� si ��selnou hodnotu prom�nn�
						pVariableValue->m_nValue = nNumberValue;

						break;
					}
				// re�ln� ��slo
					case CConfigFileLexToken::CLTReal :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTReal;
#ifdef _DEBUG
					// pomocn� prom�nn�
						// ukazatel za konec re�ln�ho ��sla v �et�zci hodnoty prom�nn�
						char *lpszRealStringEnd;

						// nech� na��st hodnotu prom�nn�
						pVariableValue->m_dbValue = strtod ( cLastToken.m_strText, 
							&lpszRealStringEnd );

						ASSERT ( *lpszRealStringEnd == 0x00 );
#else //!_DEBUG
						// nech� na��st hodnotu prom�nn�
						pVariableValue->m_dbValue = strtod ( cLastToken.m_strText, NULL );
#endif //_DEBUG

						break;
					}
				// hexadecim�ln� ��slo
					case CConfigFileLexToken::CLTHexNumber :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTInteger;

					// pomocn� prom�nn�
						// na��tan� ��seln� hodnota prom�nn�
						DWORD nNumberValue = 0;
						// zpracov�van� pozice v �et�zci ��seln� hodnoty prom�nn�
						const char *lpszNumberPosition = ((const char *)cLastToken.m_strText) + 2;

						ASSERT ( cLastToken.m_strText.GetLength () > 2 );
						ASSERT ( cLastToken.m_strText[0] == '0' );
						ASSERT ( ( cLastToken.m_strText[1] == 'x' ) || 
							( cLastToken.m_strText[1] == 'X' ) );

						// na�te vlastn� ��selnou hodnotu
						while ( *lpszNumberPosition != 0x00 )
						{	// na�te dal�� cifru ��sla
							ASSERT ( nNumberValue <= 16 * nNumberValue );

							// zapo��t� do v�sledku dal�� cifru ��sla
							nNumberValue = 16 * nNumberValue + (DWORD)GetHexadecimalDigitValue ( 
								*lpszNumberPosition++ );
						}

						// uschov� si ��selnou hodnotu prom�nn�
						pVariableValue->m_nValue = (long)nNumberValue;

						break;
					}
				// logick� hodnota
					case CConfigFileLexToken::CLTTrue :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTBoolean;

						// uschov� si hodnotu prom�nn�
						pVariableValue->m_bValue = TRUE;

						break;
					}
				// logick� hodnota
					case CConfigFileLexToken::CLTFalse :
					{
						// uschov� si typ hodnoty prom�nn�
						pVariableValue->m_eType = VVTBoolean;

						// uschov� si hodnotu prom�nn�
						pVariableValue->m_bValue = FALSE;

						break;
					}
				// chybn� typ hodnoty prom�nn�
					default:
						ASSERT ( FALSE );
				}

				// na�te z konfigura�n�ho souboru token st�edn�ku
				cConfigFileLex.GetToken ( cLastToken );

				ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSemicolon );
			}
#ifdef _DEBUG
			// na�te z konfigura�n�ho souboru token ukon�en� sekce
			cConfigFileLex.GetToken ( cLastToken );

			ASSERT ( ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin ) || 
				( cLastToken.m_eToken == CConfigFileLexToken::CLTEndOfFile ) );
#endif //_DEBUG
		}
		else
		{	// jedn� se o pr�zdnou sekci
			ASSERT ( m_nStringTableSize == 0 );
		}
	}

	// zv��� po�et odkaz� na pole prom�nn�ch sekce
	m_dwVariablesReferenceCount++;
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vr�t� p��znak existence podsekce v sekci
BOOL CConfigFileSectionInfo::IsSubsection ( LPCSTR lpszSubsectionName ) const 
{
	// jm�no podsekce nesm� b�t pr�zdn�
	ASSERT ( ( lpszSubsectionName != NULL ) && ( *lpszSubsectionName != 0x00 ) );
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );

	// zjist�, jedn�-li se o platn� jm�no podsekce
	int nIndex = m_aSubsections.GetSize ();
	while ( nIndex > 0 )
	{	// projede v�echny informace o podsekc�ch
		// z�sk� ukazatel na informace o dal�� podsekci
		CConfigFileSectionInfo *pNextSubsectionInfo = 
			(CConfigFileSectionInfo *)m_aSubsections[nIndex-1];
		// zjist�, je-li to hledan� podsekce
		if ( pNextSubsectionInfo->m_strName == lpszSubsectionName )
		{	// jedn� se o hledanou podsekci
			// vr�t� p��znak nalezen� podsekce
			return TRUE;
		}
		// nech� zkontrolovat informaci o dal�� podsekci
		--nIndex;
	}
	// nenalezli jsme po�adovanou podsekci

	// vr�t� p��znak nenalezen� podsekce
	return FALSE;
}

// vr�t� p��znak existence prom�nn� v sekci
BOOL CConfigFileSectionInfo::IsVariable ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// pokus� se naj�t po�adovanou prom�nnou
	for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
	{
		// zjist�, jedn�-li se o hledanou prom�nnou
		if ( strcmp ( lpszVariableName, m_pVariableValues[nVariableIndex].m_lpszName ) == 0 )
		{	// jedn� se o hledanou prom�nnou
			// vr�t� p��znak nalezen� hodnoty prom�nn�
			return TRUE;
		}
	}
	// prom�nn� nebyla nalezena

	// vr�t� p��znak nenalezen� hodnoty prom�nn�
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// �ten� hodnot prom�nn�ch sekce
//////////////////////////////////////////////////////////////////////

// vr�t� �et�zcovou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
LPCSTR CConfigFileSectionInfo::GetVariableStringValue ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocn� prom�nn�
	// ukazatel na hodnotu prom�nn�
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjist� jedn�-li se o �et�zcovou prom�nnou
	if ( pVariableValue->m_eType == VVTString )
	{	// jedn� se o �et�zcovou prom�nnou
		// vr�t� �et�zcovou hodnotu prom�nn�
		return pVariableValue->m_lpszValue;
	}
	// nejedn� se o �et�zcovou prom�nnou

// pomocn� prom�nn�
	// chyba konverze typu prom�nn�
	enum CConfigFileException::EError eError;

	// zjist� chybu konverze typu prom�nn�
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

	// obslou�� chybu - hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vr�t� ��selnou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
long CConfigFileSectionInfo::GetVariableIntegerValue ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocn� prom�nn�
	// ukazatel na hodnotu prom�nn�
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjist� jedn�-li se o ��selnou prom�nnou
	if ( pVariableValue->m_eType == VVTInteger )
	{	// jedn� se o ��selnou prom�nnou
		// vr�t� ��selnou hodnotu prom�nn�
		return pVariableValue->m_nValue;
	}
	// nejedn� se o ��selnou prom�nnou

// pomocn� prom�nn�
	// chyba konverze typu prom�nn�
	enum CConfigFileException::EError eError;

	// zjist� chybu konverze typu prom�nn�
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

	// obslou�� chybu - hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vr�t� re�lnou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
double CConfigFileSectionInfo::GetVariableRealValue ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocn� prom�nn�
	// ukazatel na hodnotu prom�nn�
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjist� jedn�-li se o re�lnou nebo ��selnou prom�nnou
	switch ( pVariableValue->m_eType )
	{
		case VVTReal :
			// vr�t� re�lnou hodnotu prom�nn�
			return pVariableValue->m_dbValue;
		case VVTInteger :
			// vr�t� re�lnou hodnotu ��seln� prom�nn�
			return (double)pVariableValue->m_nValue;
	}
	// nejedn� se o re�lnou ani ��selnou prom�nnou

// pomocn� prom�nn�
	// chyba konverze typu prom�nn�
	enum CConfigFileException::EError eError;

	// zjist� chybu konverze typu prom�nn�
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

	// obslou�� chybu - hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vr�t� logickou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
BOOL CConfigFileSectionInfo::GetVariableBooleanValue ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

// pomocn� prom�nn�
	// ukazatel na hodnotu prom�nn�
	struct SVariableValue *pVariableValue = &m_pVariableValues[GetVariableIndex ( lpszVariableName )];

	// zjist� jedn�-li se o logickou prom�nnou
	if ( pVariableValue->m_eType == VVTBoolean )
	{	// jedn� se o logickou prom�nnou
		// vr�t� logickou hodnotu prom�nn�
		return pVariableValue->m_bValue;
	}
	// nejedn� se o logickou prom�nnou

// pomocn� prom�nn�
	// chyba konverze typu prom�nn�
	enum CConfigFileException::EError eError;

	// zjist� chybu konverze typu prom�nn�
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

	// obslou�� chybu - hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), eError, lpszVariableName );
}

// vr�t� index hodnoty prom�nn� v poli hodnot prom�nn�ch
//    v�jimky: CConfigFileException
int CConfigFileSectionInfo::GetVariableIndex ( LPCSTR lpszVariableName ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// pokus� se naj�t po�adovanou prom�nnou
	for ( int nVariableIndex = m_nVariablesCount; nVariableIndex-- > 0; )
	{
		// zjist�, jedn�-li se o hledanou prom�nnou
		if ( strcmp ( lpszVariableName, m_pVariableValues[nVariableIndex].m_lpszName ) == 0 )
		{	// jedn� se o hledanou prom�nnou
			// vr�t� index nalezen� hodnoty prom�nn�
			return nVariableIndex;
		}
	}
	// prom�nn� nebyla nalezena

	// obslou�� chybu - hod� v�jimku s informac� o chyb� konfigura�n�ho souboru
	throw new CConfigFileException ( m_pConfigFile->GetFile (), 
		CConfigFileException::CFEUnknownVariable, lpszVariableName );
}
