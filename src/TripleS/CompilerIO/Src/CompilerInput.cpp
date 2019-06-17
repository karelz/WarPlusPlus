/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerInput - abstraktni trida pro vstup kompilatoru.
 *          Potomci jsou CCompilerFileInput a CCompilerMemoryInput.
 *          (v jinem souboru)
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

CCompilerInput::CCompilerInput()
{
	m_bOpen=false;
	m_strName.Empty();
    m_chLastChar=0;
}

CCompilerInput::~CCompilerInput()
{
	ASSERT(!m_bOpen);
}

