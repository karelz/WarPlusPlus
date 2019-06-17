// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F8D96199_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
#define AFX_STDAFX_H__F8D96199_1A95_11D3_9FB6_CF2879F40338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// the project dependencies includes
#include "Common\AfxDebugPlus\AfxDebugPlus.h"
#include "Common\Events\Events.h"
#include "Archive\Archive\Archive.h"
#include "Common\DirectX\DirectX.h"
#include "Common\ConfigFile\ConfigFile\ConfigFile.h"
#include "Windows\Windows.h"
#include "Network\Network\NetworkClient.h"
#include "Cache\CachedObject.h"
#include "Cache\IDCachedObject.h"
#include "Cache\AbstractCache.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F8D96199_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
