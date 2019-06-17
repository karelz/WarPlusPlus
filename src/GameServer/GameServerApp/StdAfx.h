// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8B3C8560_5C99_11D4_B527_00105ACA8325__INCLUDED_)
#define AFX_STDAFX_H__8B3C8560_5C99_11D4_B527_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "Archive\Archive\Archive.h"
#include "Common\Events\Events.h"
#include "Network\Network\NetworkServer.h"

// kompilátor, interpret, atp.
#include "TripleS\CompilerIO\CompilerIO.h"
#include "TripleS\CICommon\CICommon.h"
#include "TripleS\Compiler\Compiler.h"
#include "TripleS\Interpret\Interpret.h"
#include "TripleS\CodeManager\CodeManager.h"
#include "Triples\CodeManager\ScriptSetMaker.h"

#include "GameServer\Civilization\ZCivilization.h"
#include "GameServer\Civilization\Src\ZConnectedClient.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8B3C8560_5C99_11D4_B527_00105ACA8325__INCLUDED_)
