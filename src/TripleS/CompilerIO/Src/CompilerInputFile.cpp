/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerInputFile - vstup kompilatoru ze souboru
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CompilerIO.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompilerInputFile::CCompilerInputFile()
{
	m_nPos=0;
	m_nRead=0;
    m_bSetSuccessfullyCompiledFlag = false;
}

CCompilerInputFile::~CCompilerInputFile()
{
}

bool CCompilerInputFile::Open(CString& strFilename, CFileException* pFileException/*=NULL*/)
{
	ASSERT(!m_bOpen);
	
	if (!fileIn.Open(strFilename, CFile::modeRead | CFile::shareDenyWrite, pFileException)) return false;
	
	m_nRead=fileIn.Read((void*)m_pBuffer, BUFFER_SIZE);
	m_nPos=0;
    m_strFilename = strFilename;

	SetName(fileIn.GetFileName());
	m_bOpen=true;

	return true;
}

void CCompilerInputFile::Close()
{
	ASSERT(m_bOpen);

	m_bOpen=false;
	SetName(CString(""));

    fileIn.Close();

    if (m_bSetSuccessfullyCompiledFlag)
    {
        DWORD dwAttr = ::GetFileAttributes( m_strFilename);
        dwAttr |= FILE_ATTRIBUTE_READONLY;
        ::SetFileAttributes( m_strFilename, dwAttr);
    }
}

int CCompilerInputFile::GetChar()
{
	ASSERT(m_bOpen);

	if (m_nPos < m_nRead) m_chLastChar=m_pBuffer[m_nPos++];
	else 
	{
		m_nPos=0;
		if ((m_nRead=fileIn.Read((void*)m_pBuffer, BUFFER_SIZE))==0) m_chLastChar=EOF;
		else m_chLastChar=m_pBuffer[m_nPos++];
	}

    return m_chLastChar;
}

void CCompilerInputFile::Reset()
{
    fileIn.Seek(0, CFile::begin);
    m_nPos=m_nRead=0;
    m_chLastChar=0;
}

void CCompilerInputFile::OnInputCompiledAndUploadedSuccessfully()
{
    m_bSetSuccessfullyCompiledFlag = true;
}