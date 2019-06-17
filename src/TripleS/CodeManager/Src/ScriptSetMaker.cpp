// ScriptSetMaker.cpp: implementation of the CScriptSetMaker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\ScriptSetMaker.h"

#define SCRIPTSETMAKER_TEMP1	"__tmp1__"
#define SCRIPTSETMAKER_TEMP2	"__tmp2__"

//////////////////////////////////////////////////////////////////////

CScriptSetMaker::CScriptSetMaker()
{

}

CScriptSetMaker::~CScriptSetMaker()
{

}

bool CScriptSetMaker::Create( CString InputDir, CDataArchive OutputArchive)
{
	// archives
	m_strInputDir = InputDir;
	m_OutputArchive = OutputArchive;

	// temporary directories for code manager
	int len = ::GetCurrentDirectory( 0, NULL);
	char *str;

	if (len == 0) return false;

	ASSERT( sizeof(SCRIPTSETMAKER_TEMP1) == sizeof(SCRIPTSETMAKER_TEMP2));

	str = new char[len];
	VERIFY( ::GetCurrentDirectory( len, str) != 0);
	m_strCurrentDir = str;
	delete str;

	::CreateDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP1, NULL);
	::CreateDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP2, NULL);

	// codemanager
	m_CodeManager.Create( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP1, 
						  m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP2,
						  NULL);

	// compiler
	m_Compiler.Create( this);

	// code manager for compiler
	Init( &m_CodeManager);

	return true;
}

void CScriptSetMaker::Delete()
{
	m_CodeManager.Delete();
	m_Compiler.Delete();

	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP1 + "\\" + DIRECTORY_SOURCE);
	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP1 + "\\" + DIRECTORY_BYTECODE);
	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP1);
	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP2 + "\\" + DIRECTORY_SOURCE);
	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP2 + "\\" + DIRECTORY_BYTECODE);
	::RemoveDirectory( m_strCurrentDir + "\\" + SCRIPTSETMAKER_TEMP2);
}

bool CScriptSetMaker::Compile( CCompilerErrorOutput *pStdErr)
{
	// find all *.3s in input dir
	CString strPath, strScriptsFilenames;
    WIN32_FIND_DATA FD;
    HANDLE hFindFile;

    strPath.Format( "%s\\*.%s", m_strInputDir, FILE_SOURCE_EXTENSION);

    hFindFile=::FindFirstFile(strPath, &FD);

    while (hFindFile!=INVALID_HANDLE_VALUE)
    {
        if ( strcmp(FD.cFileName, ".") == 0 || strcmp(FD.cFileName, "..") == 0) 
        {
            if (!::FindNextFile(hFindFile, &FD)) break;
            continue;
        }
        
        if (strScriptsFilenames.IsEmpty()) strScriptsFilenames = FD.cFileName;
        else 
        {
            strScriptsFilenames += '\n';
            strScriptsFilenames += FD.cFileName;
        }

        if (!::FindNextFile(hFindFile, &FD)) break;
    }

    ::FindClose(hFindFile);

	if (strScriptsFilenames == "")
	{
		pStdErr->PutString("No input files.");
		return false;
	}
	
	// create the multiple file input for compiler
	CCompilerInputMultipleFile StdIn;
	
	if (!StdIn.Open( m_strInputDir, strScriptsFilenames, &m_Compiler, NULL)) return false;


	// compile
	bool res = m_Compiler.Compile( &StdIn, pStdErr, true, false);


	// close the input
	StdIn.Close();

	return res;
}

CCompilerOutput* CScriptSetMaker::UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName)
{
	CString strFilename;

	// vytvoreni jmena
	switch (Type)
	{
	case SCRIPT_UNIT: 
		strFilename = strName + "." + FILE_CODE_EXTENSION_UNIT;
		break;
	case SCRIPT_FUNCTION:
		strFilename = strName + "." + FILE_CODE_EXTENSION_FUNCTION;
		break;
	case SCRIPT_STRUCTURE:
        strFilename = strName + "." + FILE_CODE_EXTENSION_STRUCTURE;
		break;
    default:
		ASSERT(false);
	}

	if (m_CompilerOutput.Open( m_OutputArchive, strFilename)) return &m_CompilerOutput;
	else return NULL;
}

bool CScriptSetMaker::UploadScriptCodeEnd()
{
	m_CompilerOutput.Close();
	return true;
}
