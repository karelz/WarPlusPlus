%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Parser - syntakticka analyza
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLex2ndPass.h"

#include <malloc.h>
#define alloca  _alloca

#define YYSTYPE CPExpr
#define YYPARSE_PARAM pCompKrnl

#define YYDEBUG             1
#define YYERROR_VERBOSE

#define yylex   ((CCompilerKernel*)pCompKrnl)->m_pLex2ndPass->YyLex
#define yyerror ((CCompilerKernel*)pCompKrnl)->ParserError


%}

%pure_parser 
%expect 3 

/* Tokens */

%token IDENTIFIER

/* Keywords */

%token BREAK		
%token CASE
%token CONTINUE	
%token CONSTRUCTOR
%token DEFAULT
%token DESTRUCTOR
%token DO
%token DUP
%token ELSE
%token COMMAND
%token NOTIFICATION
%token EXTENDS		
%token FOR			
%token IF			
%token IN_KWD	
%token INSTANCEOF_TOKEN	
%token RETURN		
%token SENDTO
%token STRUCT
%token SUPER
%token SWITCH
%token UNIT
%token VOID_TYPE		
%token WAIT
%token WAITFIRST
%token WHILE	
/*%token TRY_TOKEN
%token CATCH_TOKEN
*/
%token THIS_KWD

/* types */
%token TYPE_BOOL
%token TYPE_CHAR
%token TYPE_INT
%token TYPE_FLOAT

/* literals */
%token LITERAL_INT
%token LITERAL_FLOAT
%token LITERAL_BOOL  // true, false
%token LITERAL_STRING
%token LITERAL_CHAR

/* event send */
%token BACKARROW


/* Assignment */
%right '='
%right PLUSASS		    // +=
%right MINUSASS	        // -=
%right MULTASS			// *=   
%right DIVASS			// /=
%right MODASS			// %=

/* Logical operators */

%left AND
%left OR

/* Equality operators */

%right EQ		
%right NEQ

/* Relational operators */

%left '<'
%left '>'
%left LEQ		
%left GEQ

/* Additive and multiply operators */

%left '+'
%left '-'
%left '*'
%left '/'
%left '%'

/* Unary operators */

%right PLUSPLUS     // ++
%right MINUSMINUS   // --
%right UNARYPLUS    // +2
%right UNARYMINUS   // -2
%right '!'

                                                 
/* Postfix operators */

%nonassoc PLUSPLUSPOSTFIX
%nonassoc MINUSMINUSPOSTFIX

/* duplicate */

%nonassoc DUP


%%

/************************************************************************************/

/* starting nonterminal */

Input:    UnitOrGlobalFunctionDeclS
        ;

UnitOrGlobalFunctionDeclS:	  UnitOrGlobalFunctionDecl
							| UnitOrGlobalFunctionDeclS UnitOrGlobalFunctionDecl
							;


/* UNIT OR GLOBAl FUNCTION DECLARATION*/

ReturnType:	  Type
			| VOID_TYPE
			;

UnitOrGlobalFunctionDecl:     ReturnType IDENTIFIER '('			 {   if (!((CCompilerKernel*)pCompKrnl)->OnGlobalFunctionBegins($1, $2)) YYERROR; }
								  FormalParamList_opt ')' Block  {   ((CCompilerKernel*)pCompKrnl)->OnGlobalFunctionEnds($7); }
							
							| ReturnType IDENTIFIER '('			 {   if (!((CCompilerKernel*)pCompKrnl)->OnGlobalFunctionBegins($1, $2)) YYERROR; }
								  error ')' Block				 {   ((CCompilerKernel*)pCompKrnl)->OnGlobalFunctionEnds($7); }

							| UNIT IDENTIFIER Super_opt { if (!((CCompilerKernel*)pCompKrnl)->OnUnitBegin($2, $3)) YYERROR; }
								  UnitBody			    { ((CCompilerKernel*)pCompKrnl)->OnUnitEnd();	}
                            | STRUCT IDENTIFIER     { if (!((CCompilerKernel*)pCompKrnl)->OnStructBegin($2)) YYERROR; }
                                '{' StructItemsDeclS '}' { ((CCompilerKernel*)pCompKrnl)->OnStructEnd();	}
							;

/* UNIT */

Super_opt:    /* empty */	{ ((CCompilerKernel*)pCompKrnl)->OnEmptySuper($$); }
            | Super
            ;

Super:  EXTENDS IDENTIFIER	{ $$=$2; }
        ;      

UnitBody:   '{' UnitItemDeclS_opt '}'
            ;

UnitItemDeclS_opt:	  /*empty*/
					| UnitItemDeclS
					;

UnitItemDeclS:	  UnitItemDecl
				| UnitItemDeclS UnitItemDecl
				;

UnitItemDecl:	  // MEMBER
				  Type SetCurrentType FieldVarDeclS ';'
				| Type SetCurrentType error
				| Type SetCurrentType error ',' FieldVarDeclS ';'

				// CONSTRUCTOR
				| CONSTRUCTOR   { ((CCompilerKernel*)pCompKrnl)->OnConstructorBegins(); }
					Block		{ ((CCompilerKernel*)pCompKrnl)->OnConstructorEnds($3); }

				// DESTRUCTOR
				| DESTRUCTOR	{ ((CCompilerKernel*)pCompKrnl)->OnDestructorBegins(); }
				    Block		{ ((CCompilerKernel*)pCompKrnl)->OnDestructorEnds($3); }

				// COMMAND / NOTIFICATION
				| COMMAND IDENTIFIER '(' OnCommandBegins FormalParamList_opt ')' Block 
						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }

				| COMMAND IDENTIFIER '(' OnCommandBegins error ')' Block
						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
				
				| NOTIFICATION IDENTIFIER '(' OnNotificationBegins FormalParamList_opt ')' Block 
						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }

				| NOTIFICATION IDENTIFIER '(' OnNotificationBegins error ')' Block
						{ ((CCompilerKernel*)pCompKrnl)->OnEventEnds($7); }
				
				// METHOD
				| VOID_TYPE SetCurrentType IDENTIFIER '(' OnMethodBegins FormalParamList_opt ')' Block
						{ ((CCompilerKernel*)pCompKrnl)->OnMethodEnds($8); }

				| VOID_TYPE SetCurrentType IDENTIFIER '(' OnMethodBegins error ')' Block
						{ ((CCompilerKernel*)pCompKrnl)->OnMethodEnds($8); }

				| Type SetCurrentType IDENTIFIER '(' OnMethodBegins FormalParamList_opt ')' Block
						{ ((CCompilerKernel*)pCompKrnl)->OnMethodEnds($8); }
					
				| Type SetCurrentType IDENTIFIER '(' OnMethodBegins error ')' Block				
						{ ((CCompilerKernel*)pCompKrnl)->OnMethodEnds($8); }
				;

OnMethodBegins:	  { if (!((CCompilerKernel*)pCompKrnl)->OnMethodBegins($-1)) YYABORT; }
				  ;

OnCommandBegins:	  { if (!((CCompilerKernel*)pCompKrnl)->OnEventBegins($-1, true)) YYABORT; }
				      ;

OnNotificationBegins:	  { if (!((CCompilerKernel*)pCompKrnl)->OnEventBegins($-1, false)) YYABORT; }
    				 ;

SetCurrentType:	{  ((CCompilerKernel*)pCompKrnl)->m_pCurrentType=&$0.m_DataType; }

/* data fields declaration */

FieldVarDeclS:    FieldVarDecl	{ ((CCompilerKernel*)pCompKrnl)->OnFieldVarDecl($1); }
                | FieldVarDeclS ',' FieldVarDecl { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDecl($3); }
                ;

FieldVarDecl:     SimpleFieldVarDecl
                | ArrayFieldVarDecl
                | SetFieldVarDecl
                ;

SimpleFieldVarDecl:   IDENTIFIER	{ ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclIdentifier($$, $1); }
                    ;

ArrayFieldVarDecl:    IDENTIFIER  '[' ConstantExpr ']'  
                                    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclArrayIdent($$, $1, $3); }
                    | ArrayFieldVarDecl '[' ConstantExpr ']'
                                    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclArrayMore($$, $1, $3); }
                    ;

SetFieldVarDecl:      IDENTIFIER '{' '}'    { ((CCompilerKernel*)pCompKrnl)->OnFieldVarDeclSet($$, $1); }
                    ;

/* Methods and events definition */

FormalParamList_opt:      /* empty */
                        | FormalParamList
                        ;

FormalParamList:      FormalParam
		            | FormalParamList ',' FormalParam
                    ;

FormalParam:    Type  SetCurrentType  ParamDecl
					{ ((CCompilerKernel*)pCompKrnl)->OnFormalParam($3); }
                ;

ParamDecl:        IDENTIFIER		 { ((CCompilerKernel*)pCompKrnl)->OnParamDeclIdentifier($$, $1); }
				| ParamDecl '[' ']'	 { ((CCompilerKernel*)pCompKrnl)->OnParamDeclArray($$, $1); }
                | IDENTIFIER '{' '}' { ((CCompilerKernel*)pCompKrnl)->OnParamDeclSet($$, $1); }
				;

/* Structure definition */

StructItemsDeclS:     StructItemsDecl
                    | StructItemsDeclS StructItemsDecl
                    ;

StructItemsDecl:      Type SetCurrentType FieldVarDeclS ';'
    				| Type SetCurrentType error ';'
	    			| Type SetCurrentType error ',' FieldVarDeclS ';'
                    ;

/* TYPES */

Type:     PrimitiveType
        | ReferenceType
        ;

PrimitiveType:    TYPE_BOOL		
                | TYPE_CHAR		
                | TYPE_INT		
                | TYPE_FLOAT	
                ;
       
ReferenceType:    IDENTIFIER    { ((CCompilerKernel*)pCompKrnl)->OnReferenceType($$, $1); }
                ;
	

/* BLOCKS */

Block:    '{'	OnBlockBegins BlockStmtS_opt '}'	{ ((CCompilerKernel*)pCompKrnl)->OnBlockEnds();  $$=$3; }
/*        | '{' OnBlockBegins error '}'               { ((CCompilerKernel*)pCompKrnl)->OnBlockEnds();  $$.SetError(); }*/
        ;

OnBlockBegins:   { ((CCompilerKernel*)pCompKrnl)->OnBlockBegins();       }
                 ;

BlockStmtS_opt:      /* empty */ { $$.SetNewCode(); }
                   | BlockStmtS  
                   ;

BlockStmtS:   BlockStmt
			| BlockStmtS BlockStmt	{ ((CCompilerKernel*)pCompKrnl)->OnBlockStatementConcat($$, $1, $2); }
            ;

BlockStmt:    LocalVarDeclStmt	
	        | Statement
            ;

/* local variables declaration */

LocalVarDeclStmt:   Type SetCurrentType LocalVarDeclS ';' { $$=$3; }
                    ;        

LocalVarDeclS:    LocalVarDecl	  { $$=$1; }
                | LocalVarDeclS ',' LocalVarDecl   { ((CCompilerKernel*)pCompKrnl)->OnLocalVarDeclConcat($$, $1, $3); }
                ;

LocalVarDecl:     LocalVarDeclNotInit VariableInit_opt { ((CCompilerKernel*)pCompKrnl)->OnVarDeclInitDone($$, $1, $2); }
                ;

LocalVarDeclNotInit:     SimpleVarDecl   { $$=$1; }
				       | ArrayVarDecl    { ((CCompilerKernel*)pCompKrnl)->OnArrayVarDecl($$, $1); }  
                       | SetVarDecl      
        				  // promenna typu pole se registruje tady
                        ;

VariableInit_opt:     /* empty */   { $$.SetNone(); }
                    | '=' Expr      { ((CCompilerKernel*)pCompKrnl)->OnVarDeclInit($$, $2); }
                    | '=' error     { $$.SetNone(); }
                    ;

SimpleVarDecl:    IDENTIFIER			{ ((CCompilerKernel*)pCompKrnl)->OnSimpleVarDecl($$, $1); }
                    // jednoducha promenna se registruje tady
                ;


ArrayVarDecl:  IDENTIFIER '[' ConstantExpr ']'	{ ((CCompilerKernel*)pCompKrnl)->OnArrayVarDeclIdent($$, $1, $3); }
             | ArrayVarDecl '[' ConstantExpr ']' { ((CCompilerKernel*)pCompKrnl)->OnArrayVarDeclMore($$, $1, $3); }
             ;

SetVarDecl:   IDENTIFIER '{' '}'   { ((CCompilerKernel*)pCompKrnl)->OnSetVarDecl($$, $1); }  
                // promenna typu mnozina se registruje tady
                   ;

/* STATEMENTS */

Statement:    Block				OnEndOfStatement
            | EmptyStmt			OnEndOfStatement
            | ExprStmt			OnEndOfStatement
            | IfStmt			OnEndOfStatement
            | SwitchStmt		OnEndOfStatement
            | DoStmt			OnEndOfStatement
            | WhileStmt			OnEndOfStatement
            | ForStmt			OnEndOfStatement
            | ContinueStmt		OnEndOfStatement
            | BreakStmt			OnEndOfStatement
            | ReturnStmt		OnEndOfStatement
            | EventCallStmt		OnEndOfStatement
            | SentToStmt		OnEndOfStatement
/*			| ExceptionStmt		OnEndOfStatement*/
            | error				OnEndOfStatement    { $$.SetNewCode(); } 
            ;

OnEndOfStatement:	{ ((CCompilerKernel*)pCompKrnl)->OnEndOfStatement(); }

/* Empty statement */

EmptyStmt:  ';'		{ $$.SetNewCode(); }
            ;

/* Expression Statement */
ExprStmt:     Expr ';'
            ;
                      

/* If Statement */
IfStmt:   IF '(' BoolExpr ')' Statement		{	((CCompilerKernel*)pCompKrnl)->OnIfStatement($$, $3, $5); }
        | IF '(' BoolExpr ')' Statement ELSE Statement
										{	((CCompilerKernel*)pCompKrnl)->OnIfStatement($$, $3, $5, $7); }
        | IF '(' error ')' Statement
        | IF '(' error ')' Statement ELSE Statement
        ;

/* Switch Statement */

SwitchStmt:       SWITCH '(' Expr ')'   { ((CCompilerKernel*)pCompKrnl)->OnSwitchBegins($3); }
					  SwitchBlock	{ ((CCompilerKernel*)pCompKrnl)->OnSwitchEnds($$); }
                | SWITCH '(' error ')' { $3.SetError(); ((CCompilerKernel*)pCompKrnl)->OnSwitchBegins($3); }
					  SwitchBlock	{ ((CCompilerKernel*)pCompKrnl)->OnSwitchEnds($$); }
                ;    

SwitchBlock:    '{'			{ ((CCompilerKernel*)pCompKrnl)->m_nLevel++; }
				SwitchBlockStmtGroupS_opt '}' { ((CCompilerKernel*)pCompKrnl)->m_nLevel--; }
                    ;

SwitchBlockStmtGroupS_opt:    /* empty */  
                            | SwitchBlockStmtGroupS
                            ;

SwitchBlockStmtGroupS:    SwitchBlockStmtGroup
                        | SwitchBlockStmtGroupS SwitchBlockStmtGroup
                        ;

SwitchBlockStmtGroup:   SwitchLabelS BlockStmtS		{ ((CCompilerKernel*)pCompKrnl)->OnSwitchBlockStmtS($2); }
                        ;

SwitchLabelS:     SwitchLabel	
                | SwitchLabelS SwitchLabel
                ;

SwitchLabel:      CASE ConstantExpr ':'   { ((CCompilerKernel*)pCompKrnl)->OnSwitchCase($2);  }
                | DEFAULT ':'			  { ((CCompilerKernel*)pCompKrnl)->OnSwitchDefault(); }
                ;

/* Do Statement */

DoStmt:    DO Statement WHILE '(' BoolExpr ')' ';'	 { ((CCompilerKernel*)pCompKrnl)->OnDoStatement($$, $2, $5); }
         | DO Statement WHILE '(' error ')' ';'   
         ;

/* While Statement */

WhileStmt:    WHILE '(' BoolExpr ')' Statement       { ((CCompilerKernel*)pCompKrnl)->OnWhileStatement($$, $3, $5); }
            | WHILE '(' error ')' Statement  
            ;

/* For Statement */

ForStmt:      FOR '(' ForInit_opt ';' BoolExpr ';' ForUpdate_opt ')' Statement
					{ ((CCompilerKernel*)pCompKrnl)->OnForStatement($$, $3, $5, $7, $9); }

            | FOR '(' ForInit_opt ';' error ';' ForUpdate_opt ')' Statement   { $$.SetNewCode(); }

            | FOR '(' LocalVarOrMember IN_KWD PrimaryExpr ')' { ((CCompilerKernel*)pCompKrnl)->OnSetForBegin($5); }
                    Block  { ((CCompilerKernel*)pCompKrnl)->OnSetForEnd($$, $3, $5, $8); }
			;

ForInit_opt:      /* empty */	{  $$.SetNewCode(); }
                | ExprList
                ;

ForUpdate_opt:    /* empty */	{  $$.SetNewCode(); }
                | ExprList
                ;

ExprList:		  Expr
				| error					{  $$.SetError(); }
				| ExprList ',' Expr		{  ((CCompilerKernel*)pCompKrnl)->OnExprList($$, $1, $3);  }
                | ExprList ',' error	{  $$.SetError(); }
                ;

/* Break Statement */

BreakStmt:    BREAK ';'		{ ((CCompilerKernel*)pCompKrnl)->OnBreakStatement($$);	}
            ;

/* Continue Statement */

ContinueStmt:   CONTINUE ';'	{ ((CCompilerKernel*)pCompKrnl)->OnContinueStatement($$);	}
                ;

/* Return statement */

ReturnStmt:   RETURN ';'		{ ((CCompilerKernel*)pCompKrnl)->OnReturnStatementVoid($$);		}
			| RETURN Expr ';'	{ ((CCompilerKernel*)pCompKrnl)->OnReturnStatementValue($$, $2);	}
	 	    | RETURN error ';'	{ ((CCompilerKernel*)pCompKrnl)->OnReturnStatementError($$);		}
            ;

/* EventCall Statement */

EventCallStmt:    PrimaryExpr BACKARROW IDENTIFIER OnEventCallBegins '(' ArgumentList_opt ')' ';' 
					{ ((CCompilerKernel*)pCompKrnl)->OnEventCallEnds($$, $1, $3, $4, $6); }				  
/*                | PrimaryExpr BACKARROW IDENTIFIER OnEventCallBegins '(' error ')' ';' 
					{ ((CCompilerKernel*)pCompKrnl)->OnEventCallEndsError($$); }
*/
                ;

OnEventCallBegins:	{ ((CCompilerKernel*)pCompKrnl)->OnEventCallBegins($$, $-2, $0); }
					;

/* SentTo Statement */

SentToStmt:   SENDTO			{  ((CCompilerKernel*)pCompKrnl)->OnSendToBegins(); }
			  '(' PrimaryExprList ')' SendToOptions  
			  '{' SendToEventList '}'
								{  ((CCompilerKernel*)pCompKrnl)->OnSendToEnds($$, $4, $6, $8); }
              ;

PrimaryExprList:  PrimaryExpr	{  ((CCompilerKernel*)pCompKrnl)->OnSendToPrimaryExpr($$, $1); }
                | PrimaryExprList ',' PrimaryExpr 
								{  ((CCompilerKernel*)pCompKrnl)->OnSendToPrimaryExprList($$, $1, $3); }
                ;

SendToOptions:    /* empty */   { $$.SetConstant(0, (CCompilerKernel*)pCompKrnl); }
				| WAIT			{ $$.SetConstant(WAIT, (CCompilerKernel*)pCompKrnl); }
                | WAITFIRST		{ $$.SetConstant(WAITFIRST, (CCompilerKernel*)pCompKrnl); }
                | NOTIFICATION  { $$.SetConstant(NOTIFICATION, (CCompilerKernel*)pCompKrnl); }
                ;

SendToEventList:      SendToEvent ';'	
					| SendToEventList SendToEvent ';'	{ ((CCompilerKernel*)pCompKrnl)->OnSendToEventList($$, $1, $2); }
                    ;

SendToEvent:      IDENTIFIER '(' OnSendToEventBegins ArgumentList_opt ')'  
						{ ((CCompilerKernel*)pCompKrnl)->OnSendToEventEnds($$, $1, $3, $4); }
                | IDENTIFIER '(' OnSendToEventBegins error ')'
						{ ((CCompilerKernel*)pCompKrnl)->OnSendToEventEndsError($$); }
                ;    

OnSendToEventBegins:	{ ((CCompilerKernel*)pCompKrnl)->OnSendToEventBegins($$, $-1); }
						;

/*ExceptionStmt:	TRY_TOKEN Block CATCH_TOKEN Block	{ ((CCompilerKernel*)pCompKrnl)->OnExceptionStmt($$, $2, $4); }
				;
*/
/* Expressions */

PrimaryExpr:      LITERAL_INT		//OK
                | LITERAL_FLOAT		//OK
                | LITERAL_BOOL		//OK
                | LITERAL_STRING	//OK
                | LITERAL_CHAR		//OK
                | '(' Expr ')'		{ $$=$2; }
				| '(' error ')'		{ $$.SetError(); }
                | FieldAccess		// OK
                | MethodInvocation  // OK
                | ArrayAccess		// OK
                | LocalVarOrMember	// OK
                | THIS_KWD          { ((CCompilerKernel*)pCompKrnl)->OnThis($$); }
                ;

ArgumentList_opt:     /* empty */	{ $$.SetNewCode();	}
                    | ArgumentList	// OK
                    ;

ArgumentList:     Expr						{ ((CCompilerKernel*)pCompKrnl)->OnArgumentFirst($$, $1);		}
                | ArgumentList ',' Expr		{ ((CCompilerKernel*)pCompKrnl)->OnArgumentNext($$, $1, $3);	}
                ;

FieldAccess:      PrimaryExpr '.' IDENTIFIER	{ ((CCompilerKernel*)pCompKrnl)->OnFieldAccess($$, $1, $3); }
                ;

MethodInvocation:     PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins ArgumentList_opt ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocPrimaryIdentEnds($$, $1, $3, $5, $6);	}

					| PrimaryExpr '.' IDENTIFIER '(' OnMethodInvocPrimaryIdentBegins error ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocEndsError($$);	}

					| IDENTIFIER '(' OnMethodInvocIdentBegins ArgumentList_opt ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocIdentEnds($$, $1, $3, $4);	}

					| IDENTIFIER '(' OnMethodInvocIdentBegins error ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocEndsError($$); }

	                | SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins ArgumentList_opt ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocSuperEnds($$, $3, $5, $6);	}

                    | SUPER '.' IDENTIFIER '(' OnMethodInvocSuperBegins error ')'
						{	((CCompilerKernel*)pCompKrnl)->OnMethodInvocEndsError($$); }

                    ;

OnMethodInvocPrimaryIdentBegins:	{ ((CCompilerKernel*)pCompKrnl)->OnMethodInvocPrimaryIdentBegins($$, $-3, $-1); }
									;

OnMethodInvocIdentBegins:	{ ((CCompilerKernel*)pCompKrnl)->OnMethodInvocIdentBegins($$, $-1); }
							;

OnMethodInvocSuperBegins:	{ ((CCompilerKernel*)pCompKrnl)->OnMethodInvocSuperBegins($$, $-1); }
							;

ArrayAccess:      PrimaryExpr '[' Expr ']'	{ ((CCompilerKernel*)pCompKrnl)->OnArrayAccess($$, $1, $3);  }	
                | PrimaryExpr '[' error ']' { ((CCompilerKernel*)pCompKrnl)->OnArrayAccessError($$, $1); }	
                ;

LocalVarOrMember:   IDENTIFIER	{ ((CCompilerKernel*)pCompKrnl)->OnLocalVarOrMember($$,$1); }
					;

DupExpr:          PrimaryExpr
                | DUP PrimaryExpr  { ((CCompilerKernel*)pCompKrnl)->OnDupExpr($$, $2); }
                ;

PostfixExpr:      DupExpr
                | PostfixExpr PLUSPLUS		%prec PLUSPLUSPOSTFIX	
					{ ((CCompilerKernel*)pCompKrnl)->OnPostfixExpr($$, $1, '+');	}
                | PostfixExpr MINUSMINUS    %prec MINUSMINUSPOSTFIX
					{ ((CCompilerKernel*)pCompKrnl)->OnPostfixExpr($$, $1, '-');	}
                ;

UnaryExpr:    PostfixExpr
            | PLUSPLUS UnaryExpr
					{ ((CCompilerKernel*)pCompKrnl)->OnUnaryExpr($$, $2, 'P');	}
            | MINUSMINUS UnaryExpr
					{ ((CCompilerKernel*)pCompKrnl)->OnUnaryExpr($$, $2, 'M');	}
            | '+' UnaryExpr  %prec UNARYPLUS
					{ ((CCompilerKernel*)pCompKrnl)->OnUnaryExpr($$, $2, '+');	}
            | '-' UnaryExpr  %prec UNARYMINUS
					{ ((CCompilerKernel*)pCompKrnl)->OnUnaryExpr($$, $2, '-');	}
            | '!' UnaryExpr
					{ ((CCompilerKernel*)pCompKrnl)->OnUnaryExpr($$, $2, '!');	}
            ;
              
MultiplExpr:      UnaryExpr	
				| MultiplExpr '*' UnaryExpr	 
					{ ((CCompilerKernel*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '*');  }
                | MultiplExpr '/' UnaryExpr
					{ ((CCompilerKernel*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '/');  }
                | MultiplExpr '%' UnaryExpr
					{ ((CCompilerKernel*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '%');  }
                ;

AddExpr:      MultiplExpr	
            | AddExpr '+' MultiplExpr
				{ ((CCompilerKernel*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '+');  }
            | AddExpr '-' MultiplExpr
				{ ((CCompilerKernel*)pCompKrnl)->OnBinaryOperation($$, $1, $3, '-');  }
            ;

RelatExpr:    AddExpr	
			| RelatExpr '<' AddExpr		{ ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '<');	}
            | RelatExpr '>' AddExpr		{ ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '>');	}
            | RelatExpr LEQ AddExpr		{ ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, 'L');	}
            | RelatExpr GEQ AddExpr		{ ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, 'G');	}
            | RelatExpr INSTANCEOF_TOKEN ReferenceType
										{ ((CCompilerKernel*)pCompKrnl)->OnInstanceOf($$, $1, $3);	}
            | AddExpr IN_KWD AddExpr    { ((CCompilerKernel*)pCompKrnl)->OnIn($$, $1, $3); }
            ;	

EqualityExpr:     RelatExpr	
                | EqualityExpr EQ RelatExpr	  { ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '='); }
                | EqualityExpr NEQ RelatExpr  { ((CCompilerKernel*)pCompKrnl)->OnRelatEqualExpr($$, $1, $3, '!'); }
                ;

AndExpr:      EqualityExpr		
				  | AndExpr AND EqualityExpr   { ((CCompilerKernel*)pCompKrnl)->OnAndOrExpr($$, $1, $3, '&'); }
            ;

OrExpr:       AndExpr
            | OrExpr OR AndExpr	{ ((CCompilerKernel*)pCompKrnl)->OnAndOrExpr($$, $1, $3, '|'); }
            ;

Expr:     PrimaryExpr AssignOperator Expr	{	((CCompilerKernel*)pCompKrnl)->OnAssign($$, $1, $2, $3);  }
		| OrExpr	//ConditionalExpr	
        ;

AssignOperator:   '='		{	$$.SetConstant('=', (CCompilerKernel*)pCompKrnl); }
                | MULTASS	{	$$.SetConstant('*', (CCompilerKernel*)pCompKrnl); }
                | DIVASS	{	$$.SetConstant('/', (CCompilerKernel*)pCompKrnl); }
                | MODASS	{	$$.SetConstant('%', (CCompilerKernel*)pCompKrnl); }
                | PLUSASS	{	$$.SetConstant('+', (CCompilerKernel*)pCompKrnl); }
                | MINUSASS	{	$$.SetConstant('-', (CCompilerKernel*)pCompKrnl); }
                ;

ConstantExpr:   Expr   {	((CCompilerKernel*)pCompKrnl)->OnConstantExpr($$, $1);	 }
                ;
	
BoolExpr:		Expr   {	((CCompilerKernel*)pCompKrnl)->OnBoolExpr($$, $1);	 }
				;