// CompilerInputMultipleFile.cpp: implementation of the CCompilerInputMultipleFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\CompilerIO.h"

#include "TripleS\CICommon\CICommon.h"
#include "TripleS\Compiler\Compiler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompilerInputMultipleFile::CCompilerInputMultipleFile()
{
	m_nPos=0;
	m_nRead=0;
    m_bSetSuccessfullyCompiledFlag = false;

    m_nFilesCount = 0;
    m_pCompiler = 0;
    m_aInputFiles.SetSize(5,5);
}

CCompilerInputMultipleFile::~CCompilerInputMultipleFile()
{
}

bool CCompilerInputMultipleFile::Open(CString &strPath, CString &strFilenames, CCompiler *pCompiler, 
                                      CFileException* pFileException/*=NULL*/)
{
	ASSERT(!m_bOpen);
    ASSERT( strFilenames!="");
    ASSERT( pCompiler != NULL);

    m_pCompiler = pCompiler;

    CString strFN;
    int pos = 0, i, len = strFilenames.GetLength();
    bool bContinue = true;

    m_nFilesCount = 0;

    do
    {
        if ( (i=strFilenames.Find('\n', pos)) == -1)
        {
            strFN = strPath + '\\' + strFilenames.Mid( pos);
            bContinue = false;
        }
        else
        {
            strFN = strPath + '\\' + strFilenames.Mid( pos, i - pos);
            pos = i+1;
        }

        m_aInputFiles.SetSize(m_nFilesCount + 1);
        if (!m_aInputFiles[m_nFilesCount].Open(strFN, CFile::modeRead | CFile::shareDenyWrite, pFileException)) 
        {
            int j;
            for (j=0; j<m_nFilesCount; j++) m_aInputFiles[j].Close();
            return false;
        }

        m_nFilesCount++;
    } while(bContinue);

    ASSERT(m_nFilesCount > 0);
	
    strFN = m_aInputFiles[0].GetFileName();
	SetName( strFN.Mid( strFN.ReverseFind('\\')));

	m_bOpen=true;
    m_nActualFile = 0;

	m_nRead = m_aInputFiles[m_nActualFile].Read((void*)m_pBuffer, BUFFER_SIZE);
	m_nPos=0;
    m_strPath = strPath;

	return true;
}

void CCompilerInputMultipleFile::Close()
{
	ASSERT(m_bOpen);

	SetName(CString(""));
	m_bOpen=false;

    int i;
    CFileStatus FS;
    CString strFilename;

    for (i=0; i<m_nFilesCount; i++)
    {
        strFilename = m_aInputFiles[i].GetFileName();
        
    	m_aInputFiles[i].Close();

        if (m_bSetSuccessfullyCompiledFlag)
        {
            strFilename = m_strPath + '\\' + strFilename;
            DWORD dwAttr = ::GetFileAttributes( strFilename);
            dwAttr |= FILE_ATTRIBUTE_READONLY;
            ::SetFileAttributes( strFilename, dwAttr);
        }
    }
}

int CCompilerInputMultipleFile::GetChar()
{
	ASSERT(m_bOpen);

	if (m_nPos < m_nRead) m_chLastChar=m_pBuffer[m_nPos++];
	else 
	{
		m_nPos=0;
		while ((m_nRead=m_aInputFiles[m_nActualFile].Read((void*)m_pBuffer, BUFFER_SIZE))==0) 
        {
            // zmena souboru
            m_nActualFile++;
            if (m_nActualFile == m_nFilesCount) 
            {
                m_chLastChar=EOF;
                break;
            }
            else
            {
            	SetName( m_aInputFiles[ m_nActualFile].GetFileName());
                m_pCompiler->ResetLineNo();
            }
        }

		if (m_chLastChar != EOF) m_chLastChar=m_pBuffer[m_nPos++];
	}

    return m_chLastChar;
}

void CCompilerInputMultipleFile::Reset()
{
    int i;
    
    for (i=0; i<m_nFilesCount; i++) m_aInputFiles[i].Seek(0, CFile::begin);
    
    m_nActualFile = 0;
    m_nPos=m_nRead=0;
    m_chLastChar=0;
    SetName( m_aInputFiles[ m_nActualFile].GetFileName());
}

void CCompilerInputMultipleFile::OnInputCompiledAndUploadedSuccessfully()
{
    m_bSetSuccessfullyCompiledFlag = true;
}
