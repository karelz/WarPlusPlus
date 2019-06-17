// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__32218D2B_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
#define AFX_STDAFX_H__32218D2B_ED20_11D3_AFF3_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>

#include <afxcoll.h>        // MFC collections
#include <afxtempl.h>       // MFC templates (e.g.CTypedPtrArray)

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

#include "Common\PersistentStorage\PersistentStorage.h"
#include "Common\Events\Events.h"

#include "Common\ServerClient\VirtualConnectionIDs.h"

#include "Network\Network\NetworkServer.h"
#include "Archive\Archive\Archive.h"

#include "TripleS\CompilerIO\CompilerIO.h"
#include "TripleS\CICommon\CICommon.h"
#include "TripleS\Compiler\Compiler.h"
#include "TripleS\Interpret\Interpret.h"
#include "TripleS\CodeManager\CodeManager.h"

#include "..\ZCivilization.h"
#include "ZUnit.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__32218D2B_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
