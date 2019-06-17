/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Obecné hlavièkové soubory
 * 
 ***********************************************************/

#ifndef __GAME_SERVER_COMMON__HEADER_INCLUDED__
#define __GAME_SERVER_COMMON__HEADER_INCLUDED__

// maximum DWORDu
#define DWORD_MAX ((DWORD)0xffffffff)

// debuggovací makra a funkce
#include "Common\AfxDebugPlus\AfxDebugPlus.h"
// memory pool
#include "Common\MemoryPool\MemoryPool.h"
#include "Common\MemoryPool\TypedMemoryPool.h"
#include "Common\MemoryPool\SelfPooledList.h"
#include "Common\MemoryPool\SelfPooledQueue.h"
#include "Common\MemoryPool\SelfPooledStack.h"
#include "Common\MemoryPool\SelfPooledPriorityQueue.h"
#include "Common\MemoryPool\PooledList.h"
#include "Common\MemoryPool\PooledQueue.h"
// read write lock
#include "Common\ReadWriteLock\ReadWriteLock.h"
// abstraktní tøída pro Save/Load dat
#include "Common\PersistentStorage\PersistentObject.h"
#include "Common\PersistentStorage\PersistentLoadException.h"

// parametry kompilace
#include "GameServerCompileSettings.h"

// obecné tøídy a šablony
#include "GeneralClasses\GeneralClasses.h"

// abstraktní datové tøídy
#include "AbstractDataClasses\BitArray.h"
#include "AbstractDataClasses\StaticBitArray.h"

// obecnì používané tøídy
#include "Archive\Archive\Archive.h"

#endif //__GAME_SERVER_COMMON__HEADER_INCLUDED__
