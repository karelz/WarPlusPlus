/* Lexikalni 'gramatika' pro cteni konfiguracnich souboru */

%option prefix="yy2nd"
%option noyywrap
%option never-interactive
%option c++
%option yyclass="CPLex2ndPass"
%option yylineno

%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Gramatika, z niz bude lexem vygenerovan automat
 * 
 ***********************************************************/

class CObject;

#define YYSTYPE CPExpr

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLex2ndPass.h"
#include "Syntax.h"

extern int yydebug;

#define YY_SKIP_YYWRAP

%}

/* exclusive start conditions */

%x COMMENT
%x STRING

/* definitions */

DIGIT	[0-9]
                                                            
%%

    /* RULES */

[ \t]+		/* empty - whitespace */

"//"[^\n]*	/* empty - one line comment */

"/*"	{  /* multiline comment */
			m_nCommentLevel=1;
            BEGIN(COMMENT);
		}

<COMMENT>{
"/*"			m_nCommentLevel++;

[^*\n/]*		/* empty - anything but a '*' or '/n' or '/' */

"*"+[^*/\n]*  	/* empty - '*' and anything but '*' and '/' */

\n				/* empty */

"*"+"/"			{
					m_nCommentLevel--;
					if (m_nCommentLevel==0) BEGIN(INITIAL);
				}
}


\"		{
			BEGIN(STRING); /* string */
			m_strString.Empty();
		}

<STRING>{
"\""		{
                BEGIN(INITIAL); 
				m_pExpr->SetConstant((LPCTSTR)m_strString, m_pCompilerKrnl);
                return LITERAL_STRING;
            }
				
\n			{	/* EOL in string */ 
				m_pCompilerKrnl->ErrorMessage("unterminated string constant");
                return 0;
			}

\\[0-7]{1,3}	{	/* octal excape sequence */
					sscanf(yytext+1, "%o", &m_nInt);
					if (m_nInt>0xff) m_pCompilerKrnl->ErrorMessage("octal constant is out-of-bounds");
					m_strString+=yytext;
				}

\\[0-9]+		{ 	/* bad octal escape sequence */
					m_pCompilerKrnl->ErrorMessage("bad escape sequence");						
					m_strString+=yytext;
				}
	
\\0[Xx][0-9A-Fa-f]{2}	{ 	/* hexadecimal excape sequence */ 
							m_strString+=yytext;
                        }
	
\\0[Xx][^0-9A-Fa-f]{2}	{   /* bad hexadecimal excape sequence */
							m_pCompilerKrnl->ErrorMessage("bad escape sequence");						
							m_strString+=yytext;
					    }
					
[^\\\n\"]+		m_strString+=yytext;

}

\r              /* empty */

\n			    {
#ifdef _DEBUG
					if (yydebug) m_pCompilerKrnl->Pause();					
#endif
				}

{DIGIT}+	    {
					m_pExpr->SetConstant(atoi(yytext), m_pCompilerKrnl);
					return LITERAL_INT;
				}
			
{DIGIT}+"."{DIGIT}*	    {
							m_pExpr->SetConstant((float)atof(yytext), m_pCompilerKrnl);
							return LITERAL_FLOAT;
						}

{DIGIT}+[Ee][+-]{0,1}{DIGIT}+	{
							m_pExpr->SetConstant((float)atof(yytext), m_pCompilerKrnl);
							return LITERAL_FLOAT;
						}

{DIGIT}+"."{DIGIT}*[Ee][+-]{0,1}{DIGIT}+		{
										m_pExpr->SetConstant((float)atof(yytext), m_pCompilerKrnl);
										return LITERAL_FLOAT;
									}

void		{
				m_pExpr->SetType(T_VOID, m_pCompilerKrnl);
				return VOID_TYPE;
			}

unit		return UNIT;
extends		return EXTENDS;
if			return IF;
else		return ELSE;
command     return COMMAND;
notification    return NOTIFICATION;
while		return WHILE;
do			return DO;
dup         return DUP;
switch		return SWITCH;
case		return CASE;	
constructor return CONSTRUCTOR;
	/*destructor	return DESTRUCTOR;*/
return		return RETURN;
break		return BREAK;
continue	return CONTINUE;
in			return IN_KWD;
for			return FOR;
sendto      return SENDTO;
wait        return WAIT;
waitfirst   return WAITFIRST;
instanceof	return INSTANCEOF_TOKEN;
default		return DEFAULT;
            /*try			return TRY_TOKEN;*/
            /*catch		return CATCH_TOKEN;  */
struct      return STRUCT;
this        return THIS_KWD;
super       return SUPER;

null        {
                m_pExpr->SetConstant((int)0, m_pCompilerKrnl);
				return LITERAL_INT;
            }


int			{
				m_pExpr->SetType(T_INT, m_pCompilerKrnl);
				return TYPE_INT;
			}

float		{
				m_pExpr->SetType(T_FLOAT, m_pCompilerKrnl);
				return TYPE_FLOAT;
			}

bool		{
				m_pExpr->SetType(T_BOOL, m_pCompilerKrnl);
				return TYPE_BOOL;	
			}

char		{
				m_pExpr->SetType(T_CHAR, m_pCompilerKrnl);
				return TYPE_CHAR;
			}

true    {
			m_pExpr->SetConstant(true, m_pCompilerKrnl);
			return LITERAL_BOOL;
		}
false   {	
			m_pExpr->SetConstant(false, m_pCompilerKrnl);
			return LITERAL_BOOL;
		}

"++"    return PLUSPLUS;
"--"    return MINUSMINUS;
"<-"    return BACKARROW;
"=="    return EQ;
"!="    return NEQ;
"<="    return LEQ;
">="    return GEQ;
"+="    return PLUSASS;
"-="    return MINUSASS;
"*="    return MULTASS;
"/="    return DIVASS;
"%="    return MODASS;
"||"    return OR;
"&&"    return AND;    


";"	|
":"	|
"("	|
")"	|
"["	|
"]"	|
"{"	|
"}"	|
"."	|
"+"	|
"-"	|
"*"	|
"/"	|
"%"	|
"<"	|
"="	|
">"	|
"!"	|
","     {
            m_pExpr->SetConstant(yytext[0], m_pCompilerKrnl);
            return yytext[0];
        }

"\\"	{
            m_pExpr->SetConstant(yytext[0], m_pCompilerKrnl);
            return yytext[0];
        }

"'"."'"	{
			m_pExpr->SetConstant(yytext[1], m_pCompilerKrnl);
			return LITERAL_CHAR;
		}

[A-Za-z_][A-Za-z_0-9]*		{
								m_pExpr->SetDeclar(CString(yytext), m_pCompilerKrnl);
								return IDENTIFIER;
							}

<<EOF>>		{ 

#ifdef _DEBUG
				if (yydebug) m_pCompilerKrnl->Pause();					
#endif
				return 0; 
            }

.	m_pCompilerKrnl->ErrorMessage("unexpected character (%c - 0x%02X)",yytext[0],yytext[0]);


