/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Seznam instrukci + pole jejich hlavnich vlastnosti
 * 
 ***********************************************************/

#if !defined(AFX_INSTRUCTIONINFO_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_)
#define AFX_INSTRUCTIONINFO_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_

// REGISTR

#define REG			int		// potrebujeme 4B signed
#define INSTRUCTION REG     // chceme stejne jako REG, 4B, aby bylo zarovnane

// "zadny" registr
#define NO_REG  0x7FFFFFFF

// INFORMACE O INSTRUKCICH (nazev, pocet parametru)

// pocet instrukci
#define INSTRUCTION_COUNT  172

// struktura pro uchovani informaci o instrukci (jmeno, pocet parametru)
struct tagInstrInfo
{
	const char *m_lpszName;
	REG m_nParamCount;
    bool m_bOp1IsResultReg;
};
typedef struct tagInstrInfo SInstrInfo;

// staticke pole s informacemi o instrukcich, inicializovani v Code.cpp
extern SInstrInfo g_aInstrInfo[];

// INSTRUKCE
enum tagInstruction 
{
// load member a indexovany load
		LOADUM,		// Load Unit Member
		LOADOM,		// Load Object Member
        LOADSM,     // Load Struct Member
		LOADIX,		// Load Indexed
// store member a indexovany store
		STOREUM,	// Store Unit Member
		STOREOM,	// Store Object Member
        STORESM,    // Store Struct Member
		STOREIX,	// Store Indexed

// presuny
		MOV,			// Move 
		MOV_CINT,		// Move Constant Integer
		MOV_CBOOL,		// Move Constant Boolean
		MOV_CCHAR,		// Move Constant Char
		MOV_CSTR,		// Move Constant String
		MOV_CFLT,		// Move Constant Float

// "pytel" - volani metod a funkci		
		CRTBAG,			// Create Bag
		DELBAG,			// Delete Bag
		ADDBAG_REG,	    // Add Bag Register
		ADDBAG_CCHAR,	// Add Bag Constant Char
		ADDBAG_CINT,	// Add Bag Constant Int
		ADDBAG_CFLT,	// Add Bag Constant Float
		ADDBAG_CBOOL,	// Add Bag Constant Bool

// volani metod
		CALLS,			// Call System
		CALLU,			// Call Unit
		CALLG,			// Call Global
		CALLP,			// Call Parent

// konverze
		CNV_CHAR2STR,	// Convert char to string
		CNV_INT2CHAR,	// Convert int to char
		CNV_INT2FLT,	// Convert int to float
		CNV_FLT2INT,	// Convert float to int
		CNV_U2U,		// Convert unit to unit

// duplikace objektu/struktury
        DUPL,            // Duplicate

// aritmeticke operace
		INC_CHAR,		// Increase char
		INC_INT,		// Increase int
		INC_FLT,		// Increase float

		DEC_CHAR,		// Decrease char
		DEC_INT,		// Decrease int
		DEC_FLT,		// Decrease float

		INV_INT,		// Invert int
		INV_FLT,		// Invert float
		NEG,			// Negate (boolean)

		ADD_CHAR,		// Add chars
		ADD_CCHAR,		// Add constant char and char
		ADD_INT,		// Add ints
		ADD_CINT,		// Add constant int and int
		ADD_FLT,		// Add floats
		ADD_CFLT,		// Add constant float and float

		SUB_CHAR,		// Subtract chars
		SUB_CCHARL,		// Subtract constant char on left and char
		SUB_CCHARR,		// Subtract constant char on right and char
		SUB_INT,		// Subtract ints
		SUB_CINTL,		// Subtract constant int on left and int
		SUB_CINTR,		// Subtract constant int on right and int
		SUB_FLT,		// Subtract floats
		SUB_CFLTL,		// Subtract constant float on left and float
		SUB_CFLTR,		// Subtract constant float on right and float

		MUL_INT,		// Multiply ints
		MUL_CINT,		// Multiply constant int and int
		MUL_FLT,		// Multiply floats
		MUL_CFLT,		// Multiply constant float and float

		DIV_INT,		// Divide ints
		DIV_CINTL,		// Divide constant int on left and int
		DIV_CINTR,		// Divide constant int on right and int
		DIV_FLT,		// Divide floats
		DIV_CFLTL,		// Divide constant float on left and float
		DIV_CFLTR,		// Divide constant float on right and float

		MOD_INT,		// Mod ints
		MOD_CINTL,		// Mod constant int on left and int
		MOD_CINTR,		// Mod constant int on right and int

// porovnavani
		LT_CHAR,		// Less than char
		LT_CCHAR,		// Less than constant char
		LT_INT,			// Less than int
		LT_CINT,		// Less than constant int
		LT_FLT,			// Less than float
		LT_CFLT,		// Less than constant float
		LT_STR,			// Less than string
		LT_CSTR,		// Less than constant string

		LE_CHAR,		// Less or equal char
		LE_CCHAR,		// Less or equal constant char
		LE_INT,			// Less or equal int
		LE_CINT,		// Less or equal constant int
		LE_FLT,			// Less or equal float
		LE_CFLT,		// Less or equal constant float
		LE_STR,			// Less or equal string
		LE_CSTR,		// Less or equal constant string

		GT_CHAR,		// Greater than char
		GT_CCHAR,		// Greater than constant char
		GT_INT,			// Greater than int
		GT_CINT,		// Greater than constant int
		GT_FLT,			// Greater than float
		GT_CFLT,		// Greater than constant float
		GT_STR,			// Greater than string
		GT_CSTR,		// Greater than constant string

		GE_CHAR,		// Greater or egual char
		GE_CCHAR,		// Greater or egual constant char
		GE_INT,			// Greater or egual int
		GE_CINT,		// Greater or egual constant int
		GE_FLT,			// Greater or egual float
		GE_CFLT,		// Greater or egual constant float
		GE_STR,			// Greater or egual string
		GE_CSTR,		// Greater or egual constant string

		EQ_CHAR,		// Egual char
		EQ_CCHAR,		// Egual constant char
		EQ_INT,			// Egual int
		EQ_CINT,		// Egual constant int
		EQ_FLT,			// Egual float
		EQ_CFLT,		// Egual constant float
        EQ_BOOL,        // Equal boolean
        EQ_CBOOL,        // Equal constant boolean
		EQ_STR,			// Egual string
		EQ_CSTR,		// Egual constant string

		NEQ_CHAR,		// Non-egual char
		NEQ_CCHAR,		// Non-egual constant char
		NEQ_INT,		// Non-egual int
		NEQ_CINT,		// Non-egual constant int
		NEQ_FLT,		// Non-egual float
		NEQ_CFLT,		// Non-egual constant float
        NEQ_BOOL,       // Non-equal boolean
        NEQ_CBOOL,      // Non-equal constant boolean
		NEQ_STR,		// Non-egual string
		NEQ_CSTR,		// Non-egual constant string

		INSTANCEOF,		// Instance of

// logicke operace
		LOGOR,			// Logical Or
		LOGAND,			// Logical And

// scitani stringu
		STRADD,			// String addition
		STRADDCL,		// String addition constant on left
		STRADDCR,		// String addition constant on right

// skoky
		JMPF,			// Jump if false
		JMPT,			// Jump if true
		JMP,			// Jump
// return
		RET,			// Return
		RETV,			// Return Value 
		RETV_CCHAR,		// Return Value Constant Char
		RETV_CINT,		// Return Value Constant Int
		RETV_CFLT,		// Return Value Constant Float
		RETV_CBOOL,		// Return Value Constant Bool 
		RETV_CSTR,		// Return Value Constant String

// sendto
		SENDTO_BEGIN,	// Send To Begind
		SENDTO_END,		// Send To Ends
		SENDTO_WAIT,	// Send To Option Wait
		SENDTO_WAIT1ST,	// Send To Option Wait First
		SENDTO_NOTIFICATION,	// Send To Option Nowait
		SENDTO_ADDITEM, // Send To Add Item
		SENDTO_SEND,	// Send To Send

// obycejne send pomoci <-
		SEND,			// Send

// operace s mnozinami
        SET_UN,         // Set Union
        SET_IS,         // Set Intersection
        SET_DIF,        // Set Difference
        SET_ELEM,       // Set Element
        SET_ADD,        // Set Add item
        SET_REM,        // Set Remove item
        SETFOR_FIRST,   // Set For cycle First
        SETFOR_NEXT,    // Set For cycle Next
        SETFOR_END,     // Set For cycle End

// porovnani na null
		OBSOLETE_ISNULL,    // Is Null
		OBSOLETE_ISNOTNULL,	// Is Not Null
		ADDBAG_NULL,	// Add Bag Null
		SETNULL,		// Set Null
		SETNULLM,		// Set Null Member

// pomocne instrukce, ktere se neojevi v kodu
		BRK,			// Break, na konci cyklu se vsechny BREAK a CONTINUE nahradi skoky
		CONT,			// Continue
		NONE			// None instruction
};
typedef enum tagInstruction EInstruction;

#endif //(AFX_INSTRUCTIONINFO_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_)