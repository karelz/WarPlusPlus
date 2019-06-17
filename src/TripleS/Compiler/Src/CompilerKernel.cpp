/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace prekladace ve tride CCompilerKernel,
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "PLex1stPass.h"
#include "PLex2ndPass.h"
#include "Syntax.h"

#include "TripleS\CompilerIO\CompilerIO.h"

// FUNKCE a promenna BISONA

// Hlavni funkce parseru.
int yyparse(void *pCompKrnl);

// prizdak, zda je parser v debug modu (a vypisuje hlasky pomoci 
// CCompilerKernel::DebugOut do okna CDebugFrame)
extern int yydebug;

#define IsReturnInstruction(inst) (inst==RET || inst==RETV || inst==RETV_CCHAR || inst==RETV_CINT || \
                                   inst==RETV_CFLT || inst==RETV_CBOOL || inst==RETV_CSTR)

//////////////////////////////////////////////////////////////////////
// CCompilerKernel::CArgInfo
//////////////////////////////////////////////////////////////////////

CCompilerKernel::CArgInfo::CArgInfo()
{
	m_pArgArray=NULL;
	m_nCurrentArgNo=0;
	m_nArgCount=0;
	m_bValid=false;
}

void CCompilerKernel::CArgInfo::Set(CArray<CType, CType&>* pArgArray, int nArgCount)
{
	m_pArgArray=pArgArray;
	m_nArgCount=nArgCount;
	m_nCurrentArgNo=0;
	m_bValid=true;
}

bool CCompilerKernel::CArgInfo::GetNext(CType &Type)
{
	if (m_nCurrentArgNo>=m_nArgCount) return false;
	else 
	{
		Type=m_pArgArray->GetAt(m_nCurrentArgNo++);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////
// CCompilerKernel::CSwitchStatement
//////////////////////////////////////////////////////////////////////

CCompilerKernel::CSwitchStatement::CSwitchStatement(int nLevel)
{
	m_nLevel=nLevel;
	m_bValid=false;
	m_ExprCode.Empty();
}

CCompilerKernel::CSwitchStatement::CSwitchStatement(int nLevel, REG nExprReg, 
											CType &DataType, CPCode &ExprCode)
{
	m_nLevel=nLevel;
	m_bValid=true;
	m_nExprReg=nExprReg;
	m_DataType=DataType;
	m_ExprCode=ExprCode;

	m_nCasesCount=0;
	m_nBlockCount=0;
	m_bHasDefault=false;

	m_aCases.SetSize(5,5);
	m_aBlocks.SetSize(5,5);
}

CCompilerKernel::CSwitchStatement::~CSwitchStatement()
{
	m_aCases.RemoveAll();
	m_aBlocks.RemoveAll();
}

void CCompilerKernel::CSwitchStatement::AddCase(CPLexVal &LexVal)
{
	if ( m_aCases.GetSize() == m_nCasesCount) m_aCases.SetSize( m_nCasesCount + 5);
	m_aCases[m_nCasesCount].m_LexVal=LexVal;
	m_aCases[m_nCasesCount].m_nBlockNo=m_nBlockCount;
	m_nCasesCount++;
}

void CCompilerKernel::CSwitchStatement::AddBlock(CPCode &Code)
{
	m_aBlocks.SetAtGrow(m_nBlockCount++, Code);
}

//////////////////////////////////////////////////////////////////////
// CCompilerKernel
//////////////////////////////////////////////////////////////////////

CCompilerKernel::CCompilerKernel( CCodeManagerForCompiler *pCodeManagerForCompiler)
    : m_Preview(&m_Namespace)
{
	ASSERT(pCodeManagerForCompiler!=NULL);
	m_pCodeManagerForCompiler=pCodeManagerForCompiler;

    m_pStdIn = NULL;
    m_pStdErr = NULL;
    m_pDbgOut = NULL;

	m_bNamespaceCreated=false;
#ifdef _DEBUG
    m_pwndDebugFrame=NULL;
#endif

    m_pLex2ndPass = NULL;

    m_pCompiledUnits = NULL;
    m_pCompiledGlobalFunctions = NULL;
    m_pCompiledStructs = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

CCompilerKernel::~CCompilerKernel()
{
#ifdef _DEBUG
    if (m_pwndDebugFrame!=NULL && ::IsWindow(m_pwndDebugFrame->GetSafeHwnd()))
    {
        m_pwndDebugFrame->DestroyWindow();  
        m_DebugFont.DeleteObject();
    }
#endif
    
    m_Namespace.RemoveVariablesToLevel(-1);
	m_Preview.RemoveAll();
	FreeSwitches();
	FreeArgInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::Reset(CCompilerInput *pStdIn, 
							CCompilerErrorOutput *pStdErr,
                            bool bUpdateCodeOnSuccess,
							bool bDebugOutput,
							CCompilerErrorOutput *pDbgOut,
                            bool bDebugParser)
{
    // stdin
	ASSERT(pStdIn!=NULL);
	ASSERT(pStdIn->IsOpen());
    m_pStdIn=pStdIn;

    // stdout
    ASSERT(pStdErr!=NULL);
	ASSERT(pStdErr->IsOpen());
    m_pStdErr=pStdErr;

    // update on success
    m_bUpdateCodeOnSuccess = bUpdateCodeOnSuccess;

    // debugout
	m_bDebugOutput=bDebugOutput;
	if (bDebugOutput)
	{
		ASSERT(pDbgOut!=NULL);
		ASSERT(pDbgOut->IsOpen());
        m_pDbgOut=pDbgOut;
	}

    if (bDebugParser) yydebug = 1;
    else yydebug = 0;


    m_nErrorCount=m_nWarningCount=0;
	m_nLevel=0;

    m_bStopCompiling=false;
	m_bGenerCode=true;

	m_Namespace.RemoveVariablesToLevel(-1);
	m_Preview.RemoveAll();

	m_pCurrentStringTable=NULL;
	m_pCurrentFloatTable=NULL;
	m_pCurrentEventHeader=NULL;
	m_pCurrentUnitPreview=NULL;
	m_pCurrentUnit=NULL;
	m_pCurrentGlobalFunction=NULL;
    m_pCurrentStruct=NULL;
	m_pCurrentMethod=NULL;
	m_pCurrentEvent=NULL;

	m_nFormalParamNo=0;

	m_pCurrentUnit=NULL;

	m_nFirstFreeRegister=1;

	FreeSwitches();
	FreeArgInfo();

	m_bSendToStatement=false;

    // zkompilovane veci
    m_pCompiledUnits = NULL;
    m_pCompiledGlobalFunctions = NULL;
    m_pCompiledStructs = NULL;

}

/////////////////////////////////////////////////////////////////////////////////////////

bool CCompilerKernel::FirstPass()
// prvni pruchod prekladace - zjistuje vsechny metody a polozky jednotek, uklada to do m_Preview.
// Pri jakekoliv chybe prvni pruchod konci (syntakticka chyba v hlavicce jednotky, metody, spatne uzavorkovani..)
// a spousti se detailni druhy pruchod k nalezeni chyby.
{
	// inicializace namespace
	if (!m_bNamespaceCreated)
	{
		if (!m_Namespace.Create(this)) 
		{
			m_bStopCompiling=true;
			return false;
		}
		else m_bNamespaceCreated=true;
	}

	// preview
	DoFirstPass();
    
    return !m_bStopCompiling;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::DoFirstPass()
{
	CPNamespace::CUnit *pUnit=NULL;
	CPNamespace::CUnitMethod *pMethod=NULL;
	CPNamespace::CGlobalFunction *pFunc;
    CPNamespace::CStruct *pStruct;
    bool bEnd=false, bEvent;
    CString strID1, strID2, str;
	int ret;
    int nArrayLevel1, nArrayLevel2;
    CPLex1stPass Lex1stPass(m_pStdIn);

	m_pLex1stPass = &Lex1stPass;

    while (!bEnd)
    {
        Lex1stPass.m_bSearchSets = false;

        // ocekava 'unit', zacatek globalni fce, struct nebo konec souboru, 
		// vse mezi tim by mel lex pozrat
        if ((ret=Lex1stPass.yylex())==0) 
        {
            bEnd=true; 
            break;
        }

		// UNIT
        if (ret==FIRSTPASS_UNIT_KEYWORD)
		{
			// identifikator jednotky
			if (Lex1stPass.yylex()!=FIRSTPASS_IDENTIFIER) goto End;

			// smazani stareho zaznamu o jednotce
            str = Lex1stPass.YYText();
			m_Namespace.RemoveType(CString(Lex1stPass.YYText()));

			// zalozeni noveho zaznamu o jednotce
			if ( m_Preview.GetUnit( str) != NULL)
			{
		        ErrorMessage("unit '%s' type redefinition", str);
				m_bStopCompiling = true;
				bEnd=true;
				break;
			}
			pUnit=m_Preview.AddUnit( str);

			if ((ret=Lex1stPass.yylex())!='{')
            {
                if (ret==FIRSTPASS_EXTENDS && Lex1stPass.yylex()!=FIRSTPASS_IDENTIFIER) goto End;
                pUnit->m_stiParent = g_StringTable.AddItem(Lex1stPass.YYText());
                if ((ret=Lex1stPass.yylex())!='{') goto End; 
            }
            else
            {
                pUnit->m_stiParent = g_StringTable.AddItem( DEFAULT_UNIT_PARENT);
            }

			do 
			{
                Lex1stPass.m_bSearchSets = true;

				// konec definice jednotky
				if ((ret=Lex1stPass.yylex())=='}') break;
				// identifikator (typ)
				else if (ret!=FIRSTPASS_IDENTIFIER) goto End;
				strID1=Lex1stPass.YYText();
                
				// identifikator (nazev metody nebo datove polozky)
				if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;

                nArrayLevel1 = Lex1stPass.m_nArrayLevel;
                
                // identifikator
				strID2=Lex1stPass.YYText();
                
                ret=Lex1stPass.yylex();

                nArrayLevel2 = Lex1stPass.m_nArrayLevel;

				if ((ret)==';')
				// datova polozka - jedna
				{
					pUnit->AddMember(strID2, strID1, nArrayLevel2, &m_Preview);
					continue;   
				}
				else if (ret==',')
				// datova polozka - vice
				{
					pUnit->AddMember(strID2, strID1, nArrayLevel2, &m_Preview);

					while (ret==',')
					{
						if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;

						strID2=Lex1stPass.YYText();
                        
                        ret=Lex1stPass.yylex();

                        nArrayLevel2 = Lex1stPass.m_nArrayLevel;

						pUnit->AddMember(strID2, strID1, nArrayLevel2, &m_Preview);

						if ((ret)!=',' && ret!=';') goto End;
					}
					continue;
				}
				else if (ret=='(')
				// metoda
				{
					bEvent=(strID1=="command" || strID1 == "notification");

					if (!bEvent) pMethod=pUnit->AddMethod(strID2, strID1, nArrayLevel1, &m_Preview);
						
					while (ret!=')')
					{
						if ((ret=Lex1stPass.yylex())==')') break;

						if (ret!=FIRSTPASS_IDENTIFIER) goto End;

						strID1=Lex1stPass.YYText();
						
						if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;
                        ret=Lex1stPass.yylex();

                        nArrayLevel1 = Lex1stPass.m_nArrayLevel;

						if (ret!=',' && ret!=')') goto End;

						// pridani argumentu
						if (!bEvent) pMethod->AddArgument(&m_Namespace, strID1, nArrayLevel1, &m_Preview);
					}

                    Lex1stPass.m_bSearchSets = false;

					pMethod=NULL;
				}
				else goto End;

				continue;

			} while (true);
		}
        // STRUKTURA
        else if (ret==FIRSTPASS_STRUCT)
        {
			// identifikator struktury
			if (Lex1stPass.yylex()!=FIRSTPASS_IDENTIFIER) goto End;

			// smazani stareho zaznamu o strukture
			m_Namespace.RemoveType(CString(Lex1stPass.YYText()));

			// zalozeni noveho zaznamu o strukture
			pStruct=m_Preview.AddStruct(CString(Lex1stPass.YYText()));

			if (Lex1stPass.yylex()!='{') goto End;

			do 
			{
                Lex1stPass.m_bSearchSets = true;

				// konec definice struktury
				if ((ret=Lex1stPass.yylex())=='}') break;
				// identifikator (typ)
				else if (ret!=FIRSTPASS_IDENTIFIER) goto End;

				strID1=Lex1stPass.YYText();

				// identifikator (nazev datove polozky)
				if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;

				strID2=Lex1stPass.YYText();

				if ((ret=Lex1stPass.yylex())==';')
				// datova polozka - jedna
				{
					pStruct->AddMember(strID2, strID1, Lex1stPass.m_nArrayLevel, &m_Preview);
					continue;   
				}
				else if (ret==',')
				// datova polozka - vice
				{
					pStruct->AddMember(strID2, strID1, Lex1stPass.m_nArrayLevel, &m_Preview);

					while (ret==',')
					{
						if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;

						strID2=Lex1stPass.YYText();

                        ret=Lex1stPass.yylex();

						pStruct->AddMember(strID2, strID1, Lex1stPass.m_nArrayLevel, &m_Preview);

						if (ret!=',' && ret!=';') goto End;
					}
					continue;
				}
				else goto End;

				continue;

			} while (true);

            Lex1stPass.m_bSearchSets = false;
        }
		// GLOBALNI FUNKCE
		else if (ret==FIRSTPASS_IDENTIFIER)
		{
            Lex1stPass.m_bSearchSets = true;

			strID1 = Lex1stPass.YYText();  // navrat.typ
            
			// identifikator (nazev globalni funkce)
			if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;

            nArrayLevel1 = Lex1stPass.m_nArrayLevel;

			// smazani stareho zaznamu oglobalni funkci
            strID2=Lex1stPass.YYText();
			m_Namespace.RemoveGlobalFunction( CString(strID2));

			if ((ret=Lex1stPass.yylex())!='(') goto End;

			pFunc=m_Preview.AddGlobalFunction(strID2, strID1, nArrayLevel1);
				
			while (ret!=')')
			{
				if ((ret=Lex1stPass.yylex())==')') break;

				if (ret!=FIRSTPASS_IDENTIFIER) goto End;

				strID1=Lex1stPass.YYText();
				
				if ((ret=Lex1stPass.yylex())!=FIRSTPASS_IDENTIFIER) goto End;
                ret=Lex1stPass.yylex();

                nArrayLevel1 = Lex1stPass.m_nArrayLevel;

				if (ret!=',' && ret!=')' && ret!='{') goto End;

        		// pridani argumentu
		    	pFunc->AddArgument(&m_Namespace, strID1, nArrayLevel1, &m_Preview);
			}

            Lex1stPass.m_bSearchSets = false;

			if ((ret=Lex1stPass.yylex())!='{') goto End;
			while ((ret=Lex1stPass.yylex())!='}' && ret!=0) ;
			if (ret==0) bEnd=true;

			pFunc=NULL;
		}
		// neco jineho
		else goto End;
    }

End:
	m_pLex1stPass = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CCompilerKernel::SecondPass()
{
    int ret;

#ifdef _DEBUG
    if (yydebug)
    {
        if (m_pwndDebugFrame==NULL)
        {
            CCreateContext cc;
    
            m_pwndDebugFrame=new CDebugFrame();
            m_pwndDebugFrame->Create(NULL, "Compiler debug");

            cc.m_pNewViewClass=RUNTIME_CLASS(CRichEditView);
            cc.m_pCurrentDoc=((CFrameWnd*)::AfxGetMainWnd())->GetActiveDocument();
            cc.m_pNewDocTemplate=cc.m_pCurrentDoc->GetDocTemplate();
            cc.m_pLastView=NULL;
            cc.m_pCurrentFrame=m_pwndDebugFrame;

            m_pwndDebugFrame->m_pwndDebugView=(CRichEditView*)m_pwndDebugFrame->CreateView(&cc);

            m_DebugFont.CreateFont(14,14,0,0,10,0,0,0,0,0,0,0,0,"Courier");
            m_pwndDebugFrame->m_pwndDebugView->SetFont(&m_DebugFont);
        }

        ASSERT(m_pwndDebugFrame!=NULL);
        ASSERT(::IsWindow(m_pwndDebugFrame->GetSafeHwnd()));
        ASSERT(m_pwndDebugFrame->m_pwndDebugView!=NULL);
        ASSERT(::IsWindow(m_pwndDebugFrame->m_pwndDebugView->GetSafeHwnd()));

        m_pwndDebugFrame->ShowWindow(SW_SHOW);

        m_pwndDebugFrame->m_pwndDebugView->SetWindowText("");
    }
#endif

	// preklad
    m_pLex2ndPass = new CPLex2ndPass(m_pStdIn, this);

    ret=yyparse(this);

    delete m_pLex2ndPass;
    m_pLex2ndPass = NULL;

    // UPLOAD NOVEHO KODU, POKUD BYL PREKLAD USPESNY
    CPCurrentUnit *pPomUnit = m_pCompiledUnits;
    CPCurrentGlobalFunction *pPomGF = m_pCompiledGlobalFunctions;
    CPCurrentStruct *pPomStruct = m_pCompiledStructs;
	CCompilerOutput *pStdOut;

    if (m_nErrorCount == 0 && m_bUpdateCodeOnSuccess)
    {
		ASSERT(m_pCodeManagerForCompiler!=NULL);

        m_pStdErr->PutString("Updating...\n");

		TRACE_COMPILED_CODE0("Prelozeny bytecode:\n===================\n\n");

        // jednotky
        for ( pPomUnit = m_pCompiledUnits;  pPomUnit != NULL;  pPomUnit = pPomUnit->m_pNext)
        {
            try
            {
			    pStdOut=m_pCodeManagerForCompiler->UploadScriptCodeStart(SCRIPT_UNIT, pPomUnit->m_strName, pPomUnit->m_strParent);
				// kdyz neprojde, nekde nastala chyba, o ktere je (vzdy?) hlaseni v logu
				
				if (pStdOut == NULL)
				{
					ErrorMessage("cannot update unit '%s' - check the log for error description.", pPomUnit->m_strName);
				}
				else
				{
	                pPomUnit->Save(pStdOut);
			    
		            m_pCodeManagerForCompiler->UploadScriptCodeEnd();
				}
            }
		    catch (CFileException *e)
		    {
			    char str[256];
			    e->GetErrorMessage(str, 256);
			    ErrorMessage(str);
		    }
		}

        // globalni funkce
        for ( pPomGF = m_pCompiledGlobalFunctions;  pPomGF != NULL;  pPomGF = pPomGF->m_pNext)
        {
            try
            {
			    pStdOut=m_pCodeManagerForCompiler->UploadScriptCodeStart(SCRIPT_FUNCTION, pPomGF->m_strName);
			    ASSERT(pStdOut!=NULL);  // kdyz neprojde, tak dva kompilatory pristupuji na jeden code manager

		        pPomGF->Save(pStdOut);

			    m_pCodeManagerForCompiler->UploadScriptCodeEnd();
            }
		    catch (CFileException *e)
		    {
			    char str[256];
			    e->GetErrorMessage(str, 256);
			    ErrorMessage(str);
		    }
        }

        // struktury
        for ( pPomStruct = m_pCompiledStructs;  pPomStruct != NULL;  pPomStruct = pPomStruct->m_pNext)
        {
		    try
		    {
			    pStdOut=m_pCodeManagerForCompiler->UploadScriptCodeStart(SCRIPT_STRUCTURE, 
				    pPomStruct->m_strName);
			    ASSERT(pStdOut!=NULL);  // kdyz neprojde, tak dva kompilatory pristupuji na jeden code manager

			    pPomStruct->Save(pStdOut);

			    m_pCodeManagerForCompiler->UploadScriptCodeEnd();
		    }
		    catch (CFileException *e)
		    {
			    char str[256];
			    e->GetErrorMessage(str, 256);
			    ErrorMessage(str);
		    }
        }

        m_pStdIn->OnInputCompiledAndUploadedSuccessfully();
	}

    // UVOLNENI ZKOMPILOVANYCH VECI Z PAMETI
    while (m_pCompiledUnits != NULL)
    {
        pPomUnit = m_pCompiledUnits;
        m_pCompiledUnits = m_pCompiledUnits->m_pNext;
        delete pPomUnit;
    }

    while (m_pCompiledGlobalFunctions != NULL)
    {
        pPomGF = m_pCompiledGlobalFunctions;
        m_pCompiledGlobalFunctions = m_pCompiledGlobalFunctions->m_pNext;
        delete pPomGF;
    }

    while (m_pCompiledStructs != NULL)
    {
        pPomStruct = m_pCompiledStructs;
        m_pCompiledStructs = m_pCompiledStructs->m_pNext;
        delete pPomStruct;
    }

	// uvolneni ostatnich veci
	delete m_pCurrentGlobalFunction;
	delete m_pCurrentUnit;
	delete m_pCurrentStruct;

    // DEBUGOVACI VYPIS TOHO, CO SE PRELOZILO
	if (m_bDebugOutput)
	{
		m_Namespace.DebugWrite(m_pDbgOut);
		m_Preview.DebugWrite(m_pDbgOut);
	}

#ifdef _DEBUG
    if (yydebug) 
	{
		Pause();
		m_pwndDebugFrame->ShowWindow(SW_HIDE);
	}
#endif

    return ret!=0;
}


#define _helper_Replace(str, strnew)    if (strstr(errstr,str)==errstr) \
                                {  \
                                    strErrOut+=strnew;      \
                                    errstr+=strlen(str); \
                                }   

void CCompilerKernel::ParserError(char *errstr)
{
    CString strErrOut;

/* Error messages produced by bison: 

  (1)	"parse error"
  (2)	"parse error, expecting `'st'' or `'st''"  (nekdy jen jednoduche uvozovky)
  (3)	"syntax error: cannot back up"
  (4)	"parser stack overflow"
  (5)	"parse error; also virtual memory exceeded"
*/

    if (strstr(errstr,"parse error")==errstr) // messages (1),(2) and (5)
	{
		if (strlen(errstr)==11)  // length of "parse error" => message (1)
		{
			if (m_pStdIn->GetLastChar()==EOF)
			{
				strErrOut="unexpected end of file";
			}
			else 
            {
                strErrOut.Format("syntax error: '%s'",m_pLex2ndPass->YYText());
            }
		}
		else
		{
			if (errstr[11]==',') // message (2)
			{
				if (strstr(errstr,"`$'")) strErrOut.Format("syntax error: '%s'",m_pLex2ndPass->YYText());
				else
				{
					errstr+=23;  // "expecting "
	
					strErrOut="syntax error: missing ";
					
					while (errstr[0])
					{
						switch (errstr[0])
						{
						case '`': 
						case '\'': 
							if (errstr[1]!='\'') strErrOut+="'"; 
							errstr++; 
							break;
						default: 
							switch (errstr[0])
							{
                            case 'C': _helper_Replace("CASE","case"); 
                                      _helper_Replace("CONSTRUCTOR","constructor");
                                      _helper_Replace("CATCH_TOKEN","catch");
                                      break;
                            case 'e': if (strstr(errstr,"error")==errstr)
                                      {
                                          errstr+=6; // delka "error" plus apostrof
                                          // ? nasleduje or?
                                          if ( strstr( errstr," or "))
                                          {
                                              errstr+=5; // delka " or " plus apostrof;
                                          }
                                          else
                                          {
                                              if (m_pStdIn->GetLastChar()!=EOF) 
                                              {
                                                  strErrOut = CString("syntax error: ") + m_pLex2ndPass->YYText() + '"';
                                              }
                                              else strErrOut = "syntax error: end of file";
                                          }
                                              
                                      }
                                      break;
                            case 'I': _helper_Replace("IDENTIFIER","identifier"); 
                                      _helper_Replace("IN_KWD","in");
                                      break;
                            case 'L': _helper_Replace("LITERAL_INT", "int");
                                      _helper_Replace("LITERAL_FLOAT", "float");
                                      _helper_Replace("LITERAL_BOOL", "true, false");
                                      _helper_Replace("LITERAL_STRING", "constant string");
                                      _helper_Replace("LITERAL_CHAR", "char");
                                      break;
                            case 'W': _helper_Replace("WHILE","while");
                                      break;
                            default:
                                // neco jineho
								char *s;
    							if ((s=strchr(errstr,'\''))!=NULL)
								{
									strErrOut+=CString(errstr).Left(s-errstr);
									errstr=s;
								}
								else
								{
									strErrOut+=errstr;
									errstr[0]=0;
								}
							}
							break;
						}
					}

					strErrOut+=" before '";
					if (m_pStdIn->GetLastChar()!=EOF) strErrOut+=m_pLex2ndPass->YYText();
                    else strErrOut+="end of file";
					strErrOut+="'";
				}
			}
			else ErrorMessage(errstr); // message (5);
		}

		ErrorMessage(strErrOut);
	}
	else ErrorMessage(errstr); // message (3),(4)

}

#undef _helper__helper_Replace

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::ErrorMessage(LPCTSTR strErrorMsgFormat, ...)
{
    CString strFormat, strMessage;

    if (m_pLex2ndPass != NULL)
	{
		strFormat.Format("%s(%d): error: %s\r\n", m_pStdIn->GetName(), m_pLex2ndPass->GetLineNo(), 
			strErrorMsgFormat);
	}
	else
	{
		ASSERT(m_pLex1stPass != NULL);
		strFormat.Format("%s(%d): error: %s\r\n", m_pStdIn->GetName(), m_pLex1stPass->GetLineNo(), 
			strErrorMsgFormat);
	}

    va_list args;
    va_start(args, strErrorMsgFormat);

    strMessage.FormatV(strFormat, args);

    m_pStdErr->PutString(strMessage);

    m_nErrorCount++ ;

	m_bGenerCode=false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::WarningMessage(int nLevel, LPCTSTR strWarningMsgFormat, ...)
{
    CString strFormat, strMessage;

    ASSERT(m_pLex2ndPass!=NULL);
    strFormat.Format("%s(%d): warning(%d): %s\r\n", m_pStdIn->GetName(), m_pLex2ndPass->GetLineNo(), 
        nLevel, strWarningMsgFormat);

    va_list args;
    va_start(args, strWarningMsgFormat);

    strMessage.FormatV(strFormat, args);

    m_pStdErr->PutString(strMessage);

    m_nWarningCount++ ;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::DebugOut(LPCTSTR strFormat, ...)
{
#ifdef _DEBUG

    CString str1, str2, text;
    int i;
    va_list args;
	
    if (!yydebug) return;

    if (strFormat[0]=='s')  str2="========> ";
    else    
    {
        ASSERT(m_pwndDebugFrame->m_pwndDebugView!=NULL);

        va_start(args, strFormat);
        str1.FormatV(strFormat, args);
    //    str.Replace("\n","\r\n");

        // nahrazeni \n  ->  \r\n
        for (i=0; i<str1.GetLength(); i++) 
        {
            if (str1[i]=='\n')  str2+="\r\n";
            else str2+=str1[i];
        }
    }

    long a;

    m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().ReplaceSel(str2);
    a=m_pwndDebugFrame->m_pwndDebugView->GetTextLength();
    m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().SetSel(a,a);
#else
    ::AfxGetMainWnd()->MessageBox("Debugovaci vystup je jen v debug verzi.","Error");
    yydebug=0;                    
#endif // _DEBUG
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CCompilerKernel::Pause()
{
    if (!yydebug) return;

    CString str;

    str.Format("\r\n%d: Press ENTER to continue or ESC to cancel compiler debug.", 
        m_pLex2ndPass->GetLineNo());

    m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().ReplaceSel(str);

    if (m_pwndDebugFrame->Pause())
    {
        long a=m_pwndDebugFrame->m_pwndDebugView->GetTextLength();
        m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().SetSel(a,a);
        m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().ReplaceSel("\r\n-----------------------------------------------------------\r\n");
        a=m_pwndDebugFrame->m_pwndDebugView->GetTextLength();
        m_pwndDebugFrame->m_pwndDebugView->GetRichEditCtrl().SetSel(a,a);
    }
    else
    {
        m_pwndDebugFrame->ShowWindow(SW_HIDE);  

        yydebug=0;
    }
}

#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////////////////

bool CCompilerKernel::AddLocalVariable(CPExpr &Var, CPNamespace::CVariable **ppVar)
{
	CType *pType;
	CPNamespace::CVariable *pTempVar;

	ASSERT(m_pCurrentMEG!=NULL);

	// jmeno
	if (m_Namespace.GetVariable(Var.m_strIdentifier)!=NULL)
	{
		ErrorMessage("'%s': redefinition", Var.m_strIdentifier);
		return false;
	}

	if (m_pCurrentUnit!=NULL && m_pCurrentUnit->HasMember(Var.m_strIdentifier, &m_Preview)!=NULL)
	{
		ErrorMessage("'%s': redefinition of unit member", Var.m_strIdentifier);
		return false;
	}

	//typ
	pType=Var.m_DataType.GetBasicType();

	if (pType->IsUnit() && 
		m_Namespace.GetType((CString)*pType->m_stiName)==NULL &&
		m_Preview.GetUnit((CString)*pType->m_stiName)==NULL)
	{
        // neni to nahodou struktura definovana potom (tzn. v Preview)?
        if (m_Preview.GetStruct((CString)*pType->m_stiName))
        {
            pType->SetStruct(pType->m_stiName);
        }
        else
        {
		    if (!m_Namespace.LoadUnitOrStructType(*pType))
            {
		        // kdyz se to nepovede
		        ErrorMessage("undefined type '%s'", (CString)*pType->m_stiName);
		        pType->SetError();
            }
			if (m_Namespace.GetType((CString)*pType->m_stiName)->m_nType == SCRIPT_STRUCTURE)
			{
				pType->SetStruct(pType->m_stiName->AddRef());
				pType->m_stiName->Release();
			}
        }
	}

    // kontrola delky jmena
    if (Var.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	CStringTableItem *stiName=g_StringTable.AddItem(Var.m_strIdentifier);
	REG reg=m_pCurrentMEG->AddLocalVariable(stiName, Var.m_DataType);
	pTempVar=m_Namespace.AddVariable(stiName, Var.m_DataType, reg, m_nLevel);
    stiName->Release();

	if (ppVar!=NULL) *ppVar=pTempVar;

	return pTempVar!=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CCompilerKernel::AddArgument(CPExpr &Arg, CPNamespace::CVariable **ppVar)
{
	CType *pType;
	CPNamespace::CVariable *pTempVar;
	
	ASSERT(m_pCurrentMEG!=NULL);
	if (m_pCurrentMEG->IsArgOrLocalVar(Arg.m_strIdentifier))
	{
		ErrorMessage("'%s' : redefinition", Arg.m_strIdentifier);
		return false;
	}

	if (m_pCurrentUnit!=NULL && m_pCurrentUnit->HasMember(Arg.m_strIdentifier, &m_Preview)!=NULL)
	{
		ErrorMessage("'%s': redefinition of unit member", Arg.m_strIdentifier);
		return false;
	}

	pType=Arg.m_DataType.GetBasicType();

	if (pType->IsUnit() && 
		m_Namespace.GetType((CString)*pType->m_stiName)==NULL &&
		m_Preview.GetUnit((CString)*pType->m_stiName)==NULL)
	{
        // neni to nahodou struktura definovana potom (tzn. v Preview)?
        if (m_Preview.GetStruct((CString)*pType->m_stiName))
        {
            pType->SetStruct(pType->m_stiName);
        }
        else
        {
            if (!m_Namespace.LoadUnitOrStructType(*pType))
            {
		        // kdyz se to nepovede
		        ErrorMessage("undefined type '%s'", (CString)*pType->m_stiName);
		        pType->SetError();
            }
			if (m_Namespace.GetType((CString)*pType->m_stiName)->m_nType == SCRIPT_STRUCTURE)
			{
				pType->SetStruct(pType->m_stiName->AddRef());
				pType->m_stiName->Release();
			}
        }
	}

	// kdyz event, tak kontrola spravnosti hlavicky
	if (m_pCurrentEventHeader!=NULL)
	{
        if ( m_pCurrentEventHeader->m_aArgs.GetSize() <= m_nFormalParamNo)
        {
            WarningMessage(1, "command/notification header doesn't match %s", 
				 m_pCurrentEventHeader->GetHeaderString());
        }
        else if (!(m_pCurrentEventHeader->m_aArgs[m_nFormalParamNo]==Arg.m_DataType))
		{
			ErrorMessage("expecting '%s' instead of '%s' (%s)", 
				m_pCurrentEventHeader->m_aArgs[m_nFormalParamNo].GetString(),
				Arg.m_DataType.GetString(), m_pCurrentEventHeader->GetHeaderString());
		}
	}

    // kontrola delky jmena
    if (Arg.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	CStringTableItem *stiName=g_StringTable.AddItem(Arg.m_strIdentifier);
	REG reg=m_pCurrentMEG->AddArgument(stiName, Arg.m_DataType);
	pTempVar=m_Namespace.AddVariable(stiName, Arg.m_DataType, reg, m_nLevel+1, true);
    stiName->Release();

	if (ppVar!=NULL) *ppVar=pTempVar;

	return pTempVar!=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::FreeArgInfo()
{
	CArgInfo *pArgInfo;
	while (!m_lArgInfoList.IsEmpty())
	{
		pArgInfo=m_lArgInfoList.GetHead();
		m_lArgInfoList.RemoveHead();
		delete pArgInfo;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::FreeSwitches()
{
	CSwitchStatement *pSwitchSt;
	while (!m_lSwitches.IsEmpty())
	{
		pSwitchSt=m_lSwitches.GetHead();
		m_lSwitches.RemoveHead();
		delete pSwitchSt;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CCompilerKernel::FindResType(CType &Type1, CType &Type2, CType &ResType)
{
    if ( Type1.IsUnit() && Type2.IsUnit()) 
    {
        ResType.SetUnit( g_StringTable.AddItem( DEFAULT_UNIT_PARENT, false));
        return true;
    }

    if (Type1.IsStruct() && Type2.IsStruct() && Type1.m_stiName == Type2.m_stiName)
    {
        ResType = Type1;
        return true;
    }

	bool b1=Type1.IsString(), b2=Type2.IsString();

    if ( (Type1.IsObject() && !b1) && (Type2.IsObject() && !b2) && Type2.m_nSOType == Type2.m_nSOType)
    {
        ResType = Type1;
        return true;
    }
	
	if (b1 || b2)
	{
		if (b1 && b2) 
		{
			ResType.SetString();
			return true;
		}
		else return false;
	}

	switch (Type1.m_nType)
	{
	case T_CHAR:
		switch (Type2.m_nType)
		{
			case T_CHAR:
				ResType.SetChar();
				break;
			case T_INT: 
				ResType.SetChar();
				break;
			default:
				return false;
		}
		break;
	case T_INT: 
		switch (Type2.m_nType)
		{
			case T_CHAR:
				ResType.SetChar();
				break;
			case T_INT: 
				ResType.SetInt();
				break;
			case T_FLOAT:
				ResType.SetFloat();
				break;
			default:
				return false;
		}
		break;
	case T_FLOAT:
		switch (Type2.m_nType)
		{
			case T_INT: 
			case T_FLOAT:
				ResType.SetFloat();
				break;
			default:
				return false;
		}
		break;
	case T_BOOL:
		switch (Type2.m_nType)
		{
			case T_BOOL:
				ResType.SetBool();
				break;
			default:
				return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

REG CCompilerKernel::GetReg()
{
	while (m_nFirstFreeRegister<m_aReservedRegisters.GetSize())
	{
		if (!m_aReservedRegisters[m_nFirstFreeRegister])
			return m_nFirstFreeRegister++;
		m_nFirstFreeRegister++;
	}

	m_aReservedRegisters.SetAtGrow(m_nFirstFreeRegister, false);
	return m_nFirstFreeRegister++;
}

bool CCompilerKernel::ReserveRegister(REG nReg)
{
	if (nReg >= m_aReservedRegisters.GetSize() || m_aReservedRegisters[nReg] || nReg == NO_REG) return false;
	else
	{
		m_aReservedRegisters[nReg]=true;
		return true;
	}
}

void CCompilerKernel::EndRegisterReservation(REG nReg)
{
	if (nReg<m_aReservedRegisters.GetSize()) m_aReservedRegisters[nReg]=false;
}

void CCompilerKernel::ResetLineNo() 
{  
    if (m_pLex2ndPass != NULL) 
    {
        m_pLex2ndPass->SetLineNo(1);

        // vypsani jmena noveho souboru na err.vystup
        m_pStdErr->Format(" %s\n", m_pStdIn->GetName());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//   Semantic actions
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//UnitDecl:   UNIT IDENTIFIER Super_opt { if (!((CCompilerKrnl*)pCompKrnl)->OnUnitBegin($2, $3)) YYERROR(); }
//				UnitBody			  { ((CCompilerKrnl*)pCompKrnl)->OnUnitEnd();	}
//            ;    
bool CCompilerKernel::OnUnitBegin(CPExpr& Name, CPExpr& Extends)
{
    CPNamespace::CUnit *pParentUnit = NULL;

	ASSERT(m_pCurrentUnit==NULL);
	ASSERT(Name.IsDeclar());

	// obnoveni generovani kodu na zacatku dalsi jednotky nebo funkce
	m_bGenerCode=true;

	m_pCurrentMEG=NULL;
	m_pCurrentType=NULL;

    // kontrola delky jmena
    if (Name.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	// kontrola jmena
	switch (m_Namespace.GetScriptType(Name.m_strIdentifier))
    {
    case SCRIPT_UNIT: 
//        ErrorMessage("unit '%s' type redefinition", Name.m_strIdentifier);
//        return false;
        break;
    case SCRIPT_OBJECT:
		ErrorMessage("name '%s' already used for a system object", Name.m_strIdentifier);
        return false;
    case SCRIPT_STRUCTURE:
        ErrorMessage("name '%s' already used for a structure", Name.m_strIdentifier);
        return false;
    case SCRIPT_FUNCTION:
        ErrorMessage("name '%s' already used for a global function", Name.m_strIdentifier);
        return false;
    case SCRIPT_SYSFUNCTION:
        ErrorMessage("name '%s' already used for a system global function", Name.m_strIdentifier);
        return false;
    }

	// extends
	ASSERT(Extends.IsDeclar());

	if ( (pParentUnit = m_Namespace.GetUnit(Extends.m_strIdentifier)) == NULL  &&
         (pParentUnit = m_Preview.GetUnit( Extends.m_strIdentifier)) == NULL)
	{
		if (!m_Namespace.LoadUnitType(Extends.m_strIdentifier))
        {
		    // kdyz se to nepovede
		    ErrorMessage("undefined unit '%s'", Extends.m_strIdentifier);
		    return false;
        }
        else
        {
            pParentUnit=m_Namespace.GetUnit(Extends.m_strIdentifier);
            //ASSERT(pParentUnit =! NULL);
        }
	}

    // vytvoreni unity
	m_pCurrentUnit=new CPCurrentUnit(Name.m_strIdentifier);
	m_pCurrentUnitPreview=m_Preview.GetUnit(Name.m_strIdentifier);
	m_pCurrentStringTable=&m_pCurrentUnit->m_StringTable;
	m_pCurrentFloatTable=&m_pCurrentUnit->m_FloatTable;

	m_pCurrentUnit->m_strParent=Extends.m_strIdentifier;
    m_pCurrentUnit->m_pParent = pParentUnit;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnUnitEnd()
{
	ASSERT(m_pCurrentUnit!=NULL);
    bool bDeleteUnit = false;

// nadefinovani konstruktoru / destruktoru, kdyz nejsou
    if (!m_pCurrentUnit->m_bConstructorDefined)
    {
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(CALLP, 0, -1);
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(RET);
    }

    if (!m_pCurrentUnit->m_bDestructorDefined)
    {
        m_pCurrentUnit->m_Destructor.m_Code.AddInstruction(CALLP, 0, -2);
        m_pCurrentUnit->m_Destructor.m_Code.AddInstruction(RET);
    }

// DEBUG
	if (m_bDebugOutput) m_pCurrentUnit->DebugWrite(m_pDbgOut);

// pridani do zkompilovanych jednotek
	if (m_bGenerCode && m_bUpdateCodeOnSuccess)
	{
        m_pCurrentUnit->m_pNext = m_pCompiledUnits;
        m_pCompiledUnits = m_pCurrentUnit;
	}
    else bDeleteUnit = true;

// pridani do namespace
	POSITION pos;
	CPNamespace::CUnit *pUnit;
	CPNamespace::CUnitMethod *pMethod;
	CPCurrentUnit::CMember *pCMember;
	CPCurrentUnit::CMethod *pCMethod;
	int i;
	CString str;

	// unita
	pUnit=m_Namespace.AddUnit(m_pCurrentUnit->m_strName, m_pCurrentUnit->m_strParent);

	// polozky
	pos=m_pCurrentUnit->m_apMembers.GetStartPosition();
	while (pos!=NULL)
	{
		m_pCurrentUnit->m_apMembers.GetNextAssoc(pos, str, pCMember);
		pUnit->AddMember((CString)*pCMember->m_cstiName, pCMember->m_DataType);
	}

	// metody
	pos=m_pCurrentUnit->m_apMethods.GetStartPosition();
	while (pos!=NULL)
	{
		m_pCurrentUnit->m_apMethods.GetNextAssoc(pos, str, pCMethod);
		pMethod=pUnit->AddMethod((CString)*pCMethod->m_cstiName, pCMethod->m_ReturnDataType);

		// argumenty
		for (i=0; i<pCMethod->m_nArgCount; i++) pMethod->AddArgument(pCMethod->m_aArgsAndLocalVars[i].m_DataType);
	}

// uvolneni a vynulovani
    if (bDeleteUnit) delete m_pCurrentUnit;
	m_pCurrentUnit=NULL;
	m_pCurrentUnitPreview=NULL;
	m_pCurrentStringTable=NULL;
	m_pCurrentFloatTable=NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////
//	 ConstantExpr:   Expr
void CCompilerKernel::OnConstantExpr(CPExpr &Res, CPExpr& Expr)
{
	if (Expr.IsError()) { Res.SetError(); return; };

	if (!Expr.IsConstant()) 
	{
		ErrorMessage("expecting constant expression");
		Res.SetErrorConstant();
		Res.m_LexVal.SetError();
	}
	else Res=Expr;
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnitItemDecl:	  CONSTRUCTOR   { ((CCompilerKrnl*)pCompKrnl)->OnConstructorBegins(); }
//					Block		{ ((CCompilerKrnl*)pCompKrnl)->OnConstructorEnds($3); }
//				;
void CCompilerKernel::OnConstructorBegins()
{
	ASSERT(m_pCurrentUnit!=NULL);

	if (m_pCurrentUnit->m_bConstructorDefined)
	{
		ErrorMessage("constructor redefinition");
		return;
	}

	m_pCurrentMEG=&m_pCurrentUnit->m_Constructor;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnConstructorEnds(CPExpr &Body)
{
	ASSERT(m_pCurrentUnit!=NULL);
	ASSERT(Body.IsCode() || Body.IsError());

	m_pCurrentUnit->m_bConstructorDefined=true;
	
   m_pCurrentUnit->m_Constructor.m_Code.Empty();
   if (Body.IsCode()) 
   {
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(CALLP, 0, -1);
        m_pCurrentUnit->m_Constructor.m_Code.AddCode( Body.m_Code);
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(RET);
   }

	m_pCurrentMEG=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnitItemDecl:	  DESTRUCTOR	{ ((CCompilerKrnl*)pCompKrnl)->OnDestructorBegins(); }
//				    Block		{ ((CCompilerKrnl*)pCompKrnl)->OnDestructorEnds($3); }
//				;
void CCompilerKernel::OnDestructorBegins()
{
	ASSERT(m_pCurrentUnit!=NULL);

	if (m_pCurrentUnit->m_bDestructorDefined)
	{
		ErrorMessage("destructor redefinition");
		return;
	}

    m_pCurrentUnit->m_Destructor.m_Code.AddInstruction(CALLP, 0, -2);

	m_pCurrentMEG=&m_pCurrentUnit->m_Destructor;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnDestructorEnds(CPExpr &Body)
{
	ASSERT(m_pCurrentUnit!=NULL);
	ASSERT(Body.IsCode() || Body.IsError());

	m_pCurrentUnit->m_bDestructorDefined=true;

   m_pCurrentUnit->m_Constructor.m_Code.Empty();
   if (Body.IsCode()) 
   {
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(CALLP, 0, -1);
        m_pCurrentUnit->m_Constructor.m_Code.AddCode( Body.m_Code);
        m_pCurrentUnit->m_Constructor.m_Code.AddInstruction(RET);
   }

	m_pCurrentMEG=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnitItemDecl:	  VOID_TYPE SetCurrentType IDENTIFIER '(' OnMethodBegins FormalParamList_opt ')' Block
//						{ ((CCompilerKrnl*)pCompKrnl)->OnMethodEnds($8); }
//				| VOID_TYPE SetCurrentType IDENTIFIER '(' OnMethodBegins error ')' Block
//						{ ((CCompilerKrnl*)pCompKrnl)->OnMethodEnds($8); }
//				| Type SetCurrentType IDENTIFIER '(' OnMethodBegins FormalParamList_opt ')' Block
//						{ ((CCompilerKrnl*)pCompKrnl)->OnMethodEnds($8); }
//				| Type SetCurrentType IDENTIFIER '(' OnMethodBegins error ')' Block				
//						{ ((CCompilerKrnl*)pCompKrnl)->OnMethodEnds($8); }
//				;
//OnMethodBegins:	  { if (!((CCompilerKrnl*)pCompKrnl)->OnMethodBegins($-1)) YYERROR; }
//				 ;
//SetCurrentType:	{  ((CCompilerKrnl*)pCompKrnl)->m_pCurrentType=&$0.m_DataType; }
//				;
bool CCompilerKernel::OnMethodBegins(CPExpr &Name)
{
	ASSERT(Name.IsDeclar());

    if (m_pCurrentUnit->HasMethodOnSelf(Name.m_strIdentifier)!=NULL)
	{
		ErrorMessage("method '%s' : redefinition", Name.m_strIdentifier);
		return false;
	}

    // kontrola delky jmena
    if (Name.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	m_nFormalParamNo=0;

	m_pCurrentMethod=m_pCurrentUnit->AddMethod(Name.m_strIdentifier, *m_pCurrentType);
	m_pCurrentMEG=m_pCurrentMethod;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnMethodEnds(CPExpr &Block)
{
	ASSERT(Block.IsCode() || Block.IsError());
	
    // check arguments if it's an overriden method
    CPNamespace::CUnitMethod *pParentMethod;
    CStringTableItem *stiParentName;
    if ( m_pCurrentUnit->GetMethodFromParent( (CString)*m_pCurrentMethod->m_cstiName, 
                                              &pParentMethod, &stiParentName, &m_Namespace) != NULL) 
    {
        if ( m_pCurrentMethod->m_nArgCount != pParentMethod->m_aArgs.GetSize()) 
        {
            goto ErrorMatchingArguments;
        }

        int i;
        for ( i=0; i<m_pCurrentMethod->m_nArgCount; i++)
        {
            if ( !(m_pCurrentMethod->m_aArgsAndLocalVars[i].m_DataType == pParentMethod->m_aArgs[i]))
            {
ErrorMatchingArguments:
                ErrorMessage("method overriding: arguments must be the same as in %s::%s", 
                            (CString)*stiParentName, (CString)*pParentMethod->m_stiName);
                return;
            }
        }
    }

	if (Block.IsError()) m_pCurrentMethod->m_Code.Empty();
	else 
	{
		if ( m_pCurrentMethod->m_ReturnDataType.IsVoid())
		{
			Block.m_Code.AddInstruction(RET);
		}
		else 
		{
			if (!Block.m_Code.m_bReturns)
			{
				ErrorMessage("not all control paths return a value");
			}
		}

		m_pCurrentMethod->m_Code=Block.m_Code;
	}

	m_pCurrentMethod=NULL;

	FreeArgInfo();
	m_bSendToStatement=false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnitItemDecl:	// EVENT
//				// COMMAND / NOTIFICATION
//				| COMMAND IDENTIFIER '(' OnCommandBegins FormalParamList_opt ')' Block 
//						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
//				| COMMAND IDENTIFIER '(' OnCommandBegins error ')' Block
//						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
//				| NOTIFICATION IDENTIFIER '(' OnNotificationBegins FormalParamList_opt ')' Block 
//						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
//				| NOTIFICATION IDENTIFIER '(' OnNotificationBegins error ')' Block
//						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
//				;
//
//OnCommandBegins:	  { if (!((CCompilerKernel*)pCompKrnl)->OnEventBegins($-1, true)) YYABORT; }
//				      ;
//OnNotificationBegins:	  { if (!((CCompilerKernel*)pCompKrnl)->OnEventBegins($-1, false)) YYABORT; }
//    				 ;
bool CCompilerKernel::OnEventBegins(CPExpr &Name, bool bIsCommand)
{
	ASSERT(Name.IsDeclar());
	
	if (Name.m_strIdentifier.FindOneOf("abcdefghijklmnopqrstuvwxyz")!=-1)
	{
		WarningMessage(2, "command/notification identifier should be uppercase");
	}

	Name.m_strIdentifier.MakeUpper();

	if (m_pCurrentUnit->HasEvent(Name.m_strIdentifier)!=NULL)
	{
		ErrorMessage("command/notification %s : redefinition", Name.m_strIdentifier);
		return false;
	}

	m_pCurrentEventHeader=m_Namespace.GetEventHeader(Name.m_strIdentifier);

    // kontrola delky jmena
    if (Name.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	m_nFormalParamNo=0;

	m_pCurrentEvent=m_pCurrentUnit->AddEvent(Name.m_strIdentifier, bIsCommand);
	m_pCurrentMEG=m_pCurrentEvent;

	return m_pCurrentEvent!=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnEventEnds(CPExpr &Block)
{
	ASSERT(Block.IsCode() || Block.IsError());

	if (Block.IsError()) m_pCurrentEvent->m_Code.Empty();
	else 
	{
		Block.m_Code.AddInstruction(RET);
		m_pCurrentEvent->m_Code=Block.m_Code;
	}

	m_pCurrentEvent=NULL;
	m_pCurrentEventHeader=NULL;

	FreeArgInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////
//FormalParam:    Type  SetCurrentType  ParamDecl
//					{ ((CCompilerKrnl*)pCompKrnl)->OnFormalParam($3); }
//                ;
bool CCompilerKernel::OnFormalParam(CPExpr &Name)
{
	ASSERT(Name.IsDeclar());

	AddArgument(Name);
	m_nFormalParamNo++;

	return true;	
}

/////////////////////////////////////////////////////////////////////////////////////////
//ParamDecl:      ParamDecl '[' ']'	{ ((CCompilerKrnl*)pCompKrnl)->OnParamDeclArray($$, $1); }
//				;
void CCompilerKernel::OnParamDeclArray(CPExpr &Res, CPExpr &ParamDecl)
{
	ASSERT(ParamDecl.IsDeclar());

	Res=ParamDecl;
	Res.m_DataType.MakeArray();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ParamDecl:      IDENTIFIER '{' '}'	{ ((CCompilerKrnl*)pCompKrnl)->OnParamDeclSet($$, $1); }
//				;
void CCompilerKernel::OnParamDeclSet(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;
    Res.m_DataType.MakeSet();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ParamDecl:        IDENTIFIER		{ ((CCompilerKrnl*)pCompKrnl)->OnParamDeclIdentifier($$, $1); }
//				;
void CCompilerKernel::OnParamDeclIdentifier(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;
}

/////////////////////////////////////////////////////////////////////////////////////////
//Block:  '{'							{ ((CCompilerKrnl*)pCompKrnl)->OnBlockBegins();       }
//			BlockStmtS_opt '}'		{ ((CCompilerKrnl*)pCompKrnl)->OnBlockEnds();  $$=$3; }
//        ;
void CCompilerKernel::OnBlockBegins()
{
	m_nLevel++;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnBlockEnds()
{
	m_nLevel--;
	m_Namespace.RemoveVariablesToLevel(m_nLevel);
}

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleFieldVarDecl:   IDENTIFIER	{ ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclIdentifier($$, $1); }
//                    ;
void CCompilerKernel::OnFieldVarDeclIdentifier(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;
}

/////////////////////////////////////////////////////////////////////////////////////////
//ArrayFieldVarDecl:    IDENTIFIER  '[' ConstantExpr ']'  
//                                    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclArrayIdent($$, $1, $3); }
//                    | ArrayFieldVarDecl '[' ConstantExpr ']'
//                                    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclArrayMore($$, $1, $3); }
//                    ;

void CCompilerKernel::OnFieldVarDeclArrayIdent(CPExpr &Res, CPExpr &Ident, CPExpr &Size)
{
	ASSERT(Ident.IsDeclar());
	ASSERT(Size.IsConstant());

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;

	if (Size.m_LexVal.IsInt()) Res.m_DataType.MakeArray(Size.m_LexVal.m_lInt);
	else Res.m_DataType.MakeArray(0);
}

void CCompilerKernel::OnFieldVarDeclArrayMore(CPExpr &Res, CPExpr &FieldVarDecl, CPExpr &Size)
{
	ASSERT(FieldVarDecl.IsDeclar());
	ASSERT(Size.IsConstant());
	
	Res=FieldVarDecl;

	if (Size.m_LexVal.IsInt()) Res.m_DataType.MakeArray(Size.m_LexVal.m_lInt);
	else Res.m_DataType.MakeArray(0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//SetFieldVarDecl:      IDENTIFIER '{' '}'    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclSet($$, $1); }
//                    ;

void CCompilerKernel::OnFieldVarDeclSet( CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;

	Res.m_DataType.MakeSet();
}

/////////////////////////////////////////////////////////////////////////////////////////
//FieldVarDeclS:    FieldVarDecl	{ ((CCompilerKrnl*)pCompKrnl)->OnFieldVarDecl($1); }
//                | FieldVarDeclS ',' FieldVarDecl { ((CCompilerKrnl*)pCompKrnl)->OnFieldVarDecl($3); }
//                ;
void CCompilerKernel::OnFieldVarDecl(CPExpr &Member)
{
	ASSERT(Member.IsDeclar());

	CType *pType;
	
	ASSERT(m_pCurrentUnit!=NULL || m_pCurrentStruct!=NULL);

	if ((m_pCurrentUnit!=NULL && m_pCurrentUnit->HasMember(Member.m_strIdentifier, &m_Preview)!=NULL)
        || (m_pCurrentStruct!=NULL && m_pCurrentStruct->HasMember(Member.m_strIdentifier)!=NULL))
	{
		ErrorMessage("'%s' : member redefinition", Member.m_strIdentifier);
		return;
	}

	pType=Member.m_DataType.GetBasicType();

	if (pType->IsUnit() && 
		m_Namespace.GetType((CString)*pType->m_stiName)==NULL &&
		m_Preview.GetUnit((CString)*pType->m_stiName)==NULL)
	{
        // neni to nahodou struktura definovana potom (tzn. v Preview)?
        if (m_Preview.GetStruct((CString)*pType->m_stiName))
        {
            pType->SetStruct(pType->m_stiName);
        }
        else
        {
            if (!m_Namespace.LoadUnitOrStructType(*pType))
            {
		        // kdyz se to nepovede
		        ErrorMessage("undefined type '%s'", (CString)*pType->m_stiName);
		        pType->SetError();
            }
			if (m_Namespace.GetType((CString)*pType->m_stiName)->m_nType == SCRIPT_STRUCTURE)
			{
				pType->SetStruct(pType->m_stiName->AddRef());
				pType->m_stiName->Release();
			}
        }
	}

    // kontrola delky jmena
    if (Member.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return;
    }

    if (m_pCurrentUnit!=NULL)
        m_pCurrentUnit->AddMember(Member.m_strIdentifier, Member.m_DataType);
	if (m_pCurrentStruct!=NULL)
        m_pCurrentStruct->AddMember(Member.m_strIdentifier, Member.m_DataType);

	return;
}

/////////////////////////////////////////////////////////////////////////////////////////
//LocalVarDeclS:    LocalVarDecl	
//                | LocalVarDeclS ',' LocalVarDecl   { ((CCompilerKrnl*)pCompKrnl)->OnLocalVarDeclConcat($$, $1, $3); }
//                ;

void CCompilerKernel::OnLocalVarDeclConcat(CPExpr &Res, CPExpr &Var1, CPExpr &Var2)
{
    Res.SetError();
    if ( Var1.IsError() || Var2.IsError()) return;

    ASSERT(Var1.IsCode());
    ASSERT(Var2.IsCode());

    Res.SetNewCode();
    Res.m_Code.CreateFromConcatenation(Var1.m_Code, Var2.m_Code);
}

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleVarDecl:    IDENTIFIER			{ ((CCompilerKrnl*)pCompKrnl)->OnSimpleVarDecl($$, $1); }
//                ;
void CCompilerKernel::OnSimpleVarDecl(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	CPNamespace::CVariable *pVar;

	Ident.m_DataType=*m_pCurrentType;
	if (!AddLocalVariable(Ident, &pVar))
	{
		Res.SetError();
	}
	else
	{	
		ASSERT( pVar != NULL);
	
		Res.SetNewCode();
		Res.m_nReg = pVar->m_nReg;
        Res.m_DataType = pVar->m_DataType;
        Res.m_bLValue = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//VariableInit_opt:     /* empty */
//                    | '=' Expr      { ((CCompilerKernel*)pCompKrnl)->OnVarDeclInit($$, $2); }
//                    | '=' error
//                    ;

void CCompilerKernel::OnVarDeclInit(CPExpr &Res, CPExpr &Expr)
{
	Res.IsError();

	if ( Expr.IsError()) return;

	if (!Expr.IsConstant()) Expr.Load();

	ASSERT( Expr.IsCode() || Expr.IsConstant());

	Res = Expr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// LocalVarDecl:     LocalVarDeclNotInit VariableInit_opt { ((CCompilerKernel*)pCompKrnl)->OnVarDeclInitDone($$, $1, $2); }

void CCompilerKernel::OnVarDeclInitDone(CPExpr &Res, CPExpr &Variable, CPExpr &Expr)
{
	Res.SetError();

	if (Variable.IsError() || Expr.IsError()) return;

	if ( Expr.IsNone())
	{
		Res.SetNewCode();
		return;
	}

    // null
    if ( Variable.m_DataType.IsObjectUnitStruct() && Expr.IsNull())
    {
        Res.SetNewCode();
        Res.AddInstructionChangeRegister( SETNULL, Variable.m_nReg);
    }
    // other init
    else
    {
        CPExpr Operator, Primary;
        Operator.SetConstant('=', this);

        OnAssign( Res, Variable, Operator, Expr);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//ArrayVarDecl:     IDENTIFIER '[' ConstantExpr ']'	{ ((CCompilerKrnl*)pCompKrnl)->OnArrayVarDeclIdent($1, $3); }
//                ;
void CCompilerKernel::OnArrayVarDeclIdent(CPExpr &Res, CPExpr &Ident, CPExpr &Size)
{
	ASSERT(Ident.IsDeclar());
	ASSERT(Size.IsConstant());

    // kontrola delky jmena
    if (Ident.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return;
    }

	Res=Ident;
	Res.m_DataType=*m_pCurrentType;

	if (Size.m_LexVal.IsInt()) Res.m_DataType.MakeArray(Size.m_LexVal.m_lInt);
	else Res.m_DataType.MakeArray();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ArrayVarDecl:   ArrayVarDecl '[' ConstantExpr ']' { ((CCompilerKrnl*)pCompKrnl)->OnArrayVarDeclMore($1, $3); }
//                ;
void CCompilerKernel::OnArrayVarDeclMore(CPExpr &Res, CPExpr &Var, CPExpr &Size)
{
	ASSERT(Var.IsDeclar());
	ASSERT(Size.IsConstant());

	Res=Var;

	if (Size.m_LexVal.IsInt()) Res.m_DataType.MakeArray(Size.m_LexVal.m_lInt);
	else Res.m_DataType.MakeArray();
}

/////////////////////////////////////////////////////////////////////////////////////////
//LocalVarDecl:   ArrayVarDecl    { ((CCompilerKrnl*)pCompKrnl)->OnArrayVarDecl($$, $1); }  
//				;
void CCompilerKernel::OnArrayVarDecl(CPExpr &Res, CPExpr &Var)
{
	ASSERT(Var.IsDeclar());

	CPNamespace::CVariable *pVar;

	if (!AddLocalVariable(Var, &pVar))
	{
		Res.SetError();
	}
	else
	{
		ASSERT( pVar != NULL);

		Res.SetNewCode();
		Res.m_nReg = pVar->m_nReg;
        Res.m_DataType = pVar->m_DataType;
        Res.m_bLValue = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnitOrGlobalFunctionDecl:     ReturnType IDENTIFIER '('			 {   ((CCompilerKrnl*)pCompKrnl)->OnGlobalFunctionBegins($1, $2); }
//								  FormalParamList_opt ')' Block  {   ((CCompilerKrnl*)pCompKrnl)->OnGlobalFunctionEnds($7); }
//							
//							  | ReturnType IDENTIFIER '('			 {   ((CCompilerKrnl*)pCompKrnl)->OnGlobalFunctionBegins($1, $2); }
//								  error ')' Block				 {   ((CCompilerKrnl*)pCompKrnl)->OnGlobalFunctionEnds($7); }
//							;
bool CCompilerKernel::OnGlobalFunctionBegins(CPExpr &RetType, CPExpr &Name)
{
	ASSERT(Name.IsDeclar());
	ASSERT(RetType.IsType());

	// obnoveni generovani kodu na zacatku dalsi jednotky nebo funkce
	m_bGenerCode=true;

	m_pCurrentMEG=NULL;
	m_pCurrentType=NULL;

	// navratovy typ
	if (RetType.m_DataType.IsUnit() && 
		m_Namespace.GetType((CString)*RetType.m_DataType.m_stiName)==NULL &&
		m_Preview.GetUnit((CString)*RetType.m_DataType.m_stiName)==NULL)
	{
        // neni to nahodou struktura definovana potom (tzn. v Preview)?
        if (m_Preview.GetStruct((CString)*RetType.m_DataType.m_stiName))
        {
            RetType.m_DataType.SetStruct(RetType.m_DataType.m_stiName);
        }
        else
        {
		    if (!m_Namespace.LoadUnitOrStructType(RetType.m_DataType))
            {
		        // kdyz se to nepovede
		        ErrorMessage("undefined type '%s'", (CString)*m_pCurrentType->m_stiName);
                m_pCurrentType->SetError();
            }
			if (m_Namespace.GetType((CString)*RetType.m_DataType.m_stiName)->m_nType == SCRIPT_STRUCTURE)
			{
				RetType.m_DataType.SetStruct( RetType.m_DataType.m_stiName->AddRef());
				RetType.m_DataType.m_stiName->Release();
			}
        }
	}

	m_nFormalParamNo=0;

	switch (m_Namespace.GetScriptType(Name.m_strIdentifier))
    {
    case SCRIPT_UNIT: 
        ErrorMessage("name '%s' already used for a unit");
        return false;
    case SCRIPT_OBJECT:
		ErrorMessage("name '%s' already used for a system object", Name.m_strIdentifier);
        return false;
    case SCRIPT_STRUCTURE:
        ErrorMessage("name '%s' already used for a structure", Name.m_strIdentifier);
        return false;
    case SCRIPT_FUNCTION:
//        ErrorMessage("global function of that name already exists");
//        return false;
        break;
    case SCRIPT_SYSFUNCTION:
        ErrorMessage("name '%s' already used for a system global function", Name.m_strIdentifier);
        return false;
    }

    // kontrola delky jmena
    if (Name.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	m_pCurrentGlobalFunction=new CPCurrentGlobalFunction(Name.m_strIdentifier, RetType.m_DataType);
	m_pCurrentMEG=m_pCurrentGlobalFunction;
	m_pCurrentStringTable=&m_pCurrentGlobalFunction->m_StringTable;
	m_pCurrentFloatTable=&m_pCurrentGlobalFunction->m_FloatTable;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnGlobalFunctionEnds(CPExpr &Block)
{
    bool bDeleteGF = false;

	ASSERT(Block.IsCode() || Block.IsError());
	
	if (Block.IsError()) m_pCurrentGlobalFunction->m_Code.Empty();
	else 
	{
		if ( m_pCurrentGlobalFunction->m_ReturnDataType.IsVoid())
		{
			Block.m_Code.AddInstruction(RET);
		}
		else
		{
			if (!Block.m_Code.m_bReturns)
			{
				ErrorMessage("not all control paths return a value");
			}
		}

		m_pCurrentGlobalFunction->m_Code=Block.m_Code;
	}

// pridani ke zkompilovanym funkcim
	if (m_bGenerCode && m_bUpdateCodeOnSuccess)
	{
        m_pCurrentGlobalFunction->m_pNext = m_pCompiledGlobalFunctions;
        m_pCompiledGlobalFunctions = m_pCurrentGlobalFunction;
	}
    else bDeleteGF = true;

	// pridani do namespace
    //!!! naloadeni z code manazera
	if (m_bGenerCode)
	{
		CPNamespace::CGlobalFunction *pFunct;
		CType Type;

		Type=m_pCurrentGlobalFunction->m_ReturnDataType;
		pFunct=m_Namespace.AddGlobalFunction(m_pCurrentGlobalFunction->m_strName, Type);

		int i;
		for (i=0; i<m_pCurrentGlobalFunction->m_nArgCount; i++) 
		{
			Type=m_pCurrentGlobalFunction->m_aArgsAndLocalVars[i].m_DataType;
			pFunct->AddArgument(Type);
		}
	}

	// DEBUG VYPIS
	if (m_bDebugOutput)	m_pCurrentGlobalFunction->DebugWrite(m_pDbgOut);

// uvolneni a vynulovani
    if (bDeleteGF) delete m_pCurrentGlobalFunction;
    m_pCurrentGlobalFunction=NULL;
	m_pCurrentStringTable=NULL;
	m_pCurrentFloatTable=NULL;

	FreeArgInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////
//LocalVarOrMember:   IDENTIFIER	{ ((CCompilerKrnl*)pCompKrnl)->OnLocalVarOrMember($$,$1); }
//					;
void CCompilerKernel::OnLocalVarOrMember(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	ASSERT(m_pCurrentStringTable!=NULL);

	CPNamespace::CVariable *pVar;

	// lokalni promenna nebo argument
	if ((pVar=m_Namespace.GetVariable(Ident.m_strIdentifier))!=NULL)
	{
		Res.SetCode(pVar->m_nReg, pVar->m_DataType);
		return;
	}

	// member current unity
	if (m_pCurrentUnit!=NULL && m_pCurrentUnit->HasMember(Ident.m_strIdentifier, &m_Preview)) 
    {
		CType Type;

        VERIFY( m_pCurrentUnit->GetMemberType(Ident.m_strIdentifier, Type, &m_Preview));

		if (!LoadDataType( Type)) Type.SetError();

		Res.SetCode(0, Type, true, CPExpr::LVALUE_UMEMBER, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		return;
    }
    
    if (m_pCurrentUnitPreview != NULL && m_pCurrentUnitPreview->HasMember(Ident.m_strIdentifier, &m_Preview))
	{
		CType Type;

        VERIFY( m_pCurrentUnitPreview->GetMemberType(Ident.m_strIdentifier, Type, &m_Preview));

		Res.SetCode(0, Type, true, CPExpr::LVALUE_UMEMBER, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		return;
	}

	// chyba
	ErrorMessage("'%s': undeclared identifier", Ident.m_strIdentifier);

    CType Type;
    Type.SetError();

    m_Namespace.AddVariable(g_StringTable.AddItem(Ident.m_strIdentifier, false),
        Type, NO_REG, m_nLevel);

	Res.SetError();
}

//////////////////////////////////////////////////////////////////////

bool CCompilerKernel::LoadDataType( CType &Type)
{
	if (Type.IsSimpleType()) return true;
	
	switch (Type.m_nType)
	{
	case T_OBJECT:
		if ( !Type.IsArray() && !Type.IsSet()) return true;
		ASSERT( Type.m_pOfType != NULL);
		return LoadDataType( *Type.m_pOfType);

	case T_UNIT:
		if ( m_Preview.GetUnit( (CString)*Type.m_stiName) != NULL) return true;
		if ( m_Namespace.GetUnit( (CString)* Type.m_stiName) != NULL) return true;
		if ( m_Namespace.LoadUnitOrStructType( Type)) return true;
		ErrorMessage("use of undefined type: '%s'", (CString)*Type.m_stiName);
		return false;

	case T_STRUCT:
		if ( m_Preview.GetStruct( (CString)*Type.m_stiName) != NULL) return true;
		if ( m_Namespace.GetStruct( (CString)* Type.m_stiName) != NULL) return true;
		if ( m_Namespace.LoadUnitOrStructType( Type)) return true;
		ErrorMessage("use of undefined type: '%s'", (CString)*Type.m_stiName);
		return false;

	case T_ERROR:
		return false;

	default:
		ASSERT( false);
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//FieldAccess:      PrimaryExpr '.' IDENTIFIER	{ ((CCompilerKrnl*)pCompKrnl)->OnFieldAccess($$, $1, $3); }
//                ;
void CCompilerKernel::OnFieldAccess(CPExpr &Res, CPExpr &Primary, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());
		
	Res.SetError();
	if (Primary.IsError()) return;;

	if (Primary.IsConstant())
	{
		ErrorMessage("left of '.%s' must have unit/object type", Ident.m_strIdentifier);
		return;
	}

	ASSERT(Primary.IsCode());  // nic jineho nezbyva

	// UNITA
	if (Primary.m_DataType.IsUnit())
	{
		CPNamespace::CUnit *pUnit;
		CPNamespace::CUnitMember *pMember;
		
		pUnit=m_Preview.GetUnit((CString)*Primary.m_DataType.m_stiName);
		if (pUnit==NULL) pUnit=m_Namespace.GetUnit((CString)*Primary.m_DataType.m_stiName);
		if (pUnit==NULL)
		{
			if (!LoadDataType( Primary.m_DataType)) return;
			pUnit=m_Namespace.GetUnit((CString)*Primary.m_DataType.m_stiName);
		}
		if( pUnit == NULL) 
        {
            Res.SetError();
            return;
        }
		
		if ((pMember=pUnit->GetMember(Ident.m_strIdentifier, &m_Preview))==NULL)
		{
			ErrorMessage("'%s' : is not a member of '%s'", Ident.m_strIdentifier, 
				Primary.m_DataType.GetString());
			return;
		}

		ASSERT(m_pCurrentStringTable!=NULL);

		Res=Primary;
		Res.Load(); 

		Res.m_DataType=pMember->m_DataType;
		Res.m_lNum=m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID;
		Res.m_nLValueType=CPExpr::LVALUE_UMEMBER;
	}
	// OBJECT
	else if (Primary.m_DataType.IsObject())
	{
		CPNamespace::CSysObject *pObj;
		CPNamespace::CSysObjectMember *pMember;
		
		pObj=m_Namespace.GetObject(Primary.m_DataType.m_nSOType);
		ASSERT(pObj!=NULL);
		
		if ((pMember=pObj->GetMemberByName(Ident.m_strIdentifier))==NULL)
		{
			ErrorMessage("'%s' : is not a member of '%s'", Ident.m_strIdentifier, 
				Primary.m_DataType.GetString());
			return;
		}

		Res=Primary;
		Res.Load();

		Res.m_DataType=pMember->m_DataType;
		Res.m_lNum=pMember->m_nID;
		Res.m_nLValueType=CPExpr::LVALUE_OMEMBER;
	}
    // STRUKTURA
    else if (Primary.m_DataType.IsStruct())
    {
		CPNamespace::CStruct *pStruct;
		
		pStruct=m_Preview.GetStruct((CString)*Primary.m_DataType.m_stiName);
		if (pStruct==NULL) 	pStruct=m_Namespace.GetStruct( (CString)*Primary.m_DataType.m_stiName);
		if (pStruct==NULL)
		{
			if (!LoadDataType( Primary.m_DataType)) return;
			pStruct=m_Namespace.GetStruct( (CString)*Primary.m_DataType.m_stiName);
		}
		ASSERT( pStruct != NULL);
		
		if (!pStruct->HasMember(Ident.m_strIdentifier))
		{
			ErrorMessage("'%s' : is not a member of '%s'", Ident.m_strIdentifier, 
				Primary.m_DataType.GetString());
			return;
		}

		ASSERT(m_pCurrentStringTable!=NULL);

		Res=Primary;
		Res.Load(); 

		pStruct->GetMemberType(Ident.m_strIdentifier,Res.m_DataType);
		Res.m_lNum=m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID;
		Res.m_nLValueType=CPExpr::LVALUE_SMEMBER;
    }
	// JINAK
	else
	{
		ErrorMessage("left of '.%s' must have unit/object type", Ident.m_strIdentifier);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//ArrayAccess:      PrimaryExpr '[' Expr ']'	{ ((CCompilerKrnl*)pCompKrnl)->OnArrayAccess($$, $1, $3);  }	
//                | PrimaryExpr '[' error ']' { ((CCompilerKrnl*)pCompKrnl)->OnArrayAccessError($$, $1); }	
//                ;
void CCompilerKernel::OnArrayAccess(CPExpr &Res, CPExpr &Primary, CPExpr &Expr)
{
	Res.SetError();
	if (Primary.IsError() || Expr.IsError()) return;

	if (Primary.IsConstant())
	{
		ErrorMessage("subscript requires array or string");
		return;
	}

	ASSERT(Primary.IsCode());  // nic jineho nezbyva
	
	if (!Primary.m_bLValue)
	{
		ErrorMessage("subscript must be l-value");
		return;
	}

	// konverze na LVALUE_PRIMARY
	Primary.Load();
    Expr.Load();

    if (!Primary.m_DataType.IsArray() && !Primary.m_DataType.IsString())
	{
		ErrorMessage("subscript requires array or string");
		return;
	}

	Res=Primary;

	// ted mame Res typu LVALUE_PRIMARY, pridat index:

	// POLE
	if (Res.m_DataType.IsArray() || Res.m_DataType.IsString()) 
	{
		CType Type;
		Type.SetInt();

		if (!Expr.ConvertToType(Type))
		{
			ErrorMessage("cannot convert from '%s' to int", Expr.m_DataType.GetString());
		}
		
		if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg());

		Res.m_Code.AddCode(Expr.m_Code);
		Res.m_lNum=Expr.m_nReg;
		Res.m_bLValue=true;
		Res.m_nLValueType=CPExpr::LVALUE_INDEX;

		if (Res.m_DataType.IsArray()) Res.m_DataType.SetOfType(); // array
		else Res.m_DataType.SetChar(); // string
	}
}

void CCompilerKernel::OnArrayAccessError(CPExpr &Res, CPExpr &Primary)
{
	CPExpr Expr;

	Expr.SetError();

	OnArrayAccess(Res, Primary, Expr);
}

void CCompilerKernel::OnEndOfStatement()
{
	ResetRegisters();
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:     PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocPrimaryIdentEnds($$, $1, $3, $5, $6);	}
//					| PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$);	}
//                    ;
//
//OnMethodInvocPrimaryIdentBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocPrimaryIdentBegins($$, $-3, $-1); }
//									;
void CCompilerKernel::OnMethodInvocPrimaryIdentBegins(CPExpr &Res, CPExpr &Primary, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	CArgInfo *pArgInfo=new CArgInfo();
	m_lArgInfoList.AddHead(pArgInfo);
	
	Res.SetError();
	if (Primary.IsError()) return;;

	if (Primary.IsConstant())
	{
		ErrorMessage("left of '.%s' must have unit/object type", Ident.m_strIdentifier);
		return;
	}

	ASSERT(Primary.IsCode());  // nic jineho nezbyva
	
	/* nemusi to byt l-value
	if (!Primary.m_bLValue)
	{
		ErrorMessage("left of '.%s' must be l-value", Ident.m_strIdentifier);
		return;
	}
	*/

	// UNITA
	if (Primary.m_DataType.IsUnit())
	{
		CPNamespace::CUnit *pUnit;
		CPNamespace::CUnitMethod *pMethod;
		
		pUnit=m_Namespace.GetUnit((CString)*Primary.m_DataType.m_stiName);
		if (pUnit == NULL) pUnit = m_Preview.GetUnit((CString)*Primary.m_DataType.m_stiName);

        if (pUnit==NULL) 
        {
            Res.SetError();
            return;
        }
		
		if ((pMethod=pUnit->GetMethod(Ident.m_strIdentifier, &m_Preview))==NULL)
		{
			ErrorMessage("'%s' : is not a method of '%s'", Ident.m_strIdentifier, 
				Primary.m_DataType.GetString());
			return;
		}

		Res.SetNewCode();
		Res.m_Code.AddInstruction(CRTBAG);
		pArgInfo->Set(&pMethod->m_aArgs, pMethod->m_aArgs.GetSize());
	}
	// OBJECT
	else if (Primary.m_DataType.IsObject())
	{
		CPNamespace::CSysObject *pObj;
		CPNamespace::CSysObjectMethod *pMethod;
		
		pObj=m_Namespace.GetObject(Primary.m_DataType.m_nSOType);
		ASSERT(pObj!=NULL);
		
		if ((pMethod=pObj->GetMethodByName(Ident.m_strIdentifier))==NULL)
		{
			ErrorMessage("'%s' : is not a method of '%s'", Ident.m_strIdentifier, 
				Primary.m_DataType.GetString());
			return;
		}

		Res.SetNewCode();
		Res.m_Code.AddInstruction(CRTBAG);
		pArgInfo->Set(&pMethod->m_aArgs, pMethod->m_aArgs.GetSize());
	}
	// JINAK
	else
	{
		ErrorMessage("left of '.%s' must have unit/object type", Ident.m_strIdentifier);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:   IDENTIFIER '(' OnMethodInvocIdentBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocIdentEnds($$, $1, $3, $4);	}
//
//					| IDENTIFIER '(' OnMethodInvocIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                    ;
//
//OnMethodInvocIdentBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocIdentBegins($$, $-1); }
//							;
void CCompilerKernel::OnMethodInvocIdentBegins(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	CPNamespace::CUnitMethod *pMethod;
	CPNamespace::CGlobalFunction *pGF;
	CPNamespace::CSysGlobalFunction *pSGF;
	CArgInfo *pArgInfo=new CArgInfo();

	m_lArgInfoList.AddHead(pArgInfo);

	Res.SetNewCode();
	Res.m_Code.AddInstruction(CRTBAG);

	// metoda - tehle jednotky
	if (m_pCurrentUnitPreview!=NULL && 
        (pMethod=m_pCurrentUnitPreview->GetMethod(Ident.m_strIdentifier, &m_Preview))!=NULL)
	{
		pArgInfo->Set(&pMethod->m_aArgs, pMethod->m_aArgs.GetSize());
		return;
	}

	// metoda - predka
	if (m_pCurrentUnit != NULL)
	{
		ASSERT( m_pCurrentUnit->m_pParent != NULL);

		if ((pMethod = m_pCurrentUnit->m_pParent->GetMethod( Ident.m_strIdentifier, &m_Preview)) != NULL)
		{
			pArgInfo->Set( &pMethod->m_aArgs, pMethod->m_aArgs.GetSize());
			return;
		}
	}

	// systemova globalni funkce
	if ((pSGF=m_Namespace.GetSysGlobalFunctionByName(Ident.m_strIdentifier))!=NULL)
	{
		pArgInfo->Set(&pSGF->m_aArgs, pSGF->m_aArgs.GetSize());
		return;
	}

	// skriptova globalni funkce
	if ((pGF=m_Preview.GetGlobalFunction(Ident.m_strIdentifier))==NULL
		&& (pGF=m_Namespace.GetGlobalFunction(Ident.m_strIdentifier))==NULL)
	{
        // neznama funkce -> pokus o natazeni od CodeManagera
        if (m_Namespace.LoadGlobalFunctionType(Ident.m_strIdentifier))
        {
            pGF=m_Namespace.GetGlobalFunction(Ident.m_strIdentifier);
        }
    }

    if (pGF!=NULL)
    {
		pArgInfo->Set(&pGF->m_aArgs, pGF->m_aArgs.GetSize());
		return;
    }	

	// chyba
	ErrorMessage("'%s': undeclared identifier", Ident.m_strIdentifier);
	Res.SetError();
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:   SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocSuperEnds($$, $3, $5, $6);	}
//
//                  | SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                    ;
//
//OnMethodInvocSuperBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocSuperBegins($$, $-1); }
//							;
void CCompilerKernel::OnMethodInvocSuperBegins(CPExpr &Res, CPExpr &Ident)
{
	ASSERT(Ident.IsDeclar());

	CPNamespace::CUnit *pUnit;
	CPNamespace::CUnitMethod *pMethod;
	CArgInfo *pArgInfo=new CArgInfo();

	m_lArgInfoList.AddHead(pArgInfo);

	Res.SetError();

	// metoda
	if (m_pCurrentUnit!=NULL)
	{
		if (m_pCurrentUnit->m_strParent.IsEmpty())
		{
			ASSERT(false);
			return;
		}
		
		if ((pUnit=m_Namespace.GetUnit(m_pCurrentUnit->m_strParent))==NULL)
		{
			ErrorMessage("'%s' : undefined parent unit", m_pCurrentUnit->m_strName);
			return;
		}

		if ((pMethod=pUnit->GetMethod(Ident.m_strIdentifier, &m_Preview))!=NULL)
		{
			Res.SetNewCode();
			Res.m_Code.AddInstruction(CRTBAG);
			pArgInfo->Set(&pMethod->m_aArgs, pMethod->m_aArgs.GetSize());
			return;
		}
		else
		{
			ErrorMessage("'%s' : is not a method of '%s'", Ident.m_strIdentifier, 
				m_pCurrentUnit->m_strParent);
			return;
		}
	}
	else
	{
		ErrorMessage("no current unit");
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//Super_opt:    /* empty */	{ ((CCompilerKrnl*)pCompKrnl)->OnEmptySuper($$); }
//            | Super
//            ;
void CCompilerKernel::OnEmptySuper(CPExpr &Res)
{
	Res.SetDeclar(CString(DEFAULT_UNIT_PARENT), this);
}

/////////////////////////////////////////////////////////////////////////////////////////
//ArgumentList:     Expr						{ ((CCompilerKrnl*)pCompKrnl)->OnArgumentFirst($$, $1);		}
//                | ArgumentList ',' Expr		{ ((CCompilerKrnl*)pCompKrnl)->OnArgumentNext($$, $1, $3);	}
//                ;
void CCompilerKernel::OnArgumentFirst(CPExpr &Res, CPExpr &Expr)
{
	OnArgument(Res, Expr);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnArgumentNext(CPExpr &Res, CPExpr &Args, CPExpr &Expr)
{
	CPExpr PartRes;
	
	OnArgument(PartRes, Expr);

	if (PartRes.IsError()) Res.SetError();
	else
	{
		Res.SetNewCode();
		Res.m_Code.CreateFromConcatenation(Args.m_Code, PartRes.m_Code);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnArgument(CPExpr &Res, CPExpr &Expr)
{
	Res.SetError();

	if (Expr.IsError()) return;

	ASSERT(Expr.IsCode() || Expr.IsConstant());

	if (Expr.IsCode() && Expr.m_bLValue) 
	{
		Expr.ConvertToRValue();
		if (Expr.IsError()) return;
	}

	// kontrola typu argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	ASSERT(pArgInfo!=NULL);  
	bool bNull = false;
	
	if (!pArgInfo->m_bValid) 
	{
		if (!m_bSendToStatement) return;
	}
	else
	{
		if (!pArgInfo->GetNext(Type))
		{
			ErrorMessage("method or function takes %d argument(s)", pArgInfo->m_nArgCount);
			return;
		}

		if (!Expr.ConvertToType(Type))
		{
			if ( Type.IsObjectUnitStruct() && Expr.IsConstant() && Expr.m_LexVal.IsInt() && Expr.m_LexVal.m_lInt == 0)
			{
				bNull = true;
			}
			else
			{
				ErrorMessage("argument %d : cannot convert from '%s' to '%s'", pArgInfo->m_nCurrentArgNo,
					Expr.m_DataType.GetString(), Type.GetString());
				return;
			}
		}
	}

	if (Expr.IsConstant())
	{
		CPLexVal LV=Expr.m_LexVal;
		Res.SetNewCode();

		switch (Expr.m_LexVal.m_nType)
		{
		case CPLexVal::LEXVAL_ERROR:	
		case CPLexVal::LEXVAL_NONE:	
			ASSERT(false);  // tohle by mela odfiltrovat kontrola typu
			break;
		case CPLexVal::LEXVAL_INT:
			if (bNull)
			{
				ASSERT( LV.m_lInt == 0);
				Res.m_Code.AddInstruction(ADDBAG_NULL);
			}
			else
			{
				Res.m_Code.AddInstruction(ADDBAG_CINT, LV.m_lInt);
			}
			break;
		case CPLexVal::LEXVAL_FLOAT:  
			ASSERT(m_pCurrentFloatTable!=NULL);
			Res.m_Code.AddInstruction(ADDBAG_CFLT, m_pCurrentFloatTable->Add(LV.m_fFloat));
			break;
		case CPLexVal::LEXVAL_BOOL:	
			Res.m_Code.AddInstruction(ADDBAG_CBOOL, LV.m_bBool);
			break;
		case CPLexVal::LEXVAL_CHAR:	
			Res.m_Code.AddInstruction(ADDBAG_CCHAR, LV.m_chChar);
			break;
		case CPLexVal::LEXVAL_STRING: 
            Res = Expr;
            Res.MoveConstantToRegister(GetReg());
			Res.m_Code.AddInstruction(ADDBAG_REG, Res.m_nReg);
			break;
		}
	}
	else // code
	{
		Res=Expr;
        Res.Load();
		Res.m_Code.AddInstruction(ADDBAG_REG, Res.m_nReg);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:   PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$);	}
//					| IDENTIFIER '(' OnMethodInvocIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                  | SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                  ;
void CCompilerKernel::OnMethodInvocEndsError(CPExpr &Res)
{
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;
}	

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:     PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocPrimaryIdentEnds($$, $1, $3, $5, $6);	}
//					| PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$);	}
//                    ;
//
//OnMethodInvocPrimaryIdentBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocPrimaryIdentBegins($$, $-3, $-1); }
//									;
void CCompilerKernel::OnMethodInvocPrimaryIdentEnds(CPExpr &Res, CPExpr &Primary, CPExpr &Ident, 
													CPExpr &Begin, CPExpr &Args)
{
	Res.SetError(); 

	if (Begin.IsError()) return;

	// vsechny kontroly byly udelany v OnMethodInvocPrimaryIdentBegins, pokud byla nejaka
	// chyba, je Begin nastaveno na error a sem uz se to nedostane.
	ASSERT(Primary.IsCode());  
	ASSERT(Ident.IsDeclar());

	// kontrola zadani vsech argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	if (pArgInfo->GetNext(Type))
	{
		ErrorMessage("'%s' takes %d argument(s)", Ident.m_strIdentifier, pArgInfo->m_nArgCount);
	}
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;

	// UNITA
	void *pMethod;

	if (Primary.m_DataType.IsUnit())
	{
		CPNamespace::CUnit *pUnit=m_Namespace.GetUnit((CString)*Primary.m_DataType.m_stiName);
        if (pUnit == NULL) pUnit = m_Preview.GetUnit((CString)*Primary.m_DataType.m_stiName);
		
        if (pUnit == NULL) 
        {
            Res.SetError();
            return;
        }
		
		pMethod=(void*)pUnit->GetMethod(Ident.m_strIdentifier, &m_Preview);
		ASSERT(pMethod!=NULL); // vsechno by uz melo byt zkontrolovano v OnMethodInvocPrimaryIdentBegins
	}
	else
	{
		ASSERT(Primary.m_DataType.IsObject());

		CPNamespace::CSysObject *pObj=m_Namespace.GetObject(Primary.m_DataType.m_nSOType);
		ASSERT(pObj!=NULL);
		
		pMethod=pObj->GetMethodByName(Ident.m_strIdentifier);
		ASSERT(pMethod!=NULL); // vsechno by uz melo byt zkontrolovano v OnMethodInvocPrimaryIdentBegins
	}

	Res.SetNewCode();
	if (!Args.IsError()) Res.m_Code.CreateFromConcatenation(Begin.m_Code, Args.m_Code);
	else Res=Begin;

	ASSERT(m_pCurrentStringTable!=NULL);

	Primary.Load();
	Res.m_Code.AddCode(Primary.m_Code);
	
	REG newreg=GetReg();
	if (Primary.m_DataType.IsUnit()) 
	{
		Res.m_Code.AddInstruction(CALLU, newreg, Primary.m_nReg, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		Res.SetCode(newreg, ((CPNamespace::CUnitMethod*)pMethod)->m_ReturnDataType, false);
	}
	else 
	{
		Res.m_Code.AddInstruction(CALLS, newreg, Primary.m_nReg, 
			((CPNamespace::CSysObjectMethod*)pMethod)->m_nID);
		Res.SetCode(newreg, ((CPNamespace::CSysObjectMethod*)pMethod)->m_ReturnDataType, false);
	}

	Res.m_Code.AddInstruction(DELBAG);
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:   IDENTIFIER '(' OnMethodInvocIdentBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocIdentEnds($$, $1, $3, $4);	}
//
//					| IDENTIFIER '(' OnMethodInvocIdentBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                    ;
//
//OnMethodInvocIdentBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocIdentBegins($$, $-1); }
//							;
void CCompilerKernel::OnMethodInvocIdentEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Begin, CPExpr &Args)
{
	if (Begin.IsError())
	{
		Res.SetError(); 
		return;
	}

	// vsechny kontroly byly udelany v OnMethodInvocIdentBegins, pokud byla nejaka
	// chyba, je Begin nastaveno na error a sem uz se to nedostane.
	ASSERT(Ident.IsDeclar());

	// kontrola zadani vsech argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	if (pArgInfo->GetNext(Type))
	{
		ErrorMessage("'%s' takes %d argument(s)", Ident.m_strIdentifier, pArgInfo->m_nArgCount);
	}
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;

	Res.SetNewCode();
	if (!Args.IsError()) Res.m_Code.CreateFromConcatenation(Begin.m_Code, Args.m_Code);
	else Res=Begin;

	CPNamespace::CUnitMethod *pMethod;
	CPNamespace::CGlobalFunction *pGF;
	CPNamespace::CSysGlobalFunction *pSGF;
	REG newreg=GetReg();

	ASSERT(m_pCurrentStringTable!=NULL);

	// metoda current unity
	if (m_pCurrentUnitPreview!=NULL && 
        (pMethod=m_pCurrentUnitPreview->GetMethod(Ident.m_strIdentifier, &m_Preview))!=NULL)
	{
		Res.m_Code.AddInstruction(CALLU, newreg, 0, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		Res.SetCode(newreg, pMethod->m_ReturnDataType, false);
	}
	// metoda predka
	else if (m_pCurrentUnit != NULL && m_pCurrentUnit->m_pParent != NULL && 
			 (pMethod = m_pCurrentUnit->m_pParent->GetMethod( Ident.m_strIdentifier, &m_Preview)) != NULL)
	{
		Res.m_Code.AddInstruction(CALLU, newreg, 0, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		Res.SetCode(newreg, pMethod->m_ReturnDataType, false);
	}
	// systemova globalni funkce
	else if ((pSGF=m_Namespace.GetSysGlobalFunctionByName(Ident.m_strIdentifier))!=NULL)
	{
		Res.m_Code.AddInstruction(CALLS, newreg, 0, pSGF->m_nID);
		Res.SetCode(newreg, pSGF->m_ReturnDataType, false);
	}
	// skriptova globalni funkce
	else 
	{
		pGF=m_Namespace.GetGlobalFunction(Ident.m_strIdentifier);
		if (pGF==NULL) pGF=m_Preview.GetGlobalFunction(Ident.m_strIdentifier);
		ASSERT(pGF!=NULL);

		Res.m_Code.AddInstruction(CALLG, newreg, 
			m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
		Res.SetCode(newreg, pGF->m_ReturnDataType, false);
	}

	Res.m_Code.AddInstruction(DELBAG);
}

/////////////////////////////////////////////////////////////////////////////////////////
//MethodInvocation:   SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins ArgumentList_opt ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocSuperEnds($$, $3, $5, $6);	}
//
//                  | SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins error ')'
//						{	((CCompilerKrnl*)pCompKrnl)->OnMethodInvocEndsError($$); }
//                    ;
//
//OnMethodInvocSuperBegins:	{ ((CCompilerKrnl*)pCompKrnl)->OnMethodInvocSuperBegins($$, $-1); }
//							;
void CCompilerKernel::OnMethodInvocSuperEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Begin, CPExpr &Args)
{
	if (Begin.IsError())
	{
		Res.SetError(); 
		return;
	}

	// vsechny kontroly byly udelany v OnMethodInvocSuperBegins, pokud byla nejaka
	// chyba, je Begin nastaveno na error a sem uz se to nedostane.
	ASSERT(Ident.IsDeclar());

	// kontrola zadani vsech argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	if (pArgInfo->GetNext(Type))
	{
		ErrorMessage("'%s' takes %d argument(s)", Ident.m_strIdentifier, pArgInfo->m_nArgCount);
	}
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;

	Res.SetNewCode();
	if (!Args.IsError()) Res.m_Code.CreateFromConcatenation(Begin.m_Code, Args.m_Code);
	else Res=Begin;

	CPNamespace::CUnit *pUnit;
	CPNamespace::CUnitMethod *pMethod;
	REG newreg=GetReg();

	ASSERT(m_pCurrentUnit!=NULL);
	ASSERT(!m_pCurrentUnit->m_strParent.IsEmpty());
	ASSERT(m_pCurrentStringTable!=NULL);

	pUnit=m_Namespace.GetUnit(m_pCurrentUnit->m_strParent);
	ASSERT(pUnit!=NULL);

	pMethod=pUnit->GetMethod(Ident.m_strIdentifier, &m_Preview);
	ASSERT(pMethod!=NULL);

	Res.m_Code.AddInstruction(CALLP, newreg, 
		m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
	Res.SetCode(newreg, pMethod->m_ReturnDataType, false);
	Res.m_Code.AddInstruction(DELBAG);
}

/////////////////////////////////////////////////////////////////////////////////////////
//BlockStmtS:   BlockStmt
//			  | BlockStmtS BlockStmt	{ ((CCompilerKrnl*)pCompKrnl)->OnBlockStatementConcat($$, $1, $2); }
//            ;
void CCompilerKernel::OnBlockStatementConcat(CPExpr &Res, CPExpr &BlockStmtS, CPExpr &BlockStmt)
{
	Res.SetNewCode();
	
	if (BlockStmt.IsError())
	{
		if (BlockStmtS.IsError()) return;
		else Res=BlockStmtS;
	}
	else
	{
		if (BlockStmtS.IsError()) Res=BlockStmt;
		else Res.m_Code.CreateFromConcatenation(BlockStmtS.m_Code, BlockStmt.m_Code);
		Res.m_Code.m_bReturns = BlockStmt.m_Code.m_bReturns;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//PostfixExpr:      PrimaryExpr
//                | PostfixExpr PLUSPLUS		%prec PLUSPLUSPOSTFIX	
//					{ ((CCompilerKrnl*)pCompKrnl)->OnPostfixExpr($$, $1, '+');	}
//                | PostfixExpr MINUSMINUS    %prec MINUSMINUSPOSTFIX
//					{ ((CCompilerKrnl*)pCompKrnl)->OnPostfixExpr($$, $1, '-');	}
//                ;
void CCompilerKernel::OnPostfixExpr(CPExpr &Res, CPExpr &Expr, char cWhich)
{
	Res.SetError();

	if (Expr.IsError()) return;

	ASSERT(Expr.IsCode() || Expr.IsConstant());

	if (Expr.IsConstant() || (Expr.IsCode() && !Expr.m_bLValue))
	{
		ErrorMessage("'%c%c' needs l-value", cWhich, cWhich);
		return;
	}

	if (!Expr.m_DataType.IsSimpleType() || Expr.m_DataType.IsBool())
	{
		ErrorMessage("type mismatch");
		return;
	}

	// zapamatovani l-value Expr
	REG nReg=Expr.m_nReg;
	long lNum=Expr.m_lNum;
	CPExpr::ELValueType nLValueType=Expr.m_nLValueType;

	Res=Expr;

	Res.ConvertToRValue();

	REG nNewReg=GetReg();

	Res.m_Code.AddInstruction(MOV, nNewReg, Res.m_nReg);
	if (cWhich=='+') 
	{
		switch (Res.m_DataType.m_nType)
		{
		case T_CHAR:
			Res.m_Code.AddInstruction(INC_CHAR, Res.m_nReg);
			break;
		case T_INT:
			Res.m_Code.AddInstruction(INC_INT, Res.m_nReg);
			break;
		case T_FLOAT:
			Res.m_Code.AddInstruction(INC_FLT, Res.m_nReg);
			break;
		default:
			ASSERT(false);
		}
	}
	else 
	{
		switch (Res.m_DataType.m_nType)
		{
		case T_CHAR:
			Res.m_Code.AddInstruction(DEC_CHAR, Res.m_nReg);
			break;
		case T_INT:
			Res.m_Code.AddInstruction(DEC_INT, Res.m_nReg);
			break;
		case T_FLOAT:
			Res.m_Code.AddInstruction(DEC_FLT, Res.m_nReg);
			break;
		default:
			ASSERT(false);
		}
	}

	Res.Store(nLValueType, nReg, lNum);

	Res.m_nReg=nNewReg;
	Res.m_bLValue=false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//UnaryExpr:    PostfixExpr
//            | PLUSPLUS UnaryExpr
//					{ ((CCompilerKrnl*)pCompKrnl)->OnUnaryExpr($$, $2, 'P');	}
//            | MINUSMINUS UnaryExpr
//					{ ((CCompilerKrnl*)pCompKrnl)->OnUnaryExpr($$, $2, 'M');	}
//            | '+' UnaryExpr  %prec UNARYPLUS
//					{ ((CCompilerKrnl*)pCompKrnl)->OnUnaryExpr($$, $2, '+');	}
//            | '-' UnaryExpr  %prec UNARYMINUS
//					{ ((CCompilerKrnl*)pCompKrnl)->OnUnaryExpr($$, $2, '-');	}
//            | '!' UnaryExpr
//					{ ((CCompilerKrnl*)pCompKrnl)->OnUnaryExpr($$, $2, '!');	}
//    

#define _helperOnUnaryExpr(member, instr, op)	\
	if (Res.IsConstant()) Res.m_LexVal.member= op Res.m_LexVal.member; \
	else	\
	{	\
		Res.ConvertToRValue();	\
		if (!IsRegister(Res.m_nReg))  \
			Res.AddInstructionChangeRegister(MOV, GetReg(), Res.m_nReg);	\
		Res.m_Code.AddInstruction(instr, Res.m_nReg);	\
	}


void CCompilerKernel::OnUnaryExpr(CPExpr &Res, CPExpr &Expr, char cWhich)
{
	Res.SetError();

	if (Expr.IsError()) return;

	ASSERT(Expr.IsCode() || Expr.IsConstant());

	if (cWhich=='P' || cWhich=='M')
	{
		if (Expr.IsConstant() || (Expr.IsCode() && !Expr.m_bLValue))
		{
			if (cWhich=='P') ErrorMessage("'++' needs l-value", cWhich, cWhich);
			else ErrorMessage("'--' needs l-value");
			return;
		}

		if (!Expr.m_DataType.IsSimpleType() || Expr.m_DataType.IsBool())
		{
			ErrorMessage("type mismatch");
			return;
		}

		// zapamatovani l-value Expr
		REG nReg=Expr.m_nReg;
		long lNum=Expr.m_lNum;
		CPExpr::ELValueType nLValueType=Expr.m_nLValueType;

		Res=Expr;

		Res.ConvertToRValue();

		if (cWhich=='P') 
		{
			switch (Res.m_DataType.m_nType)
			{
			case T_CHAR:
				Res.m_Code.AddInstruction(INC_CHAR, Res.m_nReg);
				break;
			case T_INT:
				Res.m_Code.AddInstruction(INC_INT, Res.m_nReg);
				break;
			case T_FLOAT:
				Res.m_Code.AddInstruction(INC_FLT, Res.m_nReg);
				break;
			default:
				ASSERT(false);
			}
		}
		else 
		{
			switch (Res.m_DataType.m_nType)
			{
			case T_CHAR:
				Res.m_Code.AddInstruction(DEC_CHAR, Res.m_nReg);
				break;
			case T_INT:
				Res.m_Code.AddInstruction(DEC_INT, Res.m_nReg);
				break;
			case T_FLOAT:
				Res.m_Code.AddInstruction(DEC_FLT, Res.m_nReg);
				break;
			default:
				ASSERT(false);
			}
		}

		Res.Store(nLValueType, nReg, lNum);

		Res.m_bLValue=false;
	}
	else // +,-,!
	{
		Res=Expr;
        Res.ConvertToRValue();

		switch (cWhich)
		{
		case '+':  // nedelej nic, co taky?
			break; 

		case '-':
			switch (Res.m_DataType.m_nType)
			{
			case T_INT:
				_helperOnUnaryExpr(m_lInt, INV_INT, -);
				break;
			case T_FLOAT:
				_helperOnUnaryExpr(m_fFloat, INV_FLT, -);
				break;
			default:
				ErrorMessage("type mismatch");
			}
			break;

		case '!':
			if (Res.m_DataType.IsBool()) 
            {
                _helperOnUnaryExpr(m_bBool, NEG, !)
            }
			else ErrorMessage("type mismatch");
			break;
		}
	}
}

#undef _helperOnUnaryExpr

void Pokus()
{/*
	int i=5;
	char c='A';
	float f=1.5;
	bool b=true;
	*/
}


void CCompilerKernel::OnAssign(CPExpr &Res, CPExpr &Primary, CPExpr &Oper, CPExpr &Expr)
{
	Res.IsError();
	if ( Primary.IsError() || Expr.IsError() || Expr.IsDeclar()) return;

	ASSERT(Oper.IsConstant() && Oper.m_LexVal.IsChar());
	char cWhich=Oper.m_LexVal.m_chChar;

	if (Primary.IsError() || Expr.IsError()) return;

	if (!Primary.m_bLValue)
	{
		if (cWhich=='=') ErrorMessage("left of '=' must be l-value");
		else ErrorMessage("left of '%c=' must be l-value", cWhich);
		return;
	}

	ASSERT(Primary.IsCode());
	ASSERT(Expr.IsCode() || Expr.IsConstant());

	Expr.ConvertToRValue();
	
	Res.SetNewCode();
	
	CPExpr::ELValueType nLValueType=Primary.m_nLValueType;
	REG nReg=Primary.m_nReg;
	long lNum=Primary.m_lNum;

	if (cWhich!='=')  // +=, -=, *=, /=, %=
	{
		OnBinaryOperation(Res, Primary, Expr, cWhich);
		Res.Store(nLValueType, nReg, lNum);
	}
	else	// =
	{
		if (!Expr.ConvertToType(Primary.m_DataType))
		{
			// prirazeni null
			if (Primary.m_DataType.IsObjectUnitStruct() && Expr.m_DataType.IsInt() && Expr.IsConstant()
				&& Expr.m_LexVal.m_lInt == 0)
			{
				Res = Primary;
				Res.StoreNull();
			}
			else
			{
				ErrorMessage("cannot convert from '%s' to '%s'",
					Expr.m_DataType.GetString(), Primary.m_DataType.GetString());
				return;
			}
		}
		else
		{
			if (Expr.IsConstant() && nLValueType==CPExpr::LVALUE_PRIMARY) 
			{
				Expr.MoveConstantToRegister(nReg);
				Res.m_Code.CreateFromConcatenation(Primary.m_Code, Expr.m_Code);
			}
			else 
			{
				if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg());
				Res.m_Code.CreateFromConcatenation(Primary.m_Code, Expr.m_Code);
				Res.SetCode(Expr.m_nReg, Expr.m_DataType, false);
				Res.Store(nLValueType, nReg, lNum);
			}
			Res.SetCode(Expr.m_nReg, Expr.m_DataType, false);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//MultiplExpr:      UnaryExpr	
//  			  | MultiplExpr '*' UnaryExpr	 
//					{ ((CCompilerKrnl*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '*');  }
//                | MultiplExpr '/' UnaryExpr
//					{ ((CCompilerKrnl*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '/');  }
//                | MultiplExpr '%' UnaryExpr
//					{ ((CCompilerKrnl*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '%');  }
//                ;
//AddExpr:      MultiplExpr	
//            | AddExpr '+' MultiplExpr
//				{ ((CCompilerKrnl*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '+');  }
//            | AddExpr '-' MultiplExpr
//				{ ((CCompilerKrnl*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '-');  }
//            ;

// pomocna makra pro nasledujici metodu
#define _helperOnBinaryOperation(op, member, i1, i2, i3); \
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	Res.SetConstant(Expr1.m_LexVal.member op Expr2.m_LexVal.member, this); \
		else Res.AddInstructionChangeRegister(i1, GetReg(), Expr2.m_nReg, Expr1.m_LexVal.member); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(i2, GetReg(), Expr1.m_nReg, Expr2.m_LexVal.member); \
		else Res.AddInstructionChangeRegister(i3, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnBinaryOperationDivide( member, i1, i2, i3); \
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	\
		{\
			if (Expr2.m_LexVal.member != NULL) Res.SetConstant(Expr1.m_LexVal.member / Expr2.m_LexVal.member, this); \
			else ErrorMessage("division by zero");\
		}\
		else Res.AddInstructionChangeRegister(i1, GetReg(), Expr2.m_nReg, Expr1.m_LexVal.member); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(i2, GetReg(), Expr1.m_nReg, Expr2.m_LexVal.member); \
		else Res.AddInstructionChangeRegister(i3, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnBinaryOperationModulo( member, i1, i2, i3); \
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	\
		{\
			if (Expr2.m_LexVal.member != NULL) Res.SetConstant(Expr1.m_LexVal.member % Expr2.m_LexVal.member, this); \
			else ErrorMessage("modulo by zero");\
		}\
		else Res.AddInstructionChangeRegister(i1, GetReg(), Expr2.m_nReg, Expr1.m_LexVal.member); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(i2, GetReg(), Expr1.m_nReg, Expr2.m_LexVal.member); \
		else Res.AddInstructionChangeRegister(i3, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnBinaryOperationFloat(op, member, i1, i2, i3) \
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	Res.SetConstant(Expr1.m_LexVal.member op Expr2.m_LexVal.member, this); \
		else Res.AddInstructionChangeRegister(i1, GetReg(), Expr2.m_nReg, m_pCurrentFloatTable->Add(Expr1.m_LexVal.member)); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(i2, GetReg(), Expr1.m_nReg, m_pCurrentFloatTable->Add(Expr2.m_LexVal.member)); \
		else Res.AddInstructionChangeRegister(i3, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnBinaryOperationFloatDivide( member, i1, i2, i3) \
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	\
		{\
			if (Expr2.m_LexVal.member != NULL) Res.SetConstant(Expr1.m_LexVal.member / Expr2.m_LexVal.member, this); \
			else ErrorMessage("division by zero");\
		}\
		else Res.AddInstructionChangeRegister(i1, GetReg(), Expr2.m_nReg, m_pCurrentFloatTable->Add(Expr1.m_LexVal.member)); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(i2, GetReg(), Expr1.m_nReg, m_pCurrentFloatTable->Add(Expr2.m_LexVal.member)); \
		else Res.AddInstructionChangeRegister(i3, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;


void CCompilerKernel::OnBinaryOperation(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich)
{
	Res.SetError();

	if (Expr1.IsError() || Expr2.IsError()) return;

	if (Expr1.m_DataType.IsError() || Expr2.m_DataType.IsError()) return;


	// kontrola zakladnich typu
	if (!Expr1.m_DataType.IsSimpleType() || !Expr2.m_DataType.IsSimpleType() ||
		Expr1.m_DataType.IsBool() || Expr2.m_DataType.IsBool())
	{
        // operatory pro mnoziny
        if (Expr1.m_DataType.IsSet())
        {
            // SET_UN, SET_IS, SET_DIF operace se dvema mnozinami
            if (Expr2.m_DataType.IsSet())
            {
                if (strchr("+*-",cWhich)==NULL)
                {
                    ErrorMessage("operator '%c' not defined on sets",cWhich);
                    return;
                }
                else
                {
                    if (!((*Expr1.m_DataType.m_pOfType)==(*Expr2.m_DataType.m_pOfType)))
                    {
                        ErrorMessage("sets must be of the same type for this operation");
                        return;
                    }
                
                    CType Type;
                    Type = *Expr1.m_DataType.m_pOfType;
                    Type.MakeSet();

                    Res.SetNewCode();
                    Res.SetCode(NO_REG, Type, true);
					Expr1.Load();
					Expr2.Load();
                    Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);

                    switch (cWhich)
                    {
                    case '+':
                        Res.AddInstructionChangeRegister(SET_UN, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                        break;
                    case '*':
                        Res.AddInstructionChangeRegister(SET_IS, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                        break;
                    case '-':
                        Res.AddInstructionChangeRegister(SET_DIF, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                        break;
                    }
                }
            }
            // SET_ADD, SET_REM - operatory s mnozinou a prvkem
            else
            {
                if (strchr("+-",cWhich)==NULL)
                {
                    ErrorMessage("operator '%c' not defined in this context",cWhich);
                    return;
                }
                else
                {
                    if ( !Expr2.ConvertToType( *Expr1.m_DataType.m_pOfType))
                    {
                        ErrorMessage("type mismatch");
                        return;
                    }
                
                    if (Expr2.IsConstant()) Expr2.MoveConstantToRegister( GetReg());

                    CType Type;
                    Type = *Expr1.m_DataType.m_pOfType;
                    Type.MakeSet();

                    Res.SetNewCode();
                    Res.SetCode(NO_REG, Type, true);
					Expr1.Load();
					Expr2.Load();
                    Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);

                    switch (cWhich)
                    {
                    case '+':
                        Res.AddInstructionChangeRegister(SET_ADD, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                        break;
                    case '-':
                        Res.AddInstructionChangeRegister(SET_REM, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                        break;
                    }
                }
            }
            return;
        }
        // operace na stringach - stringove '+'
        else if ((Expr1.m_DataType.IsChar() || Expr1.m_DataType.IsString()) && 
                 (Expr2.m_DataType.IsChar() || Expr2.m_DataType.IsString()))
        {
            if (cWhich!='+')
            {
                ErrorMessage("operator '%c' not defined on string",cWhich);
                return;
            }

			if (Expr1.m_DataType.IsChar()) Expr1.ConvertCharToString();
			if (Expr2.m_DataType.IsChar()) Expr2.ConvertCharToString();

			ASSERT(m_pCurrentStringTable!=NULL);
			Res.SetNewCode();

			if (Expr1.IsConstant())
			{
				if (Expr2.IsConstant())
				{
					CString Str=Expr1.m_LexVal.m_lpszString;
					Str+=Expr2.m_LexVal.m_lpszString;
					Res.SetConstant((LPCTSTR)Str, this);
				}
				else
				{
					Expr2.Load();

					Res=Expr2;
					Res.AddInstructionChangeRegister(STRADDCL, GetReg(), 
						m_pCurrentStringTable->Add(Expr1.m_LexVal.m_lpszString)->m_nID, Expr2.m_nReg);
					Res.m_DataType.SetString();
				}
			}
			else
			{
				if (Expr2.IsConstant())
				{
					Expr1.Load();

					Res=Expr1;
					Res.AddInstructionChangeRegister(STRADDCR, GetReg(), 
						Expr1.m_nReg, m_pCurrentStringTable->Add(Expr2.m_LexVal.m_lpszString)->m_nID);
					Res.m_DataType.SetString();
				}
				else
				{
					Expr1.Load();
					Expr2.Load();

					Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);
					Res.AddInstructionChangeRegister(STRADD, GetReg(), 
						Expr1.m_nReg, Expr2.m_nReg);
					Res.m_DataType.SetString();
				}
			}
			return;
		}
        else 
        {
			ErrorMessage("type mismatch");
			return;
		}
	} 
    
    // binarni operace na jednoduchych typech

	Expr1.ConvertToRValue();
	Expr2.ConvertToRValue();

	// kontrola & konverze typu
	CType ResType;

	if (!FindResType(Expr1.m_DataType, Expr2.m_DataType, ResType))
	{
		ErrorMessage("type mismatch");
		return;
	}

    if (cWhich == '%' && ResType.IsFloat())
    {
        ErrorMessage("type mismatch");
        return;
    }

	if (!Expr1.ConvertToType(ResType))
	{
		ErrorMessage("cannot convert '%s' to '%s'", Expr1.m_DataType.GetString(), 
			ResType.GetString());
		return;
	}

	if (!Expr2.ConvertToType(ResType))
	{
		ErrorMessage("cannot convert '%s' to '%s'", Expr2.m_DataType.GetString(), 
			ResType.GetString());
		return;
	}

	Res.SetNewCode();

	// spojeni kodu
	Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);

	// operace
	switch (ResType.m_nType)
	{
	case T_CHAR:
		switch (cWhich)
		{
		case '*':
		case '/':
		case '%':
			ErrorMessage("'%c' : illegal operation on characters");  // nasobit chary? :-P
			break;
		case '+': 
            _helperOnBinaryOperation(+, m_chChar, ADD_CCHAR, ADD_CCHAR, ADD_CHAR);
		case '-': 
            _helperOnBinaryOperation(-, m_chChar, SUB_CCHARL, SUB_CCHARR, SUB_CHAR);
			//  break je uz v makru
		}
		break;

	case T_INT: 
		switch (cWhich)
		{	
		case '*': _helperOnBinaryOperation(*, m_lInt, MUL_CINT, MUL_CINT, MUL_INT);
		case '/': _helperOnBinaryOperationDivide( m_lInt, DIV_CINTL, DIV_CINTR, DIV_INT);
		case '%': _helperOnBinaryOperationModulo( m_lInt, MOD_CINTL, MOD_CINTR, MOD_INT);
		case '+': _helperOnBinaryOperation(+, m_lInt, ADD_CINT, ADD_CINT, ADD_INT);
		case '-': _helperOnBinaryOperation(-, m_lInt, SUB_CINTL, SUB_CINTR, SUB_INT);
			//  break je uz v makru
		}
		break;
	
	case T_FLOAT:
		ASSERT(m_pCurrentFloatTable!=NULL);	
		switch (cWhich)
		{
		case '*': _helperOnBinaryOperationFloat(*, m_fFloat, MUL_CFLT, MUL_CFLT, MUL_FLT);
		case '/': _helperOnBinaryOperationFloatDivide( m_fFloat, DIV_CFLTL, DIV_CFLTR, DIV_FLT);
		case '+': _helperOnBinaryOperationFloat(+, m_fFloat, ADD_CFLT, ADD_CFLT, ADD_FLT);
		case '-': _helperOnBinaryOperationFloat(-, m_fFloat, SUB_CFLTL, SUB_CFLTR, SUB_FLT);
			//  break je uz v makru
		}
		break;

	default:  
		ASSERT(false);
	}

	Res.m_DataType=ResType;
	Res.m_bLValue=false;
}

#undef _helperOnBinaryOperation
#undef _helperOnBinaryOperationFloat
#undef _helperOnBinaryOperationDivide
#undef _helperOnBinaryOperationModulo
#undef _helperOnBinaryOperationFloatDivide


/////////////////////////////////////////////////////////////////////////////////////////
//RelatExpr:    AddExpr	
//			| RelatExpr '<' AddExpr		{ ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '<');	}
//            | RelatExpr '>' AddExpr		{ ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '>');	}
//            | RelatExpr LEQ AddExpr		{ ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, 'L');	}
//            | RelatExpr GEQ AddExpr		{ ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, 'G');	}
//            | RelatExpr INSTANCEOF_TOKEN ReferenceType
//										{ ((CCompilerKrnl*)pCompKrnl)->OnInstanceOf($$, $1, $3);	}
//            ;	
//EqualityExpr:     RelatExpr	
//                | EqualityExpr EQ RelatExpr	  { ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '='); }
//                | EqualityExpr NEQ RelatExpr  { ((CCompilerKrnl*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '!'); }
//                ;

// pomocna makra pro nasledujici metodu
#define _helperOnRelatEqualExpr(op, member, instr, opposite_instr, i1, i2)	\
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	Res.SetConstant(Expr1.m_LexVal.member op Expr2.m_LexVal.member, this); \
		else Res.AddInstructionChangeRegister(opposite_instr##i1, GetReg(), \
				Expr2.m_nReg, Expr1.m_LexVal.member); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(instr##i1, \
				GetReg(), Expr1.m_nReg, Expr2.m_LexVal.member); \
		else Res.AddInstructionChangeRegister(instr##i2, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnRelatEqualExprFloat(op, instr, opposite_instr, i1, i2)	\
	if (Expr1.IsConstant()) \
	{ \
		if (Expr2.IsConstant())	Res.SetConstant(Expr1.m_LexVal.m_fFloat op Expr2.m_LexVal.m_fFloat, this); \
		else Res.AddInstructionChangeRegister(opposite_instr##i1, GetReg(), \
				Expr2.m_nReg, m_pCurrentFloatTable->Add(Expr1.m_LexVal.m_fFloat)); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(instr##i1, \
				GetReg(), Expr1.m_nReg, m_pCurrentFloatTable->Add(Expr2.m_LexVal.m_fFloat)); \
		else Res.AddInstructionChangeRegister(instr##i2, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;

#define _helperOnRelatEqualExprString(op, instr, opposite_instr, i1, i2)	\
	if (Expr1.IsConstant())	\
	{	\
		if (Expr2.IsConstant())	\
			Res.SetConstant(strcmp(Expr1.m_LexVal.m_lpszString,Expr2.m_LexVal.m_lpszString) op 0, this); \
		else Res.AddInstructionChangeRegister(opposite_instr##i1, GetReg(), \
				Expr2.m_nReg, m_pCurrentStringTable->Add(Expr1.m_LexVal.m_lpszString)->m_nID); \
	} \
	else \
	{ \
		if (Expr2.IsConstant()) Res.AddInstructionChangeRegister(instr##i1, \
				GetReg(), Expr1.m_nReg, m_pCurrentStringTable->Add(Expr2.m_LexVal.m_lpszString)->m_nID); \
		else Res.AddInstructionChangeRegister(instr##i2, GetReg(), Expr1.m_nReg, Expr2.m_nReg); \
	} \
	break;



void CCompilerKernel::OnRelatEqualExpr(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich)
{	
	Res.SetError();

	if (Expr1.IsError() || Expr2.IsError()) return;

	if (Expr1.m_DataType.IsError() || Expr2.m_DataType.IsError()) return;

	if ( !(cWhich=='=' || cWhich=='!'))
	{
		if ((!Expr1.m_DataType.IsSimpleType() || Expr1.m_DataType.IsBool() || 
			 !Expr2.m_DataType.IsSimpleType() || Expr2.m_DataType.IsBool() )   &&
			  (!Expr1.m_DataType.IsString() || !Expr2.m_DataType.IsString()))
		{
			ErrorMessage("type mismatch");
			return;
		}
	}

    // porovnavani na null
    if ( (Expr1.m_DataType.IsObjectUnitStruct() && Expr2.IsNull()) ||
         (Expr2.m_DataType.IsObjectUnitStruct() && Expr1.IsNull()))
    {
        Res.SetNewCode();

        if (Expr1.m_DataType.IsObjectUnitStruct() && Expr2.IsNull())  Res = Expr1;
        else Res = Expr2;

        Res.ConvertToRValue();

        if (cWhich == '=') Res.AddInstructionChangeRegister( EQ_CINT, GetReg(), Res.m_nReg, 0); 
        else if (cWhich == '!') Res.AddInstructionChangeRegister( NEQ_CINT, GetReg(), Res.m_nReg, 0);
        else ASSERT(false);

        Res.m_DataType.SetBool();
	    Res.m_bLValue=false;
        return;
    }

	Expr1.ConvertToRValue();
	Expr2.ConvertToRValue();

	// kontrola & konverze typu
	CType ResType;

	if (!FindResType(Expr1.m_DataType, Expr2.m_DataType, ResType))
	{
		ErrorMessage("type mismatch");
		return;
	}

	if (!Expr1.ConvertToType(ResType))
	{
		ErrorMessage("cannot convert '%s' to '%s'", Expr1.m_DataType.GetString(), 
			ResType.GetString());
		return;
	}

	if (!Expr2.ConvertToType(ResType))
	{
		ErrorMessage("cannot convert '%s' to '%s'", Expr2.m_DataType.GetString(), 
			ResType.GetString());
		return;
	}

	Res.SetNewCode();

	// spojeni kodu
	Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);

    // systemovy objekt, jednotka, struktura
    if (ResType.IsObjectUnitStruct())
    {
        if (ResType.IsString())
	    {
		    switch (cWhich)
		    {
		    case '<': _helperOnRelatEqualExprString(<,  LT_, GE_, CSTR, STR);
		    case '>': _helperOnRelatEqualExprString(>,  GT_, LE_, CSTR, STR);
		    case 'L': _helperOnRelatEqualExprString(<=, LE_, GT_, CSTR, STR);
		    case 'G': _helperOnRelatEqualExprString(>=, GE_, LT_, CSTR, STR);
		    case '=': _helperOnRelatEqualExprString(==, EQ_, NEQ_, CSTR, STR);
		    case '!': _helperOnRelatEqualExprString(!=, NEQ_, EQ_, CSTR, STR);
		    default:
			    ASSERT(false);
		    }

		    Res.m_DataType.SetBool();
		    Res.m_bLValue=false;
		    return;
	    }
        else
        {
            ASSERT( !Expr1.IsConstant());
            ASSERT( !Expr2.IsConstant());

            switch (cWhich)
            {
            case '=':
                Res.AddInstructionChangeRegister(EQ_INT, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                break;
            case '!':
                Res.AddInstructionChangeRegister(NEQ_INT, GetReg(), Expr1.m_nReg, Expr2.m_nReg);
                break;
            }

		    Res.m_DataType.SetBool();
		    Res.m_bLValue=false;
		    return;
        }
    }

	// operace
	switch (ResType.m_nType)
	{
	case T_CHAR:
		switch (cWhich)
		{		
		case '<': _helperOnRelatEqualExpr(<,  m_chChar, LT_, GE_, CCHAR, CHAR);
		case '>': _helperOnRelatEqualExpr(>,  m_chChar, GT_, LE_, CCHAR, CHAR);
		case 'L': _helperOnRelatEqualExpr(<=, m_chChar, LE_, GT_, CCHAR, CHAR);
		case 'G': _helperOnRelatEqualExpr(>=, m_chChar, GE_, LT_, CCHAR, CHAR);
		case '=': _helperOnRelatEqualExpr(==, m_chChar, EQ_, NEQ_, CCHAR, CHAR);
		case '!': _helperOnRelatEqualExpr(!=, m_chChar, NEQ_, EQ_, CCHAR, CHAR);
		default:
			ASSERT(false);
		}
		break;

	case T_INT:
		switch (cWhich)
		{
		case '<': _helperOnRelatEqualExpr(<,  m_lInt, LT_, GE_, CINT, INT);
		case '>': _helperOnRelatEqualExpr(>,  m_lInt, GT_, LE_, CINT, INT);
		case 'L': _helperOnRelatEqualExpr(<=, m_lInt, LE_, GT_, CINT, INT);
		case 'G': _helperOnRelatEqualExpr(>=, m_lInt, GE_, LT_, CINT, INT);
		case '=': _helperOnRelatEqualExpr(==, m_lInt, EQ_, NEQ_, CINT, INT);
		case '!': _helperOnRelatEqualExpr(!=, m_lInt, NEQ_, EQ_, CINT, INT);
		default:
			ASSERT(false);
		}
		break;

	case T_FLOAT:
		ASSERT(m_pCurrentFloatTable!=NULL);
		switch (cWhich)
		{
		case '<': _helperOnRelatEqualExprFloat(<,  LT_, GE_, CFLT, FLT);
		case '>': _helperOnRelatEqualExprFloat(>,  GT_, LE_, CFLT, FLT);
		case 'L': _helperOnRelatEqualExprFloat(<=, LE_, GT_, CFLT, FLT);
		case 'G': _helperOnRelatEqualExprFloat(>=, GE_, LT_, CFLT, FLT);
		case '=': _helperOnRelatEqualExprFloat(==, EQ_, NEQ_, CFLT, FLT);
		case '!': _helperOnRelatEqualExprFloat(!=, NEQ_, EQ_, CFLT, FLT);
		default:
			ASSERT(false);
		}
		break;
	case T_BOOL:
		switch (cWhich)
		{
		case '=': _helperOnRelatEqualExpr(==, m_bBool, EQ_, NEQ_, CBOOL, BOOL);
		case '!': _helperOnRelatEqualExpr(!=, m_bBool, NEQ_, EQ_, CBOOL, BOOL);
		default:
			ASSERT(false);
		}
		break;
	
	default:	
		ASSERT(false);
	}

	Res.m_DataType.SetBool();
	Res.m_bLValue=false;
}

#undef _helperOnRelatEqualExpr
#undef _helperOnRelatEqualExprFloat
#undef _helperOnRelatEqualExprString

/////////////////////////////////////////////////////////////////////////////////////////
//RelatExpr:    RelatExpr INSTANCEOF_TOKEN ReferenceType
//										{ ((CCompilerKrnl*)pCompKrnl)->OnInstanceOf($$, $1, $3);	}
//				;
void CCompilerKernel::OnInstanceOf(CPExpr &Res, CPExpr &Expr, CPExpr &Type)
{
	ASSERT(Type.IsType());

	Res.SetError();

	if (Expr.IsError() || Type.m_DataType.IsError()) return;

	ASSERT(Expr.IsCode() || Expr.IsConstant());

	if (!Expr.m_DataType.IsUnit())
	{
		ErrorMessage("left of 'instanceof' operator must be a unit variable");
		return;
	}

	ASSERT(Expr.IsCode());

	if (!Type.m_DataType.IsUnit())
	{
		ErrorMessage("right of 'instanceof' operator must be unit type");
		return;
	}

	Res=Expr;
    Res.Load();
	Res.AddInstructionChangeRegister(INSTANCEOF, GetReg(), Res.m_nReg, 
		m_pCurrentStringTable->Add((CString)*Type.m_DataType.m_stiName)->m_nID);
	Res.m_DataType.SetBool();
}

/////////////////////////////////////////////////////////////////////////////////////////
//AndExpr:      EqualityExpr		
//			  | AndExpr AND EqualityExpr   { ((CCompilerKrnl*)pCompKrnl)->OnAndOrExpr($$, $1, $3, '&'); }
//            ;
//OrExpr:       AndExpr
//            | OrExpr OR AndExpr	{ ((CCompilerKrnl*)pCompKrnl)->OnAndOrExpr($$, $1, $3, '|'); }
//            ;
void CCompilerKernel::OnAndOrExpr(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich)
{
	Res.SetError();

	if (Expr1.IsError() || Expr2.IsError()) return;

	// malokdy byvaji konstanty => nez mit specialni instrukce, tak sup s nima do registru
	if (Expr1.IsConstant()) Expr1.MoveConstantToRegister(GetReg());
	if (Expr2.IsConstant()) Expr2.MoveConstantToRegister(GetReg());

	ASSERT(Expr1.IsCode());
	ASSERT(Expr2.IsCode());

	if (!Expr1.m_DataType.IsBool())
	{
		ErrorMessage("left of '%c%c' must be 'bool' type", cWhich, cWhich);
		return;
	}
	if (!Expr2.m_DataType.IsBool())
	{
		ErrorMessage("right of '%c%c' must be 'bool' type", cWhich, cWhich);
		return;
	}

    Expr1.Load();
    Expr2.Load();
	Res.SetNewCode();
	Res.m_Code.CreateFromConcatenation(Expr1.m_Code, Expr2.m_Code);
	
	if (cWhich=='&') Res.AddInstructionChangeRegister(LOGAND, GetReg(), Expr1.m_nReg, Expr2.m_nReg); 
	else Res.AddInstructionChangeRegister(LOGOR, GetReg(), Expr1.m_nReg, Expr2.m_nReg); 

	Res.m_DataType.SetBool();
}

/////////////////////////////////////////////////////////////////////////////////////////
//IfStmt:   IF '(' Expr ')' Statement		{	((CCompilerKrnl*)pCompKrnl)->OnIfStatement($$, $3, $5); }
//        | IF '(' Expr ')' Statement ELSE Statement
//										{	((CCompilerKrnl*)pCompKrnl)->OnIfStatement($$, $3, $5, $7); }
//        | IF '(' error ')' Statement
//        | IF '(' error ')' Statement ELSE Statement
//        ;
void CCompilerKernel::OnIfStatement(CPExpr &Res, CPExpr &Expr, CPExpr &Stat)
{
	Res.SetError();

	if (Expr.IsError() || Stat.IsError()) return;

	if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg()); // nastava malokdy

	ASSERT(Expr.IsCode());
	ASSERT(Expr.m_DataType.IsBool());
	ASSERT(Stat.IsCode());

  Expr.Load();
  Stat.Load();
	Res=Expr;
	Res.m_Code.AddInstruction(JMPF, Expr.m_nReg, Stat.m_Code.GetLengthInBytes() + SizeOfInstruction(JMPF));
	Res.m_Code.AddCode(Stat.m_Code);

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnIfStatement(CPExpr &Res, CPExpr &Expr, CPExpr &StatThen, CPExpr &StatElse)
{
	Res.SetError();

	if (Expr.IsError() || StatThen.IsError() || StatElse.IsError()) return;

	if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg()); // nastava malokdy

	ASSERT(Expr.IsCode());
	ASSERT(Expr.m_DataType.IsBool());
	ASSERT(StatThen.IsCode());
	ASSERT(StatElse.IsCode());
  
  Expr.Load();
  StatThen.Load();
  StatElse.Load();

  Res=Expr;

	StatThen.m_Code.AddInstruction(JMP, StatElse.m_Code.GetLengthInBytes() + SizeOfInstruction(JMP));
	Res.m_Code.AddInstruction(JMPF, Expr.m_nReg, StatThen.m_Code.GetLengthInBytes() + SizeOfInstruction(JMPF));
	Res.m_Code.AddCode(StatThen.m_Code);
	Res.m_Code.AddCode(StatElse.m_Code);

	Res.m_Code.m_bReturns = (StatThen.m_Code.m_bReturns && StatElse.m_Code.m_bReturns);
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//DoStmt:    DO Statement WHILE '(' Expr ')' ';'	 { ((CCompilerKrnl*)pCompKrnl)->OnDoStatement($$, $2, $5); }
//         | DO Statement WHILE '(' error ')' ';'   
//         ;
void CCompilerKernel::OnDoStatement(CPExpr &Res, CPExpr &Stat, CPExpr &Expr)
{
	Res.SetError();

	if (Stat.IsError() || Expr.IsError()) return;

	if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg());  // nastava malokdy

	ASSERT(Expr.IsCode());
	ASSERT(Expr.m_DataType.IsBool());
	ASSERT(Stat.IsCode());
	
    Expr.Load();
    Stat.Load();
	Res.SetNewCode();
	Res.m_Code.CreateFromConcatenation(Stat.m_Code, Expr.m_Code);
	Res.AddInstructionChangeRegister(JMPT, Expr.m_nReg, -(REG)(Res.m_Code.GetLengthInBytes()));

	// vymena break a continue za skoky
	CPCode::CInstruction *pInstr=NULL;
    long lJumpBreak=Res.m_Code.GetLengthInBytes(),
         lJumpCont=Stat.m_Code.GetLengthInBytes();

	pInstr=Res.m_Code.GetFirstInstruction();
	while (pInstr!=NULL)
	{
		if (pInstr->m_eInstr==BRK)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpBreak;
		}

		if (pInstr->m_eInstr==CONT)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpCont;
		}

        lJumpBreak-=SizeOfInstruction(pInstr->m_eInstr);
        lJumpCont-=SizeOfInstruction(pInstr->m_eInstr);

		pInstr=Res.m_Code.GetNextInstruction(pInstr);
	}

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//WhileStmt:    WHILE '(' Expr ')' Statement       { ((CCompilerKrnl*)pCompKrnl)->OnWhileStatement($$, $3, $5); }
//            | WHILE '(' error ')' Statement  
//            ;
void CCompilerKernel::OnWhileStatement(CPExpr &Res, CPExpr &Expr, CPExpr &Stat)
{
	Res.SetError();

	if (Stat.IsError() || Expr.IsError()) return;

	if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg());  // nastava malokdy

	ASSERT(Expr.IsCode());
	ASSERT(Expr.m_DataType.IsBool());
	ASSERT(Stat.IsCode());

	// sestaveni
    Expr.Load();
    Stat.Load();

	Res=Expr;
	Res.m_Code.AddInstruction(JMPF, Expr.m_nReg, 
        Stat.m_Code.GetLengthInBytes()+SizeOfInstruction(JMP)+SizeOfInstruction(JMPF));
	Res.m_Code.AddCode(Stat.m_Code);
	Res.m_Code.AddInstruction(JMP, -(REG)(Res.m_Code.GetLengthInBytes()));

	// vymena break a continue za skoky
	CPCode::CInstruction *pInstr=NULL;
    long lJumpBreak=Res.m_Code.GetLengthInBytes(),
         lJumpCont=0;
	int i=0;

	pInstr=Res.m_Code.GetFirstInstruction();
	while (pInstr!=NULL)
	{
		if (pInstr->m_eInstr==BRK)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpBreak;
		}

		if (pInstr->m_eInstr==CONT)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpCont;
		}

        lJumpBreak-=SizeOfInstruction(pInstr->m_eInstr);
        lJumpCont-=SizeOfInstruction(pInstr->m_eInstr);

		pInstr=Res.m_Code.GetNextInstruction(pInstr);
		i++;
	}

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//BoolExpr:		Expr   {	((CCompilerKrnl*)pCompKrnl)->OnBoolExpr($$, $1);	 }
//				;
void CCompilerKernel::OnBoolExpr(CPExpr &Res, CPExpr &Expr)
{
	if ((Expr.IsCode() || Expr.IsConstant()) && Expr.m_DataType.IsBool()) Res=Expr;
	else Res.SetError();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ReturnStmt:   RETURN ';'		{ ((CCompilerKrnl*)pCompKrnl)->OnReturnStatementVoid($$);		}
//			| RETURN Expr ';'	{ ((CCompilerKrnl*)pCompKrnl)->OnReturnStatementValue($$, $2);	}
//	 	    | RETURN error ';'	{ ((CCompilerKrnl*)pCompKrnl)->OnReturnStatementError($$);		}
//            ;
void CCompilerKernel::OnReturnStatementVoid(CPExpr &Res)
{
	Res.SetError();

	if (m_pCurrentGlobalFunction!=NULL && !m_pCurrentGlobalFunction->m_ReturnDataType.IsVoid())
	{
		ErrorMessage("function must return a value");
		return;
	}
	if (m_pCurrentMethod!=NULL && !m_pCurrentMethod->m_ReturnDataType.IsVoid())
	{
		ErrorMessage("method must return a value");
		return;
	}

	Res.SetNewCode();
	Res.m_Code.AddInstruction(RET);
	Res.m_Code.m_bReturns = true;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnReturnStatementValue(CPExpr &Res, CPExpr &Expr)
{
	CType RetType;

	Res.SetError();

	if (m_pCurrentEvent!=NULL) 
	{
		ErrorMessage("command/notification returning value");
		return;
	}
	else if (m_pCurrentGlobalFunction!=NULL) 
	{
		RetType=m_pCurrentGlobalFunction->m_ReturnDataType;
		if (RetType.IsVoid())
		{
			ErrorMessage("void function returning value");
			return;
		}
	}
	else if (m_pCurrentMethod!=NULL) 
	{
		RetType=m_pCurrentMethod->m_ReturnDataType;
		if (RetType.IsVoid())
		{
			ErrorMessage("void method returning value");
			return;
		}
	}
	else ASSERT(false);


	if (Expr.IsError() || Expr.IsDeclar()) return;


	Expr.ConvertToRValue();
	
	if (!Expr.ConvertToType(RetType))
	{
		// return null;
		if ( RetType.IsObjectUnitStruct() && Expr.m_DataType.IsInt() && 
			 Expr.IsConstant() && Expr.m_LexVal.m_lInt == 0)
		{
			Expr.SetNewCode();
			Expr.AddInstructionChangeRegister( SETNULL, GetReg());
			Expr.m_DataType = RetType;
		}
		else
		{
			ErrorMessage("can't convert from '%s'to '%s'", Expr.m_DataType.GetString(),
				RetType.GetString());
			return;
		}
	}
	
	ASSERT( Expr.IsCode() || Expr.IsConstant());

	if (Expr.IsConstant())
	{
		Res.SetNewCode();

		switch (Expr.m_DataType.m_nType)
		{
		case T_CHAR:
			Res.m_Code.AddInstruction( RETV_CCHAR, Expr.m_LexVal.m_chChar);
			break;
		case T_INT:
			Res.m_Code.AddInstruction( RETV_CINT, Expr.m_LexVal.m_lInt);
			break;
		case T_FLOAT:
			ASSERT(m_pCurrentFloatTable!=NULL);
			Res.m_Code.AddInstruction( RETV_CINT, 
				m_pCurrentFloatTable->Add(Expr.m_LexVal.m_fFloat));
			break;
		case T_BOOL:
			Res.m_Code.AddInstruction( RETV_CBOOL, Expr.m_LexVal.m_bBool);
			break;
		case T_OBJECT:
			if (Expr.m_DataType.IsString())
			{
				ASSERT(m_pCurrentStringTable!=NULL);
				Res.m_Code.AddInstruction(RETV_CSTR, 
					m_pCurrentStringTable->Add(Expr.m_LexVal.m_lpszString)->m_nID);
			}
			else ASSERT(false);
			break;
		default:
			ASSERT(false);
		}
	}
	else
	{
		Res = Expr;
		Res.m_Code.AddInstruction(RETV, Expr.m_nReg);
	}

	Res.m_Code.m_bReturns = true;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnReturnStatementError(CPExpr &Res)
{
	Res.SetError();
}

/////////////////////////////////////////////////////////////////////////////////////////
//BreakStmt:    BREAK ';'		{ ((CCompilerKrnl*)pCompKrnl)->OnBreakStatement($$);	}
//			  ;
void CCompilerKernel::OnBreakStatement(CPExpr &Res)
{
	Res.SetNewCode();
	Res.m_Code.AddInstruction(BRK);
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ContinueStmt:   CONTINUE ';'	{ ((CCompilerKrnl*)pCompKrnl)->OnContinueStatement($$);	}
//                ;
void CCompilerKernel::OnContinueStatement(CPExpr &Res)
{
	Res.SetNewCode();
	Res.m_Code.AddInstruction(CONT);
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//ExprList:		  Expr
//				| error					{  $$.SetError(); }
//				| ExprList ',' Expr		{  ((CCompilerKrnl*)pCompKrnl)->OnExprList($$, $1, $3);  }
//              | ExprList ',' error	{  $$.SetError(); }
//              ;
void CCompilerKernel::OnExprList(CPExpr &Res, CPExpr &List, CPExpr &Expr)
{
	Res.SetError();

	if (List.IsError() || Expr.IsError()) return;

	ASSERT(List.IsCode());

	if (Expr.IsConstant()) 
	// ignorovat
	{
		Res=List; 
	}
	else
	{
		ASSERT(Expr.IsCode());
		Res.SetNewCode();
		Res.m_Code.CreateFromConcatenation(List.m_Code, Expr.m_Code);
		Res.SetCode(Expr.m_nReg, Expr.m_DataType);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//ForStmt:    FOR '(' ForInit_opt ';' BoolExpr ';' ForUpdate_opt ')' Statement
//				  { ((CCompilerKrnl*)pCompKrnl)->OnForStatement($$, $3, $5, $7, $9);
//
//			| FOR '(' ForInit_opt ';' error ';' ForUpdate_opt ')' Statement
//			;
void CCompilerKernel::OnForStatement(CPExpr &Res, CPExpr &Init, CPExpr &BoolExpr, 
									 CPExpr &Update, CPExpr &Stat)
{
	Res.SetError();

	if (Init.IsError() || BoolExpr.IsError() || Update.IsError() || Stat.IsError())
		return;

	if (BoolExpr.IsConstant()) BoolExpr.MoveConstantToRegister(GetReg()); // nastane malokdy

	ASSERT(Init.IsCode());
	ASSERT(Update.IsCode());
	ASSERT(Stat.IsCode());
	ASSERT(BoolExpr.IsCode());
	ASSERT(BoolExpr.m_DataType.IsBool());

    Init.Load();
    BoolExpr.Load();
    Update.Load();

	Res.SetNewCode();
	Res.m_Code.CreateFromConcatenation(Init.m_Code, BoolExpr.m_Code);
	Res.m_Code.AddInstruction(JMPF, BoolExpr.m_nReg, 
			Stat.m_Code.GetLengthInBytes()+Update.m_Code.GetLengthInBytes()+SizeOfInstruction(JMP)+SizeOfInstruction(JMPF));
	Res.m_Code.AddCode(Stat.m_Code);
	Res.m_Code.AddCode(Update.m_Code);
	Res.m_Code.AddInstruction(JMP,
        -(REG)(Res.m_Code.GetLengthInBytes())+Init.m_Code.GetLengthInBytes());

	// vymena break a continue za skoky
	CPCode::CInstruction *pInstr=NULL;
    long lJumpBreak=Res.m_Code.GetLengthInBytes(), 
        lJumpCont=Res.m_Code.GetLengthInBytes()-Update.m_Code.GetLengthInBytes()-SizeOfInstruction(JMP);

	pInstr=Res.m_Code.GetFirstInstruction();
	while (pInstr!=NULL)
	{
		if (pInstr->m_eInstr==BRK)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpBreak;
		}

		if (pInstr->m_eInstr==CONT)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpCont;
		}

        lJumpBreak-=SizeOfInstruction(pInstr->m_eInstr);
        lJumpCont-=SizeOfInstruction(pInstr->m_eInstr);

		pInstr=Res.m_Code.GetNextInstruction(pInstr);
	}

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//SwitchStmt:     SWITCH '(' Expr ')'   { ((CCompilerKrnl*)pCompKrnl)->OnSwitchBegins($3); }
//				    SwitchBlock	{ ((CCompilerKrnl*)pCompKrnl)->OnSwitchEnds($$); }
//			    | SWITCH '(' error ')' { $3.SetError(); ((CCompilerKrnl*)pCompKrnl)->OnSwitchBegins($3); }
//				    SwitchBlock	{ ((CCompilerKrnl*)pCompKrnl)->OnSwitchEnds($$); }
//				;    
void CCompilerKernel::OnSwitchBegins(CPExpr &Expr)
{
	CSwitchStatement *pSwitchStat;

	if (Expr.IsError() || Expr.m_DataType.IsError())
	{
		pSwitchStat=new CSwitchStatement(m_nLevel);
	}
	else
	{
		if (Expr.IsConstant()) Expr.MoveConstantToRegister(GetReg()); // nastane malokdy
        Expr.Load();

		ASSERT(Expr.IsCode());

		pSwitchStat=new CSwitchStatement(m_nLevel, Expr.m_nReg, Expr.m_DataType, Expr.m_Code);
		if (IsRegister(Expr.m_nReg)) ReserveRegister(Expr.m_nReg);
	}

	m_lSwitches.AddHead(pSwitchStat);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSwitchEnds(CPExpr &Res)
{
	CSwitchStatement *pSwitchStat=NULL;

	Res.SetError();

	do
	{
		if (pSwitchStat!=NULL) 
		{
			m_lSwitches.RemoveHead();
			delete pSwitchStat;
		}
		
		if (m_lSwitches.IsEmpty()) ASSERT(false);

		pSwitchStat=m_lSwitches.GetHead();

	} while (pSwitchStat->m_nLevel!=m_nLevel);

	m_lSwitches.RemoveHead();

	// error -> konec
	if (!pSwitchStat->m_bValid)
	{
		delete pSwitchStat;
		return;
	}

	// sestaveni
	CPCode::CInstruction *pInstr=NULL;
	int i,pSwitchStatCasesCount,oldBlockNo=0;
    long lJumpBlock;

    pSwitchStatCasesCount=pSwitchStat->m_nCasesCount;

    // vypocet delky rozskokovaci casti
	switch (pSwitchStat->m_DataType.m_nType)
	{
	case T_CHAR:
        lJumpBlock=pSwitchStatCasesCount*(SizeOfInstruction(EQ_CCHAR)+SizeOfInstruction(JMPT));
        break;
	case T_INT:
        lJumpBlock=pSwitchStatCasesCount*(SizeOfInstruction(EQ_CINT)+SizeOfInstruction(JMPT));
		break;
	case T_FLOAT:
        lJumpBlock=pSwitchStatCasesCount*(SizeOfInstruction(EQ_CFLT)+SizeOfInstruction(JMPT));
		break;
	case T_BOOL:
        lJumpBlock=pSwitchStatCasesCount*(SizeOfInstruction(EQ_CBOOL)+SizeOfInstruction(JMPT));
		break;
	case T_OBJECT:
		if (!pSwitchStat->m_DataType.IsString()) ASSERT(false);
        lJumpBlock=pSwitchStatCasesCount*(SizeOfInstruction(EQ_CSTR)+SizeOfInstruction(JMPT));
		break;
    default: ASSERT(false);
	}

    lJumpBlock+=SizeOfInstruction(JMP);

	Res.SetNewCode();	
	Res.m_Code=pSwitchStat->m_ExprCode;

    // sestaveni rozskokovaci casti
	for (i=0; i<pSwitchStat->m_nCasesCount; i++)
	{
		switch (pSwitchStat->m_DataType.m_nType)
		{
		case T_CHAR:
			Res.AddInstructionChangeRegister(EQ_CCHAR, GetReg(), 
				pSwitchStat->m_nExprReg, pSwitchStat->m_aCases[i].m_LexVal.m_chChar);
    		lJumpBlock-=SizeOfInstruction(EQ_CCHAR);			
            break;
		case T_INT:
			Res.AddInstructionChangeRegister(EQ_CINT, GetReg(), 
				pSwitchStat->m_nExprReg, pSwitchStat->m_aCases[i].m_LexVal.m_lInt);
            lJumpBlock-=SizeOfInstruction(EQ_CINT);
			break;
		case T_FLOAT:
			ASSERT(m_pCurrentFloatTable!=NULL);
			
			Res.AddInstructionChangeRegister(EQ_CFLT, GetReg(), 
				pSwitchStat->m_nExprReg, m_pCurrentFloatTable->Add(pSwitchStat->m_aCases[i].m_LexVal.m_fFloat));
            lJumpBlock-=SizeOfInstruction(EQ_CFLT);
			break;
		case T_BOOL:
			Res.AddInstructionChangeRegister(EQ_CBOOL, GetReg(), 
				pSwitchStat->m_nExprReg, pSwitchStat->m_aCases[i].m_LexVal.m_bBool);
            lJumpBlock-=SizeOfInstruction(EQ_CBOOL);
			break;
		case T_OBJECT:
			if (!pSwitchStat->m_DataType.IsString()) ASSERT(false);

			ASSERT(m_pCurrentStringTable!=NULL);
			Res.AddInstructionChangeRegister(EQ_CSTR, GetReg(), 
				pSwitchStat->m_nExprReg, m_pCurrentStringTable->Add(pSwitchStat->m_aCases[i].m_LexVal.m_lpszString)->m_nID);
            lJumpBlock-=SizeOfInstruction(EQ_CSTR);
			break;
        default: ASSERT(false);
		}

        if (oldBlockNo!=pSwitchStat->m_aCases[i].m_nBlockNo)
		{
			lJumpBlock+=pSwitchStat->m_aBlocks[oldBlockNo].GetLengthInBytes();
			oldBlockNo=pSwitchStat->m_aCases[i].m_nBlockNo;
		}

		Res.AddInstructionChangeRegister(JMPT, Res.m_nReg, lJumpBlock);
        lJumpBlock-=SizeOfInstruction(JMPT);

        if (i==pSwitchStatCasesCount-1 && pSwitchStat->m_bHasDefault)
        {
            lJumpBlock+=pSwitchStat->m_aBlocks[oldBlockNo].GetLengthInBytes();
        }
	}

    if (pSwitchStat->m_bHasDefault) 
    {
		Res.AddInstructionChangeRegister(JMP, lJumpBlock);
        lJumpBlock-=SizeOfInstruction(JMP);
    }
	else Res.AddInstructionChangeRegister(BRK);

	// bloky
	bool bAllReturns = true;
	for (i=0; i<pSwitchStat->m_nBlockCount; i++)
	{
		Res.m_Code.AddCode(pSwitchStat->m_aBlocks[i]);
		if ( !pSwitchStat->m_aBlocks[i].m_bReturns) bAllReturns = false;
	}

	// nahrazeni breaku
    long lJumpBreak=Res.m_Code.GetLengthInBytes();
	pInstr=Res.m_Code.GetFirstInstruction();
	while (pInstr!=NULL)
	{
		if (pInstr->m_eInstr==BRK)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpBreak;
		}
        lJumpBreak-=SizeOfInstruction(pInstr->m_eInstr);
		pInstr=Res.m_Code.GetNextInstruction(pInstr);
	}

	Res.m_Code.m_bReturns = bAllReturns;
	Res.m_DataType.SetVoid();

	if (IsRegister(pSwitchStat->m_nExprReg)) EndRegisterReservation(pSwitchStat->m_nExprReg);

	delete pSwitchStat;
}

/////////////////////////////////////////////////////////////////////////////////////////
//SwitchBlockStmtGroup:   SwitchLabelS BlockStmtS		{ ((CCompilerKrnl*)pCompKrnl)->OnSwitchBlockStmtS($2); }
//                        ;
void CCompilerKernel::OnSwitchBlockStmtS(CPExpr &BlockStmtS)
{
	CSwitchStatement *pSwitchStat=NULL;

	ASSERT(!m_lSwitches.IsEmpty());

	pSwitchStat=m_lSwitches.GetHead();
	ASSERT(pSwitchStat!=NULL);
	ASSERT(pSwitchStat->m_nLevel==m_nLevel-1);

	if (!BlockStmtS.IsCode()) pSwitchStat->m_bValid=false;

	if (!pSwitchStat->m_bValid) return;

	pSwitchStat->AddBlock(BlockStmtS.m_Code);
}


/////////////////////////////////////////////////////////////////////////////////////////
//SwitchLabel:      CASE ConstantExpr ':'   { ((CCompilerKrnl*)pCompKrnl)->OnSwitchCase($2);  }
//                | DEFAULT ':'			  { ((CCompilerKrnl*)pCompKrnl)->OnSwitchDefault(); }
//                ;
void CCompilerKernel::OnSwitchCase(CPExpr &Expr)
{
	CSwitchStatement *pSwitchStat=NULL;

	ASSERT(!m_lSwitches.IsEmpty());

	pSwitchStat=m_lSwitches.GetHead();
	ASSERT(pSwitchStat!=NULL);
	ASSERT(pSwitchStat->m_nLevel==m_nLevel-1);

	if (!pSwitchStat->m_bValid) return;

	if (!Expr.IsConstant()) pSwitchStat->m_bValid=false;
		
	if (!(Expr.m_DataType==pSwitchStat->m_DataType))
	{
		ErrorMessage("cannot convert from '%s' to '%s'", Expr.m_DataType.GetString(), 
			pSwitchStat->m_DataType.GetString());
		pSwitchStat->m_bValid=false;
	}

	if (pSwitchStat->m_bHasDefault)
	{
		ErrorMessage("no 'case' is allowed after the 'default:' statement");
		pSwitchStat->m_bValid=false;
	}

	if (!pSwitchStat->m_bValid) return;

	pSwitchStat->AddCase(Expr.m_LexVal);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSwitchDefault()
{
	CSwitchStatement *pSwitchStat=NULL;

	ASSERT(!m_lSwitches.IsEmpty());

	pSwitchStat=m_lSwitches.GetHead();
	ASSERT(pSwitchStat!=NULL);
	ASSERT(pSwitchStat->m_nLevel==m_nLevel-1);

	if (!pSwitchStat->m_bValid) return;

	pSwitchStat->m_bHasDefault=true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//SentToStmt:   SENDTO			{  ((CCompilerKrnl*)pCompKrnl)->OnSendToBegins(); }
//			  '(' PrimaryExprList ')' SendToOptions  
//			  '{' SendToEventList '}'
//								{  ((CCompilerKrnl*)pCompKrnl)->OnSendToEnds($$, $4, $6, $8); }
//              ;
void CCompilerKernel::OnSendToBegins()
{
	m_bSendToStatement=true;

}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSendToEnds(CPExpr &Res, CPExpr &PrimaryExprList, 
								   CPExpr &Options, CPExpr &EventList)
{
	m_bSendToStatement=false;

	Res.SetError();

	if (PrimaryExprList.IsError() || EventList.IsError()) return;

	ASSERT(PrimaryExprList.IsCode());
	ASSERT(EventList.IsCode());
	ASSERT(Options.IsConstant() && Options.m_LexVal.IsInt());
	ASSERT(Options.m_LexVal.m_lInt==WAIT || Options.m_LexVal.m_lInt==NOTIFICATION ||
		   Options.m_LexVal.m_lInt==WAITFIRST || Options.m_LexVal.m_lInt==0);

    PrimaryExprList.Load();

	Res.SetNewCode();
	Res.m_Code.AddInstruction(SENDTO_BEGIN);
	Res.m_Code.AddCode(PrimaryExprList.m_Code);

	switch (Options.m_LexVal.m_lInt)
	{
	case WAIT:
		Res.m_Code.AddInstruction(SENDTO_WAIT);
		break;
	case WAITFIRST:
		Res.m_Code.AddInstruction(SENDTO_WAIT1ST);
		break;
	case NOTIFICATION:
		Res.m_Code.AddInstruction(SENDTO_NOTIFICATION);
		break;
	}

	Res.m_Code.AddCode(EventList.m_Code);

	Res.m_Code.AddInstruction(SENDTO_END);

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////
//PrimaryExprList:  PrimaryExpr	{  ((CCompilerKrnl*)pCompKrnl)->OnSendToPrimaryExpr($$, $1); }
//                | PrimaryExprList ',' PrimaryExpr 
//								{  ((CCompilerKrnl*)pCompKrnl)->OnSendToPrimaryExprList($$, $1, $3); }
//                ;
void CCompilerKernel::OnSendToPrimaryExpr(CPExpr &Res, CPExpr &Expr)
{
	Res.SetError();

	if (Expr.IsError()) return;

	if (!Expr.m_DataType.IsUnit())
	{
// Vitek        ASSERT( Expr.m_DataType.m_pOfType != NULL);
        if ((Expr.m_DataType.m_pOfType == NULL) || !Expr.m_DataType.IsSet() || !Expr.m_DataType.m_pOfType->IsUnit())
        {
    		ErrorMessage("sendto statement is applicable only to units or to the sets of units");
	    	return;
        }
	}

	ASSERT(Expr.IsCode());
	if (Expr.m_bLValue) Expr.Load();  // ->LVALUE_PRIMARY

	Res.SetNewCode();
	Res=Expr;
	Res.m_Code.AddInstruction(SENDTO_ADDITEM, Expr.m_nReg);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSendToPrimaryExprList(CPExpr &Res, CPExpr &List, CPExpr &Expr)
{
	CPExpr Pom;

	Res.SetError();

	if (List.IsError()) return;

	OnSendToPrimaryExpr(Pom, Expr);

    List.Load();
    Expr.Load();

	if (Pom.IsError()) return;

	Res.SetNewCode();
	Res.m_Code.CreateFromConcatenation(List.m_Code, Pom.m_Code);
	Res.SetCode(Expr.m_nReg, Expr.m_DataType, true);
}

/////////////////////////////////////////////////////////////////////////////////////////
//SendToEvent:    IDENTIFIER '(' OnSendToEventBegins ArgumentList_opt ')'  
//					{ ((CCompilerKrnl*)pCompKrnl)->OnSendToEventEnds($$, $1, $3, $4); }
//              | IDENTIFIER '(' OnSendToEventBegins error ')'
//					{ ((CCompilerKrnl*)pCompKrnl)->OnSendToEventEndsError($$); }
//              ;    
void CCompilerKernel::OnSendToEventBegins(CPExpr &Res, CPExpr &Ident)
{
	CPNamespace::CEventHeader* pEH;
	CArgInfo *pArgInfo;
	
	pArgInfo=new CArgInfo();
	m_lArgInfoList.AddHead(pArgInfo);

	ASSERT(Ident.IsDeclar());

	if ((pEH=m_Namespace.GetEventHeader(Ident.m_strIdentifier))!=NULL)
	{
		pArgInfo->Set(&pEH->m_aArgs, pEH->m_aArgs.GetSize());
	}

	Res.SetNewCode();
	Res.m_Code.AddInstruction(CRTBAG);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSendToEventEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Begin, CPExpr &Args)
{
	// kontrola zadani vsech argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	if (pArgInfo->m_bValid && pArgInfo->GetNext(Type))
	{
		ErrorMessage("command/notification %s : takes %d argument(s)", Ident.m_strIdentifier, pArgInfo->m_nArgCount);
	}
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;

	ASSERT(m_pCurrentStringTable!=NULL);

	Res=Begin;
	Res.m_Code.AddCode(Args.m_Code);
	Res.m_Code.AddInstruction(SENDTO_SEND, m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
	Res.m_Code.AddInstruction(DELBAG);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnSendToEventEndsError(CPExpr &Res)
{
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
//SendToEventList:    SendToEvent ';'	
//					| SendToEventList SendToEvent ';'	{ ((CCompilerKrnl*)pCompKrnl)->OnSendToEventList($$, $1, $2); }
//                  ;
void CCompilerKernel::OnSendToEventList(CPExpr &Res, CPExpr &List, CPExpr &SentToEvent)
{
	Res.SetError();

	if (List.IsError()) return;
	if (SentToEvent.IsError()) return;

	ASSERT(List.IsCode());
	ASSERT(SentToEvent.IsCode());

	Res.SetNewCode();
	Res.m_Code.CreateFromConcatenation(List.m_Code, SentToEvent.m_Code);
}

/////////////////////////////////////////////////////////////////////////////////////////
//EventCallStmt:    PrimaryExpr BACKARROW IDENTIFIER OnEventCallBegins '(' ArgumentList_opt ')' ';' 
//					{ ((CCompilerKrnl*)pCompKrnl)->OnEventCallEnds($$, $1, $3, $4, $6); }				  
//               | PrimaryExpr BACKARROW IDENTIFIER OnEventCallBegins '(' error ')' ';' 
//					{ ((CCompilerKrnl*)pCompKrnl)->OnEventCallEndsError($$); }
//                ;
void CCompilerKernel::OnEventCallBegins(CPExpr &Res, CPExpr &Primary, CPExpr &Ident)
{
	CPNamespace::CEventHeader* pEH;
	CArgInfo *pArgInfo;
	
	pArgInfo=new CArgInfo();
	m_lArgInfoList.AddHead(pArgInfo);

	ASSERT(Ident.IsDeclar());
	Res.SetError();

	if (Primary.IsError()) return;

	ASSERT(Primary.IsCode() || Primary.IsConstant());
	
	if (!Primary.m_DataType.IsUnit())
	{
		ErrorMessage("left of '<-' must be a unit");
		return;
	}

	if ((pEH=m_Namespace.GetEventHeader(Ident.m_strIdentifier))!=NULL)
	{
		pArgInfo->Set(&pEH->m_aArgs, pEH->m_aArgs.GetSize());
	}

	Res.SetNewCode();
	Res.m_Code.AddInstruction(CRTBAG);

    m_bSendToStatement = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnEventCallEnds(CPExpr &Res, CPExpr &Primary, CPExpr &Ident, CPExpr &Begin, CPExpr &Args)
{
	Res.SetError();

    m_bSendToStatement = false;

	// kontrola zadani vsech argumentu
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	CType Type;
	if (pArgInfo->m_bValid && pArgInfo->GetNext(Type))
	{
		ErrorMessage("command/notification %s : takes %d argument(s)", Ident.m_strIdentifier, pArgInfo->m_nArgCount);
	}
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;

	if (Primary.IsError() || Begin.IsError() || Args.IsError()) return;

	ASSERT(Primary.IsCode());
	ASSERT(Primary.m_DataType.IsUnit());
	ASSERT(Args.IsCode());

	ASSERT(m_pCurrentStringTable!=NULL);

	if (Primary.m_bLValue) Primary.Load();

	Res=Begin;
	Res.m_Code.AddCode(Primary.m_Code);
    Res.m_Code.AddCode(Args.m_Code);    
	Res.m_Code.AddInstruction(SEND, Primary.m_nReg, 
		m_pCurrentStringTable->Add(Ident.m_strIdentifier)->m_nID);
	Res.m_Code.AddInstruction(DELBAG);

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCompilerKernel::OnEventCallEndsError(CPExpr &Res)
{
	CArgInfo *pArgInfo=m_lArgInfoList.GetHead();
	m_lArgInfoList.RemoveHead();
	delete pArgInfo;
}

void CCompilerKernel::OnSetVarDecl(CPExpr &Res, CPExpr &Ident)
{
    Res.SetError();
	ASSERT(Ident.IsDeclar());

	CPNamespace::CVariable *pVar;

	Res=Ident;
    Res.m_DataType=*m_pCurrentType;
    Res.m_DataType.MakeSet();

	if (!AddLocalVariable(Res, &pVar))
	{
		Res.SetError();
	}
	else
	{
		ASSERT( pVar != NULL);

		Res.SetNewCode();
		Res.m_nReg = pVar->m_nReg;
        Res.m_DataType = pVar->m_DataType;
        Res.m_bLValue = true;
	}
}

void CCompilerKernel::OnIn(CPExpr &Res, CPExpr &Var, CPExpr &Set)
{
    Res.SetError();

    if (Var.IsError() || Set.IsError()) return;

    if (Var.IsConstant()) Var.MoveConstantToRegister(GetReg());

    ASSERT(Var.IsCode());
    ASSERT(Set.IsCode());
    
    if (!Set.m_DataType.IsSet())
    {
        ErrorMessage("right of 'in' must be a set", Set.m_strIdentifier);
        return;
    }

    if (!Var.ConvertToType(*Set.m_DataType.m_pOfType))
    {
        ErrorMessage("cannot convert from '%s' to '%s'", Var.m_DataType.GetString(), 
            Set.m_DataType.m_pOfType->GetString());
        return;
    }

    Var.Load();
	Set.Load();

    Res.SetNewCode();
    Res.m_Code.CreateFromConcatenation( Var.m_Code, Set.m_Code);
    Res.AddInstructionChangeRegister(SET_ELEM, GetReg(), Var.m_nReg, Set.m_nReg);
    Res.m_DataType.SetBool();
	Res.m_bLValue=false;
}

void CCompilerKernel::OnReferenceType(CPExpr &Res, CPExpr &Ident)
{
    ASSERT(Ident.IsDeclar());

	Res.SetType(this);
	Res.m_DataType.SetError();

	Res.m_DataType.Set(Ident.m_strIdentifier, 0, &m_Namespace, &m_Preview); 

	if (Res.m_DataType.IsError()) 
	{
		// jeste to muze byt unit nebo struktura, ktera je jen v preview
		
		// unit
		if (m_Preview.GetUnit(  Ident.m_strIdentifier) != NULL) 
		{
			Res.m_DataType.SetUnit( g_StringTable.AddItem( Ident.m_strIdentifier, false));
		}
		// struct
		else if (m_Preview.GetStruct( Ident.m_strIdentifier) != NULL) 
		{
			Res.m_DataType.SetStruct( g_StringTable.AddItem( Ident.m_strIdentifier, false));
		}
		// none -> error
		else
		{
			ErrorMessage("undefined type '%s'", Ident.m_strIdentifier);
		}
	}

	if (Res.m_DataType.IsObject())
	{
		CPNamespace::CSysObject *pObject;
		if (( pObject = m_Namespace.GetObject(Ident.m_strIdentifier))!=NULL && !pObject->m_bCanCreateInstance)
		{
			ErrorMessage("creating instances of '%s' is not allowed", Ident.m_strIdentifier);
			Res.m_DataType.SetError();
			return;
		}
	}
}

void CCompilerKernel::OnSetForBegin(CPExpr &Set)
{
    if (Set.IsCode())
    {
        Set.Load();
        if (IsRegister(Set.m_nReg)) ReserveRegister(Set.m_nReg);
    }
}

void CCompilerKernel::OnSetForEnd(CPExpr &Res, CPExpr &Var, CPExpr &Set, CPExpr &Block)
{
    Res.SetError();

    if (Var.IsError() || Set.IsError()) return;

    if (Var.IsConstant() || (Var.IsCode() && Var.m_bLValue && Var.m_nLValueType!=CPExpr::LVALUE_PRIMARY)) 
    {
        ErrorMessage("left of 'in' must be a variable");
        return;
    }

    ASSERT(Var.IsCode());
    ASSERT(Set.IsCode());

    REG nReservedRegister = -1;
	if (IsRegister(Set.m_nReg)) nReservedRegister = Set.m_nReg;
    
	/*
    if (!Set.m_bLValue)
    {
        ErrorMessage("right of 'in' must be a l-value");
        return;
    }
	*/

	Set.Load();

    if (!Set.m_DataType.IsSet())
    {
        ErrorMessage("right of 'in' must be a set", Set.m_strIdentifier);
        return;
    }

    if (!(Var.m_DataType==*Set.m_DataType.m_pOfType))
    {
        ErrorMessage("left of 'in' must be of type '%s'", Set.m_DataType.m_pOfType->GetString());
        return;
    }

    long lJump=Block.m_Code.GetLengthInBytes()+
        SizeOfInstruction(SETFOR_NEXT)+SizeOfInstruction(JMPT);

	// poskladani dohromady
    Res=Set;
    Res.AddInstructionChangeRegister(SETFOR_FIRST, GetReg(), Var.m_nReg, Set.m_nReg);
    Res.m_Code.AddInstruction(JMPF, Res.m_nReg, lJump + SizeOfInstruction(JMPF)); 
    Res.m_Code.AddCode(Block.m_Code);
    Res.m_Code.AddInstruction(SETFOR_NEXT, Res.m_nReg, Var.m_nReg, Set.m_nReg);
    Res.m_Code.AddInstruction(JMPT, Res.m_nReg, -lJump+SizeOfInstruction(JMPT));
    Res.m_Code.AddInstruction(SETFOR_END, Set.m_nReg);

	// vymena break a continue za skoky
	CPCode::CInstruction *pInstr=NULL;
    long lJumpBreak=Res.m_Code.GetLengthInBytes() - SizeOfInstruction(SETFOR_END), 
         lJumpCont=Res.m_Code.GetLengthInBytes() - SizeOfInstruction(SETFOR_END) - SizeOfInstruction(JMPT)  
												 - SizeOfInstruction(SETFOR_NEXT);

	pInstr=Res.m_Code.GetFirstInstruction();
	while (pInstr!=NULL)
	{
		if (pInstr->m_eInstr==BRK)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpBreak;
		}

		if (pInstr->m_eInstr==CONT)
		{
			pInstr->m_eInstr=JMP;
			pInstr->m_nOp[0]=lJumpCont;
		}

        lJumpBreak-=SizeOfInstruction(pInstr->m_eInstr);
        lJumpCont-=SizeOfInstruction(pInstr->m_eInstr);

		pInstr=Res.m_Code.GetNextInstruction(pInstr);
	}

	Res.m_Code.m_bReturns = false;
	Res.m_DataType.SetVoid();

	if (nReservedRegister != -1) EndRegisterReservation( nReservedRegister);
}


bool CCompilerKernel::OnStructBegin(CPExpr &Ident)
{
    ASSERT(Ident.IsDeclar());

	// obnoveni generovani kodu na zacatku dalsi jednotky nebo funkce
	m_bGenerCode=true;

	m_pCurrentMEG=NULL;
	m_pCurrentType=NULL;
    m_nFormalParamNo=0;

	switch (m_Namespace.GetScriptType(Ident.m_strIdentifier))
    {
    case SCRIPT_UNIT: 
        ErrorMessage("name '%s' already used for a unit", Ident.m_strIdentifier);
        return false;
    case SCRIPT_OBJECT:
		ErrorMessage("name '%s' already used for a system object", Ident.m_strIdentifier);
        return false;
    case SCRIPT_STRUCTURE:
//        ErrorMessage("structure of that name already exists");
//        return false;
        break;
    case SCRIPT_FUNCTION:
        ErrorMessage("name '%s' already used for a global function", Ident.m_strIdentifier);
        return false;
    case SCRIPT_SYSFUNCTION:
        ErrorMessage("name '%s' already used for a system global function", Ident.m_strIdentifier);
        return false;
    }

    // kontrola delky jmena
    if (Ident.m_strIdentifier.GetLength()>=MAX_IDENTIFIER_LENGTH)
    {
        ErrorMessage("identifier too long (the maximum is %d character)", MAX_IDENTIFIER_LENGTH);
        return false;
    }

	m_pCurrentStruct=new CPCurrentStruct(Ident.m_strIdentifier);
	m_pCurrentStringTable=&m_pCurrentStruct->m_StringTable;
	m_pCurrentFloatTable=NULL;

    return true;
}

void CCompilerKernel::OnStructEnd()
{
    bool bDeleteStruct = false;

// pridani ke zkompilovanym strukturam
	if (m_bGenerCode && m_bUpdateCodeOnSuccess)
	{
        m_pCurrentStruct->m_pNext = m_pCompiledStructs;
        m_pCompiledStructs = m_pCurrentStruct;
	}
    else bDeleteStruct = true;

	// pridani do namespace
    //!!! naloadeni z code managera
	if (m_bGenerCode)
	{
		CPNamespace::CStruct *pStruct;
		CType Type;

		pStruct=m_Namespace.AddStruct(m_pCurrentStruct->m_strName);

		int i;
		for (i=0; i<m_pCurrentStruct->m_nMemberCount; i++) 
		{
			Type=m_pCurrentStruct->m_aMemberTypes[i];
			pStruct->AddMember((CString)*m_pCurrentStruct->m_aMemberNames[i], Type);
		}
	}

    // DEBUG VYPIS
	if (m_bDebugOutput)	m_pCurrentStruct->DebugWrite(m_pDbgOut);

// uvolneni a vynulovani
    if (bDeleteStruct) delete m_pCurrentStruct;
    m_pCurrentStruct=NULL;
    m_pCurrentStringTable=NULL;
}

void CCompilerKernel::OnDupExpr(CPExpr &Res, CPExpr &Ob)
{
    CPNamespace::CSysObject *pObj;

    Res.SetError();

    if (Ob.IsError()) return;

    ASSERT(Ob.IsCode());

    if (Ob.m_DataType.IsSimpleType() || Ob.m_DataType.IsUnit())
    {
        ErrorMessage("right of 'dup' must be an object or structure");
        return;
    }

	if ( Ob.m_DataType.IsObject())
	{
		if ((pObj=m_Namespace.GetObject(Ob.m_DataType.m_nSOType))!=NULL &&
			!pObj->m_bCanCreateInstance)
		{
			ErrorMessage("can't duplicate the object");
			return;
		}
	}
	else 
	{
		ASSERT( Ob.m_DataType.IsStruct() || Ob.m_DataType.IsError());
	}

    Res=Ob;
    Res.Load();
    Res.AddInstructionChangeRegister(DUPL, GetReg(), Res.m_nReg);
}

void CCompilerKernel::OnThis(CPExpr &Res)
{
    Res.SetError();

    if (m_pCurrentUnit == NULL)
    {
        ErrorMessage("no current unit");
        return;
    }

    Res.SetNewCode();
    Res.m_nReg = 0;
    Res.m_DataType.SetUnit(g_StringTable.AddItem(m_pCurrentUnit->m_strName, false));
}