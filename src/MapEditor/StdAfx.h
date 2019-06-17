// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__43E086C9_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_STDAFX_H__43E086C9_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>

#include "Common\AfxDebugPlus\AfxDebugPlus.h"
#include "Archive\Archive\Archive.h"
#include "Common\Events\Events.h"
#include "Common\DirectX\DirectX.h"
#include "GameClient\Windows\Windows.h"
#include "GameClient\Common\CommonExceptions.h"
#include "GameClient\Common\ConfigMacros.h"
#include "GameClient\Common\Colors.h"

#include "Constants.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__43E086C9_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
