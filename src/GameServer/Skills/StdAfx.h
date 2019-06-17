// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__DEB85F35_0AF5_11D4_B01A_004F49068BD6__INCLUDED_)
#define AFX_STDAFX_H__DEB85F35_0AF5_11D4_B01A_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>
#include <afxtempl.h>
#include <iostream.h>  // needed for stream "endl" manipulator

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SUnitType.h"
#include "GameServer\GameServer\SUnit.h"
#include "GameServer\GameServer\SSkillType.h"

#include "GameServer\GameServer\SUnit.inl"

// obecnì používané tøídy
#include "Common\Events\Events.h"

// kompilátor, interpret, atp.
#include "TripleS\CompilerIO\CompilerIO.h"
#include "TripleS\CICommon\CICommon.h"
#include "TripleS\Compiler\Compiler.h"
#include "TripleS\Interpret\Interpret.h"
#include "TripleS\CodeManager\CodeManager.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__DEB85F35_0AF5_11D4_B01A_004F49068BD6__INCLUDED_)
