/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPExpr reprezentujici 
 *          semantickou hodnotu kazdeho terminalu a neterminalu
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////////
///  CPLexVal  implementation

CPLexVal::CPLexVal()	
{	
	m_lpszString=NULL;  
	m_nType=LEXVAL_NONE;
} 

CPLexVal::CPLexVal(CPLexVal &anotherOne)
{
	m_lpszString=NULL;  
	m_nType=LEXVAL_NONE;

	*this=anotherOne;
}

CPLexVal::~CPLexVal()
{
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL)
	{
		delete [] m_lpszString;
		m_lpszString=NULL;
	}
}

CPLexVal::operator=(CPLexVal &anotherOne)
{
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete m_lpszString;

	m_nType=anotherOne.m_nType;

	switch (m_nType)
	{
	case LEXVAL_ERROR:
		break;
	case LEXVAL_NONE:
		break;
	case LEXVAL_INT:
		m_lInt=anotherOne.m_lInt;
		break;
	case LEXVAL_FLOAT:
		m_fFloat=anotherOne.m_fFloat;
		break;
	case LEXVAL_BOOL: 
		m_bBool=anotherOne.m_bBool;
		break;
	case LEXVAL_CHAR:
		m_chChar=anotherOne.m_chChar;
		break;
	case LEXVAL_STRING:
		if (anotherOne.m_lpszString==NULL) m_lpszString=NULL;
		else
		{
			m_lpszString=new char[strlen(anotherOne.m_lpszString)+1];
			strcpy(m_lpszString, anotherOne.m_lpszString);
		}
		break;
	}
}

void CPLexVal::SetInt(int lInt)		
{
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_INT;		
	m_lInt=lInt;
}

void CPLexVal::SetFloat(float fFloat)	
{	
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_FLOAT;	
	m_fFloat=fFloat;
}

void CPLexVal::SetBool(bool bBool)	
{	
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_BOOL;	
	m_bBool=bBool;		
}

void CPLexVal::SetChar(char chChar)	
{	
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_CHAR;	
	m_chChar=chChar;	
}

void CPLexVal::SetString(LPCTSTR lpszString)
{
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_STRING;	
	m_lpszString=new char [strlen(lpszString)+1];	
	strcpy(m_lpszString, lpszString);
}

void CPLexVal::SetError()	
{	
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_ERROR;	
}

void CPLexVal::SetNone()	
{	
	if (m_nType==LEXVAL_STRING && m_lpszString!=NULL) delete [] m_lpszString;
	m_nType=LEXVAL_NONE;	
}

bool CPLexVal::IsType(CType&Type)
{
	switch (m_nType)
	{
	case LEXVAL_ERROR:	return false;
	case LEXVAL_NONE:	return false;
	case LEXVAL_INT:	return Type.IsInt();
	case LEXVAL_FLOAT:  return Type.IsFloat();
	case LEXVAL_BOOL:	return Type.IsBool();
	case LEXVAL_CHAR:	return Type.IsChar();
	case LEXVAL_STRING: return Type.IsString();
	}

	return false;
}

CString CPLexVal::GetString()
{
	switch (m_nType)
	{
	case LEXVAL_NONE:	ASSERT(false);
	case LEXVAL_INT:	return "int";
	case LEXVAL_FLOAT:  return "float";
	case LEXVAL_BOOL:	return "bool";
	case LEXVAL_CHAR:	return "char";
	case LEXVAL_STRING: return "string";
	default: return "error_type";
	}
}

/////////////////////////////////////////////////////////////////////////////////
///  CPExpr implementation

CPExpr::CPExpr()
{
	SetDefaults();
    m_pCompilerKrnl = NULL;
}

CPExpr::~CPExpr()
{
}

CPExpr::CPExpr(CPExpr &anotherOne)
{
	// SetDefaults(); neni treba volat, vola se v operatoru =
	//!! *this=anotherOne;
}

CPExpr::operator=(CPExpr &anotherOne)
{
	ASSERT_VALID(&anotherOne);

	SetDefaults();

    m_pCompilerKrnl = anotherOne.m_pCompilerKrnl;

	m_nType=anotherOne.m_nType;

	switch (anotherOne.m_nType)
	{
	case E_NONE: 
		break;
	case E_CONSTANT:
		m_LexVal=anotherOne.m_LexVal;
		m_DataType=anotherOne.m_DataType;
		m_bLValue=false;
		break;
	case E_CODE:
		m_DataType=anotherOne.m_DataType;
		m_nReg=anotherOne.m_nReg;
		m_Code=anotherOne.m_Code;
		m_bLValue=anotherOne.m_bLValue;
		m_nLValueType=anotherOne.m_nLValueType;
		if (m_nLValueType!=LVALUE_PRIMARY) m_lNum=anotherOne.m_lNum;
		break;
	case E_TYPE:
		m_DataType=anotherOne.m_DataType;
		break;
	case E_DECLAR:
		m_strIdentifier=anotherOne.m_strIdentifier;
		m_DataType=anotherOne.m_DataType;
		break;
	}
}

void CPExpr::SetDefaults()
{
	m_nType=E_NONE;
	m_DataType.SetNone();
	m_nReg=NO_REG;
	m_bLValue=false;		
	m_strIdentifier.Empty();
	m_LexVal.SetNone();
	m_nLValueType=LVALUE_PRIMARY;
	m_lNum=0;
	m_Code.Empty();
}

void CPExpr::SetCode(REG nReg, CType&DataType, bool bLValue/*=true*/, 
		ELValueType nLValueType/*==LVALUE_PRIMARY*/, long lNum/*=0*/)
{
	// nemazat kod!
	m_nReg=nReg;
	m_DataType=DataType;
	m_bLValue=bLValue;
	m_nLValueType=nLValueType;
	m_lNum=lNum;
	m_nType=E_CODE;
}

void CPExpr::MoveConstantToRegister(REG nReg)
{
	ASSERT(IsConstant());
    ASSERT(m_pCompilerKrnl!=NULL);

	CPLexVal LexVal=m_LexVal;
	CType Type;

	Type.Set(m_LexVal);
	SetNewCode();
	m_DataType=Type;
	m_bLValue=false;

	switch (LexVal.m_nType)
	{
	case CPLexVal::LEXVAL_ERROR:
		m_nReg=nReg;
		break;
	case CPLexVal::LEXVAL_NONE:
		m_nReg=nReg;
		break;
	case CPLexVal::LEXVAL_INT:
		AddInstructionChangeRegister(MOV_CINT, nReg, LexVal.m_lInt);
		break;
	case CPLexVal::LEXVAL_FLOAT:
		ASSERT(m_pCompilerKrnl->m_pCurrentFloatTable!=NULL);
		AddInstructionChangeRegister(MOV_CFLT, nReg, 
			m_pCompilerKrnl->m_pCurrentFloatTable->Add(LexVal.m_fFloat));
		break;
	case CPLexVal::LEXVAL_BOOL:
		m_Code.AddInstruction(MOV_CBOOL, nReg, LexVal.m_bBool);
		break;
	case CPLexVal::LEXVAL_CHAR:
		m_Code.AddInstruction(MOV_CCHAR, nReg, LexVal.m_chChar);
		break;
	case CPLexVal::LEXVAL_STRING:
		ASSERT(m_pCompilerKrnl->m_pCurrentStringTable!=NULL);

		m_Code.AddInstruction(MOV_CSTR, nReg, 
			m_pCompilerKrnl->m_pCurrentStringTable->Add(LexVal.m_lpszString)->m_nID);
	}

	m_nReg=nReg;
}

void CPExpr::Load()
{
	if (!IsCode()) return;

    ASSERT(m_pCompilerKrnl!=NULL);

	switch (m_nLValueType)
	{
	case LVALUE_PRIMARY:		// obycejna promenna
		break;
	
	case LVALUE_UMEMBER:		// unit.member
		AddInstructionChangeRegister(LOADUM, m_pCompilerKrnl->GetReg(), m_nReg, m_lNum);
		break;

	case LVALUE_SMEMBER:		// struct.member
		AddInstructionChangeRegister(LOADSM, m_pCompilerKrnl->GetReg(), m_nReg, m_lNum);
		break;

	case LVALUE_OMEMBER:		// object.member
		AddInstructionChangeRegister(LOADOM, m_pCompilerKrnl->GetReg(), m_nReg, m_lNum);
		break;					

	case LVALUE_INDEX:	       // object[]
		AddInstructionChangeRegister(LOADIX, m_pCompilerKrnl->GetReg(), m_nReg, m_lNum);
		break;
	}

	m_nLValueType=LVALUE_PRIMARY;
}

void CPExpr::Store(ELValueType nLValueType, REG nReg, long lNum)
{   
    CPCode::CInstruction *pInst;

	switch (nLValueType)
	{
	case LVALUE_PRIMARY:
        if (m_nReg!=nReg) 
        {
            // optimalizace zbytecnych MOVu
            if ((pInst=m_Code.GetLastInstruction())!=NULL &&
                g_aInstrInfo[pInst->m_eInstr].m_bOp1IsResultReg)
            {
                pInst->m_nOp[0]=nReg;
            }
            else m_Code.AddInstruction(MOV, nReg, m_nReg);
        }
		break;
	case LVALUE_UMEMBER:   
		m_Code.AddInstruction(STOREUM, m_nReg, nReg, lNum);
		break;
	case LVALUE_SMEMBER:   
		m_Code.AddInstruction(STORESM, m_nReg, nReg, lNum);
		break;
	case LVALUE_OMEMBER:
		m_Code.AddInstruction(STOREOM, m_nReg, nReg, lNum);
		break;
	case LVALUE_INDEX:	
		m_Code.AddInstruction(STOREIX, m_nReg, nReg, lNum);
		break;
	}
}

void CPExpr::StoreNull()
{   
	switch (m_nLValueType)
	{
	case LVALUE_PRIMARY:
		m_Code.AddInstruction( SETNULL, m_nReg); 
		break;
	case LVALUE_UMEMBER:   
	case LVALUE_SMEMBER:   
	case LVALUE_OMEMBER:
	case LVALUE_INDEX:	
		m_Code.AddInstruction(SETNULLM, m_nReg, m_lNum);
		break;
	}
	m_DataType.SetVoid();
}

void CPExpr::ConvertToRValue()
{
	if (!IsCode()) return;
	if (!m_bLValue) return;

	Load();
	m_bLValue=false;
}

bool CPExpr::ConvertToType(CType&DestType)
{
	if (m_DataType.IsError() || DestType.IsError()) return true;

	if (m_DataType==DestType) return true;

    ASSERT(m_pCompilerKrnl!=NULL);

	switch (m_DataType.m_nType)
	{
	case T_ERROR:
		return true;  // error_type jde "konvertovat" na vsechno
	
	case T_VOID:
	case T_BOOL:
    case T_CHAR:
		return false; // tyto typy nejdou konvertovat na nic
                      // (char jde konvertovat na string, a to je jinde)
	
	case T_INT: // int jde konvertovat na char a float
		if (DestType.IsChar())
		{
			if (IsCode()) AddInstructionChangeRegister(CNV_INT2CHAR, m_pCompilerKrnl->GetReg(), m_nReg);
			else m_LexVal.SetChar((char)m_LexVal.m_lInt);
            m_DataType.SetChar();
			return true;
		}
		else if (DestType.IsFloat()) 
		{
			if (IsCode()) AddInstructionChangeRegister(CNV_INT2FLT, m_pCompilerKrnl->GetReg(), m_nReg);
			else m_LexVal.SetFloat((float)m_LexVal.m_lInt);
            m_DataType.SetFloat();
			return true;
		}
		else return false;

	case T_FLOAT: // float jde konvertovat pouze na int
		if (DestType.IsInt())
		{
			if (IsCode()) AddInstructionChangeRegister(CNV_FLT2INT, m_pCompilerKrnl->GetReg(), m_nReg);
			else m_LexVal.SetInt((int)m_LexVal.m_fFloat);
            m_DataType.SetFloat();
			return true;
		}
		else return false;
	
	case T_UNIT:  
        
        ASSERT(m_pCompilerKrnl->m_pCurrentStringTable!=NULL);
	
        AddInstructionChangeRegister(CNV_U2U, m_pCompilerKrnl->GetReg(), m_nReg, 
            m_pCompilerKrnl->m_pCurrentStringTable->Add( (CString)*DestType.m_stiName)->m_nID);
        return true;
	
	case T_OBJECT:	// objekty
		if (IsConstant() && m_LexVal.IsString() && DestType.IsString()) 
		{
			MoveConstantToRegister(m_pCompilerKrnl->GetReg());
			return true;
		}
		else return false; 
	}

	return false;
}

void CPExpr::ConvertCharToString()
{
	ASSERT(IsCode() || IsConstant());
	ASSERT(m_DataType.IsChar());
    ASSERT(m_pCompilerKrnl!=NULL);

	ConvertToRValue();

	if (IsConstant())
	{
		char pom[2];
		pom[0]=m_LexVal.m_chChar;
		pom[1]=0;
		m_LexVal.SetString(pom);
		m_DataType.SetString();
	}
	else
	{
		AddInstructionChangeRegister(CNV_CHAR2STR, m_pCompilerKrnl->GetReg(), m_nReg);
	}
}

void CPExpr::AddInstructionChangeRegister(EInstruction eInstr, REG nOp1, 
										 REG nOp2, REG nOp3)
{
	ASSERT(IsCode());

	m_Code.AddInstruction(eInstr, nOp1, nOp2, nOp3);

	m_nReg=nOp1;
}

/*
void CPExpr::CreateFromConcatenationChangeReg(CPCode &Code1, CPCode &Code2, REG nFinalReg, CType&FinalDataType)
{
	ASSERT(IsCode());

	m_Code.CreateFromConcatenation(Code1, Code2);
	m_nReg=nFinalReg;
	m_DataType=FinalDataType;
}
*/
void CPExpr::SetConstant(char chChar, CCompilerKernel *pCompilerKrnl)
{
	SetDefaults();
	m_nType=E_CONSTANT;
	m_bLValue=false;
	m_LexVal.SetChar(chChar);
	m_DataType.Set(m_LexVal);

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetConstant(int nInt, CCompilerKernel *pCompilerKrnl)
{
	SetDefaults();
	m_nType=E_CONSTANT;
	m_bLValue=false;
	m_LexVal.SetInt(nInt);
	m_DataType.Set(m_LexVal);

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetConstant(float fFloat, CCompilerKernel *pCompilerKrnl)
{
	SetDefaults();
	m_nType=E_CONSTANT;
	m_bLValue=false;
	m_LexVal.SetFloat(fFloat);
	m_DataType.Set(m_LexVal);

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetConstant(bool bBool, CCompilerKernel *pCompilerKrnl)
{
	SetDefaults();
	m_nType=E_CONSTANT;
	m_bLValue=false;
	m_LexVal.SetBool(bBool);
	m_DataType.Set(m_LexVal);

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetConstant(LPCTSTR lpszString, CCompilerKernel *pCompilerKrnl)
{
	SetDefaults();
	m_nType=E_CONSTANT;
	m_bLValue=false;
	m_LexVal.SetString(lpszString);
	m_DataType.Set(m_LexVal);

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetDeclar(CString &strIdentifier, CCompilerKernel *pCompilerKrnl)	
{	
	SetDefaults();  
	m_nType=E_DECLAR;		
	m_strIdentifier=strIdentifier;

    m_pCompilerKrnl = pCompilerKrnl;
}

void CPExpr::SetType(EDataType nType, CCompilerKernel *pCompilerKrnl)
{	
	SetDefaults(); 
	m_nType=E_TYPE;
	m_DataType.Set(nType);

    m_pCompilerKrnl = pCompilerKrnl;
}
