// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(_STDAFX__HEADER_INCLUDED_)
#define _STDAFX__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxmt.h>
#include <afxwin.h>
#include <iostream.h>
#include <winsock2.h>

#include "Common\Events\Events.h"

// enable conditional TRACEs
#ifdef _DEBUG
    
    // external boolean must be defined in the main application,
    // which uses network library
    // if this bool is set (TRUE), then all TRACEs work as usual
    // otherwise they do nothing
    extern BOOL g_bTraceNetwork;

    // define new macro and use it always instead of TRACE
    #define TRACE_NETWORK if (g_bTraceNetwork) TRACE

#else  // not _DEBUG

    // no boolean defined, no condition, just do it just
    // like TRACE in non-debug mode does...
    #define TRACE_NETWORK TRACE

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_STDAFX__HEADER_INCLUDED_)
