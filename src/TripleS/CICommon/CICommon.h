#ifndef _CICOMMON_H
#define _CICOMMON_H

// Pripony souboru s bytekodem
#define FILE_CODE_EXTENSION_UNIT   "cun"  // Compiled UNit
#define FILE_CODE_EXTENSION_FUNCTION   "cfu"  // Compiled FUnction
#define FILE_CODE_EXTENSION_STRUCTURE "str"  // STRucture

// Pripony souboru se zdrojaky
#define FILE_SOURCE_EXTENSION  "3s"

// Jmena adresaru s bytekodem a zdrojakama
#define DIRECTORY_USER          "User"
#define DIRECTORY_SYSTEM        "System"
#define DIRECTORY_BYTECODE      "Bytecode"
#define DIRECTORY_SOURCE        "Source"

// prvni byty hlavicek souboru s kodem - musi mit vsechny stejnou delku!!!!!
#define SCRIPTFILE_FIRSTBYTES_LENGTH 3
#define UNITFILE_FIRTSBYTES	"\xAA\xBB\x01"
#define FUNCTIONFILE_FIRTSBYTES	"\xAA\xBB\x02"
#define STRUCTFILE_FIRTSBYTES	"\xAA\xBB\x03"

// Jmeno defaultniho predka (implementovaneho v C++)
#define DEFAULT_UNIT_PARENT     "Unit"

// Maximalni delka jmena jednotky.
#define MAX_IDENTIFIER_LENGTH    100

// Maximalni velikost pole
#define MAX_ARRAY_SIZE  1000

// Maximalni delka stringu ve stringtable 
// (v kodove stringtable, pouziva se pri nacitani tabulky)
#define MAX_STRING_LENGTH   1024

// soubor s interfacema objektu
#ifndef NAMESPACE_FILE
#define NAMESPACE_FILE	"Namespace.nsp"
#endif

// soubor s interfacem defaultni jednotky
#ifndef DEFAULTUNIT_FILE
#define DEFAULTUNIT_FILE	"DefaultUnit.nsp"
#endif

// pro vypisovani hlasek, globalni funkce jsou "globals::Funkce()"
#define GLOBAL_FUNCTIONS_CLASS_NAME "globals"


// maximalni pocet instrukci kdy muze bezet jeden proces (pak je zabit, protoze se asi zacyklil)
#define MAX_QUANTUM 5000

//
// Typy skriptu
//

typedef enum { SCRIPT_UNIT, SCRIPT_FUNCTION, SCRIPT_STRUCTURE,  // prave skripty
               SCRIPT_OBJECT, SCRIPT_SYSFUNCTION,                // systemove veci
               SCRIPT_UNKNOWN } EScriptType;                    // neznamy

// 
// Datove typy
//

enum tagDataType {T_ERROR=1, T_VOID=2, T_CHAR=3, T_INT=4, T_FLOAT=5, T_BOOL=6, 
	T_OBJECT=7, T_UNIT=8, T_STRUCT=9};
typedef enum tagDataType EDataType;
//
// Common headery
//

#include "Common\MemoryPool\TypedMemoryPool.h"
#include "Common\MemoryPool\MultiThreadTypedMemoryPool.h"

//
// Makra pro to, aby objekty mohly byt alokovany z poolu
//

#define DECLARE_POOL( ClassName)    \
public: \
    void* operator new( size_t nSize, LPCSTR, int)  { ASSERT( nSize == sizeof(ClassName));      \
                                                      return m_Pool.Allocate();    }    \
    void* operator new( size_t nSize)               { return m_Pool.Allocate();    }    \
    void  operator delete( void* p, LPCSTR, int)    { m_Pool.Free( (ClassName*)p); }    \
    void  operator delete( void* p)                 { m_Pool.Free( (ClassName*)p); }   \
     \
    static CMemoryTypedPool<ClassName> m_Pool;    \

#define DEFINE_POOL( ClassName, Size)   CTypedMemoryPool<ClassName> ClassName::m_Pool(Size);


#define DECLARE_MULTITHREAD_POOL( ClassName)    \
public: \
    void* operator new( size_t nSize, LPCSTR, int)  { ASSERT( nSize == sizeof(ClassName));      \
                                                      return m_Pool.Allocate();    }    \
    void* operator new( size_t nSize)               { return m_Pool.Allocate();    }    \
    void  operator delete( void* p, LPCSTR, int)    { m_Pool.Free( (ClassName*)p); }    \
    void  operator delete( void* p)                 { m_Pool.Free( (ClassName*)p); }   \
   \
    static CMultiThreadTypedMemoryPool<ClassName> m_Pool;    \

#define DEFINE_MULTITHREAD_POOL( ClassName, Size)	CMultiThreadTypedMemoryPool<ClassName> ClassName::m_Pool(Size);

//
// Debug trace
//

#ifndef __COMPILED_CODE_TRACE_DEFINED__
#define __COMPILED_CODE_TRACE_DEFINED__
#ifdef _DEBUG
	extern BOOL g_bTraceCompiledCode;

	#define TRACE_COMPILED_CODE if ( g_bTraceCompiledCode ) TRACE_NEXT 
	#define TRACE_COMPILED_CODE0(text) do { if ( g_bTraceCompiledCode ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_COMPILED_CODE1(text,p1) do { if ( g_bTraceCompiledCode ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_COMPILED_CODE2(text,p1,p2) do { if ( g_bTraceCompiledCode ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_COMPILED_CODE3(text,p1,p2,p3) do { if ( g_bTraceCompiledCode ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
	#define TRACE_COMPILED_CODE4(text,p1,p2,p3,p4) do { if ( g_bTraceCompiledCode ) { TRACE3_NEXT(text,p1,p2,p3,p4); } } while ( 0 )
	#define TRACE_COMPILED_CODE5(text,p1,p2,p3,p4,p5) do { if ( g_bTraceCompiledCode ) { TRACE3_NEXT(text,p1,p2,p3,p4,p5); } } while ( 0 )

#else //!_DEBUG

	#define TRACE_COMPILED_CODE TRACE
	#define TRACE_COMPILED_CODE0 TRACE0
	#define TRACE_COMPILED_CODE1 TRACE1
	#define TRACE_COMPILED_CODE2 TRACE2
	#define TRACE_COMPILED_CODE3 TRACE3
	#define TRACE_COMPILED_CODE4 TRACE4
	#define TRACE_COMPILED_CODE5 TRACE5

#endif //!_DEBUG
#endif //__COMPILED_CODE_TRACE_DEFINED__

//
// CICommon headery
//

#include "Src\FloatTable.h"
#include "Src\Instructions.h"
#include "Src\SystemObjectsConst.h"
#include "Src\StringException.h"
#include "Src\StringTable.h"


#endif //_CICOMMON_H