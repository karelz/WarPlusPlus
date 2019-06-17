// includes.h: common definitions and declarations
//
//////////////////////////////////////////////////////////////////////

#ifndef __INCLUDES_H_
#define __INCLUDES_H_

// Include ZLIBky
//#ifndef _ZLIB_DLL
//#define _ZLIB_DLL
//#endif

#ifndef _WINDOWS
#define _WINDOWS
#endif

#include "../Zlib/zlib.h"

// Navratova hodnota CCompressDataSink a CCompressDataSource
enum ECompressData {
	compressDataOK	=0,	// Vse probehlo v poradku
	compressDataEOF	=1, // Narazili jsme na EOF
	
	compressDataError=-1	// Doslo k chybe
};

// Defaultni velikost store bufferu
#define COMPRESSION_STORE_BUFFER_DEFAULT_SIZE	65535

// Defaultni velikost read bufferu
#define COMPRESSION_READ_BUFFER_DEFAULT_SIZE	65535

#endif // __INCLUDES_H_