/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerErrorOutput - abstraktni trida pro vystup kompilatoru.
 *          Potomci CCompilerFileOutput a CCompilerMemoryOutput.
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

CCompilerErrorOutput::CCompilerErrorOutput()
{
	m_bOpen=false;
}

CCompilerErrorOutput::~CCompilerErrorOutput()
{
//	ASSERT(!m_bOpen);
}


void CCompilerErrorOutput::PutString(LPCTSTR pstrOut)
{
	PutChars(pstrOut, strlen(pstrOut));
}

void CCompilerErrorOutput::Format(LPCTSTR pstrFormat, ...)
{
    va_list args;
    CString str;
    
    va_start(args, pstrFormat);
    str.FormatV(pstrFormat, args);
    PutChars(str, str.GetLength());
}
