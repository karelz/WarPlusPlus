/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CPCode a CPCode::CInstruction, tridy pro 
 *          reprezentaci instrukci a kusu kodu
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"

#include "TripleS\CompilerIO\CompilerIO.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CPCode::CInstruction
//////////////////////////////////////////////////////////////////////

CPCode::CInstruction::CInstruction(EInstruction eInstr, REG nOp0, REG nOp1, REG nOp2)
{
	m_eInstr=eInstr;
	m_nOp[0]=nOp0;
	m_nOp[1]=nOp1;
	m_nOp[2]=nOp2;

	m_nRefCount=0;
	m_pNext=NULL;
}

CPCode::CInstruction::~CInstruction()
{
}

CPCode::CInstruction* CPCode::CInstruction::Attach()
{
	m_nRefCount++;
	return this;
}

void CPCode::CInstruction::Detach()
{
	if (--m_nRefCount==0) 
	{
		if (m_pNext!=NULL) 
		{
			m_pNext->Detach();
			m_pNext=NULL;
		}
		delete this;
	}
}

//////////////////////////////////////////////////////////////////////
// CPCode
//////////////////////////////////////////////////////////////////////
		
CPCode::CPCode()	
{	
	m_pFirstInstr=m_pLastInstr=NULL;
	m_nInstrCount=0;
    m_nLengthInBytes=0;
	m_bReturns = false;
}	

CPCode::~CPCode()
{
	if (m_pFirstInstr!=NULL) 
	{
		ASSERT(m_pLastInstr!=NULL);
		
		m_pFirstInstr->Detach();
		m_pLastInstr->Detach();
	}
}

inline void CPCode::AddInstruction(EInstruction nInst, REG nOp[])
{
	AddInstruction(nInst, nOp[0], nOp[1], nOp[2]);
}

void CPCode::AddInstruction(EInstruction nInst, REG nOp0, REG nOp1, REG nOp2)
{
	CInstruction *pInstr=new CInstruction(nInst, nOp0, nOp1, nOp2);

 	if (m_pFirstInstr==NULL)
	{
		m_pFirstInstr=pInstr->Attach();
		m_pLastInstr=pInstr->Attach();
		m_nInstrCount=1;
        m_nLengthInBytes=SizeOfInstruction(nInst);
	}
	else
	{
		ASSERT(m_pLastInstr->m_pNext==NULL);
		m_pLastInstr->m_pNext=pInstr->Attach();
		m_pLastInstr->Detach();
		m_pLastInstr=pInstr->Attach();
		m_nInstrCount++;
        m_nLengthInBytes+=SizeOfInstruction(nInst);
	}
}

void CPCode::AddCode(CPCode &Addin)
{
	if (m_pFirstInstr==NULL)
	{
		if (Addin.m_pFirstInstr==NULL) return;
		else
		{
			m_pFirstInstr=Addin.m_pFirstInstr->Attach();
			m_pLastInstr=Addin.m_pLastInstr->Attach();
			m_nInstrCount=Addin.m_nInstrCount;
            m_nLengthInBytes=Addin.m_nLengthInBytes;
		}
	}
	else
	{
		if (Addin.m_pFirstInstr==NULL) return;
		else
		{
			m_pLastInstr->m_pNext=Addin.m_pFirstInstr->Attach();
			m_pLastInstr->Detach();
			m_pLastInstr=Addin.m_pLastInstr->Attach();
			m_nInstrCount+=Addin.m_nInstrCount;
            m_nLengthInBytes+=Addin.m_nLengthInBytes;
		}
	}

}

void CPCode::CreateFromConcatenation(CPCode &First, CPCode &Second)
{
	if (m_pFirstInstr!=NULL)
	{
		ASSERT(m_pLastInstr!=NULL);
		m_pFirstInstr->Detach();
		m_pLastInstr->Detach();
	}

	if (First.m_pFirstInstr==NULL)
	{
		if (Second.m_pFirstInstr==NULL)
		{
			m_pLastInstr=m_pFirstInstr=NULL;
			m_nInstrCount=0;
            m_nLengthInBytes=0;
		}
		else
		{
			m_pFirstInstr=Second.m_pFirstInstr->Attach();
			m_pLastInstr=Second.m_pLastInstr->Attach();
			m_nInstrCount=Second.m_nInstrCount;
            m_nLengthInBytes=Second.m_nLengthInBytes;
		}
	}
	else
	{
		if (Second.m_pFirstInstr==NULL)
		{
			m_pFirstInstr=First.m_pFirstInstr->Attach();
			m_pLastInstr=First.m_pLastInstr->Attach();
			m_nInstrCount=First.m_nInstrCount;
            m_nLengthInBytes=First.m_nLengthInBytes;
		}
		else
		{
			First.m_pLastInstr->m_pNext=Second.m_pFirstInstr->Attach();
			m_pFirstInstr=First.m_pFirstInstr->Attach();
			m_pLastInstr=Second.m_pLastInstr->Attach();
			m_nInstrCount=First.m_nInstrCount+Second.m_nInstrCount;
            m_nLengthInBytes=First.m_nLengthInBytes+Second.m_nLengthInBytes;
		}
	}
}

CPCode::operator=(CPCode &anotherOne)
{
	if (anotherOne.m_pFirstInstr==NULL) 
	{
		m_pFirstInstr=NULL;
		m_pLastInstr=NULL;
	}
	else 
	{
		m_pFirstInstr=anotherOne.m_pFirstInstr->Attach();
		m_pLastInstr=anotherOne.m_pLastInstr->Attach();
	}

	m_nInstrCount=anotherOne.m_nInstrCount;
    m_nLengthInBytes=anotherOne.m_nLengthInBytes;
	m_bReturns=anotherOne.m_bReturns;
}

void CPCode::Empty()
{
	if (m_pFirstInstr!=NULL)
	{
		ASSERT(m_pLastInstr!=NULL);
		m_pFirstInstr->Detach();
		m_pLastInstr->Detach();
	}
	m_pFirstInstr=m_pLastInstr=NULL;
	m_nInstrCount=0;
    m_nLengthInBytes=0;
}

void CPCode::DebugWrite(CCompilerErrorOutput *pOut)
{
	CInstruction *pInstr=m_pFirstInstr;
	int i=1,j;
    long lInstrBytePos=0;

	while (pInstr!=NULL)
	{
		pOut->Format("%2d (%4d):  %-12s ", i++, lInstrBytePos, g_aInstrInfo[pInstr->m_eInstr].m_lpszName);
		
		for (j=0; j<g_aInstrInfo[pInstr->m_eInstr].m_nParamCount; j++)
		{
			if (j==0) pOut->Format("%2d", pInstr->m_nOp[0]);
			else pOut->Format(", %2d", pInstr->m_nOp[j]);
		}
		pOut->Format("\r\n");

        lInstrBytePos+=SizeOfInstruction(pInstr->m_eInstr);
    	pInstr=pInstr->m_pNext;
	}
}

void CPCode::Save(CCompilerOutput *pOut)
{
	CInstruction *pInst;	
	char *pBuffer;
	long nPos=0;
	int j;
    long pom;

	if (m_nInstrCount==0)
	{
		pOut->PutLong(0);
		return;
	}

	// kontrola a vyrazeni z kodu pomocnych instrukci BRK, CONT, NONE
	CInstruction *pPom;
	
	ASSERT ( m_pFirstInstr != NULL);

	while ( m_pFirstInstr != NULL && ( m_pFirstInstr->m_eInstr == BRK || m_pFirstInstr->m_eInstr == CONT))
	{
		pPom = m_pFirstInstr;
		m_pFirstInstr = m_pFirstInstr->m_pNext;
		pPom->m_pNext = NULL;
		pPom->Detach();
	}

	pInst = m_pFirstInstr;
	if (pInst != NULL)
	{
		while (pInst->m_pNext != NULL)
		{
			if ( pInst->m_pNext->m_eInstr == BRK ||
				 pInst->m_pNext->m_eInstr == CONT || 
				 pInst->m_pNext->m_eInstr == NONE)
			{
				ASSERT( pInst->m_pNext->m_eInstr != NONE);

				pPom = pInst->m_pNext;
				pInst->m_pNext = pInst->m_pNext->m_pNext;
				pPom->m_pNext = NULL;
				pPom->Detach();
			}
			else
			{
				pInst = pInst->m_pNext;
			}
		}
	}

#ifdef _DEBUG
	// assert the length of the code in bytes
	INT nLenInBytes = 0;
	pInst = GetFirstInstruction();
	while ( pInst != NULL)
	{
		nLenInBytes += SizeOfInstruction( pInst->m_eInstr);
		pInst=GetNextInstruction(pInst);
	}

	ASSERT( m_nLengthInBytes == nLenInBytes);

	INT nCurPos = 0;
#endif

	// ulozeni kodu
	pBuffer=new char[ pom = m_nLengthInBytes ];

	pInst=GetFirstInstruction();
	while (pInst!=NULL)
	{
		TRACE_COMPILED_CODE2("%4d: %-12s", nCurPos, g_aInstrInfo[pInst->m_eInstr].m_lpszName);
		*(INSTRUCTION*)(pBuffer+nPos)=(INSTRUCTION)(pInst->m_eInstr);
		nPos+=sizeof(INSTRUCTION);

		for (j=0; j<g_aInstrInfo[pInst->m_eInstr].m_nParamCount; j++) 
		{
			TRACE_COMPILED_CODE1("% d\t", pInst->m_nOp[j]);
			*(REG*)(pBuffer+nPos)=pInst->m_nOp[j];
			nPos+=sizeof(REG);
		}
		TRACE_COMPILED_CODE0("\n");

#ifdef _DEBUG
		nCurPos += SizeOfInstruction( pInst->m_eInstr);
#endif
		pInst=GetNextInstruction(pInst);
	}

	pOut->PutLong(nPos);
    pOut->PutChars(pBuffer, nPos);

	delete pBuffer;
}

/*#ifdef _DEBUG_LOAD
void CPCode::Load(CFile &fileIn)
{
	long nLength;
	EInstruction eInstr;
	REG nOp[3];
    short s;

	Empty();

	fileIn.Read(&nLength, sizeof(long));
	ASSERT(nLength>=0);

	while (nLength>0)
	{
		fileIn.Read(&s, sizeof(short));
        eInstr=(EInstruction)s;
		fileIn.Read(&nOp, sizeof(REG)*g_aInstrInfo[eInstr].m_nParamCount);
		AddInstruction(eInstr, nOp);
		nLength-=(sizeof(short)+g_aInstrInfo[eInstr].m_nParamCount*sizeof(REG));
	}
}
#endif
*/

long CPCode::GetLengthInBytes()
{
    return m_nLengthInBytes;
}
