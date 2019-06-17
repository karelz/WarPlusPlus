%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Parser pro nacteni namespace z konfig.souboru
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLexNsp.h"

void nsperror(char*);

#include <malloc.h>
#define alloca  _alloca

#define YYSTYPE SNspTokenData
#define YYPARSE_PARAM pNamespace

#define YYDEBUG             1
#define YYERROR_VERBOSE

#define nsplex   ((CPNamespace*)pNamespace)->m_pLexNsp->YyLex
#define nsperror ((CPNamespace*)pNamespace)->ParserError

// Kvuli hlaskam pro druhy pruchod prekladace,
// u namespace se nikdy se nepouzije 
// (v souboru djgpp\lib\bison.sim je to natvrdo prepsane, 
// aby debug hlasky sly do debug okna).
void *pCompKrnl;

%}

%pure_parser 
%expect 1  // expected collisions

/* Tokens */

%token NSP_IDENTIFIER

/* Keywords */

%token NSP_OBJECT
%token NSP_EVENT
%token NSP_FUNCTIONS
%token NSP_NUMBER
%token NSP_NOINSTANCES

%%

/************************************************************************************/

/* starting nonterminal */

Input:    InterfaceS
		  ;

InterfaceS:	  Interface
			| InterfaceS Interface
								;

Interface:	 // object
             NSP_OBJECT NSP_IDENTIFIER		 { if (!((CPNamespace*)pNamespace)->OnObjectBegin($2.m_strString)) YYERROR;	}
				   '{' ObjectMembersAndMethodS '}'	 { ((CPNamespace*)pNamespace)->OnObjectEnd();	}

             // noinstances object
           | NSP_NOINSTANCES NSP_OBJECT NSP_IDENTIFIER    { if (!((CPNamespace*)pNamespace)->OnNoinstancesObjectBegin($3.m_strString)) YYERROR;	}
                    '{' ObjectMembersAndMethodS '}'  	 { ((CPNamespace*)pNamespace)->OnNoinstancesObjectEnd();	}

             // event header
		   | NSP_EVENT NSP_IDENTIFIER	 { if (!((CPNamespace*)pNamespace)->OnEventBegin($2.m_strString)) YYERROR;	}
  				'(' ArgList_opt ')' ';'	 { ((CPNamespace*)pNamespace)->OnEventEnd();	}
             // functions
		   | NSP_FUNCTIONS  '{' FunctionS '}'
           ;

ObjectMembersAndMethodS:	  ObjectMemberOrMethod
					        | ObjectMembersAndMethodS	ObjectMemberOrMethod
					;

ObjectMemberOrMethod:		  NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER ArrayOrSetDecl_opt ';'
	    					    {	if (!((CPNamespace*)pNamespace)->OnObjectMember($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString, $6.m_nNumber)) YYERROR;	}
            
			        		| NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER '('	{	if (!((CPNamespace*)pNamespace)->OnObjectMethodBegin($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString)) YYERROR;	}
					        	ArgList_opt ')' ';'								{	((CPNamespace*)pNamespace)->OnObjectMethodEnd();					}
						
					;


FunctionS:	  Function
			| FunctionS Function
			;

Function:	NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER '('	{	if (!((CPNamespace*)pNamespace)->OnFunctionBegin($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString)) YYERROR;	}
						ArgList_opt ')' ';'						{	((CPNamespace*)pNamespace)->OnFunctionEnd();					}
					;

ArrayOrSetDecl_opt:	  /* empty */	{ $$.m_nNumber=0;	}
			    	| ArrayOrSetDecl
				    ;

ArrayOrSetDecl:       ArrayDecl
                    | SetDecl
                    ;

ArrayDecl:		 '[' ']'				{	$$.m_nNumber=1;	}
		    	| ArrayDecl '[' ']'		{	$$.m_nNumber=$1.m_nNumber+1;	}
				;

SetDecl:        '{' '}'                 {	$$.m_nNumber=-1;	}
               ;

ArgList_opt:	  /* empty */
				| ArgList
				;

ArgList:	  Arg			
			| ArgList ',' Arg
			; 

Arg:	  NSP_IDENTIFIER NSP_IDENTIFIER ArrayOrSetDecl_opt { ((CPNamespace*)pNamespace)->OnArgument($1.m_strString, $3.m_nNumber);  }
		| NSP_IDENTIFIER ArrayOrSetDecl_opt   { ((CPNamespace*)pNamespace)->OnArgument($1.m_strString, $2.m_nNumber);  }
		;

