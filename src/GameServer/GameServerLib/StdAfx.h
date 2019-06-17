// StdAfx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__319B44BB_82D7_11D3_9B20_004F4906CFC9__INCLUDED_)
#define AFX_STDAFX_H__319B44BB_82D7_11D3_9B20_004F4906CFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>			// MFC core and standard components
#include <afxext.h>			// MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxmt.h>			// MFC synchronization objects
#include <afxtempl.h>		// MFC templates

#include "GameServerCommon.h"

// abstraktní datové tøídy
#include "AbstractDataClasses\RunTimeIDCreation.h"

// obecnì používané tøídy
#include "Common\Events\Events.h"

// kompilátor, interpret, atp.
#include "TripleS\CompilerIO\CompilerIO.h"
#include "TripleS\CICommon\CICommon.h"
#include "TripleS\Compiler\Compiler.h"
#include "TripleS\Interpret\Interpret.h"
#include "TripleS\CodeManager\CodeManager.h"

// civilizace
#include "GameServer\Civilization\ZCivilization.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__319B44BB_82D7_11D3_9B20_004F4906CFC9__INCLUDED_)
