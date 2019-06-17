/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Editace skriptu - syntax highlight
 *   Autor: Helena Kupková
 *  
 *   Popis: Globalni promenne (pro lexe)
 * 
 ***********************************************************/

#include "ScEditLex.h"

// globalni pointer na objekt CScEditLex

extern CScEditLex *gpScEditLex;

// globalni funkce ScEdit parseru na uvolneni jeho bufferu
void FreeScEditLexBuffer();

// globalni promenne pri cteni
extern DWORD g_nRow;
extern WORD g_nColumn;

// hlavni funkce lexu
int sceditlex();
void sceditrestart(FILE *);