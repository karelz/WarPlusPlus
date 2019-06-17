/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Seznam instrukci + pole jejich hlavnich vlastnosti
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CICommon.h"

// naplneni globalniho pole s informacemi o instrukcich
// (nazev, pocet parametru)
SInstrInfo g_aInstrInfo[INSTRUCTION_COUNT]={
		{"LOADUM",		3,true}, 		{"LOADOM",		3,true}, 	{"LOADSM",		3,true},
        {"LOADIX",		3,true},		{"STOREUM",		3,false},	{"STOREOM",		3,false}, 	
        {"STORESM",		3,false},       {"STOREIX",		3,false},
		{"MOV",			2,false},		{"MOV_CINT",	2,false}, 	{"MOV_CBOOL",	2,false},
		{"MOV_CCHAR",	2,false},		{"MOV_CSTR",	2,false}, 	{"MOV_CFLT",	2,false},
		{"CRTBAG",		0,false},		{"DELBAG",		0,false},	{"ADDBAG_REG",	1,false},
        {"ADDBAG_CCHAR",1,false},	    {"ADDBAG_CINT",	1,false},	{"ADDBAG_CFLT",	1,false},
        {"ADDBAG_CBOOL",1,false},	    
		{"CALLS",		3,false},		{"CALLU",		3,false},	{"CALLG",		2,false},			
		{"CALLP",		2,false},	
		{"CNV_CHAR2STR",2,true },	    {"CNV_INT2CHAR",2,true },	{"CNV_INT2FLT",	2,true },
        {"CNV_FLT2INT",	2,true },		{"CNV_U2U",		3,false}, 	{"DUPL",        2,true },
        {"INC_CHAR",	1,false},	    {"INC_INT",		1,false},	{"INC_FLT",		1,false},	
		{"DEC_CHAR",	1,false},		{"DEC_INT",		1,false},	{"DEC_FLT",		1,false},	
		{"INV_INT",		1,false},		{"INV_FLT",		1,false},	
		{"NEG",			1,false},		{"ADD_CHAR",	3,true },	{"ADD_CCHAR",	3,true },	
		{"ADD_INT",		3,true },		{"ADD_CINT",	3,true },	{"ADD_FLT",		3,true },	
		{"ADD_CFLT",	3,true },		{"SUB_CHAR",	3,true },	{"SUB_CCHARL",	3,true },
		{"SUB_CCHARR",	3,true },		{"SUB_INT",		3,true },	{"SUB_CINTL",	3,true },	
		{"SUB_CINTR",	3,true },		{"SUB_FLT",		3,true },	{"SUB_CFLTL",	3,true },	
		{"SUB_CFLTR",	3,true },		{"MUL_INT",		3,true },	{"MUL_CINT",	3,true },	
		{"MUL_FLT",		3,true },		{"MUL_CFLT",	3,true },	{"DIV_INT",		3,true },	
		{"DIV_CINTL",	3,true },		{"DIV_CINTR",	3,true },	{"DIV_FLT",		3,true },	
		{"DIV_CFLTL",	3,true },		{"DIV_CFLTR",	3,true },	{"MOD_INT",  	3,true },	
		{"MOD_CINTL",	3,true },		{"MOD_CINTR",	3,true },	{"LT_CHAR",  	3,true },
		{"LT_CCHAR",  	3,true },		{"LT_INT",  	3,true },	{"LT_CINT",  	3,true },	
		{"LT_FLT",  	3,true },		{"LT_CFLT",  	3,true },	{"LT_STR",  	3,true },	
		{"LT_CSTR",  	3,true },		{"LE_CHAR",  	3,true },	{"LE_CCHAR",  	3,true },
		{"LE_INT",  	3,true },		{"LE_CINT",  	3,true },	{"LE_FLT",  	3,true },	
		{"LE_CFLT",  	3,true },		{"LE_STR",  	3,true },	{"LE_CSTR",  	3,true },	
		{"GT_CHAR",  	3,true },		{"GT_CCHAR",  	3,true }, 	{"GT_INT",  	3,true },	
		{"GT_CINT",  	3,true },		{"GT_FLT",  	3,true },	{"GT_CFLT",  	3,true },	
		{"GT_STR",  	3,true },		{"GT_CSTR",  	3,true },	{"GE_CHAR",  	3,true },
		{"GE_CCHAR",  	3,true },		{"GE_INT",  	3,true },	{"GE_CINT",  	3,true },	
		{"GE_FLT",  	3,true },		{"GE_CFLT",  	3,true },	{"GE_STR",  	3,true },	
		{"GE_CSTR",  	3,true },		{"EQ_CHAR",  	3,true },	{"EQ_CCHAR",  	3,true },
		{"EQ_INT",  	3,true },		{"EQ_CINT",  	3,true },	{"EQ_FLT",  	3,true },	
        {"EQ_CFLT",  	3,true },		{"EQ_BOOL",     3,true },   {"EQ_CBOOL",    3,true },
        {"EQ_STR",  	3,true },		{"EQ_CSTR",  	3,true },	
		{"NEQ_CHAR",  	3,true }, 	    {"NEQ_CCHAR",  	3,true },	{"NEQ_INT",  	3,true },	
		{"NEQ_CINT",  	3,true },	    {"NEQ_FLT",  	3,true },	{"NEQ_CFLT",  	3,true },	
		{"NEQ_BOOL",    3,true },       {"NEQ_CBOOL",   3,true },
        {"NEQ_STR",  	3,true },		{"NEQ_CSTR",	3,true },	{"INSTANCEOF",	3,false},
		{"LOGOR",  		3,true }, 	    {"LOGAND",  	3,true }, 	{"STRADD",  	3,true },
		{"STRADDCL", 	3,true },	 	{"STRADDCR", 	3,true },	{"JMPF",		2,false},
		{"JMPT", 		2,false},		{"JMP",			1,false}, 	{"RET",			0,false},
		{"RETV",		1,false},		{"RETV_CCHAR",	1,false},	{"RETV_CINT",	1,false},
		{"RETV_CFLT",	1,false},       {"RETV_CBOOL",	1,false},   {"RETV_CSTR",	1,false},
		{"SENDTO_BEGIN",0,false}, 	    {"SENDTO_END",	0,false},
		{"SENDTO_WAIT",	0,false}, 	   	{"SENDTO_WAIT1ST",0,false}, {"SENDTO_NOTIFICATION",0,false},
		{"SENDTO_ADDITEM",1,false},     {"SENDTO_SEND",	1,false},	{"SEND",		2,false},
        {"SET_UN",      3,true },       {"SET_IS",      3,true },   {"SET_DIF",     3,true },
        {"SET_ELEM",    3,true },       {"SET_ADD",     3,true },   {"SET_REM",     3, true},
        {"SETFOR_FIRST",3,false},       {"SETFOR_NEXT", 3,false},   {"SETFOR_END",  1,false},
        {"ISNULL",		2,false}, 	    {"ISNOTNULL",	2,false},
		{"ADDBAG_NULL", 0,false},       {"SETNULL",		1,false},   {"SETNULLM",    2,false},
        {"BRK",         1,false},       {"CONT",        1,false},  // budou nahrazeny JMP
        {"NONE",        0}
};
		
