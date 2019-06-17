
#if !defined(_SPECIALBLT_HEADER__INCLUDED_)
#define _SPECIALBLT_HEADER__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////
// The only exported function

void Special_Blt(BYTE *pBuffer, DWORD dwScanLineSize, const CRect &rectSrc, const CRect &rectDest, BOOL b32bit);


#endif  // !defined(_SPECIALBLT_HEADER__INCLUDED_)
