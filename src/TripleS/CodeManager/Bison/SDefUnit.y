%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Parser pro nacteni interface defaultni jednotky.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CodeManager.h"
#include "LexDefaultUnit.h"

#include <malloc.h>
#define alloca  _alloca

#define YYSTYPE SLDefaultUnitTokenData

#define YYDEBUG             1
#define YYERROR_VERBOSE

#define SDefUnitlex CCodeManager::m_pLexDefaultUnit->YyLex
#define SDefUniterror CCodeManager::m_pLexDefaultUnit->YyError

extern void *pCompKrnl;

%}

%pure_parser 
%expect 0  // expected collisions

/* Tokens */

%token LDF_UNIT
%token LDF_IDENTIFIER
%token LDF_OBJECT
%token LDF_STRUCT

%%

/************************************************************************************/

/* starting nonterminal */

Input:    DefaultUnit
		  ;

DefaultUnit:    LDF_UNIT LDF_IDENTIFIER
                    '{'  { CCodeManager::OnDefaultUnitBegin($2.m_strString); }
                    MembersAndMethodS '}' { CCodeManager::OnDefaultUnitEnd(); }
                ;


MembersAndMethodS:    MembersOrMethod
                    | MembersAndMethodS MembersOrMethod
                               ;

MembersOrMethod:   // simple type
                   LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER  ';'
                        {	CCodeManager::OnDefaultUnitMember(SCRIPT_UNKNOWN, $1.m_strString, $3.m_strString, $2.m_nNumber); }
                 | // structure
                   LDF_STRUCT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER ';'
                        {	CCodeManager::OnDefaultUnitMember(SCRIPT_STRUCTURE, $2.m_strString, $4.m_strString, $3.m_nNumber); }

                 | // object member
                   LDF_OBJECT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER ';'
                        {	CCodeManager::OnDefaultUnitMember(SCRIPT_OBJECT, $2.m_strString, $4.m_strString, $3.m_nNumber); }

                 | // unit member
                   LDF_UNIT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER ';'
                        {	CCodeManager::OnDefaultUnitMember(SCRIPT_UNIT, $2.m_strString, $4.m_strString, $3.m_nNumber); }

                 | // metoda - simple return type
                   LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER '('	{	CCodeManager::OnDefaultUnitMethodBegin(SCRIPT_UNKNOWN, $1.m_strString, $2.m_nNumber, $3.m_strString);	}
					               ArgList_opt ')' ';'	{	CCodeManager::OnDefaultUnitMethodEnd();	}

                 | // metoda - structure return type
                   LDF_STRUCT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER '('	{	CCodeManager::OnDefaultUnitMethodBegin(SCRIPT_STRUCTURE, $2.m_strString, $3.m_nNumber, $4.m_strString);	}
					               ArgList_opt ')' ';'	{	CCodeManager::OnDefaultUnitMethodEnd();	}

                 | // metoda - object return type
                   LDF_OBJECT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER '('	{	CCodeManager::OnDefaultUnitMethodBegin(SCRIPT_OBJECT, $2.m_strString, $3.m_nNumber, $4.m_strString);	}
					               ArgList_opt ')' ';'	{	CCodeManager::OnDefaultUnitMethodEnd();	}

                 | // metoda - unit return type
                   LDF_UNIT LDF_IDENTIFIER ArrayOrSetDecl_opt LDF_IDENTIFIER '('	{	CCodeManager::OnDefaultUnitMethodBegin(SCRIPT_UNIT, $2.m_strString, $3.m_nNumber, $4.m_strString);	}
					               ArgList_opt ')' ';'	{	CCodeManager::OnDefaultUnitMethodEnd();	}
                 ;

ArrayOrSetDecl_opt:	  /* empty */	{ $$.m_nNumber=0;	}
		    		| ArrayOrSetDecl
			    	;

ArrayOrSetDecl:     ArrayDecl
                  | SetDecl
                  ;

ArrayDecl:     '[' ']'				{	$$.m_nNumber=1;	}
             | ArrayDecl '[' ']'	{	$$.m_nNumber=$1.m_nNumber+1;	}
             ;

SetDecl:    '{' '}'               {   $$.m_nNumber=-1; }
			 ;

ArgList_opt:	  /* empty */
				| ArgList
				;

ArgList:	  Arg			
			| ArgList ',' Arg
			; 

Arg:	  LDF_IDENTIFIER  LDF_IDENTIFIER ArrayOrSetDecl_opt { CCodeManager::OnDefaultUnitArgument(SCRIPT_UNKNOWN, $1.m_strString, $3.m_nNumber);  }
		| LDF_IDENTIFIER ArrayOrSetDecl_opt   { CCodeManager::OnDefaultUnitArgument(SCRIPT_UNKNOWN, $1.m_strString, $2.m_nNumber);  }

        | LDF_STRUCT LDF_IDENTIFIER  LDF_IDENTIFIER ArrayOrSetDecl_opt { CCodeManager::OnDefaultUnitArgument(SCRIPT_STRUCTURE, $2.m_strString, $4.m_nNumber);  }
		| LDF_STRUCT LDF_IDENTIFIER ArrayOrSetDecl_opt   { CCodeManager::OnDefaultUnitArgument(SCRIPT_STRUCTURE, $2.m_strString, $3.m_nNumber);  }

        | LDF_OBJECT LDF_IDENTIFIER  LDF_IDENTIFIER ArrayOrSetDecl_opt { CCodeManager::OnDefaultUnitArgument(SCRIPT_OBJECT, $2.m_strString, $4.m_nNumber);  }
		| LDF_OBJECT LDF_IDENTIFIER ArrayOrSetDecl_opt   { CCodeManager::OnDefaultUnitArgument(SCRIPT_OBJECT, $2.m_strString, $3.m_nNumber);  }
		  
        | LDF_UNIT LDF_IDENTIFIER  LDF_IDENTIFIER ArrayOrSetDecl_opt { CCodeManager::OnDefaultUnitArgument(SCRIPT_UNIT, $2.m_strString, $4.m_nNumber);  }
		| LDF_UNIT LDF_IDENTIFIER ArrayOrSetDecl_opt   { CCodeManager::OnDefaultUnitArgument(SCRIPT_UNIT, $2.m_strString, $3.m_nNumber);  }
		;

