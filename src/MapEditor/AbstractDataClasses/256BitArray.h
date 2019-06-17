// 256BitArray.h: implementation of the C256BitArray template.
//
//////////////////////////////////////////////////////////////////////

#ifndef __256_BIT_ARRAY__HEADER_INCLUDED__
#define __256_BIT_ARRAY__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StaticBitArray.h"

//////////////////////////////////////////////////////////////////////
// Bitové pole 256-ti bitù (tj. velikosti 32B)
typedef CStaticBitArray<BYTE,256> C256BitArray;

#endif //__256_BIT_ARRAY__HEADER_INCLUDED__
